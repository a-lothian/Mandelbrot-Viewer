#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32  // predefined in vs2022 stdlib
#define HAVE_STRUCT_TIMESPEC
#endif
#include "benchmark.h"
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

#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

#define MAX_ITERATIONS 100000

void cleanup(struct RenderContext* rc, struct ThreadPool* tp, struct viewport* vp) {
    if (tp != NULL) {
        for (int i = 0; i < tp->count; i++) {
            free(tp->jobs[i].iteration_out);
        }
    }

    free(tp->jobs);
    free(tp->threads);
    free(rc->buffer);
    free(vp);

    SDL_DestroyTexture(rc->texture);
    SDL_DestroyRenderer(rc->renderer);
    SDL_DestroyWindow(rc->window);
    SDL_Quit();
}

int calculateIterations(double zoom) {
    if (zoom <= 0.0)
        return 5000;

    double magnification = 1.0 / zoom;

    // "100 per decade" heuristic
    int iter = 40 + (100 * log10(magnification));

    if (iter < 32) {
        return 32;
    }
    iter = iter > MAX_ITERATIONS ? MAX_ITERATIONS : iter;
    return iter;
}

void drawBuffer(struct RenderContext* rc, struct ThreadPool* tp, struct PaletteState* ps) {
    // rejoin existing threads
    if (tp->threads != NULL) {
        tp->kill = true;

        for (int i = 0; i < tp->count; i++) {
            pthread_join(tp->threads[i], NULL);
        }

        tp->kill = false;
    }

    // begin new render
    SDL_RenderClear(rc->renderer);

    for (int i = 0; i < tp->count; i++) {
        tp->jobs[i].start_render_frac = 8;
        tp->jobs[i].render_smooth = ps->smooth;
        tp->jobs[i].palette = ps->generated;
        tp->jobs[i].use_simd = true;
        pthread_create(&tp->threads[i], NULL, calculateMandelbrotRoutine, &tp->jobs[i]);
    }
}

int init_app(struct RenderContext* rc, struct ThreadPool* tp, struct PaletteState* ps, struct viewport** vp_out, int arg_thread_num) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialise SDL\n");
        return 1;
    }

    rc->width = SCRN_WIDTH;
    rc->height = SCRN_HEIGHT;
    rc->window = SDL_CreateWindow("Mandelbrot Set", SCRN_WIDTH, SCRN_HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!rc->window) {
        fprintf(stderr, "Failed to initialise SDL resources\n");
        SDL_Quit();
        return 1;
    }
    rc->renderer = SDL_CreateRenderer(rc->window, NULL);
    if (!rc->renderer) {
        fprintf(stderr, "Failed to initialise SDL resources\n");
        SDL_DestroyWindow(rc->window);
        SDL_Quit();
        return 1;
    }
    rc->texture = SDL_CreateTexture(rc->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
    rc->buffer = malloc(sizeof(Uint32) * SCRN_HEIGHT * SCRN_WIDTH);

    if (!rc->texture || !rc->buffer) {
        fprintf(stderr, "Failed to initialise SDL resources\n");
        // vp not yet allocated
        free(rc->buffer);
        SDL_DestroyTexture(rc->texture);
        SDL_DestroyRenderer(rc->renderer);
        SDL_DestroyWindow(rc->window);
        SDL_Quit();
        return 1;
    }

    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);

    // allow --threads arg to override
    tp->count = arg_thread_num == 0 ? get_num_logical_cores() : arg_thread_num;
    tp->threads = calloc(tp->count, sizeof(pthread_t));

    if (!vp || !tp->threads) {
        fprintf(stderr, "Failed to allocate memory\n");
        cleanup(rc, tp, vp);
        return 1;
    }

    tp->jobs = calloc(tp->count, sizeof(struct RenderJob));
    if (!tp->jobs) {
        fprintf(stderr, "Failed to allocate render data\n");
        cleanup(rc, tp, vp);
        return 1;
    }

    for (int i = 0; i < tp->count; i++) {
        tp->jobs[i].iteration_out = malloc(SCRN_WIDTH * sizeof(int));
        if (!tp->jobs[i].iteration_out) {
            fprintf(stderr, "Failed to allocate iter_scratch\n");
            cleanup(rc, tp, vp);
            return 1;
        }
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
        tp->jobs[i].start_render_frac = 8;
    }

    *vp_out = vp;
    return 0;
}

// returns false when the application should quit
bool process_events(struct ThreadPool* tp, struct PaletteState* ps, struct viewport* vp, struct RenderContext* rc) {
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

            case SDLK_RETURN:
                ZoomOnMouse(vp, 0.95);
                break;
            case SDLK_APOSTROPHE:
                ZoomOnMouse(vp, 1.15);
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

void shutdown_app(struct RenderContext* rc, struct ThreadPool* tp, struct viewport* vp) {
    // stop all render threads before freeing shared resources
    tp->kill = true;
    for (int i = 0; i < tp->count; i++) {
        pthread_join(tp->threads[i], NULL);
    }
    cleanup(rc, tp, vp);
}

int main(int argc, char* argv[]) {
    // check for benchmark call
    struct BenchmarkOpts bench_opts = {.threads = 0, .smooth = false, .scalar = false, .sweep = false, .no_optimisations = false};
    bool do_benchmark = false;
    int thread_count_override = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--benchmark") == 0) {
            do_benchmark = true;
        } else if (strcmp(argv[i], "--smooth") == 0) {
            bench_opts.smooth = true;
        } else if (strcmp(argv[i], "--fast") == 0) {
            bench_opts.smooth = false;
        } else if (strcmp(argv[i], "--scalar") == 0) {
            bench_opts.scalar = true;
        } else if (strcmp(argv[i], "--sweep") == 0) {
            bench_opts.sweep = true;
        } else if (strcmp(argv[i], "--nooptimisation") == 0) {
            bench_opts.no_optimisations = true;
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            bench_opts.threads = atoi(argv[++i]);
            thread_count_override = bench_opts.threads;
        }
    }

    if (do_benchmark) {
        if (bench_opts.sweep)
            run_sweep(bench_opts);
        else
            run_benchmark(bench_opts);
        return 0;
    }

    printf(
        "Mandelbrot Viewer in C by Alex Lothian\n"
        " Click + Drag to Navigate. Scroll to Zoom.\n"
        " < : Half Maximum Iterations\n"
        " > : Double Maximum Iterations\n"
        " / : Toggle Cyclic Shading Mode\n\n");

    struct RenderContext rc = {0};
    struct ThreadPool tp = {0};
    struct PaletteState ps = {0};
    struct viewport* vp = NULL;

    if (init_app(&rc, &tp, &ps, &vp, thread_count_override) != 0) {
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
