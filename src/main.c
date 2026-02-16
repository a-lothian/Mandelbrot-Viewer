#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN32 // predefined in vs2022 stdlib
#define HAVE_STRUCT_TIMESPEC
#endif
#include <pthread.h>
#include <math.h>

#include "mandelbrot.h"
#include "inputHandler.h"
#include "core_count.h"
#include "colour_palette.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SCRN_HEIGHT 720
#define SCRN_WIDTH 1280

#define TARGET_FPS 60      // change if supported!
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

struct AppState {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Uint32* renderBuffer;

    struct viewport* vp;
    struct mandelbrotRoutineData* renderData;
    pthread_t* threads;
    long core_count;
    volatile bool kill_threads;

    Uint32 generated_palette[PALETTE_SIZE];
    const Uint32* colour_palette;
    int palette_index;

    bool draw_smooth;
    double iteration_multiplier;
};

void cleanup(struct AppState* app) {
    free(app->renderData);
    free(app->threads);
    free(app->renderBuffer);
    free(app->vp);
    SDL_DestroyTexture(app->texture);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
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

    return iter;
}

void drawBuffer(struct AppState* app) {
    app->kill_threads = true;

    for (int i = 0; i < app->core_count; i++) {
        pthread_join(app->threads[i], NULL);
    }

    app->kill_threads = false;
    SDL_RenderClear(app->renderer);

    for (int i = 0; i < app->core_count; i++) {
        app->renderData[i].start_render_frac = 16;
        app->renderData[i].render_smooth = app->draw_smooth;
        app->renderData[i].palette = app->generated_palette;
        pthread_create(&app->threads[i], NULL, calculateMandelbrotRoutine, &app->renderData[i]);
    }
}

int init_app(struct AppState* app) {
    SDL_Init(SDL_INIT_VIDEO);

    app->window = SDL_CreateWindow("Mandelbrot Set", SCRN_WIDTH, SCRN_HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    app->renderer = SDL_CreateRenderer(app->window, NULL);
    app->texture = SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
    app->renderBuffer = malloc(sizeof(Uint32) * SCRN_HEIGHT * SCRN_WIDTH);

    if (!app->window || !app->renderer || !app->texture || !app->renderBuffer) {
        fprintf(stderr, "Failed to initialise SDL resources\n");
        cleanup(app);
        return 1;
    }

    app->vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);
    app->core_count = get_num_logical_cores();
    app->threads = calloc(app->core_count, sizeof(pthread_t));

    if (!app->vp || !app->threads) {
        fprintf(stderr, "Failed to allocate memory\n");
        cleanup(app);
        return 1;
    }

    app->renderData = malloc(app->core_count * sizeof(struct mandelbrotRoutineData));
    if (!app->renderData) {
        fprintf(stderr, "Failed to allocate render data\n");
        cleanup(app);
        return 1;
    }

    // palette
    app->palette_index = 0;
    app->colour_palette = list_palettes[0];
    generateColourPalette(app->colour_palette, 8, app->generated_palette, PALETTE_SIZE);

    // render options
    app->draw_smooth = true;
    app->iteration_multiplier = 1.0;
    app->kill_threads = false;
    app->vp->iterations = calculateIterations(app->vp->zoom) * app->iteration_multiplier;

    // distribute rows across threads
    int rows_per_thread = SCRN_HEIGHT / app->core_count;
    for (int i = 0; i < app->core_count; i++) {
        app->renderData[i].start_y = i * rows_per_thread;
        app->renderData[i].end_y = (i == app->core_count - 1) ? SCRN_HEIGHT : (i + 1) * rows_per_thread;
        app->renderData[i].scrn_width = SCRN_WIDTH;
        app->renderData[i].vp = app->vp;
        app->renderData[i].palette = app->generated_palette;
        app->renderData[i].palette_size = PALETTE_SIZE;
        app->renderData[i].render_smooth = app->draw_smooth;
        app->renderData[i].local_buffer = app->renderBuffer;
        app->renderData[i].kill_signal = &app->kill_threads;
        app->renderData[i].start_render_frac = 16;
    }

    return 0;
}

// returns false when the application should quit
bool process_events(struct AppState* app) {
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
                app->iteration_multiplier *= 2;
                break;

            case SDLK_COMMA:
                if (app->iteration_multiplier > 0.0625) {
                    app->iteration_multiplier /= 2;
                }
                break;

            // use / to toggle smooth (cyclic) shading
            case SDLK_SLASH:
                app->draw_smooth = !app->draw_smooth;
                break;

            // use M to change colour palette
            case SDLK_M:
                app->colour_palette = cyclePalettes(&app->palette_index);
                generateColourPalette(app->colour_palette, 8, app->generated_palette, PALETTE_SIZE);
                break;

            case SDLK_ESCAPE:
                return false;

            default:
                break;
            }
            redraw = true;
        }

        if (handle_mouse_events(&event, app->vp)) {
            redraw = true;
        }
    }

    if (redraw) {
        int it = (int)(calculateIterations(app->vp->zoom) * app->iteration_multiplier);
        app->vp->iterations = (it < 1) ? 1 : it;
        drawBuffer(app);
    }

    return true;
}

void shutdown_app(struct AppState* app) {
    // stop all render threads before freeing shared resources
    app->kill_threads = true;
    for (int i = 0; i < app->core_count; i++) {
        pthread_join(app->threads[i], NULL);
    }
    cleanup(app);
}

int main(int argc, char* argv[]) {
    printf("Mandelbrot Viewer in C by Alex Lothian\n"
           " Click + Drag to Navigate. Scroll to Zoom.\n"
           " < : Half Maximum Iterations\n"
           " > : Double Maximum Iterations\n"
           " / : Toggle Cyclic Shading Mode\n\n\n");

    struct AppState app = {0};

    if (init_app(&app) != 0) {
        return 1;
    }

    drawBuffer(&app);

    while (true) {
        Uint64 frameStart = SDL_GetTicks();

        if (!process_events(&app)) {
            break;
        }

        // transfer buffer in RAM to VRAM
        SDL_UpdateTexture(app.texture, NULL, app.renderBuffer, sizeof(Uint32) * SCRN_WIDTH);

        // draw VRAM
        SDL_RenderTexture(app.renderer, app.texture, NULL, NULL);
        SDL_RenderPresent(app.renderer);

        Uint64 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }

    shutdown_app(&app);
    return 0;
}
