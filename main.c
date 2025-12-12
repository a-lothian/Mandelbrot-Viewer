#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "mandelbrot.h"

#include <SDL2/SDL.h>

#define SCRN_HEIGHT 600
#define SCRN_WIDTH 800

int half_SCRN_HEIGHT = SCRN_HEIGHT / 2;
int half_SCRN_WIDTH = SCRN_WIDTH / 2;

bool running = true;

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* pwindow = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCRN_WIDTH, SCRN_HEIGHT, 0);
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, -1, SDL_RENDERER_ACCELERATED);

    if (!prenderer) {
        fprintf(stderr, "Renderer Failed to init");
        SDL_DestroyWindow(pwindow);
        SDL_DestroyRenderer(prenderer);
        return 1;
    }

    int iterations;
    float zoom = 0.005;

    // draw onto screen
    for (int y = 0; y < SCRN_HEIGHT; y++) {
        for (int x = 0; x < SCRN_WIDTH; x++) {
            iterations = calculateMandelbrot(((float)x - half_SCRN_WIDTH) * zoom, ((float)y - half_SCRN_HEIGHT) * zoom, 128);
            iterations *= 16;
            iterations -= 1;  // scale to 255
            SDL_SetRenderDrawColor(prenderer, iterations, iterations, iterations, 255);
            SDL_RenderDrawPoint(prenderer, x, y);
        }
    }

    SDL_RenderPresent(prenderer);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN) {
                SDL_DestroyWindow(pwindow);
                SDL_DestroyRenderer(prenderer);
                running = false;
            }
        }
    }
}