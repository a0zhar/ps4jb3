#pragma once
#include "../8cc.h"
Set* set_add(Set* s, char* v);
bool set_has(Set* s, char* v);
Set* set_union(Set* a, Set* b);
Set* set_intersection(Set* a, Set* b);