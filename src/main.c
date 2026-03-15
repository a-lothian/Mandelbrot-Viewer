#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32  // predefined in vs2022 stdlib
#define HAVE_STRUCT_TIMESPEC
#endif
#include "colour_palette.h"
#include "core_count.h"
#include "inputHandler.h"
#include "mandelbrot.h"
#include "render_context.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <pthread.h>

#define SCRN_HEIGHT 720
#define SCRN_WIDTH 1280

#define TARGET_FPS 60  // change if supported!
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

void cleanup(struct RenderContext* rc, struct ThreadPool* tp, struct viewport* vp)
{
    free(tp->jobs);
    free(tp->threads);
    free(rc->buffer);
    free(vp);
    SDL_DestroyTexture(rc->texture);
    SDL_DestroyRenderer(rc->renderer);
    SDL_DestroyWindow(rc->window);
    SDL_Quit();
}

int calculateIterations(double zoom)
{
    if (zoom <= 0.0)
        return 5000;

    double magnification = 1.0 / zoom;

    // "100 per decade" heuristic
    int iter = 40 + (100 * log10(magnification));

    if (iter < 32) {
        return 32;
    }

    return iter;
}

void drawBuffer(struct RenderContext* rc, struct ThreadPool* tp, struct PaletteState* ps)
{
    tp->kill = true;

    for (int i = 0; i < tp->count; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    tp->kill = false;
    SDL_RenderClear(rc->renderer);

    for (int i = 0; i < tp->count; i++) {
        tp->jobs[i].start_render_frac = 16;
        tp->jobs[i].render_smooth = ps->smooth;
        tp->jobs[i].palette = ps->generated;
        pthread_create(&tp->threads[i], NULL, calculateMandelbrotRoutine, &tp->jobs[i]);
    }
}

int init_app(struct RenderContext* rc, struct ThreadPool* tp, struct PaletteState* ps, struct viewport** vp_out)
{
    SDL_Init(SDL_INIT_VIDEO);

    rc->width = SCRN_WIDTH;
    rc->height = SCRN_HEIGHT;
    rc->window = SDL_CreateWindow("Mandelbrot Set", SCRN_WIDTH, SCRN_HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    rc->renderer = SDL_CreateRenderer(rc->window, NULL);
    rc->texture = SDL_CreateTexture(rc->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
    rc->buffer = malloc(sizeof(Uint32) * SCRN_HEIGHT * SCRN_WIDTH);

    if (!rc->window || !rc->renderer || !rc->texture || !rc->buffer) {
        fprintf(stderr, "Failed to initialise SDL resources\n");
        // partial cleanup — vp not yet allocated
        free(rc->buffer);
        SDL_DestroyTexture(rc->texture);
        SDL_DestroyRenderer(rc->renderer);
        SDL_DestroyWindow(rc->window);
        SDL_Quit();
        return 1;
    }

    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);
    tp->count = get_num_logical_cores();
    tp->threads = calloc(tp->count, sizeof(pthread_t));

    if (!vp || !tp->threads) {
        fprintf(stderr, "Failed to allocate memory\n");
        free(vp);
        cleanup(rc, tp, vp);
        return 1;
    }

    tp->jobs = malloc(tp->count * sizeof(struct RenderJob));
    if (!tp->jobs) {
        fprintf(stderr, "Failed to allocate render data\n");
        cleanup(rc, tp, vp);
        return 1;
    }

    // palette
    ps->index = 0;
    ps->smooth = true;
    ps->current = list_palettes[0];
    generateColourPalette(ps->current, 8, ps->generated, PALETTE_SIZE);

    // render options
    tp->kill = false;
    vp->iterations = calculateIterations(vp->zoom) * vp->iteration_multiplier;

    // distribute rows across threads
    int rows_per_thread = SCRN_HEIGHT / tp->count;
    for (int i = 0; i < tp->count; i++) {
        tp->jobs[i].start_y = i * rows_per_thread;
        tp->jobs[i].end_y = (i == tp->count - 1) ? SCRN_HEIGHT : (i + 1) * rows_per_thread;
        tp->jobs[i].scrn_width = SCRN_WIDTH;
        tp->jobs[i].vp = vp;
        tp->jobs[i].palette = ps->generated;
        tp->jobs[i].palette_size = PALETTE_SIZE;
        tp->jobs[i].render_smooth = ps->smooth;
        tp->jobs[i].buffer = rc->buffer;
        tp->jobs[i].kill_signal = &tp->kill;
        tp->jobs[i].start_render_frac = 16;
    }

    *vp_out = vp;
    return 0;
}

// returns false when the application should quit
bool process_events(struct ThreadPool* tp, struct PaletteState* ps, struct viewport* vp, struct RenderContext* rc)
{
    SDL_Event event;
    bool redraw = false;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            switch (event.key.key) {
            // use < and > to change max_iterations
            case SDLK_PERIOD:
                vp->iteration_multiplier *= 2;
                break;

            case SDLK_COMMA:
                if (vp->iteration_multiplier > 0.0625) {
                    vp->iteration_multiplier /= 2;
                }
                break;

            // use / to toggle smooth (cyclic) shading
            case SDLK_SLASH:
                ps->smooth = !ps->smooth;
                break;

            // use M to change colour palette
            case SDLK_M:
                ps->current = cyclePalettes(&ps->index);
                generateColourPalette(ps->current, 8, ps->generated, PALETTE_SIZE);
                break;

            case SDLK_ESCAPE:
                return false;

            default:
                break;
            }
            redraw = true;
        }

        if (handle_mouse_events(&event, vp)) {
            redraw = true;
        }
    }

    if (redraw) {
        int it = (int)(calculateIterations(vp->zoom) * vp->iteration_multiplier);
        vp->iterations = (it < 1) ? 1 : it;
        drawBuffer(rc, tp, ps);
    }

    return true;
}

void shutdown_app(struct RenderContext* rc, struct ThreadPool* tp, struct viewport* vp)
{
    // stop all render threads before freeing shared resources
    tp->kill = true;
    for (int i = 0; i < tp->count; i++) {
        pthread_join(tp->threads[i], NULL);
    }
    cleanup(rc, tp, vp);
}

int main(int argc, char* argv[])
{
    printf("Mandelbrot Viewer in C by Alex Lothian\n"
           " Click + Drag to Navigate. Scroll to Zoom.\n"
           " < : Half Maximum Iterations\n"
           " > : Double Maximum Iterations\n"
           " / : Toggle Cyclic Shading Mode\n\n\n");

    struct RenderContext rc = {0};
    struct ThreadPool tp = {0};
    struct PaletteState ps = {0};
    struct viewport* vp = NULL;

    if (init_app(&rc, &tp, &ps, &vp) != 0) {
        return 1;
    }

    drawBuffer(&rc, &tp, &ps);

    while (true) {
        Uint64 frameStart = SDL_GetTicks();

        if (!process_events(&tp, &ps, vp, &rc)) {
            break;
        }

        // transfer buffer in RAM to VRAM
        SDL_UpdateTexture(rc.texture, NULL, rc.buffer, sizeof(Uint32) * SCRN_WIDTH);

        // draw VRAM
        SDL_RenderTexture(rc.renderer, rc.texture, NULL, NULL);
        SDL_RenderPresent(rc.renderer);

        Uint64 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }

    shutdown_app(&rc, &tp, vp);
    return 0;
}
