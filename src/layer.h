#ifndef LAYER_H
#define LAYER_H

#include <stdbool.h>
#include <SDL_types.h>

#define MAX_LAYERS 8

typedef enum {
    TILES,
    ENTITIES
} layer_type_t;

enum layer_flags {
    LAYER_HIDDEN = 0x00000001,
    LAYER_LOCKED = 0x00000002
};

typedef struct layer_data {
    int id;
    char name[256];
    layer_type_t type;
    Uint32 flags;
} layer_data_t;

bool layer_has_flags(const layer_data_t *layer_data, Uint32 flags);

void layer_set_name(layer_data_t *layer_data, const char *name);

#endif
