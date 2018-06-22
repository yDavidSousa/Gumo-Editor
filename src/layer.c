#include <layer.h>
#include <mem.h>
#include <stdio.h>

void layer_set_name(layer_data_t *layer_data, const char *name){
    strcpy(layer_data->name, name);
}

bool layer_has_flags(const layer_data_t *layer_data, Uint32 flags) {
    return (layer_data->flags & flags) == flags;
}
 /*
  *

/////////////////////////////////////////
void add_layer(layer_data_t *layer_data, int *num_layers, const char *name, const layer_type_t type){

    if(*num_layers == MAX_LAYERS){
        printf("Maximum allowed number of layers exceeded!\n");
        return;
    }

    const int index = *num_layers;

    layer_data[index].index = index;
    strcpy(layer_data[index].name, name);
    layer_data[index].type = type;
    layer_data[index].flags = 0;

    *num_layers += 1;
}

void remove_layer(layer_data_t *layer_data, int *num_layers, const int index){

    if(*num_layers == 0){
        printf("Does not contain any layer!\n");
        return;
    }

    for (int i = index; i < *num_layers - 1; ++i)
        layer_data[i] = layer_data[i+1];

    *num_layers -= 1;
}

  * */