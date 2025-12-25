#include <SDL3/SDL_stdinc.h>  // for Uint32
#include <stdlib.h>

// linear interpolation inspired by https://stackoverflow.com/questions/21835739/smooth-color-transition-algorithm
Uint32 lerp_color(Uint32 colour1, Uint32 colour2, float p) {
    // Clamp p
    if (p < 0.0f)
        p = 0.0f;
    if (p > 1.0f)
        p = 1.0f;

    // extract channels (assuming 0xFFRRGGBB)
    int r1 = (colour1 >> 16) & 0xFF;
    int g1 = (colour1 >> 8) & 0xFF;
    int b1 = colour1 & 0xFF;

    int r2 = (colour2 >> 16) & 0xFF;
    int g2 = (colour2 >> 8) & 0xFF;
    int b2 = colour2 & 0xFF;

    // calculate interpolated values
    // 0.5f is for rounding to nearest int
    int r = (int)((1.0f - p) * r1 + p * r2 + 0.5f);
    int g = (int)((1.0f - p) * g1 + p * g2 + 0.5f);
    int b = (int)((1.0f - p) * b1 + p * b2 + 0.5f);

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

Uint32* generateColourPalette(Uint32* colours, int num_colours, int steps) {
    Uint32* palette = (Uint32*)malloc(sizeof(Uint32) * steps);

    for (int i = 0; i < steps; i++) {
        float p = (float)i / (float)(steps - 1);

        // percentage to number of transitions
        // 4 colors = 3 transitions A->B, B->C, C->D
        float position = p * (num_colours - 1);

        // left / right colour index
        int idx1 = (int)position;
        int idx2 = idx1 + 1;

        // Clamp right index to avoid segfault
        if (idx2 >= num_colours) {
            idx2 = num_colours - 1;
        }

        // find distance between colour1 & colour2
        float t_local = position - idx1;

        palette[i] = lerp_color(colours[idx1], colours[idx2], t_local);
    }

    return palette;
}

const Uint32 palette_inferno[8] = {0xFF000000, 0xFF07002B, 0xFF2A005E, 0xFF7C0000, 0xFFFF0000, 0xFFFF8000, 0xFFFFFF00, 0xFFFFFFFF};

const Uint32 palette_psych[8] = {0xFF2E003E, 0xFF8900FF, 0xFF0022FF, 0xFF00CCFF, 0xFF00FF00, 0xFFFFFF00, 0xFFFF0000, 0xFFFFFFFF};