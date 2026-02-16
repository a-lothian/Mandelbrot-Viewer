#ifndef COLOUR_PALETTE
#define COLOUR_PALETTE

#include <SDL3/SDL_stdinc.h>  // for Uint32

#define PALETTE_SIZE 2048

extern const Uint32* list_palettes[12];

void generateColourPalette(const Uint32* colours, int num_colours, Uint32* out, int steps);
const Uint32* cyclePalettes(int* index);

#endif