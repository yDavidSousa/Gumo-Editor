#include <stdio.h>
#include <malloc.h>
#include <mem.h>

#include <SDL.h>
#include <SDL_image.h>

#include <utils.h>
#include <tilemap.h>

const int FPS = 60;

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

typedef enum tools{
    Brush = 0,
    Erase = 1,
    Filter = 2,
    Select = 3,
} tools_t;

typedef struct cursor_ui {
    SDL_Texture *texture;
    SDL_Rect *src_rect;
    SDL_Rect dst_rect;
} cursor_ui_t;

config_t config;
cursor_ui_t cursor;

void free_cursor(cursor_ui_t cursor){
    SDL_DestroyTexture(cursor.texture);
    free(cursor.src_rect);
}

void save_tilemap(tilemap_t *tilemap, const char *path){
    char file_name[80];
    strcpy(file_name, path);
    strcat(file_name, tilemap->name);
    strcat(file_name, ".txt");

    FILE *file = fopen(file_name, "w");

    if(!file){
        printf("Couldn't open file\n");
        return;
    }

    fprintf(file, "#TILEMAP_INFO\n");
    fprintf(file,"\t%d //number of tilesets\n", tilemap->num_tilesets);
    fprintf(file,"\t%d //number of layers\n", tilemap->num_layers);
    fprintf(file,"\t%d //number of columns\n", tilemap->num_columns);
    fprintf(file,"\t%d //number of rows\n", tilemap->num_rows);
    fprintf(file,"\t%d //map_width\n", tilemap->map_width);
    fprintf(file,"\t%d //map_height\n", tilemap->map_height);
    fprintf(file,"\t%d //tile_width\n", tilemap->tile_width);
    fprintf(file,"\t%d //tile_height\n", tilemap->tile_height);
    fprintf(file, "\n");

    fprintf(file, "#LAYERS_INFO\n");

    char layer_type_buffer[80];
    for (int i = 0; i < tilemap->num_layers; ++i){

        switch (tilemap->layers[i].type){
            case TILES:
                strcpy(layer_type_buffer, TILES_LAYER_TYPE);
                break;
            case ENTITIES:
                strcpy(layer_type_buffer, ENTITIES_LAYER_TYPE);
                break;
        }

        fprintf(file, "\t%s %d %s %d %d\n",
                tilemap->layers[i].name,
                tilemap->layers[i].index,
                layer_type_buffer,
                tilemap->layers[i].locked,
                tilemap->layers[i].hidden
        );
    }

    fprintf(file, "\n");

    fprintf(file, "#TILESETS_INFO\n");
    for (int i = 0; i < tilemap->num_tilesets; ++i)
        fprintf(file, "\t%s %s %d %d\n",
            tilemap->tilesets[i].name,
            tilemap->tilesets[i].image.source,
            tilemap->tilesets[i].tile_width,
            tilemap->tilesets[i].tile_width
        );
    fprintf(file, "\n");

    for (int l = 0; l < tilemap->num_layers; ++l) {
        fprintf(file, "%s\n", tilemap->layers[l].name);

        if(tilemap->layers[l].type == TILES){
            for (int r = 0; r < tilemap->num_rows; ++r) {
                fputs("\t", file);
                for (int c = 0; c < tilemap->num_columns; ++c)
                    fprintf(file, "%d,%d ",
                            tilemap->tileset_data[l][r][c],
                            tilemap->tile_data[l][r][c]
                    );
                fputs("\n", file);
            }
        }

        if(tilemap->layers[l].type == ENTITIES){
            for (int r = 0; r < tilemap->num_rows; ++r) {
                for (int c = 0; c < tilemap->num_columns; ++c){
                    if(tilemap->entity_data[l][r][c] != EMPTY_TILE){
                        int index =  tilemap->entity_data[l][r][c];
                        fprintf(file, "\t%d %s %d %d %d %d\n",
                                tilemap->entities[index].id,
                                tilemap->entities[index].name,
                                c * tilemap->tile_width,
                                r * tilemap->tile_height,
                                tilemap->entities[index].w,
                                tilemap->entities[index].h
                        );
                    }
                }
            }
        }

        fputs("\n", file);
    }

    printf("File saved with successful!\n");
    fclose(file);
}

