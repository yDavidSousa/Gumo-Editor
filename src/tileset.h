#ifndef TILESET_H
#define TILESET_H

#include <SDL_image.h>

#define DEFAULT_FILE_NAME_LEN 256
#define EMPTY_TILE (-1)

typedef struct image_data {
    char source [DEFAULT_FILE_NAME_LEN];
    SDL_Texture *texture;
    int width, height;
} image_data_t;

typedef struct tile_data {
    int id;
    int x, y;
} tile_data_t;

typedef struct tileset {
    char name[DEFAULT_FILE_NAME_LEN];
    image_data_t image;
    tile_data_t *tile;
    int tile_width, tile_height;
    int num_tiles, cur_tile;
} tileset_t;

#endif