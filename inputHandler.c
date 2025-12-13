#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

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
};

float mapRange(float x, float inMin, float inMax, float outMin, float outMax) {
    // clamping
    x = fminf(inMax, x);
    x = fmaxf(inMin, x);

    return (outMin + ((float)x - inMin) * (outMax - outMin) / (inMax - inMin));
}

struct viewport* init_viewport(int width, int height) {
    struct viewport* vp = (struct viewport*)malloc(sizeof(struct viewport));

    vp->screen_width = width;
    vp->screen_height = height;

    vp->is_dragging = false;
    vp->drag_start_x = 0;
    vp->drag_start_y = 0;

    vp->current_offset_x = 0.0f;
    vp->current_offset_y = 0.0f;

    vp->initial_offset_x = 0.0f;
    vp->initial_offset_y = 0.0f;

    vp->zoom = 0.01f;

    return vp;
}

// true when screen redraw is required
bool handle_mouse_events(SDL_Event* event, struct viewport* state) {
    bool redraw_required = false;

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT) {
            state->is_dragging = true;

            state->drag_start_x = event->button.x;
            state->drag_start_y = event->button.y;

            state->initial_offset_x = state->current_offset_x;
            state->initial_offset_y = state->current_offset_y;
        }
        break;

    case SDL_MOUSEMOTION:
        if (state->is_dragging) {
            int current_x = event->motion.x;
            int current_y = event->motion.y;

            int dx = current_x - state->drag_start_x;
            int dy = current_y - state->drag_start_y;

            state->current_offset_x = state->initial_offset_x - (float)dx * state->zoom;
            state->current_offset_y = state->initial_offset_y - (float)dy * state->zoom;

            redraw_required = true;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT) {
            state->is_dragging = false;
        }
        break;

    case SDL_MOUSEWHEEL:
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        float world_x = ((float)(mx - (state->screen_width / 2)) * state->zoom + state->current_offset_x);
        float world_y = ((float)(mx - (state->screen_height / 2)) * state->zoom + state->current_offset_y);

        // variable zoom speed
        float factor = 1.0f;
        float zoomFac = event->wheel.y;
        if (zoomFac > 0) {
            factor = mapRange(zoomFac, 0.0f, 100.0f, 1.0f, 2.5f);
        } else if (zoomFac < 0) {
            factor = mapRange(zoomFac, -100.0f, 0.0f, 0.001f, 1.0f);
        }

        state->zoom *= factor;
        redraw_required = true;

        break;
    }
    return redraw_required;
}