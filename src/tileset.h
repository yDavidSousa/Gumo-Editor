#ifndef TILESET_H
#define TILESET_H

#include <SDL_image.h>

#define MAX_TILESETS 8
#define EMPTY_TILE (-1)

typedef struct image_data {
    char source [256];
    SDL_Texture *texture;
    int width, height;
} image_data_t;

typedef struct tile_info {
    int id;
    int x, y;
} tile_info_t;

typedef struct tileset {
    int id;
    char name[256];
    image_data_t image;
    tile_info_t *tile;
    int tile_width, tile_height;
    int num_tiles;
    short selected_tile;
} tileset_t;

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset);

void add_tileset(SDL_Renderer *renderer, const char *name, const char *image_source, int tile_width, int tile_height);

void remove_tileset(int index);

void render_tile(SDL_Renderer *renderer, short id, const SDL_Rect *dst_rect);

void destroy_tilesets();

int filter_tileset(int id);

void get_selected_tile(int tileset_index, short *value);

#endif