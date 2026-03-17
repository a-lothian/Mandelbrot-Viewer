#ifndef SIMD_HANDLER
#define SIMD_HANDLER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// compute one row of Mandelbrot iteration counts using SIMD

void mandelbrot_simd_row(
    double x0_start,
    double y0,
    double zoom_step,
    int max_iterations,
    int* out_iterations,
    int pixel_count);

void mandelbrot_simd_print_targets(void);

#ifdef __cplusplus
}
#endif

#endif