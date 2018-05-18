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

const int TILES = 9 * 5;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;

const int LEVEL_WIDTH = 960;
const int LEVEL_HEIGHT = 544;

const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 32;

const int FPS = 60;

#define NO_TILE (-1)

//Structures

typedef enum tools{
    Brush,
    Erase,
    Filter,
    Select,
} tools_t;

typedef struct grid {
    int tile;
    int x, y;
} grid_t; // remove

typedef struct tile {
    int id;
    SDL_Texture *texture;
    SDL_Rect srcrect;
    int w, h;
} tile_t;

typedef struct layer {
    int index;
    char *name;
    int hidden;
} layer_t;

typedef struct tilemap {
    grid_t data[200][200][200];
    layer_t layerinfo[8];
    tile_t *tileinfo;
    int cur_tile, cur_layer;
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

tile_t *get_tile(tile_t *tiles, int type){
    for (int i = 0; i < TILES; ++i) {
        if(tiles[i].id == type)
            return &tiles[i];
    }

    return NULL;
}

void render_tile(SDL_Renderer *renderer, grid_t *tilemap, tile_t *tiles){

    if(tilemap->tile == NO_TILE)
        return;

    tile_t *tile = get_tile(tiles, tilemap->tile);
    SDL_Rect desR = (SDL_Rect){tilemap->x, tilemap->y, tile->w, tile->h};
    SDL_RenderCopy(renderer, tile->texture, &tile->srcrect, &desR);
}

void put_tile(grid_t *tilemap, int x, int y, int tile){

    if(tilemap->tile == tile)
        return;

    tilemap->tile = tile;
    tilemap->x = snap_to_grid(x, TILE_WIDTH, 0);
    tilemap->y = snap_to_grid(y, TILE_HEIGHT, 0);
}

void remove_tile(grid_t *tilemap){

    if(tilemap->tile == NO_TILE)
        return;

    tilemap->tile = NO_TILE;
    tilemap->x = tilemap->y = 0;
}

void filter_tile(grid_t *tilemap,int *cur_tile){

    if(tilemap->tile == NO_TILE)
        return;

    *cur_tile = tilemap->tile;
}

void save_file(){
    // FILE
    FILE *file = fopen("content/map.txt", "w");

    if(file){
        //WRITE LAYERS
        fprintf(file, "-LAYERS\n");
        for (int i = 0; i < LAYERS; ++i)
            fprintf(file, "\tLAYER_%d %d\n", i+1, i);

        //WRITE SPRITE_SHEET
        fprintf(file, "-SPRITE_SHEET\n");
        fprintf(file, "\tcontent/tile2.png");
        fprintf(file, " //sprite sheet path\n");

        //WRITE SIZES
        fprintf(file, "-SIZES\n");;
        fprintf(file,"\t%d //width\n", LEVEL_WIDTH);
        fprintf(file,"\t%d //height\n", LEVEL_HEIGHT);
        fprintf(file,"\t%d //tile size\n", TILE_WIDTH);

        //WRITE TILEMAP PER LAYERS
        for (int j = 0; j < LAYERS; ++j) {
            fprintf(file, "-LAYER_%d\n", j+1);
            for (int i = 0; i < TILES_Y; ++i) {
                fputs("\t", file);
                for (int k = 0; k < TILES_X; ++k)
                    fprintf(file, "%d", tilemap[k][i][j].tile);
                fputs("\n", file);
            }
        }
        fclose(file);
    }
    else {
        printf("Couldn't open file\n");
    }
}

void read_file() {
    char *file_content;
    FILE *file = fopen("content/map.txt", "r");

    if (file) {
        fseek(file, 0, SEEK_END);
        int len = ftell(file);
        fseek(file, 0, SEEK_SET);

        printf("length: %i\n", len);

        file_content = calloc(len, sizeof(char));
        fread((void *) file_content, sizeof(char), len, file);

        printf("content: %s\n", file_content);

        const int SEEK_TITLE = 0;
        const int READ_TITLE = 1;
        const int READ_CONTENT = 2;

        int STATE = SEEK_TITLE;

        char title_buffer[256] = {};
        int title_index = 0;

        int file_content_index = 0;
        char cur = file_content[file_content_index];
        do {
            if (STATE == SEEK_TITLE) {
                if (cur == '-') {
                    STATE = READ_TITLE;
                }
            } else if (STATE == READ_TITLE) {
                if (cur == '\n') {
                    title_buffer[title_index + 1] = '\0';

                    title_index = 0;
                    STATE = READ_CONTENT;
                } else {
                    title_buffer[title_index++] = cur;
                }
            } else if (STATE == READ_CONTENT) {
                if (strcmp(title_buffer, "LAYERS") == 0) {
                    printf("Reading LAYERS\n");

                    title_index = 0;
                    STATE = SEEK_TITLE;
                }
                if (strcmp(title_buffer, "SPRITE_SHEET") == 0) {
                    printf("Reading SPRITE_SHEET\n");

                    title_index = 0;
                    STATE = SEEK_TITLE;
                }
                if (strcmp(title_buffer, "SIZES") == 0) {
                    printf("Reading SPRITE_SHEET\n");

                    title_index = 0;
                    STATE = SEEK_TITLE;
                }
            }

            cur = file_content[++file_content_index];
        } while (cur);

        fclose(file);
        free(file_content);
    } else {
        printf("Couldn't open file\n");
    }
}

//Main Function

int main(int argc, char *args[]) {

    read_file();

    const int TILES_X = LEVEL_WIDTH / TILE_WIDTH;
    const int TILES_Y = LEVEL_HEIGHT / TILE_HEIGHT;
    const int LAYERS = 4;

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

    int cur_tile = 0;
    int cur_layer = 0;
    int mouseX, mouseY;

    tools_t cur_tool = Brush;
    grid_t tilemap[TILES_X][TILES_Y][LAYERS];
    tile_t *tiles = (tile_t *) malloc(TILES * sizeof(tile_t));

    for (int i = 0; i < TILES_X * TILES_Y * LAYERS; ++i)
        tilemap[0][0][i].tile = NO_TILE;

    SDL_Texture *texture = load_texture(renderer, "content/tile2.png");
    SDL_Rect *spritesheet = split_image(texture, 9, 5);

    for (int i = 0; i < TILES; ++i) {
        tiles[i].id = i;
        tiles[i].texture = texture;
        tiles[i].srcrect.x = spritesheet[i].x;
        tiles[i].srcrect.y = spritesheet[i].y;
        tiles[i].srcrect.w = spritesheet[i].w;
        tiles[i].srcrect.h = spritesheet[i].h;
        tiles[i].w = tiles[i].h = TILE_WIDTH;
    }

    while (!quit){
        //Time loop
        prev_time = cur_time;
        cur_time = SDL_GetTicks();

        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0){
            SDL_Scancode scanCode = event.key.keysym.scancode;
            int c, r, l;
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if(scanCode == SDL_SCANCODE_ESCAPE) quit = 1;
                    if(scanCode == SDL_SCANCODE_1) cur_layer = 0;
                    if(scanCode == SDL_SCANCODE_2) cur_layer = 1;
                    if(scanCode == SDL_SCANCODE_3) cur_layer = 2;
                    if(scanCode == SDL_SCANCODE_4) cur_layer = 3;
                    if ((event.key.keysym.mod & KMOD_CTRL)) {
                        if(scanCode == SDL_SCANCODE_B) cur_tool = Brush;
                        if(scanCode == SDL_SCANCODE_E) cur_tool = Erase;
                        if(scanCode == SDL_SCANCODE_F) cur_tool = Filter;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    cur_tile += event.wheel.y;
                    cur_tile = cur_tile < 0 ? TILES - 1 : cur_tile % TILES;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEMOTION:
                    c = (mouseX / TILE_WIDTH);
                    r = (mouseY / TILE_HEIGHT);
                    l = cur_layer;

                    if(event.button.button == SDL_BUTTON_LEFT){
                        switch(cur_tool){
                            case Brush:
                                put_tile(&tilemap[c][r][l], mouseX, mouseY, cur_tile);
                                break;
                            case Erase:
                                remove_tile(&tilemap[c][r][l]);
                                break;
                            case Filter:
                                filter_tile(&tilemap[c][r][l], &cur_tile);
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

        for (int j = 0; j < LAYERS; ++j)
            for (int i = 0; i < TILES_X * TILES_Y; ++i){
                    if(cur_tool == Brush && j == cur_layer){
                        tile_t *tile = get_tile(tiles, cur_tile);
                        SDL_Rect dstrect = {};
                        dstrect.x = snap_to_grid(mouseX, TILE_WIDTH, 0);
                        dstrect.y = snap_to_grid(mouseY, TILE_HEIGHT, 0);
                        dstrect.w = dstrect.h = TILE_WIDTH;
                        SDL_RenderCopy(renderer, tile->texture, &tile->srcrect, &dstrect);
                    }
                    render_tile(renderer, &tilemap[0][i][j], tiles);
            }

        SDL_RenderPresent(renderer);

        if(cur_time < FPS)
            SDL_Delay(cur_time - (Uint32)prev_time);
    }

    free(tiles);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}