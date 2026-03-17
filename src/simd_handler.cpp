#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/simd_handler.cpp"
#include "hwy/foreach_target.h"
#include "hwy/highway.h"

#include "simd_handler.h"
#include "mandelbrot.h"

// highway foreach_target will repeatedly compile this file with different
// SIMD targets to allow dynamic runtime selection of the best technology
// supported by the hardware / OS

HWY_BEFORE_NAMESPACE();
namespace mandelbrot_hwy {
namespace HWY_NAMESPACE {           // expands to AVX2, SSE4, etc
namespace hn = hwy::HWY_NAMESPACE;  // uses corresponding instructions

static HWY_INLINE bool isKnownInside(double cx, double cy) {
    double x1 = cx + 1.0;
    if (x1 * x1 + cy * cy <= 0.0625) return true;
    double x = cx - 0.25;
    double q = x * x + cy * cy;
    return q * (q + x) <= 0.25 * cy * cy;
}

void SimdRow(double x0_start, double y0, double zoom, int max_iterations, int* out_iterations, int pixel_count) {
    const hn::ScalableTag<double> d;  // uses widest SIMD register availible for doubles, to allow highest level of parallel
    const int N = hn::Lanes(d);

    // set constants used in hotloop across all lanes (v=vector)
    // SSE4, 2 lanes: vFour = [4.0, 4.0]
    // AVX2, 4 lanes: vFour = [4.0, 4.0, 4.0, 4.0]

    const auto vFour = hn::Set(d, 4.0);
    const auto vOne = hn::Set(d, 1.0);
    const auto vY0 = hn::Set(d, y0);
    const auto vMax = hn::Set(d, (double)max_iterations);
    const auto vStep = hn::Set(d, zoom);

    HWY_ALIGN double sequence_arr[HWY_MAX_BYTES / sizeof(double)];
    for (int i = 0; i < N; i++) {
        sequence_arr[i] = (double)i;
    }
    const auto vSequence = hn::Load(d, sequence_arr);  // [0.0, 1.0, 2.0, 3.0...]
    const auto all_lanes = hn::FirstN(d, N);           // bitset for masking complete lanes

    HWY_ALIGN double result_arr[HWY_MAX_BYTES / sizeof(double)];

    // iterating mandelbrot formula z = z^2 + c
    // c = x0_start + px * zoom + pixel_offset * zoom
    //
    // [x0_start + px * zoom] are constants per batch, [pixel_offset * zoom] are vectorized

    int px = 0;
    for (; px + (int)N <= pixel_count; px += (int)N) {
        // compute constant C
        auto cx_vec = hn::Add(hn::Set(d, x0_start + px * zoom),
                              hn::Mul(vSequence, vStep));

        // early escape/known inside optimisations
        // bulb check
        auto x1 = hn::Add(cx_vec, vOne);
        auto cy2 = hn::Set(d, y0 * y0);
        auto bulb = hn::Le(hn::Add(hn::Mul(x1, x1), cy2), hn::Set(d, 0.0625));

        // cardioid check
        auto xm = hn::Sub(cx_vec, hn::Set(d, 0.25));
        auto q = hn::Add(hn::Mul(xm, xm), cy2);
        auto cardiod = hn::Le(hn::Mul(q, hn::Add(q, xm)),
                              hn::Mul(hn::Set(d, 0.25), cy2));

        auto escaped = hn::Or(bulb, cardiod);

        auto escaped_iter = vMax;
        auto x_vec = hn::Zero(d);
        auto y_vec = hn::Zero(d);

        // stop iterating if all lanes have escaped
        if (hn::AllFalse(d, hn::AndNot(escaped, all_lanes))) {
            hn::Store(escaped_iter, d, result_arr);
            for (int i = 0; i < N; i++) {
                out_iterations[px + i] = (int)result_arr[i];
            }
            continue;  // get next block of pixels
        }

        auto oldx = hn::Zero(d);
        auto oldy = hn::Zero(d);
        int cd = 20;

        for (int iter = 0; iter < max_iterations; iter++) {
            auto x2 = hn::Mul(x_vec, x_vec);
            auto y2 = hn::Mul(y_vec, y_vec);
            auto mag2 = hn::Add(x2, y2);

            // check escaped lanes
            auto esc_now = hn::AndNot(escaped, hn::Gt(mag2, vFour));
            if (!hn::AllFalse(d, esc_now)) {
                escaped_iter = hn::IfThenElse(esc_now, hn::Set(d, (double)iter), escaped_iter);
                escaped = hn::Or(escaped, esc_now);
                if (hn::AllFalse(d, hn::AndNot(escaped, all_lanes))) {
                    break;
                }
            }

            // compute next iteration
            // even on escaped lanes, avoiding additional conditional checks
            auto twox = hn::Add(x_vec, x_vec);
            y_vec = hn::MulAdd(twox, y_vec, vY0);
            x_vec = hn::Add(hn::Sub(x2, y2), cx_vec);

            // periodic distance check
            if (iter > 50 && --cd == 0) {
                cd = 20;
                auto not_esc = hn::AndNot(escaped, all_lanes);
                if (!hn::AllFalse(d, not_esc)) {
                    auto dx = hn::Sub(x_vec, oldx);
                    auto dy = hn::Sub(y_vec, oldy);
                    auto d2 = hn::Add(hn::Mul(dx, dx), hn::Mul(dy, dy));
                    auto ref = hn::Mul(hn::Set(d, 1e-24), hn::Add(mag2, vOne));
                    auto periodic = hn::And(not_esc, hn::Lt(d2, ref));
                    escaped = hn::Or(escaped, periodic);
                }
                oldx = x_vec;
                oldy = y_vec;
            }
        }

        hn::Store(escaped_iter, d, result_arr);
        for (int i = 0; i < N; i++) {
            out_iterations[px + i] = (int)result_arr[i];
        }
    }

    // remaining pixels in row completed with scalar function
    for (; px < pixel_count; px++) {
        double cx = x0_start + px * zoom;
        out_iterations[px] = calculateMandelbrot(cx, y0, max_iterations);
    }
}

}  // namespace HWY_NAMESPACE
}  // namespace mandelbrot_hwy
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

#include <stdio.h>
namespace mandelbrot_hwy {
HWY_EXPORT(SimdRow);

void CallSimdRow(double x0_start, double y0, double zoom_step, int max_iterations, int* out_iterations, int pixel_count) {
    HWY_DYNAMIC_DISPATCH(SimdRow)(x0_start, y0, zoom_step, max_iterations, out_iterations, pixel_count);
}
}

// debug compiled exports
extern "C" void mandelbrot_simd_print_targets(void) {
    int64_t compiled = HWY_TARGETS;
    printf("SIMD compiled targets\n");
    for (int64_t t = 1; t != 0; t <<= 1) {
        if (compiled & t)
            printf("  %s\n", hwy::TargetName(t));
    }
}

extern "C" void mandelbrot_simd_row(
    double x0_start,
    double y0,
    double zoom_step,
    int max_iterations,
    int* out_iterations,
    int pixel_count) {
    mandelbrot_hwy::CallSimdRow(x0_start, y0, zoom_step, max_iterations, out_iterations, pixel_count);
}

#endif