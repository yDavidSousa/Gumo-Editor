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
                    if (tilemap->tile_data[l][r][c] == EMPTY_TILE)
                        continue;

                    const SDL_Rect des_buffer = (SDL_Rect) {
                            tilemap->offset_x + c * (tilemap->tile_width + tilemap->zoom),
                            tilemap->offset_y + r * (tilemap->tile_height + tilemap->zoom),
                            (tilemap->tile_width + tilemap->zoom),
                            (tilemap->tile_height + tilemap->zoom)
                    };

                    render_tile(renderer, tilemap->tile_data[l][r][c], &des_buffer);
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
    destroy_tilesets();
    free(tilemap);
}

//TILE FUNCTIONS

void put_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    //TODO(David): Check here if there's some tileset in current tilemap

    if(tilemap->layers[tilemap->cur_layer].locked)
        return;

    int position_x = (x - tilemap->offset_x);
    int position_y = (y - tilemap->offset_y);

    if(position_x < 0 || position_y < 0)
        return;

    const int l = tilemap->cur_layer;
    int c = position_x / (tilemap->tile_width + tilemap->zoom);
    int r = position_y / (tilemap->tile_height + tilemap->zoom);

    get_selected_tile(tilemap->cur_tileset, &tilemap->tile_data[l][r][c]);
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

    int position_x = (x - tilemap->offset_x);
    int position_y = (y - tilemap->offset_y);

    if(position_x < 0 || position_y < 0) return;

    const int l = tilemap->cur_layer;
    int c = position_x / (tilemap->tile_width + tilemap->zoom);
    int r = position_y / (tilemap->tile_height + tilemap->zoom);

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

    int position_x = (x - tilemap->offset_x);
    int position_y = (y - tilemap->offset_y);

    if(position_x < 0 || position_y < 0) return;

    const int l = tilemap->cur_layer;
    int c = position_x / (tilemap->tile_width + tilemap->zoom);
    int r = position_y / (tilemap->tile_height + tilemap->zoom);

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

