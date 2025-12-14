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

    float current_offset_x;
    float current_offset_y;

    float initial_offset_x;
    float initial_offset_y;

    float zoom;

    int iterations;
};

struct viewport* init_viewport();
bool handle_mouse_events(SDL_Event* event, struct viewport* state);

#endif