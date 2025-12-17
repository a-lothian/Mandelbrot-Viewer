#ifndef MANDELBROT_VIEW
#define MANDELBROT_VIEW

#include <stdbool.h>
#include <SDL2/SDL.h>

struct viewport {
    int screen_width;
    int screen_height;

    bool is_dragging;
    int drag_start_x;
    int drag_start_y;

    double current_offset_x;
    double current_offset_y;

    double initial_offset_x;
    double initial_offset_y;

    double zoom;

    int iterations;
};

struct viewport* init_viewport(int width, int height);
bool handle_mouse_events(SDL_Event* event, struct viewport* state);

#endif