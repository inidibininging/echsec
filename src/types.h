
#ifndef TYPES__H
#define TYPES__H
#include <stdint.h>
#include <stdlib.h>
#include "meta.h"
#define SPACE_SIZE uint16_t

// ==============================
// LEXER STUFF
// ==============================
//this is the static char buffer size used by the lexer (2 because of the null terminator)
#define LEXER_BUFFER_MULTIPLICATOR 2

//symbol is wrapper for char*. a string sort of 
typedef struct t_char
{
    //this is the token type. token type will be set during the lexing process and in the on_parse function, passed to the lexer function
    int8_t type;

    //the string content
    char* name;

    //the size of the string. used in order to prevent strlen calls
    size_t size;
} symbol;

//expression in the program. the expressions will be stored within a flat list in main.c
//this is obviously unoptimized and should change in the future. this is me trying to force implementation without knowing what I'm doing
//ALSO MIND THE DATA TYPE used across anything done here => uint8_t and imagine the implications for any source code written in this @#$%ty "language"
//The code is meant to be held small.
typedef struct t_var_expr {
    //parser expression type => specified within parser.h
    uint8_t operation_type;
    
    //the left side expression or the only expression (in case of return endif etc.)
    symbol name_ref;
    
    //this is either the right expression OR the list of arguments within a function / internal function / execution
	//Im having trouble freeing stuff. If it doesnt change I might try to change value_ref to symbol** for storing a list of pointers pointing to the real symbols.
    //symbol* value_ref;
	SPACE_SIZE value_ref_idx;

    uint8_t value_ref_length;

    //this is where the expression scope is
    /*
        example:

        the owner->     if blabalba is lol
                            something
                        end if
    */
    SPACE_SIZE scope_ref;

	// tells wethere the structure has been initialized
	//uint8_t b_init;
} var_expr;

typedef struct t_var_mem {
    //the owner of the variable (function)
    SPACE_SIZE scope_ref_idx;

    //the (name of the variable) this should be copied because it can change
    char* name;

    //the (value of the variable) this should be copied because it can change
    char* value;

    //the type (for e.g a string or a number)
    SPACE_SIZE type;
} var_mem;

    
// I refuse to use __VA_ARGS__
#define DECL_THREADS(N) SPACE_SIZE *t_##N; SPACE_SIZE tc_##N;
typedef struct t_callstack_threads {
    DECL_THREADS(0)

} callstack_threads;
FUNC_TYPE callstack_threads_init(callstack_threads** threads);
FUNC_TYPE callstack_threads_destroy(callstack_threads** threads);

#endif
