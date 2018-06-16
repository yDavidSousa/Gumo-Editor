#ifndef TILESET_H
#define TILESET_H

#include <SDL_image.h>

#define EMPTY_TILE (-1)

typedef struct tileinfo {
    int id;
    int x, y;
} tileinfo_t;

typedef struct tileset{
    char name[256];
    char image_source[256];
    SDL_Texture *image_texture;
    int tile_width, tile_height;
    int num_tiles, cur_tile;
    tileinfo_t *tileinfo;
} tileset_t;

#endif