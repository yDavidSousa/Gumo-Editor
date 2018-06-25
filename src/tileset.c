#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <tileset.h>
#include <utils.h>

/*
 * SERIALIZE
 * 0234 ---- TILE_ID - [TILE_ID]
 * 0003 ---- TILESET_ID - [TILESET_ID * 1000]
 * 3234 ---- GUMO_ID - [TILESET_ID + TILE_ID]
 * DESERIALIZE
 * 3234 ---- GUMO_ID - [(TILESET_ID * 1000) + TILE_ID]
 * 0234 ---- TILE_ID - [GUMO_ID % 1000]
 * 0003 ---- TILESET_ID - [(GUMO_ID - TILE_ID) / 1000]
 */

#define TILE_ID(tile_id) ((tile_id) % (1000))
#define TILESET_ID(tileset_id) ((tileset_id) * (1000))
#define GUMO_ID(tileset_id, tile_id) (((tileset_id) * (1000)) + (tile_id))

tileset_t tileset[MAX_TILESETS];

void load_tileset(SDL_Renderer *renderer, tileset_t *tileset){
    tileset->image.texture = load_texture(renderer, tileset->image.source);
    SDL_QueryTexture(tileset->image.texture, NULL, NULL, &tileset->image.width, &tileset->image.height);

    const int column = tileset->image.width / tileset->tile_width;
    const int row = tileset->image.height / tileset->tile_height;

    tileset->num_tiles = column * row;
    tileset->tile = malloc(tileset->num_tiles * sizeof(tile_info_t));

    SDL_Rect *tiles = split_image(tileset->image.texture, column, row);

    tileset->selected_tile = 0;
    for (int i = 0; i < tileset->num_tiles; ++i) {
        tileset->tile[i].id = (short)GUMO_ID(tileset->id, i);
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

int select_tile(const int id){
    const int tile_index = TILE_ID(id);
    const int tileset_index = (id - tile_index) / 1000;

    tileset[tileset_index].selected_tile = tile_index;

    return tileset_index;
}

void get_selected_tile(int tileset_index, short *value){
    *(value) = (short)GUMO_ID(tileset_index, tileset[tileset_index].selected_tile);
}

tileset_t *get_tileset(const int index){
    return &tileset[index];
}

//TODO(David): Do this not return empty elements
tileset_t *get_tilesets(){
    return &tileset[0];
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

void render_tileset(SDL_Renderer *renderer, const int id, const SDL_Rect *dst_rect){
    SDL_Rect src_buffer = (SDL_Rect) {0, 0, dst_rect->w, dst_rect->h};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, dst_rect);
    SDL_RenderCopy(renderer, tileset[id].image.texture, &src_buffer, dst_rect);
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

void destroy_tilesets(){
    for (int i = 0; i < num_tilesets; ++i)
        remove_tileset(i);
}