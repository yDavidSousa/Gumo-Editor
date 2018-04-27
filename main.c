#include <stdio.h>
#include <malloc.h>
#include <SDL.h>

// Brush - 1 | Erase - 2

//fazer marcação tile
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int LEVEL_WIDTH = 640;
const int LEVEL_HEIGHT = 480;

const int TILE_WIDTH = 32;
const int TILE_HEIGHT = 32;

const int TOTAL_TILES = 35;

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

color_t get_color(int type){
    color_t result;

    switch (type){
        case 0:
            result = (color_t){0, 0, 0, 255};
            break;
        case 1:
            result = (color_t){255, 0, 0, 255};
            break;
        case 2:
            result = (color_t){0, 255, 0, 255};
            break;
        case 3:
            result = (color_t){0, 0, 255, 255};
            break;
        default:
            break;
    }

    return  result;
}

int snap_to_grid(int value, int increment, int offset){
    return (value/increment) * increment + offset;
}

void erase_tile(tile_t *tile){

    if(tile->type == -1)
        return;

    tile->type = -1;
    tile->position = (vector_t){0, 0};
    tile->size = (scale_t){0, 0};
    tile->color = (color_t){0, 0, 0, 0};
}

void set_tile(tile_t *tile, int x, int y, int type){
    tile->position.x = snap_to_grid(x, TILE_WIDTH, 0);
    tile->position.y = snap_to_grid(y, TILE_HEIGHT, 0);
    tile->size.w = TILE_WIDTH;
    tile->size.h = TILE_HEIGHT;
    tile->type = type;
    tile->color = get_color(type);
}

void render_tile(SDL_Renderer *renderer, tile_t *tile){

    if(tile->type == -1)
        return;

    SDL_SetRenderDrawColor(renderer, (Uint8)tile->color.r, (Uint8)tile->color.g, (Uint8)tile->color.b, (Uint8)tile->color.a);
    SDL_Rect tileRect = {(int)tile->position.x, (int)tile->position.y, (int)tile->size.w, (int)tile->size.h};
    SDL_RenderFillRect(renderer, &tileRect);
}

int main(int argc, char *args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    int quit = 0;

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Tilemap Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int currentType = 0;
    int mouseX, mouseY;
    int tileX = SCREEN_WIDTH / TILE_WIDTH;
    int tileY = SCREEN_HEIGHT / TILE_HEIGHT;
    tile_t tiles[tileX][tileY];

    int function = 0;

    while (!quit){
        SDL_SetRenderDrawColor(renderer, (Uint8)255, (Uint8)255, (Uint8)255, (Uint8)255);
        SDL_RenderClear(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0){
            SDL_Scancode scanCode = event.key.keysym.scancode;

            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if(scanCode == SDL_SCANCODE_1) function = 0;
                    if(scanCode == SDL_SCANCODE_2) function = 1;
                    break;
                default:
                    break;
            }

            if(event.type == SDL_MOUSEWHEEL)
            {
                if(event.wheel.y > 0)
                    currentType++;
                else if(event.wheel.y < 0)
                    currentType--;

                currentType >= 3 ? 0 : currentType;
            }
            else if(event.type == SDL_MOUSEMOTION){
                SDL_GetMouseState(&mouseX, &mouseY);

                if(event.button.button == SDL_BUTTON_LEFT){
                    int tX = (mouseX / TILE_WIDTH);
                    int tY = (mouseY / TILE_HEIGHT);

                    if(function == 0)
                        set_tile(&tiles[tX][tY], mouseX, mouseY, currentType);
                    else
                        erase_tile(&tiles[tX][tY]);
                }
                else if(event.button.button == SDL_BUTTON_RIGHT){
                    printf("CAMERA MOVEMENT");
                }
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                SDL_GetMouseState(&mouseX, &mouseY);

                if(event.button.button == SDL_BUTTON_LEFT){
                    int tX = (mouseX / TILE_WIDTH);
                    int tY = (mouseY / TILE_HEIGHT);

                    if(function == 0)
                        set_tile(&tiles[tX][tY], mouseX, mouseY, currentType);
                    else
                        erase_tile(&tiles[tX][tY]);
                }
                else if(event.button.button == SDL_BUTTON_RIGHT){
                        printf("CAMERA MOVEMENT");
                }
            }
        }

       for (int x = 0; x < tileX; ++x) {
          for (int y = 0; y < tileY; ++y) {
            render_tile(renderer, &tiles[x][y]);
      }
}

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}