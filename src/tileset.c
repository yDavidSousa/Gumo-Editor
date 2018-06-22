#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <tileset.h>
#include <utils.h>

/*
 * 3234 ---- GUMO_ID - [(TILESET_ID * 1000) + TILE_ID]
 * 0234 ---- TILE_ID - [GUMO_ID % 1000]
 * 0003 ---- TILESET_ID - [(GUMO_ID - TILE_ID) / 1000]
 */

tileset_t tileset[MAX_TILESETS];
int num_tilesets;

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset){
    tileset->image.texture = load_texture(renderer, tileset->image.source);
    SDL_QueryTexture(tileset->image.texture, NULL, NULL, &tileset->image.width, &tileset->image.height);

    const int column = tileset->image.width / tileset->tile_width;
    const int row = tileset->image.height / tileset->tile_height;

    tileset->num_tiles = column * row;
    tileset->tile = malloc(tileset->num_tiles * sizeof(tile_info_t));

    SDL_Rect *tiles = split_image(tileset->image.texture, column, row);

    const int gumo_tileset_id = tileset->id * 1000;
    tileset->selected_tile = gumo_tileset_id;
    for (int i = 0; i < tileset->num_tiles; ++i) {
        tileset->tile[i].id = gumo_tileset_id + i;
        tileset->tile[i].x = tiles[i].x;
        tileset->tile[i].y = tiles[i].y;
    }

    free(tiles);
}

void add_tileset(SDL_Renderer *renderer, const char *name, const char *image_source, const int tile_width, const int tile_height){

    if(num_tilesets == MAX_TILESETS){
        printf("Maximum allowed number of tilesets exceeded!\n");
        return;
    }

    int index = num_tilesets;

    for (int i = 0; i < num_tilesets; ++i) {
        if(tileset[i].id == -1){
            index = i;
            break;
        }
    }

    strcpy(tileset[index].name, name);
    strcpy(tileset[index].image.source, image_source);
    tileset[index].tile_width = tile_width;
    tileset[index].tile_height = tile_height;
    tileset[index].selected_tile = 0;
    tileset[index].id = index;
    load_tileset(renderer, &tileset[index]);
    num_tilesets++;
}

void remove_tileset(const int index){
    tileset[index].id = -1;
    tileset[index].name[0] = '\0';
    tileset[index].image.source[0] = '\0';
    tileset[index].num_tiles = 0;
    tileset[index].selected_tile = 0;
    tileset[index].tile_width = 0;
    tileset[index].tile_height = 0;
    free(tileset[index].tile);
    SDL_DestroyTexture(tileset->image.texture);
}

void render_tile(SDL_Renderer *renderer, const short id, const SDL_Rect *dst_rect){
    const int tile_index = id % 1000;
    const int tileset_index = (id - tile_index) / 1000;
    const tileset_t *tileset_buffer = &tileset[tileset_index];

    SDL_Rect src_buffer = (SDL_Rect) {
            tileset_buffer->tile[tile_index].x,
            tileset_buffer->tile[tile_index].y,
            tileset_buffer->tile_width,
            tileset_buffer->tile_height
    };

    SDL_RenderCopy(renderer, tileset_buffer->image.texture, &src_buffer, dst_rect);
}

void destroy_tilesets(){
    for (int i = 0; i < num_tilesets; ++i)
        remove_tileset(i);
}

int filter_tileset(const int id){
    const int tile_index = id % 1000;
    const int tileset_index = (id - tile_index) / 1000;

    tileset[tileset_index].selected_tile = tile_index;

    return tileset_index;
}

void get_selected_tile(int tileset_index, short *value){
    *(value) = tileset[tileset_index].selected_tile;
}