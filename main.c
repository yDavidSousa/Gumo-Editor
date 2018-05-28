#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <SDL.h>
#include <SDL_image.h>

/*
 * int index = (TILES_X * r) + c;
 * const char *PATH = "C:\\Users\\David Sousa\\Documents\\Projects\\Games\\Tilemap-Editor\\content\\map.txt";
 * */

//Const & Defines
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;
const int FPS = 60;

#define MAX_TILE_X 200
#define MAX_TILE_Y 200
#define MAX_LAYERS 8
#define EMPTY_TILE (-1)

//Structures

typedef enum tools{
    Brush,
    Erase,
    Filter,
    Select,
} tools_t;

typedef struct tile {
    int id;
    SDL_Texture *texture;
    SDL_Rect srcrect;
    int w, h;
} tile_t;

typedef struct layer {
    int index;
    char name[256];
    int hidden;
} layer_t;

typedef struct tilemap {
    int data[MAX_LAYERS][MAX_TILE_Y][MAX_TILE_X];
    layer_t layerinfo[MAX_LAYERS];
    tile_t *tileinfo;
    int cur_tile, cur_layer;
    int max_x, max_y, num_layers;
    int width, height;
    int num_tiles;
    char tile_spritesheet[256];
} tilemap_t;

//Util Functions

