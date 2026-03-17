#ifndef MANDELBROT_CALC
#define MANDELBROT_CALC

#include <SDL3/SDL_stdinc.h>  // for Uint32 type
#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct viewport;

struct RenderJob {
    int start_y, end_y, scrn_width;
    struct viewport* vp;
    Uint32* palette;
    int palette_size;
    Uint32* buffer;
    _Atomic bool* kill_signal;
    int start_render_frac;
    bool render_smooth;
    bool use_simd;
    int* iteration_out;
    bool no_optimisations;
};

struct ThreadPool {
    pthread_t* threads;
    struct RenderJob* jobs;
    long count;
    _Atomic bool kill;
};

int calculateMandelbrot(double x0, double y0, int iterations);
int calculateMandelbrotOpts(double x0, double y0, int iterations, bool no_optimisations);
void* calculateMandelbrotRoutine(void* arg);

#ifdef __cplusplus
}
#endif

#endif
