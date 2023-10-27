#pragma once
#include "../8cc.h"
Buffer* to_utf16(char* p, int len);
Buffer* to_utf32(char* p, int len);
void write_utf8(Buffer* b, uint32_t rune);