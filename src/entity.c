#include <stdio.h>
#include <mem.h>
#include <entity.h>
#include <tileset.h>

entity_data_t entities[MAX_ENTITIES];

void add_entity(const char *name, const short link){
    if(num_entities == MAX_ENTITIES){
        printf("Maximun allowed number of entities exceeded!\n");
        return;
    }

    int index = num_entities;
    for (int i = 0; i < num_entities; ++i) {
        if(entities[i].id == EMPTY_ENTITY){
            index = i;
            break;
        }
    }

    entities[index].id = index;
    strcpy(entities[index].name, name);
    entities[index].link = link;

    num_entities += 1;
}

//TODO(David): Finish someday.
void remove_entity(const int index){
    entities[index].id = EMPTY_ENTITY;
}

void render_entity(SDL_Renderer *renderer, const int id, const SDL_Rect *dst_rect){
    if(entities[id].link == -1){
        //TODO(David): Make set the color.
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, dst_rect);
    } else
        render_tile(renderer, entities[id].link, dst_rect);
}

entity_data_t *get_entity(const int id){
    return &entities[id];
}