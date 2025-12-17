#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "mandelbrot.h"
#include "inputHandler.h"

#define SDL_MAIN_HANDLED  // updates entry point on windows
#include <SDL2/SDL.h>

#define PRNT_VP 0

#define SCRN_HEIGHT 600
#define SCRN_WIDTH 800

const int halfHeight = SCRN_HEIGHT / 2;
const int halfWidth = SCRN_WIDTH / 2;

bool running = true;


int calculateMandelbrotPix(int x, int y, struct viewport* vp)
{
    double real_coord = ((double)(x - halfWidth) * vp->zoom) + vp->current_offset_x;
    double imag_coord = ((double)(y - halfHeight) * vp->zoom) + vp->current_offset_y;

    return calculateMandelbrot(real_coord, imag_coord, vp->iterations);
}


void drawMandelbrot(SDL_Renderer* _prenderer, SDL_Texture* screen, struct viewport* vp) {
    if (PRNT_VP) printf("zoom=%lf offx=%lf offy=%lf iters=%d\n",
        vp->zoom, vp->current_offset_x, vp->current_offset_y, vp->iterations);

    int iterations;
    void* pixels;
    int pitch;

    SDL_LockTexture(screen, NULL, &pixels, &pitch);
    Uint32* target_pixels = (Uint32*)pixels;

    // draw onto screen
    for (int y = 0; y < SCRN_HEIGHT; y++) {
        for (int x = 0; x < SCRN_WIDTH; x++) {
            iterations = calculateMandelbrotPix(x, y, vp);

            Uint8 brightness;

            if (iterations >= vp->iterations) {
                brightness = 0;
            } else {
                 // scale iterations to fit within 0-255 range relative to the max
                brightness = (Uint8)((iterations * 255) / vp->iterations);
            }

            // coloring (0xFFRRGGBB format for ARGB8888)
            Uint32 color = (0xFF << 24) | (brightness << 16) | (brightness << 8) | brightness;

            // write to buffer
            target_pixels[y * SCRN_WIDTH + x] = color;
        }
    }
    SDL_UnlockTexture(screen);

    // transfer buffer in RAM to VRAM, then draw VRAM
    SDL_RenderCopy(_prenderer, screen, NULL, NULL);
    SDL_RenderPresent(_prenderer);
}

int main() {
    printf("Mandelbrot Viewer in C by Alex Lothian\n Click + Drag to Navigate. Scroll to Zoom.\n < : Half Maximum Iterations\n > : Double Maximum Iterations\n\n\n");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* pwindow = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCRN_WIDTH, SCRN_HEIGHT, 0);
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, -1, SDL_RENDERER_ACCELERATED);
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
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                // use < and > to change max_iterations
                switch (event.key.keysym.sym)
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