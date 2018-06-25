#include <stdio.h>
#include <malloc.h>
#include <mem.h>

#include <SDL.h>

#include <utils.h>
#include <tilemap.h>
#include "gumo_serializer.h"

const int FPS = 60;
const int MAX_ZOOM = 8;
const int MIN_ZOOM = -8;

typedef struct window {
    char title[256];
    char icon_source[256];
    int width, height;
    int x, y;
    Uint32 flags;
} window_t;

typedef struct config {
    char author[256];
    char version[256];
    window_t window;
} config_t;

typedef enum tools {
    TOOL_BRUSH,
    TOOL_ERASER,
    TOOL_FILTER,
    TOOL_SELECT,
} tool_t;

typedef struct cursor_ui {
    SDL_Texture *texture;
    SDL_Rect *src_rect;
    SDL_Rect dst_rect;
    int down_x, down_y;
} cursor_ui_t;

config_t config;
cursor_ui_t cursor;

void set_window_name(SDL_Window *window, const char *file_name, const char *app_name){
    char window_name[256];
    strcpy(window_name, file_name);
    strcat(window_name, " - ");
    strcat(window_name, app_name);
    SDL_SetWindowTitle(window, window_name);
}

void free_cursor(cursor_ui_t cursor){
    SDL_DestroyTexture(cursor.texture);
    free(cursor.src_rect);
}

void init_default_config(config_t *config){
    strcpy(config->author, "unknown");
    strcpy(config->version, "0.0");

    strcpy(config->window.title, "None Title");
    config->window.width = 640;
    config->window.height = 480;
    config->window.x = SDL_WINDOWPOS_CENTERED;
    config->window.y = SDL_WINDOWPOS_CENTERED;
    config->window.flags = 0;
}

