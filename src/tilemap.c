#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <utils.h>
#include <tilemap.h>

void set_tileset(tilemap_t *tilemap, const int tileset){
    tilemap->cur_tileset = tileset;
}

void add_layer(tilemap_t *tilemap, const char *name, const layer_type_t type){
    if(tilemap->num_layers == MAX_LAYERS){
        printf("Maximum allowed number of layers exceeded!\n");
        return;
    }

    const int index = tilemap->num_layers;

    tilemap->layers[index].id = index;
    strcpy(tilemap->layers[index].name, name);
    tilemap->layers[index].type = type;
    tilemap->layers[index].flags = 0;

    clear_layer(tilemap, index);

    tilemap->num_layers += 1;
}

void remove_layer(tilemap_t *tilemap, const int index){
    if(tilemap->num_layers == 0){
        printf("Does not contain any entity!\n");
        return;
    }

    for (int i = index; i < tilemap->num_layers - 1; ++i) {
        tilemap->layers[i] = tilemap->layers[i+1];

        for (int r = 0; r < tilemap->tiles_per_height; ++r)
            for (int c = 0; c < tilemap->tiles_per_width; ++c){
                tilemap->data[i][r][c] = tilemap->data[i+1][r][c];
            }
    }

    tilemap->num_layers -= 1;
}

void set_layer(tilemap_t *tilemap, const int layer){
    if(layer >= tilemap->num_layers){
        printf("There's no such layer!\n");
        return;
    }

    tilemap->cur_layer = layer;
}

void clear_layer(tilemap_t *tilemap, const int layer){
    for (int row = 0; row < tilemap->tiles_per_height; ++row)
        for (int column = 0; column < tilemap->tiles_per_width; ++column){
            tilemap->data[layer][row][column] = EMPTY_TILE;
            tilemap->entity_data[layer][row][column] = EMPTY_TILE;
        }
}

tilemap_t *create_tilemap(const char *name, const int width, const int height, const int tile_width, const int tile_height){
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));

    strcpy(tilemap->name, name);
    tilemap->map_width = width;
    tilemap->map_height = height;
    tilemap->tile_width = tile_width;
    tilemap->tile_height = tile_height;
    tilemap->tiles_per_width = width / tile_width;
    tilemap->tiles_per_height = height / tile_height;
    tilemap->num_layers = 0;
    tilemap->cur_layer = 0;
    tilemap->cur_tileset = 0;
    tilemap->cur_entity = 0;

    return tilemap;
}

void render_tilemap(SDL_Renderer *renderer, const tilemap_t *tilemap){

    SDL_SetRenderDrawColor(renderer, 196, 196, 196, 255);
    SDL_Rect canvas = (SDL_Rect){
            tilemap->offset_x,
            tilemap->offset_y,
            tilemap->map_width + (tilemap->zoom * tilemap->tiles_per_width),
            tilemap->map_height + (tilemap->zoom * tilemap->tiles_per_height)
    };
    SDL_RenderFillRect(renderer, &canvas);
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderDrawLine(renderer, tilemap->offset_x + 32, tilemap->offset_y, tilemap->offset_x + 32, tilemap->offset_y + tilemap->map_height);
    for (int l = 0; l < tilemap->num_layers; ++l) {
        if(layer_has_flags(&tilemap->layers[l], LAYER_HIDDEN))
            continue;
        for (int r = 0; r < tilemap->tiles_per_height; ++r) {
            for (int c = 0; c < tilemap->tiles_per_width; ++c) {

                if (tilemap->data[l][r][c] == EMPTY_TILE)
                    continue;

                SDL_Rect dst_rect = {};
                switch(tilemap->layers[l].type){
                    case TILES:
                        dst_rect = (SDL_Rect) {
                                tilemap->offset_x + c * (tilemap->tile_width + tilemap->zoom),
                                tilemap->offset_y + r * (tilemap->tile_height + tilemap->zoom),
                                (tilemap->tile_width + tilemap->zoom),
                                (tilemap->tile_height + tilemap->zoom)
                        };

                        render_tile(renderer, tilemap->data[l][r][c], &dst_rect);
                        break;
                    case ENTITIES:
                        dst_rect = (SDL_Rect) {
                                tilemap->offset_x + c * (tilemap->tile_width + tilemap->zoom),
                                tilemap->offset_y + r * (tilemap->tile_height + tilemap->zoom),
                                (tilemap->tile_width + tilemap->zoom),
                                (tilemap->tile_height + tilemap->zoom)
                        };

                        render_entity(renderer, tilemap->data[l][r][c], &dst_rect);
                        break;
                }
            }
        }
    }
}

void remove_tilemap(tilemap_t *tilemap){
    destroy_tilesets();
    free(tilemap);
}

void put_tile(tilemap_t *tilemap, const int layer,const int row, const int column){
    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_LOCKED)){
        printf("Current layer locked!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_HIDDEN)){
        printf("Current layer hidden!\n");
        return;
    }

    if(tilemap->layers[layer].type == TILES)
        get_selected_tile(tilemap->cur_tileset, &tilemap->data[layer][row][column]);
    else if(tilemap->layers[layer].type == ENTITIES)
        tilemap->data[layer][row][column] = (short)tilemap->cur_entity;
}

void remove_tile(tilemap_t *tilemap, const int layer, const int row, const int column){
    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_LOCKED)){
        printf("Current layer locked!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_HIDDEN)){
        printf("Current layer hidden!\n");
        return;
    }

    tilemap->data[layer][row][column] = EMPTY_TILE;
}

void filter_tile(tilemap_t *tilemap, const int layer, const int row, const int column){
    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_LOCKED)){
        printf("Current layer locked!\n");
        return;
    }

    if(layer_has_flags(&tilemap->layers[layer], LAYER_HIDDEN)){
        printf("Current layer hidden!\n");
        return;
    }

    if(tilemap->layers[layer].type == TILES)
        tilemap->cur_tileset = select_tile(tilemap->data[layer][row][column]);
    else if(tilemap->layers[layer].type == ENTITIES)
        tilemap->cur_entity = tilemap->data[layer][row][column];
}