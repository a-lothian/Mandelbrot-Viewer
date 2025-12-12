#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#define SCRN_HEIGHT 600
#define SCRN_WIDTH 800

bool running = true;

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* pwindow = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCRN_WIDTH, SCRN_HEIGHT, 0);
    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, -1, SDL_RENDERER_ACCELERATED);

    if (!prenderer) {
        fprintf(stderr, "Renderer Failed to init");
        SDL_DestroyWindow(pwindow);
        SDL_DestroyRenderer(prenderer);
        return 1;
    }

    // draw onto screen
    for (int y = 0; y < SCRN_HEIGHT; y++) {
        for (int x = 0; x < SCRN_WIDTH; x++) {
            SDL_SetRenderDrawColor(prenderer, x * 255 / SCRN_WIDTH, y * 255 / SCRN_HEIGHT, 0, 255);
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