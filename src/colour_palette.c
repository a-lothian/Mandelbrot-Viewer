#include <SDL3/SDL_stdinc.h>  // for Uint32

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

void generateColourPalette(const Uint32* colours, int num_colours, Uint32* out, int steps) {
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

        out[i] = lerp_color(colours[idx1], colours[idx2], t_local);
    }
}

const Uint32 palette_inferno[8] = {0xFF000000, 0xFF07002B, 0xFF2A005E, 0xFF7C0000, 0xFFFF0000, 0xFFFF8000, 0xFFFFFF00, 0xFFFFFFFF};
const Uint32 palette_psych[8] = {0xFF000000, 0xFF8900FF, 0xFF0022FF, 0xFF00CCFF, 0xFF00FF00, 0xFFFFFF00, 0xFFFF0000, 0xFFFFFFFF};
const Uint32 palette_rainbow[8] = {0xFFFF0000, 0xFFFF8000, 0xFFFFFF00, 0xFF00FF00, 0xFF00FFFF, 0xFF0000FF, 0xFF8000FF, 0xFFFF0080};
const Uint32 palette_vaporwave[8] = {0xFF200050, 0xFF600090, 0xFFC04080, 0xFFFF60B0, 0xFF00E0FF, 0xFF80FFC0, 0xFFFFFF80, 0xFF5000A0};
const Uint32 palette_stripey[8] = {0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF};
const Uint32 palette_fire_ice[8] = {0xFF000040, 0xFF0000FF, 0xFF0080FF, 0xFF80FFFF, 0xFF400000, 0xFFFF0000, 0xFFFF8000, 0xFFFFFF00};
const Uint32 palette_cga_high[8] = {0xFF000000, 0xFFFF5555, 0xFF55FFFF, 0xFFFFFFFF, 0xFF0000AA, 0xFFAA00AA, 0xFF00AAAA, 0xFFAAAAAA};
const Uint32 palette_alien_goo[8] = {0xFF100000, 0xFF400020, 0xFF600000, 0xFF804000, 0xFF006000, 0xFF00FF00, 0xFF80FF00, 0xFFCCFFCC};
const Uint32 palette_midnight_gold[8] = {0xFF000000, 0xFF101030, 0xFF000060, 0xFF402010, 0xFF806020, 0xFFD0A040, 0xFFFFF0A0, 0xFF202040};
const Uint32 palette_neon_chaos[8] = {0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF00FFFF, 0xFF000000, 0xFF00FF00, 0xFF000000, 0xFFFFFF00};
const Uint32 palette_cmyk[8] = {0xFF000000, 0xFF00FFFF, 0xFF0080FF, 0xFFFF00FF, 0xFFFF0080, 0xFFFFFF00, 0xFF808080, 0xFFFFFFFF};
const Uint32 palette_halloween[8] = {0xFF000000, 0xFF220044, 0xFF440088, 0xFF6600CC, 0xFF8800FF, 0xFFFF6600, 0xFFFF9900, 0xFFFFFFFF};

const Uint32* const list_palettes[12] = {
    palette_inferno,
    palette_psych,
    palette_rainbow,
    palette_vaporwave,
    palette_stripey,
    palette_fire_ice,
    palette_cga_high,
    palette_alien_goo,
    palette_midnight_gold,
    palette_neon_chaos,
    palette_cmyk,
    palette_halloween};

const Uint32* cyclePalettes(int* index) {
    *index = (*index + 1) % 12;
    return list_palettes[*index];
}