#ifndef RENDER_CTX_H
#define RENDER_CTX_H

#include <SDL3/SDL.h>

struct RenderContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Uint32* buffer;
    int width;
    int height;
};

#endif
