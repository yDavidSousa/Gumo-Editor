#include <stdio.h>
#include <tilemap.h>

void save_gtm_file(const char *file_name, tilemap_t *tilemap){

    FILE *file = fopen(file_name, "w");
    if(!file){
        printf("Couldn't open file\n");
        return;
    }

    fprintf(file, "#TILEMAP_INFO\n");
    fprintf(file,"\t%d //map_width\n", tilemap->map_width);
    fprintf(file,"\t%d //map_height\n", tilemap->map_height);
    fprintf(file,"\t%d //tile_width\n", tilemap->tile_width);
    fprintf(file,"\t%d //tile_height\n", tilemap->tile_height);
    fprintf(file,"\t%d //tiles per width\n", tilemap->tiles_per_width);
    fprintf(file,"\t%d //tiles per height\n", tilemap->tiles_per_height);
    fprintf(file,"\t%d //number of layers\n", tilemap->num_layers);
    fprintf(file, "\n");

    fprintf(file, "#LAYERS_INFO\n");
    for (int i = 0; i < tilemap->num_layers; ++i){
        fprintf(file, "\t%d %s\n",
                tilemap->layers[i].id,
                tilemap->layers[i].name
        );
    }
    fprintf(file, "\n");

    fprintf(file, "#TILESETS_INFO\n");
    tileset_t *tilesets = get_tilesets();
    for (int j = 0; j < num_tilesets; ++j) {
        if (tilesets[j].id != -1){
            fprintf(
                    file,
                    "\t%d %s %s %d %d\n",
                    tilesets[j].id,
                    tilesets[j].name,
                    tilesets[j].image.source,
                    tilesets[j].tile_width,
                    tilesets[j].tile_height
            );
        }
    }
    fprintf(file, "\n");

    for (int l = 0; l < tilemap->num_layers; ++l) {
        fprintf(file, "%s\n", tilemap->layers[l].name);

        for (int r = 0; r < tilemap->tiles_per_height; ++r) {
            fputs("\t", file);
            for (int c = 0; c < tilemap->tiles_per_width; ++c) {
                if (tilemap->layers[l].type == TILES)
                    fprintf(file, "%d ", tilemap->data[l][r][c]);
                if (tilemap->layers[l].type == ENTITIES) {
                    if(tilemap->data[l][r][c] != EMPTY_TILE){
                        entity_data_t *entity = get_entity((int) tilemap->data[l][r][c]);
                        fprintf(file, "%d %s %d %d\n",
                                entity->id,
                                entity->name,
                                c * tilemap->tile_width,
                                r * tilemap->tile_height
                        );
                    }
                }
            }
            fputs("\n", file);
        }

        fputs("\n", file);
    }

    printf("File saved with successful!\n");
    fclose(file);
}

/*
void load_gtm_file(FILE *file, tilemap_t *tilemap) {

// FILE *file = fopen(file_path, "r");
    if (!file){
        printf("Couldn't open file\n");
        return;
    }

    char buffer[256] = {};
    while(!feof(file)){
        fscanf(file, "%s", buffer);

        if(strcmp(buffer, "#TILEMAP_INFO") == 0){
            fscanf(file, "%d %*[^\n]", &tilemap->num_layers);
            fscanf(file, "%d %*[^\n]", &tilemap->tiles_per_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tiles_per_height);
            fscanf(file, "%d %*[^\n]", &tilemap->map_width);
            fscanf(file, "%d %*[^\n]", &tilemap->map_height);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_height);
        } else if(strcmp(buffer, "#LAYERS_INFO") == 0){
            char layer_type_buffer[80];
            for (int i = 0; i < tilemap->num_layers; ++i) {
                fscanf(file, "%s %d %s",
                       tilemap->layers[i].name,
                       &tilemap->layers[i].index,
                       layer_type_buffer
                );

                if(strcmp(layer_type_buffer, TILES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = TILES;
                else if (strcmp(layer_type_buffer, ENTITIES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = ENTITIES;
            }
        } else if(strcmp(buffer, "#TILESETS_INFO") == 0){
        } else {
            for (int i = 0; i < tilemap->num_layers; ++i) {
                if(strcmp(buffer, tilemap->layers[i].name) == 0){
                    for (int r = 0; r < tilemap->tiles_per_height; ++r)
                        for (int c = 0; c < tilemap->tiles_per_width; ++c)
                            fscanf(file, "%d ",
                                   &tilemap->data[i][r][c]
                            );
                    break;
                }
            }
        }
    }

    printf("Tilemap loaded with successful!\n");
    fclose(file);
}

 */