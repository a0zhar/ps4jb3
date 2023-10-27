#pragma once
#include "../8cc.h"
void set_output_file(FILE* fp);
void close_output_file(void);
void emit_toplevel(Node* v);