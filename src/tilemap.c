#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <utils.h>
#include <tilemap.h>

//TILESET FUNCTIONS

void set_tileset(tilemap_t *tilemap, const int tileset){
    if(tileset >= tilemap->num_tilesets){
        printf("There's no such tilesets!\n");
        return;
    }

    tilemap->cur_tileset = tileset;
}

void add_tileset(tilemap_t *tilemap, const char *name, const char *image_source, const int tile_width, const int tile_height){

    if(tilemap->num_tilesets == MAX_TILESETS){
        printf("Maximum allowed number of tilesets exceeded!\n");
        return;
    }

    const int index = tilemap->num_tilesets;
    strcpy(tilemap->tilesets[index].name, name);
    strcpy(tilemap->tilesets[index].image.source, image_source);
    tilemap->tilesets[index].tile_width = tile_width;
    tilemap->tilesets[index].tile_height = tile_height;
    tilemap->tilesets[index].cur_tile = 0;
    tilemap->cur_tileset = index;
    tilemap->num_tilesets++;
}

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset){
    tileset->image.texture = load_texture(renderer, tileset->image.source);
    SDL_QueryTexture(tileset->image.texture, NULL, NULL, &tileset->image.width, &tileset->image.height);

    const int column = tileset->image.width / tileset->tile_width;
    const int row = tileset->image.height / tileset->tile_height;

    tileset->num_tiles = column * row;
    tileset->tile = malloc(tileset->num_tiles * sizeof(tile_data_t));

    SDL_Rect *tiles = split_image(tileset->image.texture, column, row);

    for (int i = 0; i < tileset->num_tiles; ++i) {
        tileset->tile[i].id = i;
        tileset->tile[i].x = tiles[i].x;
        tileset->tile[i].y = tiles[i].y;
    }

    free(tiles);
}

//TODO(David): Show warning if use any tile in this tilesets and don't render;
//TODO(David): Think some solution for the rest of the tiles
void remove_tileset(tileset_t *tileset){
    tileset->num_tiles = 0;
    tileset->cur_tile = 0;
    tileset->tile_width = 0;
    tileset->tile_height = 0;
    free(tileset->tile);
    SDL_DestroyTexture(tileset->image.texture);
}

//LAYER FUNCTIONS

void add_layer(tilemap_t *tilemap, const char *name, const layer_type_t type){

    if(tilemap->num_layers == MAX_LAYERS){
        printf("Maximum allowed number of layers exceeded!\n");
        return;
    }

    const int index = tilemap->num_layers;

    tilemap->layers[index].index = index;
    strcpy(tilemap->layers[index].name, name);
    tilemap->layers[index].type = type;
    tilemap->layers[index].locked = 0;
    tilemap->layers[index].hidden = 0;

    for (int r = 0; r < tilemap->num_rows; ++r)
        for (int c = 0; c < tilemap->num_columns; ++c){
            tilemap->tile_data[index][r][c] = EMPTY_TILE;
            tilemap->tileset_data[index][r][c] = EMPTY_TILE;
            tilemap->entity_data[index][r][c] = EMPTY_TILE;
        }

    tilemap->num_layers++;
}

void remove_layer(tilemap_t *tilemap, const int index){

    if(tilemap->num_layers == 0){
        printf("Does not contain any entity!\n");
        return;
    }

    for (int i = index; i < tilemap->num_layers - 1; ++i) {
        tilemap->layers[i] = tilemap->layers[i+1];

        for (int r = 0; r < tilemap->num_rows; ++r)
            for (int c = 0; c < tilemap->num_columns; ++c){
                tilemap->tileset_data[i][r][c] = tilemap->tileset_data[i+1][r][c];
                tilemap->tile_data[i][r][c] = tilemap->tile_data[i+1][r][c];
            }
    }

    tilemap->num_layers--;
}

void set_layer(tilemap_t *tilemap, const int layer){
    if(layer >= tilemap->num_layers){
        printf("There's no such layer!\n");
        return;
    }

    tilemap->cur_layer = layer;
}

void change_layer_visibility(tilemap_t *tilemap, const int layer){
    tilemap->layers[layer].hidden = tilemap->layers[layer].hidden == 1 ? 0 : 1;
}

//TILEMAP FUNCTIONS

tilemap_t *create_tilemap(const char *name, const int width, const int height, const int tile_width, const int tile_height){
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));

    strcpy(tilemap->name, name);
    tilemap->map_width = width;
    tilemap->map_height = height;
    tilemap->tile_width = tile_width;
    tilemap->tile_height = tile_height;
    tilemap->num_columns = width / tile_width;
    tilemap->num_rows = height / tile_height;
    tilemap->num_layers = 0;
    tilemap->num_tilesets = 0;
    tilemap->num_entities = 0;
    tilemap->cur_layer = 0;
    tilemap->cur_tileset = 0;
    tilemap->cur_entity = 0;

    return tilemap;
}

