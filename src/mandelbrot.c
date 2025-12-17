#include "mandelbrot.h"
#include <stdio.h>
#include <math.h>

struct coord {
    float x;
    float y;
};

// where x(0), y(0) is the starting point.

// new x (real)
// x(n+1) = x(n)^2 - y(n)^2 + x(0)
float calculateX(float inX, float inY, float initialX) {
    return pow(inX, 2) - pow(inY, 2) + initialX;
}

// new y (imaginary)
// y(n+1) = 2 * x(n) * y(n) + y(0)
float calculateY(float inX, float inY, float initialY) {
    return 2 * inX * inY + initialY;
}

// calculate if the point(x(n+1),y(n+1)) will escape given enough iterations
// returns number of iterations required to escape bounding length of 4
int calculateMandelbrot(float x0, float y0, int iterations) {
    float x_n = x0;
    float y_n = y0;

    float x_next;
    float y_next;

    for (int i = 0; i < iterations; i++) {
        x_next = calculateX(x_n, y_n, x0);
        y_next = calculateY(x_n, y_n, y0);

        x_n = x_next;
        y_n = y_next;

        // printf("x: %f , y: %f\n", x_n, y_n);

        if ((x_n * x_n + y_n * y_n) > 4.0) {
            return i;  // Return the escape iteration count
        }
    }

    return iterations;
}