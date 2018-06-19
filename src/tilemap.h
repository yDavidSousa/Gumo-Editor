#ifndef TILEMAP_H
#define TILEMAP_H

#include <layer.h>
#include <tileset.h>
#include <entity.h>

#define MAX_COLUMNS 200
#define MAX_ROWS 200
#define MAX_LAYERS 4
#define MAX_TILESETS 4
#define MAX_ENTITIES 20

typedef struct tilemap {
    char name[256];
    int tile_data[MAX_LAYERS][MAX_ROWS][MAX_COLUMNS];
    int tileset_data[MAX_LAYERS][MAX_ROWS][MAX_COLUMNS];
    int entity_data[MAX_LAYERS][MAX_ROWS][MAX_COLUMNS];
    layer_data_t layers[MAX_LAYERS];
    tileset_t tilesets[MAX_TILESETS];
    entity_data_t entities[MAX_ENTITIES];
    int num_columns, num_rows, num_layers, num_tilesets, num_entities;
    int map_width, map_height;
    int tile_width, tile_height;
    int cur_layer, cur_tileset, cur_entity;
    int zoom, offset_x, offset_y;
} tilemap_t;

//TILESET FUNCTIONS

void set_tileset(tilemap_t *tilemap, int tileset);

void add_tileset(tilemap_t *tilemap, const char *name, const char *image_source, int tile_width, int tile_height);

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset);

void remove_tileset(tileset_t *tileset);

//LAYER FUNCTIONS

void add_layer(tilemap_t *tilemap, const char *name, layer_type_t type);

void remove_layer(tilemap_t *tilemap, int index);

void set_layer(tilemap_t *tilemap, int layer);

void change_layer_visibility(tilemap_t *tilemap, int layer);

//TILEMAP FUNCTIONS

tilemap_t *create_tilemap(const char *name, int width, int height, int tile_width, int tile_height);

//TODO(David): Better this function
void render_tilemap(SDL_Renderer *renderer, const tilemap_t *tilemap);

void clear_tilemap(tilemap_t *tilemap);

void remove_tilemap(tilemap_t *tilemap);

//TILE FUNCTIONS

void put_tile(tilemap_t *tilemap, int x, int y);

void remove_tile(tilemap_t *tilemap, int x, int y);

void filter_tile(tilemap_t *tilemap, int x, int y);

//ENTITY FUNCTION

void put_entity(tilemap_t *tilemap, int x, int y);

void remove_entity(tilemap_t *tilemap, int x, int y);

void add_entity(tilemap_t *tilemap, const char *name);

void delete_entity(tilemap_t *tilemap, int id);

#endif