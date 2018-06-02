#include <stdio.h>
#include <malloc.h>
#include <mem.h>

#include <SDL.h>
#include <SDL_image.h>

/*
 * int index = (TILES_X * r) + c;
 * http://docs.mapeditor.org/en/stable/reference/tmx-map-format/
 * */

//Const & Defines
const int FPS = 60;

#define MAX_TILE_X 200
#define MAX_TILE_Y 200
#define MAX_LAYERS 8
#define MAX_TILESETS 4

#define EMPTY_TILE (-1)

//Structures

typedef enum tools{
    Brush,
    Erase,
    Filter,
    Select,
} tools_t;

typedef struct window {
    char title[256];
    char icon[256];
    int width, height;
    int x, y;
    Uint32 flags;
} window_t;

typedef struct config {
    char author[256];
    char version[256];
    window_t window;
} config_t;

typedef struct tileinfo {
    int id;
    int x, y;
} tileinfo_t;

typedef struct layerinfo {
    int index;
    char name[256];
    int visible;
} layerinfo_t;

typedef struct tileset{
    int active;
    char name[256];
    char image_source[256];
    SDL_Texture *image_texture;
    int tile_width, tile_height;
    int num_tiles, cur_tile;
    tileinfo_t *tileinfo;
} tileset_t;

typedef struct tilemap {
    char name[256];
    int data[MAX_LAYERS][MAX_TILE_Y][MAX_TILE_X];
    layerinfo_t layerinfo[MAX_LAYERS];
    tileset_t tileset[MAX_TILESETS];
    int max_x, max_y, num_layers, num_tilesets;
    int map_width, map_height;
    int tile_width, tile_height;
    int cur_layer, cur_tileset;
} tilemap_t;

//Util Functions

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path){
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect *split_image(SDL_Texture *texture, const int column, const int row) {

    int width, height;
    int i = 0;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect *image = (SDL_Rect *) malloc((column * row) * sizeof(SDL_Rect));
    width = width / column;
    height = height / row;

    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < column; ++c) {
            image[i].x = width * c;
            image[i].y = height * r;
            image[i].w = width;
            image[i].h = height;
            i++;
        }
    }

    return image;
}

int snap_to_grid(const int value, const int increment, const int offset){
    return (value/increment) * increment + offset;
}

//Tilemap Functions

void add_tileset(tilemap_t *tilemap, const char *name, const char *image_source, const int tile_width, const int tile_height){
    for (int i = 0; i < MAX_TILESETS; ++i) {
        if(tilemap->tileset[i].active == 0){
            tilemap->tileset[i].active = 1;
            tilemap->num_tilesets++;
            strcpy(tilemap->tileset[i].name, name);
            strcpy(tilemap->tileset[i].image_source, image_source);
            tilemap->tileset[i].tile_width = tile_width;
            tilemap->tileset[i].tile_height = tile_height;
            break;
        }
    }
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

void remove_tileset(tileset_t *tileset){
    tileset->active = 0;
    tileset->tile_width = 0;
    tileset->tile_height = 0;
    free(tileset->tileinfo);
    SDL_DestroyTexture(tileset->image_texture);
}

void add_layer(tilemap_t *tilemap, const char *name, const int index){
    int current_index = tilemap->num_layers;
    tilemap->num_layers ++;
    strcpy(tilemap->layerinfo[current_index].name, name);
    tilemap->layerinfo[current_index].index = index;

    for (int r = 0; r < tilemap->max_y; ++r)
        for (int c = 0; c < tilemap->max_x; ++c)
            tilemap->data[current_index][r][c] = EMPTY_TILE;
}

//TODO(David): relocate layers after remove one
void remove_layer(tilemap_t *tilemap, const int index){
    tilemap->num_layers --;
    tilemap->layerinfo[index].index = -1;

    for (int r = 0; r < tilemap->max_y; ++r)
        for (int c = 0; c < tilemap->max_x; ++c)
            tilemap->data[index][r][c] = EMPTY_TILE;
}

tilemap_t *create_tilemap(const char *name, const int width, const int height, const int tile_width, const int tile_height){
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));

    strcpy(tilemap->name, name);
    tilemap->map_width = width;
    tilemap->map_height = height;
    tilemap->tile_width = tile_width;
    tilemap->tile_height = tile_height;
    tilemap->max_x = width / tile_width;
    tilemap->max_y = height / tile_height;
    tilemap->num_layers = 0;
    add_layer(tilemap, "Layer_1", 0);
    tilemap->cur_layer = 0;
    tilemap->cur_tileset = 0;

    for (int i = 0; i < MAX_TILESETS; ++i)
        tilemap->tileset[i].active = 0;

    return tilemap;
}

