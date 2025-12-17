#include "mandelbrot.h"
#include <stdio.h>
#include <math.h>

struct coord {
    double x;
    double y;
};

static inline int isKnownInside(double x0, double y0) {
    // test 1. If x0, y0 is within distance of 1/4 from point (-1,0)
    // it is guaranteed to be inside
    double x1 = x0 + 1.0f;
    if (x1 * x1 + y0 * y0 <= 0.0625) return 1;


    // test 2. If x0, y0, falls within known cardiod region, it is inside.
    double x = x0 - 0.2f;
    double q = x * x + y0 * y0;
    if (q * (q + x) <= 0.25 * y0 * y0) return 1;


    return 0;
}

// calculate if the point(x(n+1),y(n+1)) will escape given enough iterations
// returns number of iterations required to escape bounding length of 4
int calculateMandelbrot(double x0, double y0, int max_iterations) {

    if (isKnownInside(x0, y0)) return max_iterations;

    double x = 0.0;
    double y = 0.0;

    double x_next;
    double y_next;

    for (int i = 0; i < max_iterations; i++) {
        double x2 = x * x;
        double y2 = y * y;

        if (x2 + y2 > 4.0) {
            return i; // Return the escape iteration count
        }

        double xy = x * y;

        y = (2.0 * xy) + y0;
        x = (x2 - y2) + x0;
    }

    return max_iterations;
}