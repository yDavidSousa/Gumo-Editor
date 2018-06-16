#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <utils.h>
#include <tilemap.h>

//TILESET FUNCTIONS

void set_tileset(tilemap_t *tilemap, const int tileset){
    if(tileset >= tilemap->num_tilesets){
        printf("There's no such tileset!\n");
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
    strcpy(tilemap->tileset[index].name, name);
    strcpy(tilemap->tileset[index].image_source, image_source);
    tilemap->tileset[index].tile_width = tile_width;
    tilemap->tileset[index].tile_height = tile_height;
    tilemap->tileset[index].cur_tile = 0;
    tilemap->cur_tileset = index;
    tilemap->num_tilesets++;
}

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset){
    SDL_Rect image = {};

    tileset->image_texture = load_texture(renderer, tileset->image_source);
    SDL_QueryTexture(tileset->image_texture, NULL, NULL, &image.w, &image.h);

    const int column = image.w / tileset->tile_width;
    const int row = image.h / tileset->tile_height;

    SDL_Rect *tiles = split_image(tileset->image_texture, column, row);

    tileset->num_tiles = column * row;
    tileset->tileinfo = malloc(tileset->num_tiles * sizeof(tileinfo_t));

    for (int i = 0; i < tileset->num_tiles; ++i) {
        tileset->tileinfo[i].id = i;
        tileset->tileinfo[i].x = tiles[i].x;
        tileset->tileinfo[i].y = tiles[i].y;
    }

    free(tiles);
}

//TODO(David): Show warning if use any tile in this tileset and don't render;
//TODO(David): Think some solution for the rest of the tiles
void remove_tileset(tileset_t *tileset){
    tileset->num_tiles = 0;
    tileset->cur_tile = 0;
    tileset->tile_width = 0;
    tileset->tile_height = 0;
    free(tileset->tileinfo);
    SDL_DestroyTexture(tileset->image_texture);
}

//LAYER FUNCTIONS

void add_layer(tilemap_t *tilemap, const char *name, const layer_type_t type){

    if(tilemap->num_layers == MAX_LAYERS){
        printf("Maximum allowed number of layers exceeded!\n");
        return;
    }

    const int index = tilemap->num_layers;

    tilemap->layerinfo[index].index = index;
    strcpy(tilemap->layerinfo[index].name, name);
    tilemap->layerinfo[index].type = type;
    tilemap->layerinfo[index].locked = 0;
    tilemap->layerinfo[index].hidden = 0;

    for (int r = 0; r < tilemap->num_rows; ++r)
        for (int c = 0; c < tilemap->num_columns; ++c){
            tilemap->tile_data[index][r][c] = EMPTY_TILE;
            tilemap->tileset_data[index][r][c] = EMPTY_TILE;
        }

    tilemap->num_layers++;
}

void remove_layer(tilemap_t *tilemap, const int index){

    for (int i = index; i < tilemap->num_layers - 1; ++i) {
        tilemap->layerinfo[i] = tilemap->layerinfo[i+1];

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
    tilemap->layerinfo[layer].hidden = tilemap->layerinfo[layer].hidden == 1 ? 0 : 1;
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
    tilemap->cur_layer = 0;
    tilemap->cur_tileset = 0;

    return tilemap;
}

//TODO(David): Better this function
void render_tilemap(SDL_Renderer *renderer, const tilemap_t *tilemap){
    int tile_buffer, tileset_buffer;
    int width_buffer, height_buffer;

    for (int l = 0; l < tilemap->num_layers; ++l) {
        if(tilemap->layerinfo[l].hidden) continue;

        for (int r = 0; r < tilemap->num_rows; ++r) {
            for (int c = 0; c < tilemap->num_columns; ++c) {

                tileset_buffer = tilemap->tileset_data[l][r][c];
                tile_buffer = tilemap->tile_data[l][r][c];

                if (tileset_buffer == EMPTY_TILE || tile_buffer == EMPTY_TILE)
                    continue;

                width_buffer = tilemap->tileset[tileset_buffer].tile_width;
                height_buffer = tilemap->tileset[tileset_buffer].tile_height;

                SDL_Rect des_buffer = (SDL_Rect) {
                        c * width_buffer,
                        r * height_buffer,
                        width_buffer,
                        height_buffer
                };

                SDL_Rect src_buffer = (SDL_Rect) {
                        tilemap->tileset[tileset_buffer].tileinfo[tile_buffer].x,
                        tilemap->tileset[tileset_buffer].tileinfo[tile_buffer].y,
                        width_buffer,
                        height_buffer
                };

                SDL_RenderCopy(
                        renderer,
                        tilemap->tileset[tileset_buffer].image_texture,
                        &src_buffer,
                        &des_buffer
                );
            }
        }
    }
}

void remove_tilemap(tilemap_t *tilemap){
    for (int i = 0; i < tilemap->num_tilesets; ++i)
        remove_tileset(&tilemap->tileset[i]);

    free(tilemap);
}

//TILE FUNCTIONS

void put_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tileset!\n");
        return;
    }

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    const int cur_tileset = tilemap->cur_tileset;
    const int cur_tile = tilemap->tileset[cur_tileset].cur_tile;
    const int l = tilemap->cur_layer;

    if(tilemap->layerinfo[l].locked)
        return;

    const int c = (x / tilemap->tileset[cur_tileset].tile_width);
    const int r = (y / tilemap->tileset[cur_tileset].tile_height);

    if(tilemap->tileset_data[l][r][c] == cur_tileset && tilemap->tile_data[l][r][c] == cur_tile)
        return;

    tilemap->tileset_data[l][r][c] = cur_tileset;
    tilemap->tile_data[l][r][c] = cur_tile;
}

void remove_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tileset!\n");
        return;
    }

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    const int cur_tileset = tilemap->cur_tileset;
    const int l = tilemap->cur_layer;

    if(tilemap->layerinfo[l].locked)
        return;

    const int c = (x / tilemap->tileset[cur_tileset].tile_width);
    const int r = (y / tilemap->tileset[cur_tileset].tile_height);

    if(tilemap->tileset_data[l][r][c] == EMPTY_TILE || tilemap->tile_data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->tileset_data[l][r][c] = EMPTY_TILE;
    tilemap->tile_data[l][r][c] = EMPTY_TILE;
}

void filter_tile(tilemap_t *tilemap, const int x, const int y){

    if(tilemap->num_tilesets == 0){
        printf("Does not contain any tileset!\n");
        return;
    }

    if(tilemap->num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    const int cur_tileset = tilemap->cur_tileset;
    const int l = tilemap->cur_layer;
    const int c = (x / tilemap->tileset[cur_tileset].tile_width);
    const int r = (y / tilemap->tileset[cur_tileset].tile_height);

    if(tilemap->tileset_data[l][r][c] == EMPTY_TILE || tilemap->tile_data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->cur_tileset = tilemap->tileset_data[l][r][c];
    tilemap->tileset[tilemap->cur_tileset].cur_tile = tilemap->tile_data[l][r][c];
}



