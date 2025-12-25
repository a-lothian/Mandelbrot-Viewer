#include "inputHandler.h"
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

double mapRange(double x, double inMin, double inMax, double outMin, double outMax) {
    // clamping
    x = fmin(inMax, x);
    x = fmax(inMin, x);

    return (outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin));
}

struct viewport* init_viewport(int width, int height) {
    struct viewport* vp = malloc(sizeof(struct viewport));

    vp->screen_width = width;
    vp->screen_height = height;

    vp->is_dragging = false;
    vp->drag_start_x = 0;
    vp->drag_start_y = 0;

    vp->current_offset_x = -0.72;
    vp->current_offset_y = 0.0;

    vp->initial_offset_x = 0.01;
    vp->initial_offset_y = 0.0;

    vp->zoom = 0.0032;

    vp->iterations = 64;

    return vp;
}

// true when screen redraw is required
bool handle_mouse_events(SDL_Event* event, struct viewport* state) {
    bool redraw_required = false;

    switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT) {
            state->is_dragging = true;

            state->drag_start_x = event->button.x;
            state->drag_start_y = event->button.y;

            state->initial_offset_x = state->current_offset_x;
            state->initial_offset_y = state->current_offset_y;
        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
        if (state->is_dragging) {
            int current_x = event->motion.x;
            int current_y = event->motion.y;

            int dx = current_x - state->drag_start_x;
            int dy = current_y - state->drag_start_y;

            state->current_offset_x = state->initial_offset_x - (double)dx * state->zoom;
            state->current_offset_y = state->initial_offset_y - (double)dy * state->zoom;

            redraw_required = true;
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_LEFT) {
            state->is_dragging = false;
        }
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        float mx, my;
        SDL_GetMouseState(&mx, &my);

        double mouse_screen_x = (double)mx - (state->screen_width * 0.5);
        double mouse_screen_y = (double)my - (state->screen_height * 0.5);

        double world_x = state->current_offset_x + mouse_screen_x * state->zoom;
        double world_y = state->current_offset_y + mouse_screen_y * state->zoom;

        double zoom_intensity = 0.25;
        double factor = 1.0;

        if (event->wheel.y > 0) {
            factor = 1.0 / (1.0 + (event->wheel.y * zoom_intensity));
        } else if (event->wheel.y < 0) {
            factor = 1.0 + (-event->wheel.y * zoom_intensity);
        }

        state->zoom *= factor;

        state->current_offset_x = world_x - mouse_screen_x * state->zoom;
        state->current_offset_y = world_y - mouse_screen_y * state->zoom;

        // printf("zoomFac: %f | factor: %lf | new_zoom: %le\n", event->wheel.y, factor, state->zoom);

        redraw_required = true;
        break;
    }
    return redraw_required;
}