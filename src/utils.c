#include <malloc.h>
#include <utils.h>

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

float mathf_min(const float a, const float b){
    return (a <= b) ? a : b;
}

float mathf_max(const float a, const float b){
    return (a >= b) ? a : b;
}

void mathf_clamp(int *value,const int min,const int max){
    if(*(value) <= min)
        *(value) = min;
    else if (*value >= max)
        *(value) = max;
    else
        *(value) = *value;
}

bool mathf_range(float value, float min, float max){
    return (value >= mathf_min(min, max) && value <= mathf_max(min, max)) ? true : false;
}