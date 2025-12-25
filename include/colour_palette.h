#ifndef COLOUR_PALETTE
#define COLOUR_PALETTE

#include <SDL3/SDL_stdinc.h>  // for Uint32

Uint32* generateColourPalette(Uint32* colours, int num_colours, int steps);
const Uint32 palette_inferno[8];
const Uint32 palette_psych[8];

#endif