#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <SDL_image.h>

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path);

SDL_Rect *split_image(SDL_Texture *texture, int column, int row);

int snap_to_grid(int value, int increment, int offset);

#endif