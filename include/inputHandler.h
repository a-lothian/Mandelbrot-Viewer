#ifndef MANDELBROT_VIEW
#define MANDELBROT_VIEW

#include <SDL3/SDL.h>
#include <stdbool.h>

struct viewport {
    int screen_width;
    int screen_height;

    bool is_dragging;
    float drag_start_x;
    float drag_start_y;

    double current_offset_x;
    double current_offset_y;

    double initial_offset_x;
    double initial_offset_y;

    double zoom;

    int iterations;
    double iteration_multiplier;
};

struct viewport* init_viewport(int width, int height);
void ZoomOnMouse(struct viewport* vp, double zoom_factor);
bool handle_mouse_events(SDL_Event* event, struct viewport* state);

#endif