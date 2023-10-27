#pragma once
#include "../8cc.h"
void lex_init(char* filename);
char* get_base_file();
void skip_cond_incl(void);
char* read_header_file_name(bool* std);
bool is_keyword(Token* tok, int c);
void token_buffer_stash(Vector* buf);
void token_buffer_unstash();
void unget_token(Token* tok);
Token* lex_string(char* s);
Token* lex(void);