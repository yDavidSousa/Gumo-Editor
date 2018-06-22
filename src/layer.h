#ifndef LAYER_H
#define LAYER_H

#include <stdbool.h>
#include <SDL_types.h>

#define MAX_LAYERS 8

#define TILES_LAYER_TYPE "TILES"
#define ENTITIES_LAYER_TYPE "ENTITIES"

typedef enum layer_type {
    TILES,
    ENTITIES
} layer_type_t;

enum layer_flags {
    LAYER_HIDDEN = 0x00000001,
    LAYER_LOCKED = 0x00000002
};

typedef struct layer_data {
    int index;
    char name[256];
    layer_type_t type;
    Uint32 flags;
} layer_data_t;

bool layer_has_flags(const layer_data_t *layer_data, Uint32 flags);

void layer_set_name(layer_data_t *layer_data, const char *name);

//void add_layer(layer_data_t *layer_data, int *num_layers, const char *name, layer_type_t type);

//void remove_layer(layer_data_t *layer_data, int *num_layers, int index);

#endif
