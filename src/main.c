#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "mandelbrot.h"
#include "inputHandler.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define PRNT_VP 1

#define SCRN_HEIGHT 600
#define SCRN_WIDTH 800

const int halfHeight = SCRN_HEIGHT / 2;
const int halfWidth = SCRN_WIDTH / 2;

Uint64 CLKfrequency;

bool running = true;


void drawMandelbrot(SDL_Renderer* _prenderer, SDL_Texture* screen, struct viewport* vp) {
    if (PRNT_VP) printf("zoom=%lf offx=%lf offy=%lf iters=%d\n",
        vp->zoom, vp->current_offset_x, vp->current_offset_y, vp->iterations);

    

    int iterations;
    void* pixels;
    int pitch; // bytes per row

    SDL_LockTexture(screen, NULL, &pixels, &pitch);

    Uint8* row = (Uint8*)pixels;

    const double zoom = vp->zoom; // DISTANCE BETWEEN PIXELS IN WORLD SPACE
    const double startX = vp->current_offset_x - (double)halfWidth * zoom;
    const double startY = vp->current_offset_y - (double)halfHeight * zoom;

    const double inverseMax = 255.0 / (double)vp->iterations;

    // draw onto screen
    for (int y = 0; y < SCRN_HEIGHT; y++) {

        Uint32* out = (Uint32*)row; 

        // worldspace coordinates
        double x0 = startX;
        double y0 = startY + (double)y * zoom;

        for (int x = 0; x < SCRN_WIDTH; x++) {
            iterations = calculateMandelbrot(x0, y0, vp->iterations);

            Uint8 brightness;

            if (iterations >= vp->iterations) {
                brightness = 0;
            } else {
                 // scale iterations to fit within 0-255 range relative to the max
                brightness = (Uint8)(iterations * inverseMax);
            }
            
            // coloring (0xFFRRGGBB format for ARGB8888) + write to buffer
            out[x] = 0xFF000000u | (brightness << 16) | (brightness << 8) | brightness;

            x0 += zoom; // update worldspace x for next loop
        }
        row += pitch; // update worldspace y for next loop
    }
    SDL_UnlockTexture(screen);

    // transfer buffer in RAM to VRAM, then draw VRAM
    SDL_RenderTexture(_prenderer, screen, NULL, NULL);
    SDL_RenderPresent(_prenderer);
}

int main(int argc, char* argv[]) {
    printf("Mandelbrot Viewer in C by Alex Lothian\n Click + Drag to Navigate. Scroll to Zoom.\n < : Half Maximum Iterations\n > : Double Maximum Iterations\n\n\n");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* pwindow = SDL_CreateWindow("Mandelbrot Set", SCRN_WIDTH, SCRN_HEIGHT, 0);
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, NULL);
    SDL_Texture* scrnTexture = SDL_CreateTexture(prenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRN_WIDTH, SCRN_HEIGHT);

    if (!prenderer) {
        fprintf(stderr, "Renderer Failed to init");
        SDL_DestroyWindow(pwindow);
        SDL_DestroyRenderer(prenderer);
        return 1;
    }

    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);

    drawMandelbrot(prenderer, scrnTexture, vp);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                // use < and > to change max_iterations
                switch (event.key.key)
                {
                case SDLK_PERIOD:
                    vp->iterations *= 2;
                    break;

                case SDLK_COMMA:
                    if (vp->iterations > 2) vp->iterations /= 2;
                    break;
                    
                case SDLK_ESCAPE:
                    running = false;
                    break;

                default:
                    break;
                }
                drawMandelbrot(prenderer, scrnTexture, vp);
            }

            if (handle_mouse_events(&event, vp)) {
                drawMandelbrot(prenderer, scrnTexture, vp);
            }
        }
    }

    SDL_DestroyWindow(pwindow);
    SDL_DestroyRenderer(prenderer);

    return 0;
}