void render_tilemap(SDL_Renderer *renderer, tilemap_t *tilemap){
    SDL_Rect des_buffer;
    SDL_Rect src_buffer;
    int cur_tile, tile_width, tile_height;

    for (int l = 0; l < tilemap->num_layers; ++l)
        for (int r = 0; r < tilemap->max_y; ++r)
            for (int c = 0; c < tilemap->max_x; ++c) {

                if(tilemap->data[l][r][c] == EMPTY_TILE)
                    continue;

                cur_tile = tilemap->data[l][r][c];
                tile_width = tilemap->tileset[0].tile_width;
                tile_height = tilemap->tileset[0].tile_height;

                des_buffer = (SDL_Rect){ c * tile_width, r * tile_height, tile_width, tile_height};
                src_buffer = (SDL_Rect){ tilemap->tileset[0].tileinfo[cur_tile].x, tilemap->tileset[0].tileinfo[cur_tile].y, tile_width, tile_height};

                SDL_RenderCopy(renderer,  tilemap->tileset[0].image_texture, &src_buffer, &des_buffer);
            }
}

void remove_tilemap(tilemap_t *tilemap){
    remove_tileset(&tilemap->tileset[0]);
    free(tilemap);
}

void put_tile(tilemap_t *tilemap, const int x, const int y){
    int cur_tile = tilemap->tileset[0].cur_tile;
    int l = tilemap->cur_layer;
    int c = (x / tilemap->tileset[0].tile_width);
    int r = (y / tilemap->tileset[0].tile_height);

    if(tilemap->data[l][r][c] == cur_tile)
        return;

    tilemap->data[l][r][c] = cur_tile;
}

