#ifndef COLOUR_PALETTE
#define COLOUR_PALETTE

#include <SDL3/SDL_stdinc.h>  // for Uint32

extern const Uint32* list_palettes[12];

Uint32* generateColourPalette(const Uint32* colours, int num_colours, int steps);
Uint32* cyclePalettes(int* index);

#endif