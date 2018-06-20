#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <tileset.h>
#include <utils.h>

/*
 * 3234 -> GUMO_ID
 * 3    -> TILESET_ID [(GUMO_ID - TILE_ID) / 1000]
 *  234 -> TILE_ID [GUMO_ID % 1000]
 */

tileset_t tileset[MAX_TILESETS];
int tileset_count;

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset){
    tileset->image.texture = load_texture(renderer, tileset->image.source);
    SDL_QueryTexture(tileset->image.texture, NULL, NULL, &tileset->image.width, &tileset->image.height);

    const int column = tileset->image.width / tileset->tile_width;
    const int row = tileset->image.height / tileset->tile_height;

    tileset->num_tiles = column * row;
    tileset->tile = malloc(tileset->num_tiles * sizeof(tile_data_t));

    SDL_Rect *tiles = split_image(tileset->image.texture, column, row);

    const int gumo_tileset_id = tileset->id * 1000;
    tileset->cur_tile = gumo_tileset_id;
    for (int i = 0; i < tileset->num_tiles; ++i) {
        tileset->tile[i].id = gumo_tileset_id + i;
        tileset->tile[i].x = tiles[i].x;
        tileset->tile[i].y = tiles[i].y;
    }

    free(tiles);
}

void add_tileset(SDL_Renderer *renderer, const char *name, const char *image_source, const int tile_width, const int tile_height){

    if(tileset_count == MAX_TILESETS){
        printf("Maximum allowed number of tilesets exceeded!\n");
        return;
    }

    const int index = tileset_count;
    strcpy(tileset[index].name, name);
    strcpy(tileset[index].image.source, image_source);
    tileset[index].tile_width = tile_width;
    tileset[index].tile_height = tile_height;
    tileset[index].cur_tile = 0;
    tileset[index].id = index;
    load_tileset(renderer, &tileset[index]);
    tileset_count++;
}

void destroy_tilesets(){
    for (int i = 0; i < tileset_count; ++i)
        remove_tileset(&tileset[i]);
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

void render_tile(SDL_Renderer *renderer, const int id, const SDL_Rect *dst_rect){
    const int tile_index = id % 1000;
    const int tileset_index = (id - tile_index) / 1000; //MAYBE THIS RETURN ERROR
    const tileset_t *tileset_buffer = &tileset[tileset_index];

    SDL_Rect src_buffer = (SDL_Rect) {
            tileset_buffer->tile[tile_index].x,
            tileset_buffer->tile[tile_index].y,
            tileset_buffer->tile_width,
            tileset_buffer->tile_height
    };

    SDL_RenderCopy(renderer, tileset_buffer->image.texture, &src_buffer, dst_rect);
}

void get_selected_tile(int tileset_index, int *value){
    *(value) = tileset[tileset_index].cur_tile;
}