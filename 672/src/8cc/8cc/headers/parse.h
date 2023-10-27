#pragma once
#ifndef _PARSE_H
#define _PARSE_H
#include "../8cc.h"
char* make_tempname(void);
char* make_label(void);
bool is_inttype(Type* ty);
bool is_flotype(Type* ty);
void* make_pair(void* first, void* second);
int eval_intexpr(Node* node, Node** addr);
Node* read_expr(void);
Vector* read_toplevels(void);
void parse_init(void);
char* fullpath(char* path);
#endif