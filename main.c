#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

bool running = true;

int main() {
    SDL_Init(0);
    SDL_Window* pwindow = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_RESIZABLE);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }
}