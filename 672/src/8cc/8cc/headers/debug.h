#pragma once
#ifndef _DEBUG_H
#define _DEBUG_H
#include "../8cc.h"
char* ty2s(Type* ty);
char* node2s(Node* node);
char* tok2s(Token* tok);
void logMessageToFileF(int JustNewLine, int incTimeDate, const char *fmt, ...);
#endif