#include <stdlib.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "token_def.h"
#include "config.h"
#include "types.h" 
#include "file_stuff.h"
#include "file_fns.h"
#include "meta.h"
#include "symbol_fns.h"
#include "lex.h"
#include "parser.h"
#include "machine.h"

#if USE_WASM == 1
#include <emscripten/emscripten.h>
#endif
#if DEBUG_MAIN == 0
#undef DBUG
#define DBUG(msg) (void)0;
#undef HLINE
#define HLINE (void)0;
#endif

#if MAIN_MODE == MAIN_MODE_REAL
#define MAIN_FUNC main
#define MAIN_TEST test_alloc
#endif

#if MAIN_MODE == MAIN_MODE_TEST
#define MAIN_FUNC test_alloc
#define MAIN_TEST main
#endif


// this is the cache of all tokens found in the current file (excluding T_SPACE)
symbol *symbols = NULL;

// this is the expression that result out of symbols
var_expr *expressions = NULL;

// this is an index that contains:
// - indexes on the symbols list
// e.g
//
// 	symbols could look like this:
//
//  0 1 2 3 4 5 6
//	: a ( a , b )
//	
//	var_expr could look like this after using var_expr_ref_add:
//	0
//
//	var_expr:
//	op_type = P_FUNC
//	name_ref = ptr to idx 2 in symbols
//	value_ref_idx = 0
//	value_ref_length = 2
//  
//  arguments could look like this:
//	0 : 3
//  1 : 5
//
//  idx 0 points to 3, thus symbols[3] -> name
//  idx 1 points to 5, thus symbols[5] -> name 
//
//  since strings are hold inside a single idx => dont think that tokens are stored singularly in a space
SPACE_SIZE *arguments = NULL;

// this is where variables will be stored
var_mem *variables = NULL;

// this is where the callstack is held
SPACE_SIZE *callstack = NULL;

SPACE_SIZE symbols_count = 0;
SPACE_SIZE expressions_count = 0;
SPACE_SIZE variables_count = 0;
SPACE_SIZE arguments_count = 0;
SPACE_SIZE call_stack_count = 0;

#define _F1(X ,A, AT, T) T X(AT A);
#define _F2(X ,A, AT, B, BT, T) T X(AT A, BT B);
#define _F3(X ,A, AT, B, BT, C, CT, T) T X(AT A, BT B, CT C);

#define F1(A,T) _F1(x, A, T, T)

#define THREAD_NUM 16
#define MAX_CALL_STACK 320
#define CALLSTACK_THREAD THREAD_NUM * MAX_CALL_STACK
#define THREAD_MEMORY 

#define FARG(a, b, c) a, b, c
#define FARR_(a, b, c) a[b - c]
#define FL_(a, b, c) FARR_(a, b, c)

//long stuff for getting stuff
#define FROM_LAST_SYMBOL(n) FL_(symbols, symbols_count, n)
#define FROM_LAST_EXPR(n) FL_(expressions, expressions_count,  n)
#define FROM_LAST_VAR(n) FL_(variables, variables_count, n)
#define FROM_LAST_ARGUMENT(n) FL_(arguments, arguments_count,  n)
#define FROM_LAST_CALL_STACK(n) FL_(call_stack, call_stack_count, n)

//shortcuts for lazy writing
#define FLS(n) FROM_LAST_SYMBOL(n)
#define FLXP(n) FROM_LAST_EXPR(n)
#define FLV(n) FROM_LAST_VAR(n)
#define FLA(n) FROM_LAST_ARGUMENT(n)
#define FLCS(n) FROM_LAST_CALL_STACK(n)

// THE OLD ONE. CAT_TO_LAST_SYMBOL_AND_ASSIGN_TYPE means PUSH name to the last symbol. WTF. duuuud. this is so garbage
#define CAT_TO_LAST_SYMBOL_AND_ASSIGN_TYPE(_name, _type) \
	symbol_cat_name(&FLS(1), _name);                     \
	FLS(1).type = _type;

int8_t lr = 0;
int8_t level = 0;
int8_t rule_index = 0;

