#include "mandelbrot.h"
#include <stdio.h>
#include <math.h>

struct coord {
    float x;
    float y;
};

static inline int isKnownInside(float x0, float y0) {
    // test 1. If x0, y0 is within distance of 1/4 from point (-1,0)
    // it is guaranteed to be inside
    float x1 = x0 + 1.0f;
    if (x1 * x1 + y0 * y0 <= 0.0625f) return 1;


    // test 2. If x0, y0, falls within known cardiod region, it is inside.
    float x = x0 - 0.25f;
    float q = x * x + y0 * y0;
    if (q * (q + x) <= 0.25f * y0 * y0) return 1;


    return 0;
}

// calculate if the point(x(n+1),y(n+1)) will escape given enough iterations
// returns number of iterations required to escape bounding length of 4
int calculateMandelbrot(float x0, float y0, int max_iterations) {

    if (isKnownInside(x0, y0)) return max_iterations;

    float x = 0.0f;
    float y = 0.0f;

    float x_next;
    float y_next;

    for (int i = 0; i < max_iterations; i++) {
        float x2 = x * x;
        float y2 = y * y;

        if (x2 + y2 > 4.0f) {
            return i; // Return the escape iteration count
        }

        float xy = x * y;

        y = (2.0f * xy) + y0;
        x = (x2 - y2) + x0;
    }

    return max_iterations;
}