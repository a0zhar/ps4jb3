#pragma once
#include "../8cc.h"
Dict* make_dict(void);
void* dict_get(Dict* dict, char* key);
void dict_put(Dict* dict, char* key, void* val);
Vector* dict_keys(Dict* dict);