/*
  copies the current token to the symbols cache.
  if the cache is not big enough, it will be expanded
  this function is invoked when a token has been identified by lex_next_to_buffer_2
  spaces will not be copied to the symbols cache
	UNLESS it is a Tag (string)
*/
int8_t on_token_found_string = 0;
int8_t on_token_flush_string = -1;
enum on_token_mode
{
	reallocate = 0,
	concatenate = 1,
	allocate = 2
};
enum on_token_mode ots = reallocate;

void symbols_destroy()
{
	for (SPACE_SIZE sidx = 0; sidx < symbols_count; sidx++)
	{
		if(symbols[sidx].name != NULL) {
		  free(symbols[sidx].name);
		}
 	}
	free(symbols);
}

void expressions_destroy()
{		
	for (SPACE_SIZE sidx = 0; sidx < expressions_count; sidx++)
	{
		if(expressions[sidx].name_ref.name != NULL) {
			free(expressions[sidx].name_ref.name);
		}
	}
	free(expressions);
}

void arguments_destroy() {	
	free(arguments);
} 

void call_stack_destroy() {
	free(callstack);
}

void variables_destroy() {
	//TODO
	for(SPACE_SIZE vidx = 0; vidx < variables_count; vidx++) {
		free(variables[vidx].name);
		free(variables[vidx].value);
	}
	free(variables);
}

void print_all_lex() {
  SPACE_SIZE op = symbols_count;
  printf("[\n"); 
  while(op != 0) {
    lex_emit(&FLS(op));
	if(op !=  1) {
		printf(",");
	}
    op--;
  }
  printf("]\n");

}

void print_all_expr() {
  SPACE_SIZE op = 0;
  printf("[\n"); 

  while(op != expressions_count) {
	//printf("%i op\n", op); 
    //parser_emit_reconstruct(&expressions[op]);
  	if(&expressions[op] == NULL) {
		continue;
 	}
	parser_emit(&expressions[op], 
			symbols, 
			&arguments, 
			&arguments_count);
	if(expressions_count > 1 && op != expressions_count - 1){
		printf(",");
	}
    op++;
  }
  printf("]\n");
}

#define STRING_EXPERIMENT

// function called by the lexer through a function pointer. 
// the idea is to be able to hook on the lexer function if it finds something
// this is obviously BADLY WRITTEN and should change in the future in order to mitigate errors
int8_t on_token(const char *name, int8_t type, int16_t position)
{
	// print this if you want to know what is stored inside the last symbol
	lex_print(type);
	
#ifdef STRING_EXPERIMENT
	if (type == T_SPACE &&
		on_token_found_string == 0)
	{
		DBUG("skip T_SPACE");
		return FUNC_OK;
	} // TODO: space should be lexed as a chunk of space (space + size) in order to reduce memory and allocations

	if (type == T_STRING_IDENTIFIER)
	{
		DBUG("T_STRING_IDENTIFIER detected in on_token");
		HLINE
		if (on_token_found_string == 1)
		{
			// activate the T_SPACE constraint
			DBUG("string flush set to 1, ots set to reallocate");
			on_token_flush_string = 1;
			on_token_found_string = 0;
			ots = reallocate;
			//lex_emit(&FLS(1));
		}
		else
		{
			// deactivate the T_SPACE constraint
			DBUG("string flush set to 0, ots set to allocate");
			on_token_flush_string = 0;
			on_token_found_string = 1;
			ots = allocate;
			
			// add string identifier to symbol list. this is invoked for the first string identifier found
			//lex_emit(&FLS(1));

			//create the new starting string identifier
			symbols_count = symbols_count + 1;
			symbols = realloc(symbols, sizeof(symbol) * symbols_count);
			if(symbols == NULL) {
				DIE("symbols cannot be reallocated");
			}
			FLS(1).name = NULL;
			FLS(1).size = 0;
			FLS(1).type = T_STRING_IDENTIFIER;
			symbol_copy_name(&FLS(1), STRING_IDENTIFIER);

			//symbol_cat_name(&FLS(1), name);
			
			//lex_emit(&FLS(1));
			return FUNC_OK;
		}
		HLINE
	}

#else
	if (type == T_SPACE)
	{
		DBUG("skip T_SPACE");
		return FUNC_OK;
	}
#endif
	if (ots == concatenate)
	{
		if (FLS(1).name == NULL)
		{
			DBUG("FLS(1).name is NULL");
		}
		if (name == NULL)
		{
			DBUG("name provided is NULL");
		}
		DBUG("FLS(1).type = T_STRING");
		FLS(1).type = T_STRING;
		DBUG("symbol_cat_name(&FLS(1), name)");
		symbol_cat_name(&FLS(1), name);
	}
	
	//checks if the symbols array should be malloc'ed (allocate) or realloc'ed (reallocate)
	if (ots == reallocate ||
		ots == allocate)
	{
		DBUG("(re)allocating...");
		if (symbols_count == 0)
		{
			symbols_count = symbols_count + 1;
			symbols = calloc(symbols_count, sizeof(symbol));
		}
		else
		{
			symbols_count = symbols_count + 1;
			symbols = realloc(symbols, sizeof(symbol) * symbols_count);
		}
    if(symbols == NULL) {
      DIE("cannot reallocate memory for symbols");
    }

		//reset the last symbol to zero values
		FLS(1).name = NULL;
		FLS(1).size = 0;

		if (ots == reallocate)
		{
			// this is the default copy stuff  (NO STRINGS will be copied here)
			FLS(1).type = type;
			// THIS IS STRCPY
			symbol_copy_name(&FLS(1), name);
			HLINE
			DBUG("reallocate\n");
			//lex_emit(&FLS(1));
		}
		if (ots == allocate)
		{
			// this forwards the copy stuff to the part above (ots == concatenate)
			FLS(1).type = T_STRING;
			symbol_copy_name(&FLS(1), name);
			ots = concatenate;
			HLINE
			DBUG("allocate\n");
			//lex_emit(&FLS(1));

		}
		DBUG("symbol_copy_to invoked on on_token");
	}
	if (ots == allocate &&
		on_token_found_string == 1)
	{
		ots = concatenate;
	}
	return FUNC_OK;
}

