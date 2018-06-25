#ifndef ENTITY_H
#define ENTITY_H

#include <SDL_render.h>

#define MAX_ENTITIES 100
#define EMPTY_ENTITY (-1)

typedef struct entity_data {
    int id;
    char name[256];
    short link;
} entity_data_t;

int num_entities;

void add_entity(const char *name, short link);

void remove_entity(int index);

void render_entity(SDL_Renderer *renderer, int id, const SDL_Rect *dst_rect);

entity_data_t *get_entity(int id);

#endif