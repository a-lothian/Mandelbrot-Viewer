#include <SDL2/SDL.h>
#include <stdbool.h>

struct viewport {
    bool is_dragging;
    int drag_start_x;
    int drag_start_y;

    float current_offset_x;
    float current_offset_y;

    float initial_offset_x;
    float initial_offset_y;
};

struct viewport* init_viewport() {
    struct viewport* vp = (struct viewport*)malloc(sizeof(struct viewport));

    vp->is_dragging = false;
    vp->drag_start_x = 0;
    vp->drag_start_y = 0;

    vp->current_offset_x = 0.0f;
    vp->current_offset_y = 0.0f;

    vp->initial_offset_x = 0.0f;
    vp->initial_offset_y = 0.0f;

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

            float PIXEL_TO_FRACTAL_SCALE = 0.005f;

            // calculate the new offset relative to the initial offset
            state->current_offset_x = state->initial_offset_x - ((float)dx * PIXEL_TO_FRACTAL_SCALE);
            state->current_offset_y = state->initial_offset_y - ((float)dy * PIXEL_TO_FRACTAL_SCALE);

            redraw_required = true;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT) {
            state->is_dragging = false;
        }
        break;
    }
    return redraw_required;
}