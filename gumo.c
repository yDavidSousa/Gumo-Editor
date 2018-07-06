#include <stdio.h>
#include <tilemap.h>
#include <mem.h>

const char *TILES_LAYER_TYPE = "TILE";
const char *ENTITIES_LAYER_TYPE = "ENTITY";

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
    fprintf(file,"\t%d //number of tilesets\n", num_tilesets);
    fprintf(file,"\t%d //number of entities\n", num_entities);
    fprintf(file, "\n");

    fprintf(file, "#LAYERS_INFO\n");
    for (int i = 0; i < tilemap->num_layers; ++i){
        char type_buffer[256];

        if(tilemap->layers[i].type == TILES)
            strcpy(type_buffer, TILES_LAYER_TYPE);
        else if(tilemap->layers[i].type == ENTITIES)
            strcpy(type_buffer, ENTITIES_LAYER_TYPE);

        fprintf(
            file,
            "\t%d %s %s\n",
            tilemap->layers[i].id,
            tilemap->layers[i].name,
            type_buffer
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

    fprintf(file, "#ENTITIES_INFO\n");
    fprintf(file, "\n");

    for (int l = 0; l < tilemap->num_layers; ++l) {
        fprintf(file, "%s\n", tilemap->layers[l].name);

        if (tilemap->layers[l].type == TILES){
            for (int r = 0; r < tilemap->tiles_per_height; ++r) {
                fputs("\t", file);
                for (int c = 0; c < tilemap->tiles_per_width; ++c)
                    fprintf(file, "%d ", tilemap->data[l][r][c]);
                fputs("\n", file);
            }
            fputs("\n", file);
        }

        if (tilemap->layers[l].type == ENTITIES) {
            int num_entities_buffer = 0;
            for (int r = 0; r < tilemap->tiles_per_height; ++r)
                for (int c = 0; c < tilemap->tiles_per_width; ++c){
                    if(tilemap->data[l][r][c] != EMPTY_TILE) {
                        num_entities_buffer++;
                    }
                }

            fprintf(file, "\t%d\n", num_entities_buffer);
            for (int r = 0; r < tilemap->tiles_per_height; ++r){
                for (int c = 0; c < tilemap->tiles_per_width; ++c){
                    if(tilemap->data[l][r][c] != EMPTY_TILE){
                        entity_data_t *entity = get_entity((int) tilemap->data[l][r][c]);
                        fprintf(file, "\t%d %s %d %d \n",
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
    }

    printf("File saved with successful!\n");
    fclose(file);
}

void load_gtm_file(const char *file_path, tilemap_t *tilemap, SDL_Renderer *renderer) {

    FILE *file = fopen(file_path, "r");
    if (!file){
        printf("Couldn't open file\n");
        return;
    }

    int num_tilesets_buffer = 0;
    char buffer[256] = {};
    while(!feof(file)){
        fscanf(file, "%s", buffer);

        if(strcmp(buffer, "#TILEMAP_INFO") == 0){

            fscanf(file, "%d %*[^\n]", &tilemap->map_width);
            fscanf(file, "%d %*[^\n]", &tilemap->map_height);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tile_height);
            fscanf(file, "%d %*[^\n]", &tilemap->tiles_per_width);
            fscanf(file, "%d %*[^\n]", &tilemap->tiles_per_height);
            fscanf(file, "%d %*[^\n]", &tilemap->num_layers);
            fscanf(file,"%d %*[^\n]", &num_tilesets_buffer);
            fscanf(file,"%d %*[^\n]", &num_entities);

        } else if(strcmp(buffer, "#LAYERS_INFO") == 0){

            char layer_type_buffer[80];
            for (int i = 0; i < tilemap->num_layers; ++i) {
                fscanf(file, "%d %s %s",
                       &tilemap->layers[i].id,
                       tilemap->layers[i].name,
                       layer_type_buffer
                );

                if(strcmp(layer_type_buffer, TILES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = TILES;
                else if (strcmp(layer_type_buffer, ENTITIES_LAYER_TYPE) == 0)
                    tilemap->layers[i].type = ENTITIES;
            }

        } else if(strcmp(buffer, "#TILESETS_INFO") == 0){

            tileset_t tileset_buffer;
            for (int j = 0; j < num_tilesets_buffer; ++j) {
                    fscanf(
                            file,
                            "\t%d %s %s %d %d\n",
                            &tileset_buffer.id,
                            tileset_buffer.name,
                            tileset_buffer.image.source,
                            &tileset_buffer.tile_width,
                            &tileset_buffer.tile_height
                    );

                add_tileset(renderer, tileset_buffer.name, tileset_buffer.image.source, tileset_buffer.tile_width, tileset_buffer.tile_height);
            }

        } else if(strcmp(buffer, "#ENTITIES_INFO") == 0){

        } else {
            for (int l = 0; l < tilemap->num_layers; ++l) {
                if(strcmp(buffer, tilemap->layers[l].name) == 0){

                    if(tilemap->layers[l].type == TILES){
                        for (int r = 0; r < tilemap->tiles_per_height; ++r)
                            for (int c = 0; c < tilemap->tiles_per_width; ++c)
                                fscanf(file, "%hu ", &tilemap->data[l][r][c]);
                    }

                    if(tilemap->layers[l].type == ENTITIES){
                        int num_entities_buffer, row, column;
                        entity_data_t entity_buffer;

                        fscanf(file, "%d", &num_entities_buffer);
                        for (int r = 0; r < num_entities_buffer; ++r){
                            fscanf(file, "%d %s %d %d ", &entity_buffer.id, entity_buffer.name, &column, &row);
                            column /= tilemap->tile_width;
                            row /= tilemap->tile_height;
                            tilemap->data[l][row][column] = (short)entity_buffer.id;
                        }
                    }

                    break;
                }
            }
        }
    }

    printf("Tilemap loaded with successful!\n");
    fclose(file);
}