void set_tag_decl_print(const var_expr *tag)
{
	if (tag == NULL)
	{
		DBUG("tag is null");
	}
	// if (tag->scope_begin_ref == NULL)
	// {
	// 	DBUG("scope_begin_ref is NULL");
	// }
	// if (tag->scope_end_ref == NULL)
	// {
	// 	DBUG("scope_end_ref is NULL");
	// }
	if (tag->name_ref.name == NULL)
	{
		DBUG("name_ref is NULL. identifier not declared");
	}
	else
	{
	}
}

void fn_decl_print(const var_expr *fn)
{
	if (fn == NULL)
	{
		DBUG("fn is NULL");
		return;
	}
	if (fn->name_ref.name == NULL)
	{
		DBUG("name_ref is NULL. function body not declared");
	}
	else
	{
		DBUG("function body declared");
		if (fn->name_ref.size <= 0)
		{
			DBUG("but its name size < 0");
		}

		if (fn->name_ref.name == NULL)
		{
			DBUG("and it's name is NULL");
		}
		else
		{
			DBUG("and it's name is not NULL. name will be written below");
			DBUG(fn->name_ref.name);
		}
	}
	if (fn->value_ref_idx == USHRT_MAX)
	{
		DBUG("arguments_ref is NULL");
	}
	else
	{
		DBUG("arguments_Ref is not NULL");
	}
}

// testing grounds for figuring out how allocs work. oh C
int MAIN_TEST(int argc, char **argv)
{
	
	var_mem** a = &variables;
	*a = calloc(1, sizeof(var_mem));
	SPACE_SIZE* ac = &variables_count;

	const char* x = "a";
	var_mem* p = a[0];
	p->name = NULL;
	p->name = malloc(sizeof(char)*strlen(x)+1);

	strcpy(p->name, x);
	*ac += 1;
	
	variables_destroy();
	return FUNC_OK;
}


#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

