#include "mandelbrot.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "inputHandler.h"

static inline int isKnownInside(double x0, double y0) {
    // test 1. If x0, y0 is within distance of 1/4 from point (-1,0)
    // it is guaranteed to be inside
    double x1 = x0 + 1.0;
    if (x1 * x1 + y0 * y0 <= 0.0625)
        return 1;

    // test 2. If x0, y0, falls within known cardioid region, it is inside.
    double x = x0 - 0.25;
    double q = x * x + y0 * y0;
    if (q * (q + x) <= 0.25 * y0 * y0)
        return 1;

    return 0;
}

// calculate if the point(x(n+1),y(n+1)) will escape given enough iterations
// returns number of iterations required to escape bounding length of 4
//
// Optimization 1, isKnownInside(): if x,y is within known regions, it will not escape.
// Optimization 2, checkInterval: if the distance between x,y and x,y from many steps ago is tiny, it will not escape.
int calculateMandelbrot(double x0, double y0, int max_iterations) {
    if (isKnownInside(x0, y0))
        return max_iterations;

    double x = 0.0;
    double y = 0.0;

    double oldx = 0.0;
    double oldy = 0.0;

    double epsilon2 = 1e-24;
    const int checkInterval = 20;
    int checkcountdown = checkInterval;

    for (int i = 0; i < max_iterations; i++) {
        double x2 = x * x;
        double y2 = y * y;

        // escape check
        if (x2 + y2 > 4.0) {
            return i;  // Return the escape iteration count
        }

        double xy = x * y;

        // calculate next point
        y = (2.0 * xy) + y0;
        x = (x2 - y2) + x0;

        // Periodic distance check
        if (i > 50 && --checkcountdown == 0) {
            double dx = x - oldx;
            double dy = y - oldy;

            // scale epsilon by point magnitude
            if (dx * dx + dy * dy < epsilon2 * (x2 + y2 + 1.0)) {
                return max_iterations;  // inside set
            }
            oldx = x;
            oldy = y;

            checkcountdown = checkInterval;
        }
    }

    return max_iterations;
}

// assume min is 0 for both inputs
int fast_map_range(int value, int in_max, int out_max) {
    value = value >= in_max ? 0 : value;  // clamp max
    return (value * out_max) / in_max;
}

void* calculateMandelbrotRoutine(void* arg) {
    struct mandelbrotRoutineData* data = (struct mandelbrotRoutineData*)arg;
    int pitch = sizeof(Uint32) * data->vp->screen_width;

    int halfWidth = data->vp->screen_width / 2;
    int halfHeight = data->vp->screen_height / 2;

    Uint8* row;

    const double zoom = data->vp->zoom;  // DISTANCE BETWEEN PIXELS IN WORLD SPACE
    double world_top = data->vp->current_offset_y - ((double)halfHeight * zoom);
    double world_left = data->vp->current_offset_x - ((double)halfWidth * zoom);

    double palette_scale = (double)(data->palette_size) / (double)data->vp->iterations;  // for cyclic rendering

    // render fraction halves; 8 -> 4 -> 2 -> 1 -> return
    while (data->start_render_frac >= 1) {
        row = (Uint8*)data->local_buffer + (data->start_y * pitch);

        // draw onto screen
        // render factor 8: render every 8th pixel, copy to other pixels, then half render factor + repeat until 1.
        for (int y = data->start_y; y < data->end_y; y += data->start_render_frac) {
            // check for quick return
            if (*(data->kill_signal)) {
                return NULL;
            }

            Uint32* out = (Uint32*)row;

            // worldspace coordinates
            double x0 = world_left;
            double y0 = world_top + (double)y * zoom;

            if (data->render_smooth) {
                // SMOOTH CYCLIC RENDERING
                for (int x = 0; x < data->scrn_width; x += data->start_render_frac) {
                    int iterations = calculateMandelbrot(x0, y0, data->vp->iterations);

                    int colorIndex = (int)(iterations * palette_scale);

                    Uint32 colour;
                    if (iterations == data->vp->iterations) {  // check inside of mandelbrot
                        colour = data->palette[0];
                    } else if (colorIndex >= data->palette_size) {
                        colour = data->palette[0];
                    } else {
                        int frequency = 10;
                        int col_index = iterations * frequency;

                        // wrap around the palette
                        int index = col_index % data->palette_size;

                        if (index < 0)
                            index = 0;

                        colour = data->palette[index];
                    }

                    // coloring (0xFFRRGGBB format for ARGB8888) + write to buffer
                    // copy to neighboring cells depending on render fraction
                    for (int k = 0; k < data->start_render_frac; k++) {
                        if ((x + k) < data->scrn_width) {  // check if within buffer
                            out[x + k] = colour;
                        }
                    }

                    x0 += zoom * data->start_render_frac;  // update worldspace x for next loop, with respect to render_frac
                }

            } else {
                // FAST RENDERING
                for (int x = 0; x < data->scrn_width; x += data->start_render_frac) {
                    int iterations = calculateMandelbrot(x0, y0, data->vp->iterations);

                    // coloring (0xFFRRGGBB format for ARGB8888) + write to buffer
                    // copy to neighboring cells depending on render fraction
                    for (int k = 0; k < data->start_render_frac; k++) {
                        if ((x + k) < data->scrn_width) {  // check if within buffer
                            out[x + k] = data->palette[fast_map_range(iterations, data->vp->iterations, data->palette_size - 1)];
                        }
                    }

                    x0 += zoom * data->start_render_frac;  // update worldspace x for next loop, with respect to render_frac
                }
            }

            for (int p = 1; p < data->start_render_frac; p++) {
                int target_y = y + p;
                if (target_y < data->end_y) {
                    Uint8* src = (Uint8*)data->local_buffer + (y * pitch);
                    Uint8* dst = (Uint8*)data->local_buffer + (target_y * pitch);
                    memcpy(dst, src, pitch);
                }
            }
            row += pitch * data->start_render_frac;  // update worldspace y for next loop
        }
        if (data->start_render_frac == 1) {
            return NULL;
        }
        data->start_render_frac /= 2;
    }
}