void remove_tile(tilemap_t *tilemap, const int x, const int y){
    int l = tilemap->cur_layer;
    int c = (x / tilemap->tileset[0].tile_width);
    int r = (y / tilemap->tileset[0].tile_height);

    if(tilemap->data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->data[l][r][c] = EMPTY_TILE;
}

void filter_tile(tilemap_t *tilemap, const int x, const int y){
    int l = tilemap->cur_layer;
    int c = x / tilemap->tileset[0].tile_width;
    int r = y / tilemap->tileset[0].tile_height;

    if(tilemap->data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->tileset[0].cur_tile = tilemap->data[l][r][c];
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
    fprintf(file,"\t%d //number of layers\n", tilemap->num_layers);
    fprintf(file,"\t%d //number of tilesets\n", tilemap->num_tilesets);
    fprintf(file,"\t%d //map_width\n", tilemap->map_width);
    fprintf(file,"\t%d //map_height\n", tilemap->map_height);
    fprintf(file,"\t%d //tile_width\n", tilemap->tile_width);
    fprintf(file,"\t%d //tile_height\n", tilemap->tile_height);

    fprintf(file, "\n");

    fprintf(file, "#LAYERS_INFO\n");
    for (int i = 0; i < tilemap->num_layers; ++i)
        fprintf(file, "\t%s %d\n", tilemap->layerinfo[i].name, tilemap->layerinfo[i].index);

    fprintf(file, "\n");

    fprintf(file, "#TILESETS_INFO\n");
    for (int i = 0; i < tilemap->num_tilesets; ++i)
        fprintf(file, "\t%s %s %d\n", tilemap->tileset[i].name, tilemap->tileset[i].image_source, i);

    fprintf(file, "\n");

    for (int l = 0; l < tilemap->num_layers; ++l) {
        fprintf(file, "%s\n", tilemap->layerinfo[l].name);
        for (int r = 0; r < tilemap->max_y; ++r) {
            fputs("\t", file);
            for (int c = 0; c < tilemap->max_x; ++c)
                fprintf(file, "%d,%d ", 0,tilemap->data[l][r][c]);
            fputs("\n\n", file);
        }
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
            fscanf(file, "%d %*[^\n]", &tilemap->num_layers);
            fscanf(file, "%d %*[^\n]", &tilemap->num_tilesets);
            fscanf(file, "%d %*[^\n]", &tilemap->map_width);
            fscanf(file, "%d %*[^\n]", &tilemap->map_height);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_height);
        } else if(strcmp(buffer, "#LAYERS_INFO") == 0){
            for (int i = 0; i < tilemap->num_layers; ++i) {
                fscanf(file, "%s %d", tilemap->layerinfo[i].name, &tilemap->layerinfo[i].index);
            }
        } else if(strcmp(buffer, "#TILESETS_INFO") == 0){
            for (int i = 0; i < tilemap->num_tilesets; ++i)
                fscanf(file, "%s %s %d", tilemap->tileset[i].name, tilemap->tileset[i].image_source, &tilemap->cur_tileset);
        } else {
            for (int i = 0; i < tilemap->num_layers; ++i) {
                if(strcmp(buffer, tilemap->layerinfo[i].name) == 0){
                    for (int r = 0; r < tilemap->max_y; ++r)
                        for (int c = 0; c < tilemap->max_x; ++c)
                            fscanf(file, "%d,%d ", &tilemap->cur_tileset,&tilemap->data[i][r][c]);
                }
            }
        }
    }

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
            fscanf(file, "%s", config->window.icon);
            continue;
        }
        if(strcmp(buffer, "fullscreen") == 0){
            fscanf(file, "%s", buffer);

            if(strcmp(buffer, "true") == 0){
                config->window.flags |= SDL_WINDOW_FULLSCREEN;
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
        if(strcmp(buffer, "map_width") == 0){
            fscanf(file, "%d", &config->window.width);
            continue;
        }
        if(strcmp(buffer, "map_height") == 0){
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

//Main Function

int main(int argc, char *args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    config_t config = {};

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    init_default_config(&config);
    read_config_file(&config, "../config.txt");

    window = SDL_CreateWindow(
            config.window.title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.window.width,
            config.window.height,
            config.window.flags
    );

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    int prev_time = 0;
    int cur_time = 0;
    int quit = 0;
    int mouseX, mouseY;
    int cur_tile = 0;

    tools_t cur_tool = Brush;
    tilemap_t *tilemap = create_tilemap("Tilemap_01", 960, 544, 32, 32);

    //Add and remove layerinfo
    add_layer(tilemap, "Layer_2", 1);
    add_layer(tilemap, "Layer_3", 2);

    add_layer(tilemap, "Layer_4", 3);
    remove_layer(tilemap, 3);

    //Add tileset
    add_tileset(tilemap, "Tileset_01", "content/tile.png", 32, 32);
    load_tileset(renderer, &tilemap->tileset[0]);

    while (!quit){
        //Time loop
        prev_time = cur_time;
        cur_time = SDL_GetTicks();

        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0){
            SDL_Scancode scanCode = event.key.keysym.scancode;
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if(scanCode == SDL_SCANCODE_ESCAPE) quit = 1;
                    if(scanCode == SDL_SCANCODE_1) tilemap->cur_layer = 0;
                    if(scanCode == SDL_SCANCODE_2) tilemap->cur_layer = 1;
                    if(scanCode == SDL_SCANCODE_3) tilemap->cur_layer = 2;
                    if(scanCode == SDL_SCANCODE_4) tilemap->cur_layer = 3;
                    if ((event.key.keysym.mod & KMOD_CTRL)) {
                        if(scanCode == SDL_SCANCODE_B) cur_tool = Brush;
                        if(scanCode == SDL_SCANCODE_E) cur_tool = Erase;
                        if(scanCode == SDL_SCANCODE_F) cur_tool = Filter;
                        if(scanCode == SDL_SCANCODE_S) cur_tool = Select;
                        if(scanCode == SDL_SCANCODE_S)
                            save_tilemap(tilemap, "../content/");
                        if(scanCode == SDL_SCANCODE_O)
                            load_tilemap(tilemap, "../content/Tilemap_01.txt");
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    cur_tile = tilemap->tileset[0].cur_tile;
                    int num_tiles = tilemap->tileset[0].num_tiles;
                    cur_tile += event.wheel.y;
                    tilemap->tileset[0].cur_tile = cur_tile < 0 ? num_tiles - 1 : cur_tile % num_tiles;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEMOTION:

                    if(event.button.button == SDL_BUTTON_LEFT){
                        switch(cur_tool){
                            case Brush:
                                put_tile(tilemap, mouseX, mouseY);
                                break;
                            case Erase:
                                remove_tile(tilemap, mouseX, mouseY);
                                break;
                            case Filter:
                                filter_tile(tilemap, mouseX, mouseY);
                                break;
                            case Select:
                                printf("SELECT\n");
                                break;
                        }
                    }
                    else if(event.button.button == SDL_BUTTON_RIGHT)
                        printf("CAMERA MOVEMENT\n");
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        render_tilemap(renderer, tilemap);
        SDL_RenderPresent(renderer);

        if(cur_time < FPS)
            SDL_Delay(cur_time - (Uint32)prev_time);
    }

    remove_tilemap(tilemap);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}