#if USE_WASM == 1
EXTERN EMSCRIPTEN_KEEPALIVE void echse_eval(
        const char* script, 
        const char* start_function_name)
{
    printf("hello from echsec\n");
	SPACE_SIZE c = 0;
	#if DEBUG_MACHINE == 1
	HLINE
    DBUG(script);
	DBUG("start_function_name is set to:");
	DBUG(start_function_name);
	HLINE
#endif
	// file pointer to the current src file
	// FILE esrc_file_;

		if (script != NULL)
		{
			//this is a hard reference to a file and will be read to memory. this is so wrong af that i dunno where to begin
			//obviously a low hanging fruit for future fixes						
			int8_t last_symbols_count = symbols_count;
			int8_t on_token_read_distance = 0;
			// this is @#$^. lex next to buffer 2 can execute on token more than once
			while (lex_next_to_buffer_2_wasm(on_token, script, &c) == FUNC_OK)
			{
				if (symbols_count == last_symbols_count)
				{
					continue;
				}
				/*
				  made this for skipping spaces. spaces are not recorded into the symbols pointer,
				  therefor on_parse will fail if we iterate over the same symbol again because on_parse check if we stay on the symbol
				  this could be fixed by adding T_SPACE to the symbols pointer.
				*/
				on_token_read_distance = symbols_count - last_symbols_count;
				if (on_token_read_distance > 1)
				{
					DBUG("on_token_read_distance > 1");
					// iterate with on parse, starting with symbols_count - difference of the if statement above
					// else, do the on parse "normally"
				}
				last_symbols_count = symbols_count;

				while (on_token_read_distance > 0)
				{
					// for testing purpouses
					on_token_read_distance -= 1;
				}

			}

#if EMIT_TOKEN == 1
				//if(FLS(1).type != T_STRING &&
				//	FLS(1).type != T_STRING_IDENTIFIER) {
				//	lex_print(&FLS(1));
				//}
				print_all_lex();
#endif

#if DEBUG_PARSER == 1
			if(expressions == NULL) {
				expressions_count = 1;
				expressions = calloc(expressions_count, sizeof(var_expr));
				var_expression_init(&expressions[0]);
			}

			enum on_parse_state state = start;
			enum on_parse_start_symbol start_symbol = none;
			uint8_t last_on_parse = FUNC_OK;
			SPACE_SIZE token_index = symbols_count;

		  	while(token_index != 0) {
			    //parser_emit_reconstruct(&expressions[0]);
				//lex_emit(&FLS(token_index));

				//QUICK HACK!!! forget that there are string identifiers, because this is not important while parsing
				//The string already has its own identifier
				if(FLS(token_index).type == T_STRING_IDENTIFIER){
					DBUG("skipping T_STRING_IDENTIFIER while parsing...");
					token_index--;
					continue;
				}
#if DEBUG_MAIN == 1				
				printf("token index in main: %d \n", token_index);
#endif
				last_on_parse = on_parse(
					&FLS(token_index),
					symbols_count - token_index,
					&FLXP(1),
					&expressions_count,
					&arguments,
					&arguments_count,
					&start_symbol,
					&state);

				if (last_on_parse == FUNC_NOT_OK)
				{
					exit(EXIT_FAILURE);
				}

				if (last_on_parse == PARSER_DONE)
				{
					HLINE
            		DBUG("PARSER_DONE. allocating next expr");
					expressions_count += 1;
					RESIZE(expressions, var_expr, expressions_count)
					var_expression_init(&FLXP(1));
					state = start;
					continue;
					
					//this is a quick bug fix, making 3000 bugs more. somehow the "parser" stops at the next instruction, so when we take here the next instruction, it doesn't know what to do. would work if the next next instruction is an operation, instead, we should check WHY the parser stops at that instruction
				}
				token_index--;
			}
//#if EMIT_JSON == 1
			//print_all_expr();
//#endif
#endif
			SPACE_SIZE max_call_stack = MAX_CALL_STACK;
			call_stack = calloc(sizeof(SPACE_SIZE), max_call_stack);
			if(call_stack == NULL) { 
				DIE("call stack cannot be calloc'd");
			}
			for(SPACE_SIZE cidx = 0; cidx < MAX_CALL_STACK; cidx++) {
				call_stack[cidx] = USHRT_MAX;
			}
#if DEBUG_MACHINE == 1
			HLINE
			DBUG("looking for main function...");
			DBUG(expressions_count);
			HLINE
			var_expr* start_function = find_function_of_execution(
								start_function_name,
								&FLXP(1),
								&expressions_count, 
								&FLV(1),
								&variables_count);
			DBUG("going for run");
			if(variables == NULL) {
				variables = calloc(1, sizeof(var_mem));
				const char* args_name = "args";

				variables[0].name = malloc(sizeof(char)*strlen(args_name)+1);
				strcpy(variables[0].name, args_name);
				variables[0].name[4] = '\0';

				//variables[0].init = 1;
				variables[0].value = malloc(sizeof(char)* strlen(script) + 1);
				if(variables[0].value == NULL) {
					DIE("failed trying to malloc args to args inside of the script.");					
				}
				strcpy(variables[0].value, script);
				variables[0].value[strlen(script)] = '\0';
				variables[0].scope_ref_idx = start_function->scope_ref - 1;

				variables_count = 1;
			}
			run(start_function_name,
				start_function, 
				&expressions[0],
				&FLXP(1), 
				expressions_count,
				&variables, 
				&variables_count,
				&arguments,
				&arguments_count,
				&call_stack,
				&call_stack_count,
				&max_call_stack,
				symbols);
#endif

			// fclose(esrc_file_);
			
		}
		expressions_destroy();
		expressions = NULL;

		symbols_destroy();		
		symbols = NULL;

		arguments_destroy();
		arguments = NULL;

		call_stack_destroy();
		call_stack = NULL;

		variables_destroy();
		variables = NULL;
	
	//return FUNC_OK;
}
#endif

