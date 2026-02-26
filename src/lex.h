#ifndef LEX__H
#define LEX__H
//defines the max size of any identifier
#define LEXER_EOF -1

#include <stdio.h>
#include <stdint.h>
#include "types.h"
void lex_emit(const symbol* token);
void lex_print(int8_t token);
int8_t lex_find_single_token(const char *possible_token);
int8_t lex_find_multi_token(const char *possible_token);
int8_t lex_next_to_buffer_2(FUNC_3(on_token, const char*, int8_t, int16_t, int8_t), FILE* esrc_file);
int8_t lex_next_to_buffer_2_wasm(FUNC_3(on_token, const char*, int8_t, int16_t, int8_t), const char* script, SPACE_SIZE* script_pos);
int8_t lex_next(char* c, int8_t idx, FILE* esrc_file);
#endif