void load_tilemap(tilemap_t *tilemap, const char *file_path) {
    FILE *file = fopen(file_path, "r");

    if (!file){
        printf("Couldn't open file\n");
        return;
    }

    char buffer[256] = {};
    while(!feof(file)){
        fscanf(file, "%s", buffer);

        if(strcmp(buffer, "#TILEMAP_INFO") == 0){
            fscanf(file, "%d %*[^\n]", &tilemap->num_tilesets);
            fscanf(file, "%d %*[^\n]", &tilemap->num_layers);
            fscanf(file, "%d %*[^\n]", &tilemap->num_columns);
            fscanf(file, "%d %*[^\n]", &tilemap->num_rows);
            fscanf(file, "%d %*[^\n]", &tilemap->map_width);
            fscanf(file, "%d %*[^\n]", &tilemap->map_height);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_height);
        } else if(strcmp(buffer, "#LAYERS_INFO") == 0){
            char layer_type_buffer[80];
            for (int i = 0; i < tilemap->num_layers; ++i) {
                fscanf(file, "%s %d %s %d %d",
                   tilemap->layers[i].name,
                   &tilemap->layers[i].index,
                   layer_type_buffer,
                   &tilemap->layers[i].locked,
                   &tilemap->layers[i].hidden
                );

                if(strcmp(layer_type_buffer, TILES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = TILES;
                else if (strcmp(layer_type_buffer, ENTITIES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = ENTITIES;
            }
        } else if(strcmp(buffer, "#TILESETS_INFO") == 0){
            for (int i = 0; i < tilemap->num_tilesets; ++i)
                fscanf(file, "%s %s %d %d",
                   tilemap->tilesets[i].name,
                   tilemap->tilesets[i].image.source,
                   &tilemap->tilesets[i].tile_width,
                   &tilemap->tilesets[i].tile_height
                );
        } else {
            for (int i = 0; i < tilemap->num_layers; ++i) {
                if(strcmp(buffer, tilemap->layers[i].name) == 0){
                    for (int r = 0; r < tilemap->num_rows; ++r)
                        for (int c = 0; c < tilemap->num_columns; ++c)
                            fscanf(file, "%d,%d ",
                                   &tilemap->tileset_data[i][r][c],
                                   &tilemap->tile_data[i][r][c]
                            );
                    break;
                }
            }
        }
    }

    printf("Tilemap loaded with successful!\n");
    fclose(file);
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

    int prev_time = 0;
    int cur_time = 0;
    int quit = 0;
    int cur_tile = 0;
    int cur_tileset = 0;

    tools_t cur_tool = Brush;
    tilemap_t *tilemap = create_tilemap("Tilemap_01", 960, 544, 32, 32);

    //Change window title with tilemap name
    char window_name[DEFAULT_FILE_NAME_LEN];
    strcpy(window_name, tilemap->name);
    strcat(window_name, " - ");
    strcat(window_name, config.window.title);
    SDL_SetWindowTitle(window, window_name);

    //Add and remove layer_data
    add_layer(tilemap, "Layer_1", TILES);
    add_layer(tilemap, "Layer_2", TILES);
    add_layer(tilemap, "Layer_3", ENTITIES);
    add_layer(tilemap, "Layer_4", ENTITIES);

    //Add and remove tilesets
    add_tileset(tilemap, "Tileset_8x8", "content/tileset_8x8.png", 8, 8);
    load_tileset(renderer, &tilemap->tilesets[tilemap->cur_tileset]);
    add_tileset(tilemap, "Tileset_16x16", "content/tileset_16x16.png", 16, 16);
    load_tileset(renderer, &tilemap->tilesets[tilemap->cur_tileset]);
    add_tileset(tilemap, "Tileset_32x32", "content/tileset_32x32.png", 32, 32);
    load_tileset(renderer, &tilemap->tilesets[tilemap->cur_tileset]);

    //Add and remove Entities
    add_entity(tilemap, "Player");
    add_entity(tilemap, "Enemy");

    //Cursor
    SDL_ShowCursor(SDL_DISABLE);
    cursor.texture = load_texture(renderer, "../data/grabhand.png");
    cursor.src_rect = split_image(cursor.texture, 3, 3);
    cursor.dst_rect.w = cursor.src_rect->w;
    cursor.dst_rect.h = cursor.src_rect->h;

    tilemap->zoom = 16;
    tilemap->offset_x = 100;
    tilemap->offset_y = 100;
    load_tilemap(tilemap, "../content/Tilemap_01.txt");

    while (!quit){
        //Time loop
        prev_time = cur_time;
        cur_time = SDL_GetTicks();

        SDL_GetMouseState(&cursor.dst_rect.x, &cursor.dst_rect.y);

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0){
            SDL_Scancode scanCode = event.key.keysym.scancode;
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if(scanCode == SDL_SCANCODE_F1) set_tileset(tilemap, 0);
                    if(scanCode == SDL_SCANCODE_F2) set_tileset(tilemap, 1);
                    if(scanCode == SDL_SCANCODE_F3) set_tileset(tilemap, 2);

                    if(scanCode == SDL_SCANCODE_1) set_layer(tilemap, 0);
                    if(scanCode == SDL_SCANCODE_2) set_layer(tilemap, 1);
                    if(scanCode == SDL_SCANCODE_3) set_layer(tilemap, 2);
                    if(scanCode == SDL_SCANCODE_4) set_layer(tilemap, 3);
                    if(scanCode == SDL_SCANCODE_8) change_layer_visibility(tilemap, 0);
                    if(scanCode == SDL_SCANCODE_9) remove_layer(tilemap, 0);
                    if(scanCode == SDL_SCANCODE_0) clear_tilemap(tilemap);

                    if(scanCode == SDL_SCANCODE_B) cur_tool = Brush;
                    if(scanCode == SDL_SCANCODE_E) cur_tool = Erase;
                    if(scanCode == SDL_SCANCODE_F) cur_tool = Filter;
                    if(scanCode == SDL_SCANCODE_S) cur_tool = Select;

                    if ((event.key.keysym.mod & KMOD_CTRL)) {
                        if(scanCode == SDL_SCANCODE_S)
                            save_tilemap(tilemap, "../content/");
                        if(scanCode == SDL_SCANCODE_O)
                            load_tilemap(tilemap, "../content/Tilemap_01.txt");
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    cur_tileset = tilemap->cur_tileset;
                    cur_tile = tilemap->tilesets[cur_tileset].cur_tile;
                    const int num_tiles = tilemap->tilesets[cur_tileset].num_tiles;
                    cur_tile += event.wheel.y;
                    tilemap->tilesets[cur_tileset].cur_tile = cur_tile < 0 ? num_tiles - 1 : cur_tile % num_tiles;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEMOTION:
                    if(event.button.button == SDL_BUTTON_LEFT){
                        switch(cur_tool){
                            case Brush:
                                if(tilemap->layers[tilemap->cur_layer].type == TILES)
                                    put_tile(tilemap, cursor.dst_rect.x, cursor.dst_rect.y);
                                else if(tilemap->layers[tilemap->cur_layer].type == ENTITIES)
                                    put_entity(tilemap, cursor.dst_rect.x, cursor.dst_rect.y);
                                break;
                            case Erase:
                                if(tilemap->layers[tilemap->cur_layer].type == TILES)
                                    remove_tile(tilemap, cursor.dst_rect.x, cursor.dst_rect.y);
                                else if(tilemap->layers[tilemap->cur_layer].type == ENTITIES)
                                    remove_entity(tilemap, cursor.dst_rect.x, cursor.dst_rect.y);
                                break;
                            case Filter:
                                filter_tile(tilemap, cursor.dst_rect.x, cursor.dst_rect.y);
                                break;
                            case Select:
                                printf("SELECT\n");
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        render_tilemap(renderer, tilemap);

        tileset_t tileset_buffer = tilemap->tilesets[cur_tileset];
        SDL_Rect des_buffer = (SDL_Rect) {
                880,
                32,
                tilemap->tile_width,
                tilemap->tile_height
        };
        SDL_Rect src_buffer = (SDL_Rect) {
                tileset_buffer.tile[cur_tile].x,
                tileset_buffer.tile[cur_tile].y,
                tileset_buffer.tile_width,
                tileset_buffer.tile_height
        };
        SDL_RenderCopy(renderer,tileset_buffer.image.texture, &src_buffer, &des_buffer);

        if((cursor.dst_rect.x < config.window.width && cursor.dst_rect.x > 0)
           && (cursor.dst_rect.y < config.window.height && cursor.dst_rect.y > 0))
                SDL_RenderCopy(renderer, cursor.texture, &cursor.src_rect[cur_tool], &cursor.dst_rect);

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