int MAIN_FUNC(int argc, char **argv)
{
#if USE_WASM == 1
    printf("wasm is on\n");
	return 0;
#endif
#if DEBUG_MACHINE == 1
	HLINE
	const char* start_function_name = "main";
	DBUG("start_function_name is set to:");
	DBUG(start_function_name);
	HLINE
#endif
	// file pointer to the current src file
	// FILE esrc_file_;
	for (SPACE_SIZE ac = 1; ac < argc; ac++)
	{
		if (argv[ac] != NULL)
		{
			//this is a hard reference to a file and will be read to memory. this is so wrong af that i dunno where to begin
			//obviously a low hanging fruit for future fixes			
			FILE *esrc_file_ = fopen(argv[ac], "r");
			read_file(argv[ac],
				  esrc_file_);
			int8_t last_symbols_count = symbols_count;
			int8_t on_token_read_distance = 0;
			// this is @#$^. lex next to buffer 2 can execute on token more than once
			while (lex_next_to_buffer_2(on_token, esrc_file_) == FUNC_OK)
			{
				if (symbols_count == last_symbols_count)
				{
					continue;
				}
				/*
				  made this for skipping spaces. spaces are not recorded into the symbols pointer,
				  therefor on_parse will fail if we iterate over the same symbol again because on_parse check if we stay on the symbol
				  this could be fixed by adding T_SPACE to the symbols pointer.
				*/
				on_token_read_distance = symbols_count - last_symbols_count;
				if (on_token_read_distance > 1)
				{
					DBUG("on_token_read_distance > 1");
					// iterate with on parse, starting with symbols_count - difference of the if statement above
					// else, do the on parse "normally"
				}
				last_symbols_count = symbols_count;

				while (on_token_read_distance > 0)
				{
					// for testing purpouses
					on_token_read_distance -= 1;
				}

			}

#if EMIT_TOKEN == 1
				//if(FLS(1).type != T_STRING &&
				//	FLS(1).type != T_STRING_IDENTIFIER) {
				//	lex_print(&FLS(1));
				//}
				print_all_lex();
#endif

#if DEBUG_PARSER == 1
			if(expressions == NULL) {
				expressions_count = 1;
				expressions = calloc(expressions_count, sizeof(var_expr));
				var_expression_init(&*expressions);
			}

			enum on_parse_state state = start;
			enum on_parse_start_symbol start_symbol = none;
			uint8_t last_on_parse = FUNC_OK;
			SPACE_SIZE token_index = symbols_count;

		  	while(token_index != 0) {
			    //parser_emit_reconstruct(&expressions[0]);
				//lex_emit(&FLS(token_index));

				//QUICK HACK!!! forget that there are string identifiers, because this is not important while parsing
				//The string already has its own identifier
				if(FLS(token_index).type == T_STRING_IDENTIFIER){
					DBUG("skipping T_STRING_IDENTIFIER while parsing...");
					token_index--;
					continue;
				}
#if DEBUG_MAIN == 1				
				printf("token index in main: %d \n", token_index);
#endif
				last_on_parse = on_parse(
					&FLS(token_index),
					symbols_count - token_index,
					&FLXP(1),
					&expressions_count,
					&arguments,
					&arguments_count,
					&start_symbol,
					&state);

				if (last_on_parse == FUNC_NOT_OK)
				{
					exit(EXIT_FAILURE);
				}

				if (last_on_parse == PARSER_DONE)
				{
					HLINE
            		DBUG("PARSER_DONE. allocating next expr");
					expressions_count += 1;
					RESIZE(expressions, var_expr, expressions_count)
					var_expression_init(&FLXP(1));
					state = start;
					continue;
					
					//this is a quick bug fix, making 3000 bugs more. somehow the "parser" stops at the next instruction, so when we take here the next instruction, it doesn't know what to do. would work if the next next instruction is an operation, instead, we should check WHY the parser stops at that instruction
				}
				token_index--;
			}
//#if EMIT_JSON == 1
			//print_all_expr();
//#endif
#endif
//do a check if THREAD_NUM * MAX_CALL_STACK > SPACE_SIZE
//kill programm because of overflows
#ifndef COMPARING_TYPE
#define COMPARING_TYPE unsigned long            
#endif
            if((COMPARING_TYPE)(SPACE_SIZE)~(0) < (COMPARING_TYPE)CALLSTACK_THREAD) {
                DIE("CALLSTACK_THREAD is bigger than SPACE_SIZE");
            }
            const SPACE_SIZE callstack_max_value = ~(0);
            //callstack is allocated for all possible threads, so it's fixed
            //ROP me whatever
			SPACE_SIZE max_call_stack = MAX_CALL_STACK;
			callstack = calloc(sizeof(SPACE_SIZE), CALLSTACK_THREAD);
			if(callstack == NULL) {
				DIE("callstack cannot be allocated");
			}
			for(SPACE_SIZE cidx = 0; cidx < CALLSTACK_THREAD; cidx++) {
			    callstack[cidx] = callstack_max_value;
			}
#if DEBUG_MACHINE == 1
			HLINE
			DBUG("looking for main function...");
			DBUG(expressions_count);
			HLINE
			var_expr* start_function = find_function_of_execution(
								start_function_name,
								&FLXP(1),
								&expressions_count, 
								&FLV(1),
								&variables_count);
			DBUG("going for run");
			if(variables == NULL) {
				variables = calloc(1, sizeof(var_mem));
				const char* args_name = "args";

				(*variables).name = malloc(sizeof(char)*strlen(args_name)+1);
				strcpy((*variables).name, args_name);
				(*variables).name[4] = '\0';

				//variables[0].init = 1;
				(*variables).value = malloc(sizeof(char)* strlen(argv[ac]) + 1);
				if((*variables).value == NULL) {
					DIE("failed trying to malloc args to args inside of the script.");					
				}
				strcpy((*variables).value, argv[ac]);
				(*variables).value[strlen(argv[ac])] = '\0';
				(*variables).scope_ref_idx = start_function->scope_ref - 1;
                (*variables).type = T_ALNUM;

				variables_count = 1;
			}
			run(start_function_name,
				start_function, 
				&expressions[0],
				&FLXP(1), 
				expressions_count,
				&variables, 
				&variables_count,
				&arguments,
				&arguments_count,
				&callstack,
				&call_stack_count,
				&max_call_stack,
				symbols);
#endif

			fclose(esrc_file_);
			
		}
		expressions_destroy();
		expressions = NULL;

		symbols_destroy();		
		symbols = NULL;

		arguments_destroy();
		arguments = NULL;

		call_stack_destroy();
		callstack = NULL;

		variables_destroy();
		variables = NULL;
	}
	return FUNC_OK;
}
