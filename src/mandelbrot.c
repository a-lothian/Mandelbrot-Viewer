#include "mandelbrot.h"
#include <stdio.h>
#include <math.h>

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
//
// Optimization 1, isKnownInside(): if x,y is within known regions, it will not escape.
// Optimization 2, checkInterval: if the distance between x,y and x,y from many steps ago is tiny, it will not escape.
int calculateMandelbrot(double x0, double y0, int max_iterations) {

    if (isKnownInside(x0, y0)) return max_iterations;

    double x = 0.0;
    double y = 0.0;

    double oldx = 0.0;
    double oldy = 0.0;

    const double epsilon = 1e-6;
    const int checkInterval = 20;

    for (int i = 0; i < max_iterations; i++) {
        double x2 = x * x;
        double y2 = y * y;

        if (x2 + y2 > 4.0) {
            return i; // Return the escape iteration count
        }

        double xy = x * y;

        y = (2.0 * xy) + y0;
        x = (x2 - y2) + x0;

        // Periodic distance check
        if (i % checkInterval == 0) {
            double dx = x - oldx;
            double dy = y - oldy;

            if (dx * dx + dy * dy < epsilon) {
                return max_iterations; // inside set
            }
            oldx = x;
            oldy = y;
        }
    }

    return max_iterations;
}