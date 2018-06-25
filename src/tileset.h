#ifndef TILESET_H
#define TILESET_H

#include <SDL_image.h>
#include <utils.h>

#define MAX_TILESETS 8
#define EMPTY_TILE (-1)

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
    int selected_tile;
} tileset_t;

int num_tilesets;

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset);

void add_tileset(SDL_Renderer *renderer, const char *name, const char *image_source, int tile_width, int tile_height);

int select_tile(int id);

tileset_t *get_tileset(int index);

void get_selected_tile(int tileset_index, short *value);

tileset_t *get_tilesets();

void render_tile(SDL_Renderer *renderer, short id, const SDL_Rect *dst_rect);

void render_tileset(SDL_Renderer *renderer, int id, const SDL_Rect *dst_rect);

void remove_tileset(int index);

void destroy_tilesets();

#endif