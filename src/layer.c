#include <layer.h>
#include <mem.h>
#include <stdio.h>

void layer_set_name(layer_data_t *layer_data, const char *name){
    strcpy(layer_data->name, name);
}

bool layer_has_flags(const layer_data_t *layer_data, Uint32 flags) {
    return (layer_data->flags & flags) == flags;
}