#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

typedef struct image_data {
    char source [256];
    SDL_Texture *texture;
    int width, height;
} image_data_t;

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path);

SDL_Rect *split_image(SDL_Texture *texture, int column, int row);

int snap_to_grid(int value, int increment, int offset);

float mathf_min(float a, float b);

float mathf_max(float a, float b);

void mathf_clamp(int *value, int min, int max);

bool mathf_range(float value, float min, float max);

#endif