//TODO(David): Better this function
void render_tilemap(SDL_Renderer *renderer, const tilemap_t *tilemap){
    int tileset_index, tile_index;

    for (int l = 0; l < tilemap->num_layers; ++l) {
        if(tilemap->layers[l].hidden)
            continue;

        for (int r = 0; r < tilemap->num_rows; ++r) {
            for (int c = 0; c < tilemap->num_columns; ++c) {

                if(tilemap->layers[l].type == ENTITIES){

                    int entity_index = tilemap->entity_data[l][r][c];

                    if (entity_index == EMPTY_TILE)
                        continue;

                    SDL_Rect des_buffer = (SDL_Rect) {
                            c * tilemap->tile_width,
                            r * tilemap->tile_height,
                            tilemap->tile_width,
                            tilemap->tile_height
                    };

                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
                    SDL_RenderFillRect(renderer, &des_buffer);
                    continue;
                }

                if(tilemap->layers[l].type == TILES){
                    tileset_index = tilemap->tileset_data[l][r][c];
                    tile_index = tilemap->tile_data[l][r][c];

                    if (tileset_index == EMPTY_TILE || tile_index == EMPTY_TILE)
                        continue;

                    const tileset_t tileset_buffer = tilemap->tilesets[tileset_index];

                    SDL_Rect des_buffer = (SDL_Rect) {
                            c * tilemap->tile_width,
                            r * tilemap->tile_height,
                            tilemap->tile_width,
                            tilemap->tile_height
                    };

                    SDL_Rect src_buffer = (SDL_Rect) {
                            tileset_buffer.tile[tile_index].x,
                            tileset_buffer.tile[tile_index].y,
                            tileset_buffer.tile_width,
                            tileset_buffer.tile_height
                    };

                    SDL_RenderCopy(renderer, tileset_buffer.image.texture, &src_buffer, &des_buffer);
                }
            }
        }
    }
}

void clear_tilemap(tilemap_t *tilemap){
    for (int l = 0; l < tilemap->num_layers; ++l) {
        for (int r = 0; r < tilemap->num_rows * tilemap->num_columns; ++r) {
            for (int c = 0; c < tilemap->num_columns; ++c) {
                tilemap->tile_data[l][r][c] = EMPTY_TILE;
                tilemap->tileset_data[l][r][c] = EMPTY_TILE;
            }
        }
    }
}

void remove_tilemap(tilemap_t *tilemap){
    for (int i = 0; i < tilemap->num_tilesets; ++i)
        remove_tileset(&tilemap->tilesets[i]);

    free(tilemap);
}

//TILE FUNCTIONS

void put_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tilesets!\n");
        return;
    }

    if(tilemap->layers[tilemap->cur_layer].locked)
        return;

    const int cur_tileset = tilemap->cur_tileset;
    const int cur_tile = tilemap->tilesets[cur_tileset].cur_tile;

    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tile_width);
    const int r = (y / tilemap->tile_height);

    tilemap->tileset_data[l][r][c] = cur_tileset;
    tilemap->tile_data[l][r][c] = cur_tile;
}

void remove_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tilesets!\n");
        return;
    }

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    if(tilemap->layers[tilemap->cur_layer].locked)
        return;

    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tile_width);
    const int r = (y / tilemap->tile_height);

    tilemap->tileset_data[l][r][c] = EMPTY_TILE;
    tilemap->tile_data[l][r][c] = EMPTY_TILE;
}

void filter_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tilesets!\n");
        return;
    }

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tile_width);
    const int r = (y / tilemap->tile_height);

    tilemap->cur_tileset = tilemap->tileset_data[l][r][c];
    tilemap->tilesets[tilemap->cur_tileset].cur_tile = tilemap->tile_data[l][r][c];
}

//ENTITY FUNCTIONS

void put_entity(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_entities == 0){
        printf("Does not contain any entity!\n");
        return;
    }

    const int cur_entity = tilemap->cur_entity;

    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tile_width);
    const int r = (y / tilemap->tile_height);

    tilemap->entities[cur_entity].x = c * tilemap->tile_width;
    tilemap->entities[cur_entity].y = r * tilemap->tile_height;
    tilemap->entities[cur_entity].w = tilemap->tile_width;
    tilemap->entities[cur_entity].h = tilemap->tile_height;

    tilemap->entity_data[l][r][c] = tilemap->entities[cur_entity].id;
}

void remove_entity(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_entities == 0){
        printf("Does not contain any tilesets!\n");
        return;
    }

    const int cur_entity = tilemap->cur_entity;

    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tile_width);
    const int r = (y / tilemap->tile_height);

    tilemap->entities[cur_entity].x = 0;
    tilemap->entities[cur_entity].y = 0;

    tilemap->entity_data[l][r][c] = EMPTY_TILE;
}

void add_entity(tilemap_t *tilemap, const char *name){

    if(tilemap->num_entities == MAX_ENTITIES){
        printf("Maximum allowed number of entities exceeded!\n");
        return;
    }

    const int index = tilemap->num_entities;

    tilemap->entities[index].id = index;
    strcpy(tilemap->entities[index].name, name);

    tilemap->num_entities++;
}

void delete_entity(tilemap_t *tilemap, const int id){

    if(tilemap->num_entities == 0){
        printf("Does not contain any entity!\n");
        return;
    }

    for (int i = id; i < tilemap->num_entities - 1; ++i) {
        tilemap->entities[i] = tilemap->entities[i+1];

        for (int r = 0; r < tilemap->num_rows; ++r)
            for (int c = 0; c < tilemap->num_columns; ++c)
                tilemap->entity_data[i][r][c] = tilemap->entity_data[i+1][r][c];
    }

    tilemap->num_entities--;
}

