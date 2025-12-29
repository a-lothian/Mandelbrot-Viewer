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

#define PRNT_VP 1

#define SCRN_HEIGHT 720
#define SCRN_WIDTH 1280

#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

void drawBuffer(SDL_Renderer* _prenderer, SDL_Texture* screen, Uint32* pixel_buffer, struct mandelbrotRoutineData* renderData, long core_count, pthread_t* threads, bool draw_smooth, Uint32* generated_palette) {
    *renderData[0].kill_signal = true;  // reference by pointer; kills all

    for (int i = 0; i < core_count; i++) {
        pthread_join(threads[i], NULL);
    }

    *renderData[0].kill_signal = false;
    SDL_RenderClear(_prenderer);

    for (int i = 0; i < core_count; i++) {
        renderData[i].start_render_frac = 16;
        renderData[i].render_smooth = draw_smooth;
        renderData[i].palette = generated_palette;
        pthread_create(&threads[i], NULL, calculateMandelbrotRoutine, &renderData[i]);
    }
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

int main(int argc, char* argv[]) {
    printf("Mandelbrot Viewer in C by Alex Lothian\n Click + Drag to Navigate. Scroll to Zoom.\n < : Half Maximum Iterations\n > : Double Maximum Iterations\n / : Toggle Cyclic Shading Mode\n\n\n");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* pwindow = SDL_CreateWindow("Mandelbrot Set", SCRN_WIDTH, SCRN_HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, NULL);
    SDL_Texture* scrnTexture = SDL_CreateTexture(prenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);
    Uint32* renderBuffer = malloc(sizeof(Uint32) * SCRN_HEIGHT * SCRN_WIDTH);
    Uint64 frameStart, frameTime;

    if (!prenderer) {
        fprintf(stderr, "Renderer Failed to init");
        SDL_DestroyWindow(pwindow);
        SDL_DestroyRenderer(prenderer);
        return 1;
    }

    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);

    // take a chosen array of colour and generate a 2048 colour gradient from them
    const Uint32* colour_palette = list_palettes[0];
    Uint32* generated_palette = generateColourPalette(colour_palette, 8, 2048);

    long core_count = get_num_logical_cores();
    pthread_t* threads = calloc(core_count, sizeof(pthread_t));
    volatile bool kill_threads = false;

    int rows_per_thread = SCRN_HEIGHT / core_count;

    // render options
    bool draw_smooth = true;
    double iteration_multiplier = 1.0;
    vp->iterations = calculateIterations(vp->zoom) * iteration_multiplier;
    int palette_index = 0;

    struct mandelbrotRoutineData* renderData = malloc(core_count * sizeof(struct mandelbrotRoutineData));

    // create threads and job data
    for (int i = 0; i < core_count; i++) {
        renderData[i].start_y = i * rows_per_thread;
        renderData[i].end_y = (i == core_count - 1) ? SCRN_HEIGHT : (i + 1) * rows_per_thread;  // last thread takes remaining rows
        renderData[i].scrn_width = SCRN_WIDTH;
        renderData[i].vp = vp;
        renderData[i].palette = generated_palette;
        renderData[i].palette_size = 2048;
        renderData[i].render_smooth = draw_smooth;
        renderData[i].local_buffer = renderBuffer;
        renderData[i].kill_signal = &kill_threads;
        renderData[i].start_render_frac = 16;
    }

    bool running = true;
    bool redraw = false;

    drawBuffer(prenderer, scrnTexture, renderBuffer, renderData, core_count, threads, draw_smooth, generated_palette);

    while (running) {
        frameStart = SDL_GetTicks();
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                // use < and > to change max_iterations
                switch (event.key.key) {
                case SDLK_PERIOD:
                    iteration_multiplier *= 2;
                    break;

                case SDLK_COMMA:
                    if (iteration_multiplier > 0.0625) {
                        iteration_multiplier /= 2;
                    }
                    break;

                // use / to toggle smooth (cyclic) shading
                case SDLK_SLASH:
                    draw_smooth = !draw_smooth;
                    break;

                // use M to change colour palette
                case SDLK_M:
                    colour_palette = cyclePalettes(&palette_index);
                    generated_palette = generateColourPalette(colour_palette, 8, 2048);
                    break;

                case SDLK_ESCAPE:
                    running = false;
                    break;

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
            int it = (int)(calculateIterations(vp->zoom) * iteration_multiplier);
            vp->iterations = (it < 1) ? 1 : it;
            drawBuffer(prenderer, scrnTexture, renderBuffer, renderData, core_count, threads, draw_smooth, generated_palette);
            redraw = false;
        }

        // transfer buffer in RAM to VRAM
        SDL_UpdateTexture(scrnTexture, NULL, renderBuffer, sizeof(Uint32) * SCRN_WIDTH);

        // draw VRAM
        SDL_RenderTexture(prenderer, scrnTexture, NULL, NULL);
        SDL_RenderPresent(prenderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }

    SDL_DestroyWindow(pwindow);
    SDL_DestroyRenderer(prenderer);

    return 0;
}