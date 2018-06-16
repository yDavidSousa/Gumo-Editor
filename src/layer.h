#ifndef LAYER_H
#define LAYER_H

#define TILES_LAYER_TYPE "TILES"
#define ENTITIES_LAYER_TYPE "ENTITIES"

typedef enum layer_type {
    TILES,
    ENTITIES
} layer_type_t;

typedef struct layerinfo {
    int index;
    char name[256];
    layer_type_t type;
    int locked;
    int hidden;
} layerinfo_t;

#endif
