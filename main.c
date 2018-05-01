#include <stdio.h>
#include <malloc.h>
#include <SDL.h>

//Const & Defines
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int LEVEL_WIDTH = 640;
const int LEVEL_HEIGHT = 480;

const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 32;

const int TOTAL_TILES = 300;
const int FPS = 60;

//Structures
typedef enum options{
    Brush,
    Erase
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

typedef struct tile {
    int type;
    color_t color;
    vector_t position;
    scale_t size;
} tile_t;

//Functions
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

void render_tile(SDL_Renderer *renderer, tile_t *tile){

    if(tile->type == -1)
        return;

    SDL_SetRenderDrawColor(renderer, (Uint8)tile->color.r, (Uint8)tile->color.g, (Uint8)tile->color.b, (Uint8)tile->color.a);
    SDL_Rect tileRect = {(int)tile->position.x, (int)tile->position.y, (int)tile->size.w, (int)tile->size.h};
    SDL_RenderFillRect(renderer, &tileRect);
}

void put_tile(tile_t *tile, int x, int y, int type){

    if(tile->type == type)
        return;

    tile->position.x = snap_to_grid(x, TILE_WIDTH, 0);
    tile->position.y = snap_to_grid(y, TILE_HEIGHT, 0);
    tile->size.w = TILE_WIDTH;
    tile->size.h = TILE_HEIGHT;
    tile->type = type;
    tile->color = get_color(type);
}

void remove_tile(tile_t *tile){

    if(tile->type == -1)
        return;

    tile->type = -1;
    tile->position = (vector_t){0, 0};
    tile->size = (scale_t){0, 0};
    tile->color = (color_t){0, 0, 0, 0};
}

void clean_up(tile_t *tile){
    for (int i = 0; i < TOTAL_TILES; ++i)
        remove_tile(&tile[i]);
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

    int prevTime = 0;
    int currentTime = 0;
    int quit = 0;

    int currentType = 0;
    int mouseX, mouseY;
    int gridX = SCREEN_WIDTH / TILE_WIDTH;

    options_t options = Brush;
    tile_t *grid = (tile_t *) malloc(TOTAL_TILES * sizeof(tile_t));

    for (int i = 0; i < TOTAL_TILES; ++i)
        grid[i].type = -1;

    while (!quit){
        //Time loop
        prevTime = currentTime;
        currentTime = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0){
            SDL_Scancode scanCode = event.key.keysym.scancode;
            int c, r;
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if(scanCode == SDL_SCANCODE_ESCAPE) quit = 1;
                    if(scanCode == SDL_SCANCODE_1) options = Brush;
                    if(scanCode == SDL_SCANCODE_2) options = Erase;
                    if(scanCode == SDL_SCANCODE_0) clean_up(grid);
                    break;
                case SDL_MOUSEWHEEL:
                    currentType += event.wheel.y;
                    currentType %= 3;
                    break;
                case SDL_MOUSEMOTION:
                    SDL_GetMouseState(&mouseX, &mouseY);
                    c = (mouseX / TILE_WIDTH);
                    r = (mouseY / TILE_HEIGHT);

                    if(event.button.button == SDL_BUTTON_LEFT){
                        int index = (gridX * r) + c;

                        //printf("Column: %d Row: %d\n", c, r);
                        //printf("Tile selected: %d\n", index);

                        if(options == Brush)
                            put_tile(&grid[index], mouseX, mouseY, currentType);
                        if(options == Erase)
                            remove_tile(&grid[index]);
                    }
                    else if(event.button.button == SDL_BUTTON_RIGHT)
                        printf("CAMERA MOVEMENT\n");
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_GetMouseState(&mouseX, &mouseY);
                    c = (mouseX / TILE_WIDTH);
                    r = (mouseY / TILE_HEIGHT);

                    if(event.button.button == SDL_BUTTON_LEFT){
                        int index = (gridX * r) + c;

                        //printf("Column: %d Row: %d\n", c, r);
                        //printf("Tile selected: %d\n", index);

                        if(options == Brush)
                            put_tile(&grid[index], mouseX, mouseY, currentType);
                        if(options == Erase)
                            remove_tile(&grid[index]);
                    } else if(event.button.button == SDL_BUTTON_RIGHT)
                        printf("CAMERA MOVEMENT\n");
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, (Uint8)255, (Uint8)255, (Uint8)255, (Uint8)255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < TOTAL_TILES; ++i)
            render_tile(renderer, &grid[i]);

        SDL_RenderPresent(renderer);

        if(currentTime < FPS)
            SDL_Delay(currentTime - (Uint32)prevTime);
    }

    free(grid);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}