#ifndef TILESET_H
#define TILESET_H

#include <SDL_image.h>

#define DEFAULT_FILE_NAME_LEN 256
#define EMPTY_TILE (-1)
#define MAX_TILESETS 4

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
    int id;
    char name[DEFAULT_FILE_NAME_LEN];
    image_data_t image;
    tile_data_t *tile;
    int tile_width, tile_height;
    int num_tiles, cur_tile;
} tileset_t;

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset);

void add_tileset(SDL_Renderer *renderer, const char *name, const char *image_source, int tile_width, int tile_height);

void remove_tileset(tileset_t *tileset);

void destroy_tilesets();

void render_tile(SDL_Renderer *renderer, int id, const SDL_Rect *dst_rect);

void get_selected_tile(int tileset_index, int *value);

#endif