void read_config_file(config_t *config, const char *path){
    FILE *file = fopen(path, "r");

    if(!file){
        printf("Couldn't open file\n");
        return;
    }

    char buffer[256] = {};
    while(!feof(file)){
        fscanf(file, "%s", buffer);

        if(strcmp(buffer, "author") == 0){
            fscanf(file, " %[^\n]s", config->author);
            continue;
        }
        if(strcmp(buffer, "version") == 0){
            fscanf(file, "%s", config->version);
            continue;
        }
        if(strcmp(buffer, "title") == 0){
            fscanf(file, " %[^\n]s", config->window.title);
            continue;
        }
        if(strcmp(buffer, "icon") == 0){
            fscanf(file, " %[^\n]s", config->window.icon_source);
            continue;
        }
        if(strcmp(buffer, "fullscreen") == 0){
            fscanf(file, "%s", buffer);

            if(strcmp(buffer, "true") == 0){
                config->window.flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            continue;
        }
        if(strcmp(buffer, "maximized") == 0){
            fscanf(file, "%s", buffer);

            if(strcmp(buffer, "true") == 0){
                config->window.flags |= SDL_WINDOW_MAXIMIZED;
            }
        }
        if(strcmp(buffer, "resizable") == 0){
            fscanf(file, "%s", buffer);

            if(strcmp(buffer, "true") == 0){
                config->window.flags |= SDL_WINDOW_RESIZABLE;
            }
            continue;
        }
        if(strcmp(buffer, "borderless") == 0){
            fscanf(file, "%s", buffer);

            if(strcmp(buffer, "true") == 0){
                config->window.flags |= SDL_WINDOW_BORDERLESS;
            }
            continue;
        }
        if(strcmp(buffer, "width") == 0){
            fscanf(file, "%d", &config->window.width);
            continue;
        }
        if(strcmp(buffer, "height") == 0){
            fscanf(file, "%d", &config->window.height);
            continue;
        }
        if(strcmp(buffer, "x") == 0){
            fscanf(file, "%d", &config->window.x);
            continue;
        }
        if(strcmp(buffer, "y") == 0){
            fscanf(file, "%d", &config->window.y);
            continue;
        }
    }

    fclose(file);
}

void editor_action(tilemap_t *tilemap, const tool_t tool){
    const int position_x = (cursor.dst_rect.x - tilemap->offset_x);
    const int position_y = (cursor.dst_rect.y - tilemap->offset_y);

    if(position_x < 0 || position_y < 0)
        return;

    const int column = position_x / (tilemap->tile_width + tilemap->zoom);
    const int row = position_y / (tilemap->tile_height + tilemap->zoom);

    switch(tool){
        case TOOL_BRUSH:
            put_tile(tilemap, tilemap->cur_layer, row, column);
            break;
        case TOOL_ERASER:
            remove_tile(tilemap, tilemap->cur_layer, row, column);
            break;
        case TOOL_FILTER:
            filter_tile(tilemap, tilemap->cur_layer, row, column);
            break;
        case TOOL_SELECT:
            printf("SELECT\n");
            break;
    }
}

int main(int argc, char *args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    init_default_config(&config);
    read_config_file(&config, "../data/config.txt");

    window = SDL_CreateWindow(
            config.window.title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.window.width,
            config.window.height,
            config.window.flags
    );

    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, renderer_flags);

    SDL_GetWindowSize(window, &config.window.width, &config.window.height);

    SDL_Event event;
    int prev_time = 0;
    int cur_time = 0;
    int quit = 0;

    tool_t cur_tool = TOOL_BRUSH;
    tilemap_t *tilemap = create_tilemap("Tilemap_01", 960, 544, 32, 32);
    set_window_name(window, tilemap->name, config.window.title); //Change window title with tilemap name
    tilemap->offset_x = (config.window.width - tilemap->map_width) / 2;
    tilemap->offset_y = (config.window.height - tilemap->map_height) / 2;
    tilemap->zoom = 0;

    //Add and remove layer_data
    add_layer(tilemap, "Layer_1", TILES);
    add_layer(tilemap, "Layer_2", TILES);
    add_layer(tilemap, "Layer_3", TILES);
    add_layer(tilemap, "Layer_4", ENTITIES);

    //Add and remove tilesets
    add_tileset(renderer, "Tileset_8x8", "content/tileset_8x8.png", 8, 8);
    add_tileset(renderer, "Tileset_16x16", "content/tileset_16x16.png", 16, 16);
    add_tileset(renderer, "Tileset_32x32", "content/tileset_32x32.png", 32, 32);

    //Add and remove Entities
    add_entity("Player", 1192);
    add_entity("Enemy", 1219);
    add_entity("Checkpoint", -1);

    //Cursor
    SDL_ShowCursor(SDL_DISABLE);
    cursor.texture = load_texture(renderer, "../data/grabhand.png");
    cursor.src_rect = split_image(cursor.texture, 3, 3);
    cursor.dst_rect.w = cursor.src_rect->w;
    cursor.dst_rect.h = cursor.src_rect->h;

    while (!quit){
        //Time loop
        prev_time = cur_time;
        cur_time = SDL_GetTicks();

        SDL_GetMouseState(&cursor.dst_rect.x, &cursor.dst_rect.y);

        while (SDL_PollEvent(&event) != 0){
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_S: save_gtm_file("../content/Tilemap_01.txt", tilemap); break;
                            case SDL_SCANCODE_O: /*load_tilemap(tilemap, "../content/Tilemap_01.txt.txt");*/ break;
                        }
                    } else if(event.key.keysym.mod & KMOD_SHIFT) {
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_R: remove_layer(tilemap, tilemap->cur_layer); break;
                            case SDL_SCANCODE_C: clear_layer(tilemap, tilemap->cur_layer); break;
                            case SDL_SCANCODE_H:
                                if(layer_has_flags(&tilemap->layers[tilemap->cur_layer], LAYER_HIDDEN))
                                    tilemap->layers[tilemap->cur_layer].flags &= ~LAYER_HIDDEN;
                                else
                                    tilemap->layers[tilemap->cur_layer].flags |= LAYER_HIDDEN;
                                break;
                            case SDL_SCANCODE_L:
                                if(layer_has_flags(&tilemap->layers[tilemap->cur_layer], LAYER_LOCKED))
                                    tilemap->layers[tilemap->cur_layer].flags &= ~LAYER_LOCKED;
                                else
                                    tilemap->layers[tilemap->cur_layer].flags |= LAYER_LOCKED;
                                break;
                        }
                    } else {
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_B: cur_tool = TOOL_BRUSH; break;
                            case SDL_SCANCODE_E: cur_tool = TOOL_ERASER; break;
                            case SDL_SCANCODE_F: cur_tool = TOOL_FILTER; break;
                            case SDL_SCANCODE_S: cur_tool = TOOL_SELECT; break;

                            case SDL_SCANCODE_1: set_layer(tilemap, 0); break;
                            case SDL_SCANCODE_2: set_layer(tilemap, 1); break;
                            case SDL_SCANCODE_3: set_layer(tilemap, 2); break;
                            case SDL_SCANCODE_4: set_layer(tilemap, 3); break;

                            case SDL_SCANCODE_F1: set_tileset(tilemap, 0); break;
                            case SDL_SCANCODE_F2: set_tileset(tilemap, 1); break;
                            case SDL_SCANCODE_F3: set_tileset(tilemap, 2); break;

                            case SDL_SCANCODE_RIGHT:
                                if(tilemap->layers[tilemap->cur_layer].type == ENTITIES){
                                    tilemap->cur_entity += 1;
                                    mathf_clamp(&tilemap->cur_entity, 0, num_entities - 1);
                                    printf("CURENTITY: %d\n", tilemap->cur_entity);
                                }
                                if(tilemap->layers[tilemap->cur_layer].type == TILES){
                                    tileset_t *tileset_buffer = get_tileset(tilemap->cur_tileset);
                                    tileset_buffer->selected_tile += 1;
                                    mathf_clamp(&tileset_buffer->selected_tile, 0, tileset_buffer->num_tiles - 1);
                                }
                                break;
                            case SDL_SCANCODE_LEFT:
                                if(tilemap->layers[tilemap->cur_layer].type == ENTITIES){
                                    tilemap->cur_entity -= 1;
                                    mathf_clamp(&tilemap->cur_entity, 0, num_entities - 1);
                                }
                                if(tilemap->layers[tilemap->cur_layer].type == TILES){
                                    tileset_t *tileset_buffer = get_tileset(tilemap->cur_tileset);
                                    tileset_buffer->selected_tile -= 1;
                                    mathf_clamp(&tileset_buffer->selected_tile, 0, tileset_buffer->num_tiles - 1);
                                }
                                break;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    cursor.down_x = 0;
                    cursor.down_y = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK)
                        editor_action(tilemap, cur_tool);
                    cursor.down_x = cursor.dst_rect.x - tilemap->offset_x;
                    cursor.down_y = cursor.dst_rect.y - tilemap->offset_y;
                    break;
                case SDL_MOUSEWHEEL:
                    if(event.wheel.y > 0){
                        tilemap->zoom = tilemap->zoom + 2 >= MAX_ZOOM ? MAX_ZOOM : tilemap->zoom + 2;
                    } else if(event.wheel.y < 0){
                        tilemap->zoom = tilemap->zoom - 2 <= MIN_ZOOM ? MIN_ZOOM : tilemap->zoom - 2;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    switch (SDL_GetMouseState(NULL, NULL)){
                        case SDL_BUTTON_LMASK:
                            editor_action(tilemap, cur_tool);
                            break;
                        case SDL_BUTTON_RMASK:
                            tilemap->offset_x = cursor.dst_rect.x - cursor.down_x;
                            tilemap->offset_y = cursor.dst_rect.y - cursor.down_y;

                            //TODO(David): Fixed when zoom
                            if(tilemap->offset_x + tilemap->map_width >= config.window.width)
                                tilemap->offset_x = config.window.width - tilemap->map_width;
                            if(tilemap->offset_x <= 0)
                                tilemap->offset_x = 0;
                            if(tilemap->offset_y + tilemap->map_height >= config.window.height)
                                tilemap->offset_y = config.window.height - tilemap->map_height;
                            if(tilemap->offset_y <= 0)
                                tilemap->offset_y = 0;
                            break;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        render_tilemap(renderer, tilemap);
        //render_tileset(renderer, tilemap->cur_tileset,& (SDL_Rect) { 20, 80, 160, 256});
        if(mathf_range(cursor.dst_rect.x, 1, config.window.width)
           && mathf_range(cursor.dst_rect.y, 1, config.window.height))
            SDL_RenderCopy(renderer, cursor.texture, &cursor.src_rect[0], &cursor.dst_rect);
        SDL_RenderPresent(renderer);

        if(cur_time < FPS)
            SDL_Delay(cur_time - (Uint32)prev_time);
    }

    free_cursor(cursor);
    remove_tilemap(tilemap);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}