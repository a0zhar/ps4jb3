#pragma once
#ifndef _BUFFER_H
#define _BUFFER_H
#include "../8cc.h"
Buffer* make_buffer(void);
char* buf_body(Buffer* b);
int buf_len(Buffer* b);
void buf_write(Buffer* b, char c);
void buf_append(Buffer* b, char* s, int len);
void buf_printf(Buffer* b, char* fmt, ...);
char* vformat(char* fmt, va_list ap);
char* format(char* fmt, ...);
char* quote_cstring(char* p);
char* quote_cstring_len(char* p, int len);
char* quote_char(char c);
#endif