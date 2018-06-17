#ifndef ENTITY_H
#define ENTITY_H

typedef struct entity_data {
    int id;
    char name[256];
    int x, y;
    int w, h;
} entity_data_t;

#endif