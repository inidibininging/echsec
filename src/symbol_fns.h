#ifndef SYMBOL_FNS__H
#define SYMBOL_FNS__H
#include "types.h"
symbol* symbol_new(const char* name, int8_t type);
void symbol_destroy(symbol* s);
void symbol_trim_name_capacicty(symbol* s);
int8_t symbol_copy_2(const symbol *s, symbol *sg);
int8_t symbol_copy_name(symbol *s, const char* name);
int8_t symbol_cat_name(symbol *s, const char* name);
int8_t symbol_read_next(symbol *symbol);
#endif
