#ifndef MANDELBROT_CALC
#define MANDELBROT_CALC

#include <SDL3/SDL_stdinc.h>  // for Uint32 type
#include <pthread.h>
#include <stdbool.h>

struct viewport;  // forward decl — avoids circular include with inputHandler.h

struct RenderJob {
    int start_y, end_y, scrn_width;
    struct viewport* vp;
    Uint32* palette;
    int palette_size;
    Uint32* buffer;
    volatile bool* kill_signal;
    int start_render_frac;
    bool render_smooth;
};

struct ThreadPool {
    pthread_t* threads;
    struct RenderJob* jobs;
    long count;
    volatile bool kill;
};

int calculateMandelbrot(double x0, double y0, int iterations);
void* calculateMandelbrotRoutine(void* arg);

#endif
