#include "mandelbrot.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL3/SDL_render.h>  // for SDL_texture
#include "inputHandler.h"

static inline int isKnownInside(double x0, double y0) {
    // test 1. If x0, y0 is within distance of 1/4 from point (-1,0)
    // it is guaranteed to be inside
    double x1 = x0 + 1.0;
    if (x1 * x1 + y0 * y0 <= 0.0625)
        return 1;

    // test 2. If x0, y0, falls within known cardiod region, it is inside.
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

        if (x2 + y2 > 4.0) {
            return i;  // Return the escape iteration count
        }

        double xy = x * y;

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

void* calculateMandelbrotRoutine(void* arg) {
    struct mandelbrotRoutineData* data = (struct mandelbrotRoutineData*)arg;
    int iterations;
    int pitch = sizeof(Uint32) * data->vp->screen_width;

    int halfWidth = data->vp->screen_width / 2;
    int halfHeight = data->vp->screen_height / 2;

    Uint8* row = (Uint8*)data->local_buffer + (data->start_y * pitch);

    const double zoom = data->vp->zoom;  // DISTANCE BETWEEN PIXELS IN WORLD SPACE
    double world_top = data->vp->current_offset_y - ((double)halfHeight * zoom);
    double world_left = data->vp->current_offset_x - ((double)halfWidth * zoom);

    const double inverseMax = 255.0 / (double)data->vp->iterations;

    // draw onto screen
    for (int y = data->start_y; y < data->end_y; y++) {
        // check for quick return
        if (*(data->kill_signal)) {
            return NULL;
        }

        Uint32* out = (Uint32*)row;

        // worldspace coordinates
        double x0 = world_left;
        double y0 = world_top + (double)y * zoom;

        for (int x = 0; x < data->scrn_width; x++) {
            iterations = calculateMandelbrot(x0, y0, data->vp->iterations);

            Uint8 brightness;

            if (iterations >= data->vp->iterations) {
                brightness = 0;
            } else {
                // scale iterations to fit within 0-255 range relative to the max
                brightness = (Uint8)(iterations * inverseMax);
            }

            // coloring (0xFFRRGGBB format for ARGB8888) + write to buffer
            out[x] = 0xFF000000u | (brightness << 16) | (brightness << 8) | brightness;

            x0 += zoom;  // update worldspace x for next loop
        }
        row += pitch;  // update worldspace y for next loop
    }
    return NULL;
}