SDL_Texture *load_texture(SDL_Renderer *renderer, char *path){
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect *split_image(SDL_Texture *texture, int column, int row) {

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

int snap_to_grid(int value, int increment, int offset){
    return (value/increment) * increment + offset;
}

//Tilemap Functions

tile_t *get_tile(tilemap_t *tilemap, int type){
    for (int i = 0; i < tilemap->num_tiles; ++i) {
        if(tilemap->tileinfo[i].id == type)
            return &tilemap->tileinfo[i];
    }

    return NULL;
}

tilemap_t *create_tilemap(const int width, const int height, const int tile_width, const int tile_height, const int num_tiles, const int num_layers){
    tilemap_t *tilemap = malloc(sizeof(tilemap_t));

    tilemap->width = width;
    tilemap->height = height;
    tilemap->max_x = width / tile_width;
    tilemap->max_y = height / tile_height;
    tilemap->num_tiles = num_tiles;
    tilemap->num_layers = num_layers;
    tilemap->cur_tile = 0;
    tilemap->cur_layer = 0;

    for (int l = 0; l < tilemap->num_layers; ++l)
        for (int r = 0; r < tilemap->max_y; ++r)
            for (int c = 0; c < tilemap->max_x; ++c)
                tilemap->data[l][r][c] = EMPTY_TILE;

    return tilemap;
}

void render_tilemap(SDL_Renderer *renderer, tilemap_t *tilemap){
    for (int l = 0; l < tilemap->num_layers; ++l)
        for (int r = 0; r < tilemap->max_y; ++r)
            for (int c = 0; c < tilemap->max_x; ++c) {

                if(tilemap->data[l][r][c] == EMPTY_TILE)
                    continue;

                tile_t *tile = get_tile(tilemap, tilemap->data[l][r][c]);

                int pos_x = c * tile->w;
                int pos_y = r * tile->h;

                SDL_Rect desR = (SDL_Rect){ pos_x, pos_y, tile->w, tile->h};
                SDL_RenderCopy(renderer, tile->texture, &tile->srcrect, &desR);
            }
}

void put_tile(tilemap_t *tilemap, int x, int y){
    tile_t *tile = get_tile(tilemap, tilemap->cur_tile);
    int c = (x / tile->w);
    int r = (y / tile->h);
    int l = tilemap->cur_layer;

    if(tilemap->cur_tile == tilemap->data[l][r][c])
        return;

    tilemap->data[l][r][c] = tile->id;
}

void remove_tile(tilemap_t *tilemap, int x, int y){
    tile_t *tile = get_tile(tilemap, tilemap->cur_tile);
    int c = (x / tile->w);
    int r = (y / tile->h);
    int l = tilemap->cur_layer;

    if(tilemap->data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->data[l][r][c] = EMPTY_TILE;
}

void filter_tile(tilemap_t *tilemap, int x, int y){
    tile_t *tile = get_tile(tilemap, tilemap->cur_tile);
    int c = (x / tile->w);
    int r = (y / tile->h);
    int l = tilemap->cur_layer;

    if(tilemap->data[l][r][c] == EMPTY_TILE)
        return;

    tilemap->cur_tile = tilemap->data[l][r][c];
}

void save_file(tilemap_t *tilemap, char *file_path){
    FILE *file = fopen(file_path, "w");

    if(file){

        //WRITE SIZES
        fprintf(file, "-OPTIONS\n");
        fprintf(file,"\t%d //number of layers\n", tilemap->num_layers);
        fprintf(file,"\t%d //width\n", tilemap->width);
        fprintf(file,"\t%d //height\n", tilemap->height);
        fprintf(file,"\t%d //tile size\n", 32);

        //WRITE LAYERS
        fprintf(file, "-LAYERS\n");
        for (int i = 0; i < tilemap->num_layers; ++i)
            fprintf(file, "\tLAYER_%d %d\n", i+1, i);

        //WRITE SPRITE_SHEET
        fprintf(file, "-SPRITE_SHEET\n");
        fprintf(file, "\tcontent/tile2.png");
        fprintf(file, " //sprite sheet path\n");

        //WRITE TILEMAP PER LAYERS
        for (int l = 0; l < tilemap->num_layers; ++l) {
            fprintf(file, "-LAYER_%d\n", l+1);
            for (int r = 0; r < tilemap->max_y; ++r) {
                fputs("\t", file);
                for (int c = 0; c < tilemap->max_x; ++c)
                    fprintf(file, "%d ", tilemap->data[l][r][c]);
                fputs("\n", file);
            }
        }
        printf("---- SAVED ----\n");
        fclose(file);
    }
    else {
        printf("Couldn't open file\n");
    }

}

void read_file(tilemap_t *tilemap, char *file_path) {
    char *file_content;
    FILE *file = fopen(file_path, "r");

    if (file) {
        fseek(file, 0, SEEK_END);
        int file_length = ftell(file);
        fseek(file, 0, SEEK_SET);

        file_content = calloc((size_t)file_length, sizeof(char));
        fread((void *) file_content, sizeof(char), (size_t)file_length, file);
        fclose(file);

#if DEBUG
        printf("length: %i\n", file_length);
        printf("content: %s\n", file_content);
#endif

        const int SEEK_TITLE = 0;
        const int READ_TITLE = 1;
        const int READ_CONTENT = 2;

        int STATE = SEEK_TITLE;

        int title_index = 0;
        char title_buffer[256] = {};

        int file_content_index = 0;
        char cur_file = file_content[file_content_index];

        do {
            if (STATE == SEEK_TITLE) {
                if (cur_file == '-')
                    STATE = READ_TITLE;
            } else if (STATE == READ_TITLE) {
                if (cur_file == '\n') {
                    title_buffer[title_index] = '\0';

                    title_index = 0;
                    STATE = READ_CONTENT;
                } else
                    title_buffer[title_index++] = cur_file;
            } else if (STATE == READ_CONTENT) {
                if(strcmp(title_buffer, "OPTIONS") == 0){
                    printf("-> Reading OPTIONS:\n");

                    sscanf(file_content + file_content_index, "%d\n%d\n%d\n%d", &tilemap->num_layers, &tilemap->width, &tilemap->height, &tilemap->max_x);

                    //sscanf(&file_content[file_content_index], "%i", &tilemap->num_layers);
                    //sscanf(&file_content[file_content_index + 8], "%i", &tilemap->width);
                    //sscanf(&file_content[file_content_index + 16], "%i", &tilemap->height);
                    //sscanf(&file_content[file_content_index + 24], "%i", &tilemap->max_x);

#if DEBUG
                    printf("num_layer: %i\n", tilemap->num_layers);
                    printf("width: %i\n", tilemap->width);
                    printf("height: %i\n", tilemap->height);
                    printf("tile size: %i\n", tilemap->max_x);
#endif

                    title_index = 0;
                    STATE = SEEK_TITLE;
                } else if (strcmp(title_buffer, "LAYERS") == 0) {
                        printf("-> Reading LAYERS:\n");
                    fseek(file, file_content_index, SEEK_SET);

                    for (int i = 0; i < tilemap->num_layers; ++i){
                        sscanf(file_content + file_content_index, "%s %d", tilemap->layerinfo[i].name, &tilemap->layerinfo[i].index);
                        //fscanf(file, "%s %d", tilemap->layerinfo[i].name, &tilemap->layerinfo[i].index);
                        printf("Name: %s Index: %d\n", tilemap->layerinfo[i].name, tilemap->layerinfo[i].index);
                    }

                    //file_content_index = ftell(file);
                    title_index = 0;
                    STATE = SEEK_TITLE;
                } else if (strcmp(title_buffer, "SPRITE_SHEET") == 0) {
                    printf("-> Reading SPRITE_SHEET:\n");
                    fseek(file, file_content_index, SEEK_SET);

                    fscanf(file, "%s", tilemap->tile_spritesheet);
                    printf("spritesheet: %s\n", tilemap->tile_spritesheet);

                    //file_content_index = ftell(file);
                    title_index = 0;
                    STATE = SEEK_TITLE;
                } else if (strcmp(title_buffer, "LAYER_1") == 0) {
                    printf("-> Reading LAYER_1:\n");
                    fseek(file, file_content_index+2, SEEK_SET);

                    for (int l = 0; l < tilemap->num_layers; ++l) {
                        for (int r = 0; r < tilemap->max_y; ++r) {
                            for (int c = 0; c < tilemap->max_x; ++c) {
                                fscanf(file, "%d ", &tilemap->data[l][r][c]);
                                //printf("|%d|", tilemap->data[l][r][c]);
                            }
                        }
                    }

                    //file_content_index = ftell(file);
                    title_index = 0;
                    STATE = SEEK_TITLE;
                }
            }

            //printf("file content index: %d\n", file_content_index);
            cur_file = file_content[++file_content_index];
        } while (cur_file != EOF);

        fclose(file);
        free(file_content);
    } else {
        printf("Couldn't open file\n");
    }
}

//Main Function

int main(int argc, char *args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Tilemap Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int prev_time = 0;
    int cur_time = 0;
    int quit = 0;
    int mouseX, mouseY;

    tools_t cur_tool = Brush;
    tilemap_t *tilemap = create_tilemap(960, 544, 32, 32, 45, 4);

    //TODO: Put this in function too
    SDL_Texture *texture = load_texture(renderer, "content/tile2.png");
    SDL_Rect *spritesheet = split_image(texture, 9, 5);
    tile_t *tiles = (tile_t *) malloc(tilemap->num_tiles * sizeof(tile_t));

    for (int i = 0; i < tilemap->num_tiles; ++i) {
        tiles[i].id = i;
        tiles[i].texture = texture;
        tiles[i].srcrect.x = spritesheet[i].x;
        tiles[i].srcrect.y = spritesheet[i].y;
        tiles[i].srcrect.w = spritesheet[i].w;
        tiles[i].srcrect.h = spritesheet[i].h;
        tiles[i].w = tiles[i].h = 32;
    }

    tilemap->tileinfo = tiles;

    int cur_tile = 0;
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
                        if(scanCode == SDL_SCANCODE_S) save_file(tilemap, "content/map.txt");
                        if(scanCode == SDL_SCANCODE_O) read_file(tilemap, "content/map.txt");
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    cur_tile = tilemap->cur_tile;
                    cur_tile += event.wheel.y;
                    tilemap->cur_tile = cur_tile < 0 ? tilemap->num_tiles - 1 : cur_tile % tilemap->num_tiles;
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

    free(tilemap);
    free(spritesheet);
    free(tiles);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}