#ifndef MANDELBROT_CALC
#define MANDELBROT_CALC

#include <SDL3/SDL_stdinc.h>  // for Uint32 type

struct mandelbrotRoutineData {
    int start_y;
    int end_y;
    int scrn_width;
    struct viewport* vp;
    Uint32* local_buffer;
    volatile bool* kill_signal;
    int start_render_frac;
};

int calculateMandelbrot(double x0, double y0, int iterations);
struct mandelbrotRoutineData* init_routine_data(int start_y, int end_y, int scrn_width, struct viewport* vp, Uint32* buffer);
void* calculateMandelbrotRoutine(void* arg);

#endif