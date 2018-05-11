#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <SDL.h>
#include <SDL_image.h>

//Const & Defines
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;

const int LEVEL_WIDTH = 960;
const int LEVEL_HEIGHT = 544;

const int GRID_WIDTH = 32;
const int GRID_HEIGHT = 32;

const int FPS = 60;

const int TILES = 9 * 5;

//Structures
typedef enum options{
    Brush,
    Erase,
    Filter
} options_t;

typedef struct scale {
    float w, h;
} scale_t;

typedef struct vector {
    float x, y;
} vector_t;

typedef struct color {
    int r, g, b, a;
} color_t;

typedef struct grid {
    int type;
    color_t color;
    vector_t position;
    scale_t size;
} grid_t;

typedef struct tile {
    int type;
    SDL_Texture *texture;
    SDL_Rect *spritesheet;
    SDL_Rect srcR;
} tile_t;

//Functions
SDL_Rect *slice_array(SDL_Rect *array, int start, int end) {
    int numElements = (end - start + 1);
    size_t numBytes = sizeof(SDL_Rect) * numElements;

    SDL_Rect *slice = malloc(numBytes);
    memcpy(slice, array + start, numBytes);

    return slice;
}

SDL_Texture *load_texture(SDL_Renderer *renderer, char *path){
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect *split_image(SDL_Rect *rect, int column, int row) {

    SDL_Rect *image = (SDL_Rect *) malloc((column * row) * sizeof(SDL_Rect));
    int splitWidth = rect->w / column;
    int splitHeight = rect->h / row;
    int i = 0;
#if DEBUG
    printf("RectWidth: %d RectHeight: %d\n", rect->w, rect->h);
    printf("SplitWidth: %d SplitHeight: %d\n", splitWidth, splitHeight);
#endif

    for (int y = 0; y < row; ++y) {
        for (int x = 0; x < column; ++x) {
            (image + i)->x = splitWidth * x;
            (image + i)->y = splitHeight * y;
            (image + i)->w = splitWidth;
            (image + i)->h = splitHeight;
#if DEBUG
            printf("SplitImage: %d PositionX: %d PositionY: %d\n", i, (image + i)->x, (image + i)->y);
#endif
            i++;
        }
    }

    return image;
}

int snap_to_grid(int value, int increment, int offset){
    return (value/increment) * increment + offset;
}

color_t get_color(int type){

    switch (type){
        case 0: return (color_t){0, 0, 0, 255};
        case 1: return (color_t){255, 0, 0, 255};
        case 2: return (color_t){0, 255, 0, 255};
        case 3: return (color_t){0, 0, 255, 255};
        default: return (color_t){255, 255, 255, 255};
    }
}

tile_t *get_tile(tile_t *tiles, int type){
    for (int i = 0; i < TILES; ++i) {
        if(tiles[i].type == type)
            return &tiles[i];
    }

    return NULL;
}

void render_tile(SDL_Renderer *renderer, grid_t *grid, tile_t *tiles){

    if(grid->type == -1)
        return;

    tile_t *tile = get_tile(tiles, grid->type);
    SDL_Rect desR = {};
    desR.x = (int)grid->position.x;
    desR.y = (int)grid->position.y;
    desR.w = (int)grid->size.w;
    desR.h = (int)grid->size.h;
    SDL_RenderCopy(renderer, tile->texture, &tile->srcR, &desR);

    //SDL_SetRenderDrawColor(renderer, (Uint8)grid->color.r, (Uint8)grid->color.g, (Uint8)grid->color.b, (Uint8)grid->color.a);
    //SDL_Rect tileRect = {(int)grid->position.x, (int)grid->position.y, (int)grid->size.w, (int)grid->size.h};
    //SDL_RenderFillRect(renderer, &tileRect);
}

void put_tile(grid_t *grid, int x, int y, int type){

    if(grid->type == type)
        return;

    grid->position.x = snap_to_grid(x, GRID_WIDTH, 0);
    grid->position.y = snap_to_grid(y, GRID_HEIGHT, 0);
    grid->size.w = GRID_WIDTH;
    grid->size.h = GRID_HEIGHT;
    grid->type = type;
    //grid->color = get_color(type);
}

void remove_tile(grid_t *grid){

    if(grid->type == -1)
        return;

    grid->type = -1;
    grid->position = (vector_t){0, 0};
    grid->size = (scale_t){0, 0};
    //grid->color = (color_t){0, 0, 0, 0};
}

void filter_tile(grid_t *grid,int *currentType){

    if(grid->type == -1)
        return;

    *currentType = grid->type;
}

void clean_up(grid_t *grid, int length){
    for (int i = 0; i < length; ++i)
        remove_tile(&grid[i]);
}

//Main Function
int main(int argc, char *args[]) {
    const int TOTAL_TILE_X = LEVEL_WIDTH / GRID_WIDTH;
    const int TOTAL_TILE_Y = LEVEL_HEIGHT / GRID_HEIGHT;
    const int TOTAL_TILE_Z = 4;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Tilemap Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int prevTime = 0;
    int currentTime = 0;
    int quit = 0;

    int currentType = 0;
    int currentLayer = 0;
    int mouseX, mouseY;

    grid_t grids[TOTAL_TILE_X][TOTAL_TILE_Y][TOTAL_TILE_Z];
    tile_t *tiles = (tile_t *) malloc(TILES * sizeof(tile_t));

    options_t options = Brush;

    for (int j = 0; j < TOTAL_TILE_Z; ++j)
        for (int i = 0; i < TOTAL_TILE_Y; ++i)
            for (int k = 0; k < TOTAL_TILE_X; ++k)
                grids[k][i][j].type = -1;

    SDL_Texture *texture = load_texture(renderer, "content/tile2.png");

    SDL_Rect rect = {0, 0, 0, 0};
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_Rect *spritesheet = split_image(&rect, 9, 5);

    for (int i = 0; i < TILES; ++i) {
        tiles[i].type = i;
        tiles[i].texture = texture;
        tiles[i].srcR.x = spritesheet[i].x;
        tiles[i].srcR.y = spritesheet[i].y;
        tiles[i].srcR.w = spritesheet[i].w;
        tiles[i].srcR.h = spritesheet[i].h;
    }

    while (!quit){
        //Time loop
        prevTime = currentTime;
        currentTime = SDL_GetTicks();

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
                    if(scanCode == SDL_SCANCODE_1) currentLayer = 0;
                    if(scanCode == SDL_SCANCODE_2) currentLayer = 1;
                    if(scanCode == SDL_SCANCODE_3) currentLayer = 2;
                    if(scanCode == SDL_SCANCODE_4) currentLayer = 3;
                    if ((event.key.keysym.mod & KMOD_CTRL)) {
                        if(scanCode == SDL_SCANCODE_B) options = Brush;
                        if(scanCode == SDL_SCANCODE_E) options = Erase;
                        if(scanCode == SDL_SCANCODE_F) options = Filter;
                        //if(scanCode == SDL_SCANCODE_0) clean_up(grids, TOTAL_TILES);
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    currentType += event.wheel.y;
                    currentType = currentType < 0 ? TILES - 1 : currentType % TILES;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEMOTION:
                    c = (mouseX / GRID_WIDTH);
                    r = (mouseY / GRID_HEIGHT);
                    l = currentLayer;

                    if(event.button.button == SDL_BUTTON_LEFT){
                        //int index = (TOTAL_TILE_X * r) + c;

                        switch(options){
                            case Brush:
                                put_tile(&grids[c][r][l], mouseX, mouseY, currentType);
                                break;
                            case Erase:
                                remove_tile(&grids[c][r][l]);
                                break;
                            case Filter:
                                filter_tile(&grids[c][r][l], &currentType);
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

        SDL_SetRenderDrawColor(renderer, (Uint8)255, (Uint8)255, (Uint8)255, (Uint8)255);
        SDL_RenderClear(renderer);

        for (int j = 0; j < TOTAL_TILE_Z; ++j)
            for (int i = 0; i < TOTAL_TILE_Y; ++i)
                for (int k = 0; k < TOTAL_TILE_X; ++k){
                    if(options == Brush && j == currentLayer){
                        SDL_Rect desR = {};
                        tile_t *tile = get_tile(tiles, currentType);
                        desR.x = snap_to_grid(mouseX, GRID_WIDTH, 0);
                        desR.y = snap_to_grid(mouseY, GRID_HEIGHT, 0);
                        desR.w = desR.h = GRID_WIDTH;
                        SDL_RenderCopy(renderer, tile->texture, &tile->srcR, &desR);
                    }
                    render_tile(renderer, &grids[k][i][j], tiles);
                }

        SDL_RenderPresent(renderer);

        if(currentTime < FPS)
            SDL_Delay(currentTime - (Uint32)prevTime);
    }

    free(tiles);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}