#pragma once
#ifndef _MAP_H
#define _MAP_H
#include "../8cc.h"
Map* make_map(void);
Map* make_map_parent(Map* parent);
void* map_get(Map* m, char* key);
void map_put(Map* m, char* key, void* val);
void map_remove(Map* m, char* key);
size_t map_len(Map* m);
#endif