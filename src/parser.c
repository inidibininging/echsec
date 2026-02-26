#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "parser.h"
#include "lex.h"
#include "config.h"
#include "token_def.h"
#include "symbol_fns.h"
#include "meta.h"
#include "types.h"

#if DEBUG_PARSER == 0
#undef DBUG
#define DBUG(msg) (void)0;
#undef HLINE
#define HLINE 

#endif

//var_expr_ref_add arguments applied
#define REF_ARGS_APPL expression, token_index, arguments, arguments_count
#define ADD_ARG var_expression_value_ref_add(REF_ARGS_APPL);
#define BACKTRACE_SIZE 5
#define BACKTRACE_PRINT SPACE_SIZE bt_end = token_index; \
    SPACE_SIZE bt_start = token_index - BACKTRACE_SIZE < token_index ? \
    token_index - BACKTRACE_SIZE : \
    0; \
    SPACE_SIZE bt_diff_a = bt_end - bt_start;\
    SPACE_SIZE bt_diff_b = bt_end - bt_start;\
    if(bt_diff_a >= bt_diff_b) {\
        bt_diff_a = bt_diff_b;\
    }\
    if(bt_start >= bt_end) {\
        bt_start = bt_end;\
    } else {\
        printf("%s->%s ", WARN_COLOR, ERRO_COLOR); \
        while(--bt_diff_a > 0) { \
            --token; \
        } \
        while(++bt_diff_a < bt_diff_b) { \
            printf("%s ", (++token)->name); \
        } \
        printf("-> here %s\n", DBUG_COLOR);\
    }

void print_op_types() {
	HLINE
	DBUGI1("P_UNKNOWN", P_UNKNOWN);
	DBUGI1("P_SET_TAG", P_SET_TAG);
	DBUGI1("P_SET_REF", P_SET_REF);
	DBUGI1("P_FUNCTION", P_FUNCTION);
	DBUGI1("P_EXECUTION", P_EXECUTION);
	DBUGI1("P_INTERNAL", P_INTERNAL);
	DBUGI1("P_IF", P_IF);
	DBUGI1("P_IF_EQ", P_IF_EQ);
	DBUGI1("P_IF_CONTAINS", P_IF_CONTAINS);
	DBUGI1("P_ENDIF", P_ENDIF);
	DBUGI1("P_RETURN", P_RETURN);
	HLINE
}



void parser_print(const var_expr* expression) {
#if MODE == DEBUG_MODE || DEBUG_PARSER == 1
	HLINE_DFUNC_START	
	if(expression == NULL) {
		DIE("expr given is null");
	}
	parser_print_op_type(expression);	

	if(expression->name_ref.name == NULL) {
		DBUG("expr has no name");
	}
	else {
		if(expression->name_ref.type == T_ALNUM) {
			DBUGS1("expr name", expression->name_ref.name);
			DBUGP1("expr name ptr", expression->name_ref.name);
		}

		DBUGP1("expr ptr", expression);
	}
	HLINE_DFUNC_END
#endif
}

void parser_print_op_type(const var_expr* expression) {
#if MODE == DEBUG_MODE
	HLINE_DFUNC_START
	const char* opType = "expr op type";
	if(expression == NULL){ 
		DBUG("expr is null");
		return;	
	}
	DBUGI1("expr op type as nr", expression->operation_type);
	if(expression->operation_type == P_UNKNOWN){ DBUGS1(opType, "P_UNKNOWN"); }
	if(expression->operation_type == P_SET_TAG){ DBUGS1(opType, "P_SET_TAG"); }
  if(expression->operation_type == P_SET_REF){ DBUGS1(opType, "P_SET_REF"); }
	if(expression->operation_type == P_FUNCTION){ DBUGS1(opType, "P_FUNCTION"); }
	if(expression->operation_type == P_EXECUTION){ DBUGS1(opType, "P_EXECUTION"); }
	if(expression->operation_type == P_INTERNAL){ DBUGS1(opType, "P_INTERNAL"); }
	if(expression->operation_type == P_IF){ DBUGS1(opType, "P_IF"); }
	if(expression->operation_type == P_IF_EQ){ DBUGS1(opType, "P_IF_EQ"); }
	if(expression->operation_type == P_IF_CONTAINS){ DBUGS1(opType, "P_IF_CONTAINS"); }
	if(expression->operation_type == P_ENDIF){ DBUGS1(opType, "P_ENDIF"); }
	if(expression->operation_type == P_RETURN){ DBUGS1(opType, "P_RETURN"); }
  HLINE_DFUNC_END
#endif
}

void parser_emit(const var_expr *expression,
				symbol *tokens,
				SPACE_SIZE **arguments,
				const SPACE_SIZE *arguments_count) {
#if MODE != DEBUG_MODE
  return;
#endif
  HLINE_DFUNC_START
	printf("{ \"operation_type\": %i, \"name_ref\":",expression->operation_type );
	if(&expression->name_ref == NULL || 
		expression->operation_type == P_ENDIF ||
		expression->name_ref.name == NULL) {
		printf("{}");
	}
	else {
		lex_emit(&expression->name_ref);
	}
	printf(", \"value_ref\":");
  	printf("[\n");
 	if(expression->operation_type != P_ENDIF) {
      	SPACE_SIZE op = 0;
  		while(op != expression->value_ref_length) {
			SPACE_SIZE arg_index = var_expr_value_ref_get_token_index(
					expression, 
					op, 
					arguments, 
					arguments_count);
			symbol* s =  &(tokens[arg_index]);
			if(s == NULL) {
				DIE("symbol is null. like wtf are you doing?");
			}
			
			lex_emit(s);
			if(op != expression->value_ref_length - 1 && expression->value_ref_length != 1) {
				  printf(",");
			}
    		op++;
  		}
	}
  	printf("] }\n");
  HLINE_DFUNC_END
}

symbol* parser_value_ref_copy(const var_expr* expression,
                                  SPACE_SIZE index,
                                  symbol* tokens,
                                  SPACE_SIZE **arguments,
								  const SPACE_SIZE *arguments_count)
{
	DFUNC
	DBUG("getting arg_index");
  SPACE_SIZE arg_index = var_expr_value_ref_get_token_index(
					expression,
					index,
					arguments,
					arguments_count);
	DBUGI1("arg_index", arg_index);
  symbol* s =  &(tokens[arg_index]);
  if(s == NULL) {
    ERROS1("expr name", expression->name_ref.name);
    DIE("the following expression has no value reference. symbol is null. like wtf are you doing?");
  }
	lex_emit(s);
	symbol* read_only_symbol = malloc(sizeof(symbol));
  read_only_symbol->name = NULL;
  read_only_symbol->size = 0;
  read_only_symbol->type = s->type;

  symbol_copy_name(read_only_symbol, s->name);
  return read_only_symbol;
}

symbol parser_get_cat_values(const char* separator,
								const var_expr* expression,
								symbol* tokens,
								SPACE_SIZE **arguments,
								const SPACE_SIZE *arguments_count) 
{

	symbol read_only_symbol;
	if(expression->operation_type != P_ENDIF) {
      	SPACE_SIZE op = 0;
  		while(op != expression->value_ref_length) {
			SPACE_SIZE arg_index = var_expr_value_ref_get_token_index(
					expression, 
					op, 
					arguments, 
					arguments_count);
			symbol* s =  &(tokens[arg_index]);
			if(op != 0) {
				symbol_cat_name(&read_only_symbol, separator);
			}
			else {
				read_only_symbol.name = NULL;
				read_only_symbol.size = 0;
				symbol_copy_name(&read_only_symbol, separator);

			}
			symbol_cat_name(&read_only_symbol, s->name);
			
			if(s == NULL) {
				DIE("symbol is null. like wtf are you doing?");
			}
			
			
			if(op != expression->value_ref_length - 1 && expression->value_ref_length != 1) {
				  printf(" ");
			}
    		op++;
  		}
	}
	return read_only_symbol;
}

void var_expression_init(var_expr *expression)
{
  	expression->operation_type = T_UNKNOWN_IDENTIFIER;
  	expression->name_ref.name = NULL;
	expression->value_ref_idx = USHRT_MAX;
  	expression->value_ref_length = 0;
  	expression->scope_ref = INT8_MAX;  
}

// TODO: add index position of symbol to expression
//  -- dunno what im doing
//  OK. Had an issue freeing pointers that came out with valgrind. since then I'm avoiding ptrs in structs unless they are declared as const ptr or they are flat (without mallocs and callocs etc). I know its dumb but it's better than looking countless hours for non-freed ptrs
//  every var_expr has:
//  - a reference index for a global list called arguments
void var_expression_value_ref_add(
		var_expr* expression, 		// ptr to the expr 		
		SPACE_SIZE token_index, 		// token_index inside of main.c's symbols
		SPACE_SIZE** arguments, 		// ptr to arguments list
		SPACE_SIZE* arguments_count) 	// ptr to arguments list count 
{
	if(arguments_count == NULL) {
		DIE("arguments_count used but it is NULL. are you stupid?");
	}

	if(*arguments_count == USHRT_MAX - 1) {
		DIE("son. what are you doing? there is too much code in the force and therefor there is not enough space for arguments_count. change SPACE_SIZE to uint32_t or go fog the mountains");
	}

	DBUG("add arg up +1 to ...");
	*arguments_count += 1;
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
	printf("%d\n", *arguments_count);
#endif
	if(*arguments == NULL) {
		DBUG("*arguments is NULL. performing malloc");
		*arguments = malloc(sizeof(SPACE_SIZE));
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
		printf("ptr address of arguments %p\n", arguments);
#endif
	}
	else {
		HLINE
		DBUG("*arguments is not NULL. performing realloc");
		*arguments = realloc(*arguments,
							sizeof(SPACE_SIZE) *
							(*arguments_count));
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
		printf("ptr address of *arguments: %p\n", (*arguments));
#endif
		HLINE
	}

	DBUG("(*arguments)[*arguments_count - 1] gets token_index");
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
	printf("arg idx: %d token index: %d\n", (*arguments_count) - 1, token_index);
#endif
	
	(*arguments)[(*arguments_count) - 1] = token_index;

	// add up +1 to value_ref_idx
	if(expression->value_ref_idx == USHRT_MAX) {
		expression->value_ref_idx = (*arguments_count) - 1;
		DBUG("USHRT_MAX set for expr. going up one to...");
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
		printf("%d\n", expression->value_ref_idx);
#endif
	}
	
	expression->value_ref_length += 1;
}

// gets the item index by using the stack var item_index and adds it to the ref_idx
// THEN looks for that idx inside of arguments
// the value returned is the INDEX of main.c's symbols
// if you are wondering why arguments_count is needed => for checking if you don't exceed the size of arguments_count
// WATCH OUT! Dunno what I'm doing .. ding ding ding bull@#$% detector > 9000%
SPACE_SIZE var_expr_value_ref_get_token_index(
		const var_expr *expression,
		SPACE_SIZE item_index,
		SPACE_SIZE **arguments,
		const SPACE_SIZE *arguments_count) {
		
	// translates item_index to the corresponding argument idx 
	// checks if item_index is outside of the length of the expression
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
	//printf("item_index:%d, value_ref_idx:%d\n", item_index, expression->value_ref_idx);
#endif
	if((item_index = item_index + expression->value_ref_idx) > expression->value_ref_idx + expression->value_ref_length) {
		DIE("argument index given is longer than the array index");	
	}
	if(item_index > *arguments_count) {
		DIE("arguments_count is less than argument index given");
	}
	
	SPACE_SIZE token_index = (*arguments)[item_index];
#if DEBUG_PARSER == 1 && MODE == DEBUG_MODE
	//printf("item_index:%d, token_index:%d\n", item_index, token_index);
#endif
	
	return token_index;
}

// var_expr* var_expression_new() {
// 	N2(e, var_expr, 1);
// 	e->operation_type = T_UNKNOWN_IDENTIFIER;
// 	e->name_ref.name = NULL;
// 	e->value_ref_idx = USHRT_MAX;
// 	e->value_ref_length = 0;
// 	e->scope_ref = INT8_MAX;
// 	return e;
// }

void var_expression_destroy(var_expr *expression, const SPACE_SIZE expressions_count) 
{
	if(expression == NULL) {
		DBUG(__func__);
		DIE("expression given is null");
	}
	DIE("TODO: var_expression_destroy");
}

void parser_emit_reconstruct(const var_expr *expression,
		symbol *token,
		SPACE_SIZE **arguments, 
		const SPACE_SIZE *arguments_count)
{
  #if MODE == DEBUG_MODE && DEBUG_PARSER == 1
  HLINE_DFUNC_START

  if(expression == NULL) {
    DIE("expression is null");
  }
  if (expression->operation_type == P_RETURN)
  {
    printf("%s ", RETURN_IDENTIFIER);
  }

  if (expression->operation_type == P_IF_EQ)
  {
    printf("%s", "if");

    if (expression->name_ref.type == T_STRING)
    {
      printf(" '%s'", expression->name_ref.name);
    }

    if (expression->name_ref.type == T_ALNUM)
    {
      printf(" %s", expression->name_ref.name);
    }

    if (expression->operation_type == P_IF_EQ)
    {
      printf(" %s ", EQ_IDENTIFIER);
    }
    if (expression->operation_type == P_IF_CONTAINS)
    {
      printf(" %s ", CONTAINS_IDENTIFIER);
    }

    // if(expression->value_ref->type == T_STRING) 
	// {
    //   printf(" '%s'", expression->value_ref->name);
    // }

    // if(expression->value_ref->type == T_ALNUM) 
	// {
    //   printf(" %s", expression->value_ref->name);
    // }
  }

  if (expression->operation_type == P_ENDIF)
  {
    printf("%s", ENDIF_IDENTIFIER);
  }

  if (expression->operation_type == P_FUNCTION)
  {
    printf("%s %s ", FUNCTION_IDENTIFIER, expression->name_ref.name);
  }
  if (expression->operation_type == P_EXECUTION)
  {
    printf("%s %s ", EXECUTION_IDENTIFIER, expression->name_ref.name);
  }
  if (expression->operation_type == P_INTERNAL)
  {
    printf("%s %s ", INTERNAL_FUNCTION_IDENTIFIER, expression->name_ref.name);
  }
  if (expression->operation_type == P_SET_TAG)
  {
    printf("%s %s %s = ", SET_IDENTIFIER, STRING_TYPE, expression->name_ref.name);
  }
  if (expression->operation_type == P_SET_REF)
  {
    printf("%s %s %s = ", SET_IDENTIFIER, STRING_TYPE, expression->name_ref.name);
  }
  if (expression->value_ref_length > 0)
  {
    if (expression->operation_type == P_FUNCTION ||
        expression->operation_type == P_EXECUTION ||
        expression->operation_type == P_INTERNAL)
    {
      printf("(");
    }

	//uint8_t arg_idx = expression->value_ref_idx;
	SPACE_SIZE ref_idx = 0;
	uint8_t end_idx = expression->value_ref_length;
	SPACE_SIZE token_index = 0;

	symbol* s = token;

    for (;
		ref_idx < end_idx; 
		ref_idx++)
    {
	  token_index = var_expr_value_ref_get_token_index(expression, ref_idx, arguments, arguments_count);
	  token = &token[token_index];

	  if (s->type == T_STRING)
	  {
		printf("'%s'", s->name);
	  }
	  else
	  {
		printf("%s", s->name);
	  }

	  if (ref_idx != expression->value_ref_length - 1)
	  {
		printf("%s", ",");
	  }
    }
    if (expression->operation_type == P_FUNCTION ||
        expression->operation_type == P_EXECUTION ||
        expression->operation_type == P_INTERNAL)
    {
      printf(")");
    }
	
	
  }
  printf("%s", "\n");
  HLINE_DFUNC_END
  #endif
}

int8_t parser_print_error(const int8_t current, const int8_t expected)
{
  // if(current != expected) {

  ERRO("Syntax Error.");
  printf(" current:%d expected:%d\n", current, expected);
  if (expected == P_OPTIONAL_START)
  {
    DIE("P_OPTIONAL_START");
  }
  if (expected == P_OPTIONAL_END)
  {
    DIE("P_OPTIONAL_END");
  }
  if (expected == P_REPEAT_START)
  {
    DIE("P_REPEAT_START");
  }
  if (expected == P_REPEAT_END)
  {
    DIE("P_REPEAT_END");
  }

  return FUNC_NOT_OK;
}

void on_parse_state_print(const enum on_parse_state *s)
{
  if (s == NULL)
  {
    DBUG("state is not set");
    return;
  }
  switch (s[0])
  {
  case start:
    DBUG("start");
    return;
  case found_tag_type:
    DBUG("found_tag_type");
    return;
  case write_name_ref:
    DBUG("write_name_ref");
    return;
  case await_argument_list:
    DBUG("await_argument_list");
    return;
  case wrote_next_argument_ref:
    DBUG("wrote_next_argument_ref");
    return;
  case await_next_argument_ref:
    DBUG("await_next_argument_ref");
    return;
  case await_tag_type:
    DBUG("await_tag_type");
    return;
  case await_assign_type:
    DBUG("await_assign_type");
    return;
  case await_variable_or_string:
    DBUG("await_variable_or_string");
    return;
  case await_cond_left:
    DBUG("await_cond_left");
    return;
  case found_cond_left:
    DBUG("found_cond_left");
    return;
  case await_op:
    DBUG("await_op");
    return;
  case found_op:
    DBUG("found_op");
    return;
  case await_cond_right:
    DBUG("await_cond_right");
    return;
  case found_cond_right:
    DBUG("found_cond_right");
    return;
  case done_string:
    DBUG("done_string");
    return;
  case done:
    DBUG("done");
    return;
  }
}

/*
 * JUST FOR REFERENCE
    symbol* token, 						// current token
	const SPACE_SIZE token_index, 		// the current length of the token index
    var_expr* expression, 				// current expression
	const SPACE_SIZE* expressions_count, 	// ptr to the expr list
	SPACE_SIZE** arguments,				// arguments list
	SPACE_SIZE* arguments_count,			// ptr to the arg list
    enum on_parse_start_symbol *start_symbol, 	// says what rule the "parser" is in => a function, execution, set variable etc...
    enum on_parse_state *state)					// internal state of the rule (e.g in ":a(x)" => im on "awaiting the function identifier => :")
												// */
												//
int8_t on_parse(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
	if(token == NULL) {
		DIE("leaks.. leaks everywhere...");
	}
  
  if (*state == done)
  {
    *state = start;
    start_symbol[0] = done_with_decl;
    	
	//parser_emit_reconstruct(expression);
	DBUG("TODO: parser_emit_reconstruct needs stuff in order to print expr");
    
	//moves pointer back to the next scope
	if(expression->operation_type != P_FUNCTION)
	{
		on_parse_set_tag_scope(
				token,
				token_index,
				expression,
				expressions_count,
				arguments,
				arguments_count,
				start_symbol,
				state);
	}
    return PARSER_DONE;
  }
  
  if (token->type == T_FUNCTION_IDENTIFIER ||
      start_symbol[0] == fn_decl_start)
  {
    DBUG("fn_decl_start");
    start_symbol[0] = fn_decl_start;
    expression->operation_type = P_FUNCTION;
    return on_parse_function_declaration(
        token,
		token_index,
        expression,
		expressions_count,
		arguments,
		arguments_count,
		start_symbol,
        state);
  }
  if (token->type == T_EXECUTION_IDENTIFIER ||
      start_symbol[0] == exec_decl_start)
  {
    DBUG("exec_decl_start");
    start_symbol[0] = exec_decl_start;
    expression->operation_type = P_EXECUTION;
    return on_parse_execution_declaration(
        token,
	token_index,
        expression,
	expressions_count,
        arguments,
	arguments_count,
	start_symbol,
        state);
  }
  if (token->type == T_INTERNAL_FUNCTION_IDENTIFIER ||
      start_symbol[0] == ifn_decl_start)
  {
    DBUG("internal_decl_start");
    start_symbol[0] = ifn_decl_start;
    expression->operation_type = P_INTERNAL;
    return on_parse_internal_execution_declaration(
        token,
	token_index,
        expression,
	expressions_count,
        arguments,
	arguments_count,
	start_symbol,
        state);
  }
  if (token->type == T_IF_IDENTIFIER ||
      start_symbol[0] == if_start)
  {
    return on_parse_if_declaration(
        token,
	token_index,
        expression,
	expressions_count,
        arguments,
	arguments_count,
        start_symbol,
        state);
  }


  if (token->type == T_SET_IDENTIFIER ||
      start_symbol[0] == set_tag_decl_start ||
      start_symbol[0] == set_ref_decl_start)
  {
    DBUG("checking ... for set");
    if (token->type == T_SET_IDENTIFIER &&
	  token[1].type == T_STRING_TYPE) {
      //DBUG("this is a set tag!!!!");	
      start_symbol[0] = set_tag_decl_start;
      expression->operation_type = P_SET_TAG;
    }
    if(expression->operation_type == P_SET_TAG) {
	return on_parse_set_tag_declaration(
        	token,
		token_index,
        	expression,
		expressions_count,
		arguments,
		arguments_count,
		start_symbol,
		state);
    }
    if (token->type == T_SET_IDENTIFIER &&
        token[1].type == T_NR_TYPE) {
        DBUG("this is a ref tag!!!!");	
        start_symbol[0] = set_ref_decl_start;
        expression->operation_type = P_SET_NR;
        }
        if(expression->operation_type == P_SET_NR) {
        return on_parse_set_nr_declaration(
          token,
          token_index,
          expression,
          expressions_count,
          arguments,
          arguments_count,
          start_symbol,
          state);
        }

      }

    if (token->type == T_SET_IDENTIFIER &&
	token[1].type == T_REF_TYPE) {
	DBUG("this is a ref tag!!!!");	
	start_symbol[0] = set_ref_decl_start;
	expression->operation_type = P_SET_REF;
    }
    if(expression->operation_type == P_SET_REF) {
	return on_parse_set_ref_declaration(
      token,
      token_index,
      expression,
      expressions_count,
      arguments,
      arguments_count,
      start_symbol,
      state);
    }

  

  if (token->type == T_ENDIF_IDENTIFIER ||
      start_symbol[0] == endif_start)
  {
    DBUG("endif_start");
    start_symbol[0] = endif_start;    
    return on_parse_endif_declaration(
        token,
		token_index,
        expression,
		expressions_count,
        arguments,
		arguments_count,
		start_symbol,
        state);
  }
  
  if (token->type == T_RETURN_IDENTIFIER ||
      start_symbol[0] == return_decl_start)
  {
    DBUG("return_decl_start");
    start_symbol[0] = return_decl_start;
    expression->operation_type = P_RETURN;
    return on_parse_return_declaration(
        token,
		token_index,
        expression,
		expressions_count,
        arguments,
		arguments_count,
		start_symbol,
        state);
  }

  if (*state == start &&
	  *start_symbol != fn_decl_start &&
	  *start_symbol != exec_decl_start &&
	  *start_symbol != ifn_decl_start &&
	  *start_symbol != set_tag_decl_start &&
	  *start_symbol != set_ref_decl_start &&
	  *start_symbol != return_decl_start &&
	  *start_symbol != if_start &&
	  *start_symbol != endif_start) {
	  lex_emit(token);
    BACKTRACE_PRINT
  	DIE("\nSyntax error in declaration. Parser didn't recognize current part of statement as either:\n- a function declaration\n- an execution declaration\n- an if declaration\n- a set tag declaration\n- a ref declaration\n");
  }

		
  return FUNC_NOT_OK;
}

int8_t on_parse_return_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  DFUNC
  // saves the return value into name_ref... until I dare to use binary trees and TRUE parsing
  // syntax checking
  if (token->type == T_RETURN_IDENTIFIER &&
      *state != start)
  {
    DIE("Syntax error. T_RETURN_IDENTIFIER found, but it is not a start");
    return FUNC_NOT_OK;
  }
  if (token->type == T_RETURN_IDENTIFIER &&
      *state == start)
  {
    // get ready for storing the next T_ALNUM
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_STRING_IDENTIFIER &&
      (*state != write_name_ref && state[0] != done_string))
  {
    DIE("Syntax error. type of token is a string identifier but state is NOT either write_name_ref or done_string");
    return FUNC_NOT_OK;
  }

  if (token->type == T_STRING_IDENTIFIER && *state == done_string)
  {
    *state = done;
    HLINE
    DBUG("done");
    HLINE
    return FUNC_OK;
  }
  
  // this will fall away because we dont have anything here that says "hey. this lex is a string". Should change later on or by the self-written parser
  if (
      // token->type == T_STRING &&
      *state != write_name_ref)
  {
    DIE("Syntax error. token string found, but state is not write_name_ref");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {

    //if (expression->value_ref_idx == USHRT_MAX)
    //{
      DBUG("writing string and tanga");

	  //var_expression_value_ref_add(expression, token_index, arguments, arguments_count);
	  ADD_ARG
      //expression->value_ref_idx = arguments;
      //expression->value_ref_length = 1;

      *state = done;
      HLINE
      DBUG("done");
      HLINE
    //}
    return FUNC_OK;
  }

  if (token->type == T_STRING &&
      *state == write_name_ref)
  {

    //if (expression->value_ref_idx == USHRT_MAX)
    //{
      DBUG("writing string and tanga");
	  //var_expression_value_ref_add(expression, token_index, arguments, arguments_count);
      ADD_ARG
	  //expression->value_ref_idx = arguments;
      //expression->value_ref_length = 1;
	  
      *state = done_string;
      HLINE
      DBUG("done_string");
      HLINE
    //}
    return FUNC_OK;
  }

  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;

}

int8_t on_parse_function_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DFUNC
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    DIE("Error while parsing. Found an expression with NULL value");
  }
  // syntax checking
  if (token->type == T_FUNCTION_IDENTIFIER &&
      *state != start)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a function identifier (ff), but something else was expected. ");
    return FUNC_NOT_OK;
  }

  if (token->type == T_FUNCTION_IDENTIFIER &&
      *state == start)
  {
    // get ready for storing the next T_ALNUM
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      (*state != write_name_ref &&
       *state != await_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a variable/id, but something else was expected. Are you missing a var separator (,)?");
    //printf("state is %d. see parser.h\n", *state);
    return FUNC_NOT_OK;
  }

  // write fn name
  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {
    // IT'S A TRAP! IT'S A POINTER !!! PROBABLY FROM symbols. Set this IF name_ref was NULL, otherwise, start whining
    if (expression->name_ref.name != NULL)
    {
      DIE("function name was already set. WTF ARE YOU DOING?");      
      return FUNC_NOT_OK;
    }
    else 
    {
      DBUG("proceeding with expression->name_ref = token");
      expression->name_ref.type = token->type;
      symbol_copy_name(&expression->name_ref, token->name);
    }

    if (expression->name_ref.name == NULL)
    {
      BACKTRACE_PRINT
      DIE("error while assigning a function name. name_ref is null");
      DBUG(token->name);
    }

    if (expression->name_ref.name == NULL)
    {
      BACKTRACE_PRINT
      DIE("error while copying a function name. name_ref.name is null");
      DBUG(token->name);
    }

    *state = await_argument_list;
    DBUG("await_argument_list");
    return FUNC_OK;
  }

  // beginning the argument list e.g (a,b,a) in :myfunc(a,b,a)
  if (token->type == T_L_PAREN &&
      *state == await_argument_list)
  {
    // TODO: prevent overriding
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  if ((token->type == T_ALNUM || token->type == T_STRING) &&
      *state == await_next_argument_ref)
  {
	
	// expand array if needed
	ADD_ARG
  
	*state = wrote_next_argument_ref;
    DBUG("wrote_next_argument_ref");

    if (expression == NULL)
    {
      DIE("OH NO! RESIZING THE expression LENGTH");
      return FUNC_NOT_OK;
    }

    return FUNC_OK;
  }

  if (token->type == T_VAR_SEPARATOR &&
      *state != wrote_next_argument_ref)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a var separator but no argument was given before");
    return FUNC_NOT_OK;
  }

  // write fn argument (2..n)
  if (token->type == T_VAR_SEPARATOR &&
      *state == wrote_next_argument_ref)
  {
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  // this means :a(a,) is totally valid. need to check if this is actually ok
  if (token->type == T_R_PAREN &&
      (*state != await_next_argument_ref &&
       *state != wrote_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a right parenthesis, but something else was expected. Are you missing the left parenthesis, variables or var separator (,)?");
    return FUNC_NOT_OK;
  }

  if (token->type == T_R_PAREN &&
      (*state == await_next_argument_ref ||
       *state == wrote_next_argument_ref))
  {
    *state = done;
    DBUG("T_R_PAREN.. we are at the end");
    return FUNC_OK;
  }

  BACKTRACE_PRINT
  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;
}

int8_t on_parse_execution_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state) 
{
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    DIE("Error while parsing. Found an expression with NULL value");
  }
  // syntax checking
  if (token->type == T_EXECUTION_IDENTIFIER &&
      *state != start)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found an execution identifier, but something else was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_EXECUTION_IDENTIFIER &&
      *state == start)
  {
    // get ready for storing the next T_ALNUM
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      (*state != write_name_ref &&
       *state != await_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a variable/id, but something else was expected. Are you missing a var separator (,) ?");
    printf("state is %d. see parser.h\n", *state);
    return FUNC_NOT_OK;
  }

  // write fn name
  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {
    // IT'S A TRAP! IT'S A POINTER !!! PROBABLY FROM symbols. Set this IF name_ref was NULL, otherwise, start whining
    if (expression->name_ref.name != NULL)
    {
      DIE("function name was already set. WTF ARE YOU DOING?");
      DBUG(expression->name_ref.name);
      return FUNC_NOT_OK;
    }
    //expression->name_ref = token;
    expression->name_ref.name = malloc(sizeof(char)*token->size);
    expression->name_ref.type = token->type;
    symbol_copy_name(&expression->name_ref, token->name);
    DBUG(token->name);
    if (expression->name_ref.name == NULL)
    {
      DIE("error while copying a function name");
    }

    *state = await_argument_list;
    DBUG("await_argument_list");
    return FUNC_OK;
  }

  // beginning the argument list e.g (a,b,a) in :myfunc(a,b,a)
  if (token->type == T_L_PAREN &&
      *state == await_argument_list)
  {
    // TODO: prevent overriding
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  if ((token->type == T_ALNUM || token->type == T_STRING) &&
      *state == await_next_argument_ref)
  {
    // expand array if needed
	ADD_ARG
	  
	*state = wrote_next_argument_ref;
    DBUG("wrote_next_argument_ref");

    if (expression == NULL)
    {
      DIE("OH NO! RESIZING THE expression LENGTH");
      return FUNC_NOT_OK;
	}

    return FUNC_OK;
  }


  if(token->type == T_STRING_IDENTIFIER && 
      (*state == await_next_argument_ref ||
      *state == wrote_next_argument_ref)) {
    //DBUG("TODO: Found some T_STRING_IDENTIFIER with either await_next_argument_ref or wrote_next_argument_ref");
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a string identifier, but something else was expected");
    return FUNC_OK;
  }

  if (token->type == T_VAR_SEPARATOR &&
      *state != wrote_next_argument_ref)
  {
    DIE("Syntax error. Parser found a var separator (,) but no argument was given before");
    return FUNC_NOT_OK;
  }

  // write fn argument (2..n)
  if (token->type == T_VAR_SEPARATOR &&
      *state == wrote_next_argument_ref)
  {
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  // this means :a(a,) is totally valid. need to check if this is actually ok
  if (token->type == T_R_PAREN &&
      (*state != await_next_argument_ref &&
       *state != wrote_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a right parenthesis but something else was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_R_PAREN &&
      (*state == await_next_argument_ref ||
       *state == wrote_next_argument_ref))
  {
    *state = done;
    DBUG("T_R_PAREN.. we are at the end");
    return FUNC_OK;
  }
  //on_parse_state_print(state);
  BACKTRACE_PRINT
  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;
}

int8_t on_parse_internal_execution_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    DIE("Error while parsing. Found an expression with NULL value");
  }
  // syntax checking
  if (token->type == T_INTERNAL_FUNCTION_IDENTIFIER &&
      *state != start)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found an internal function identifier, but it was expecting something else");
    return FUNC_NOT_OK;
  }

  if (token->type == T_INTERNAL_FUNCTION_IDENTIFIER &&
      *state == start)
  {
    // get ready for storing the next T_ALNUM
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      (*state != write_name_ref &&
       *state != await_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found variable/id, but something else was expected. Are you missing a separator (,) ?");
    printf("state is %d. see parser.h\n", *state);
    return FUNC_NOT_OK;
  }

  // write fn name
  if ((token->type == T_ALNUM) &&
      *state == write_name_ref)
  {
    // IT'S A TRAP! IT'S A POINTER !!! PROBABLY FROM symbols. Set this IF name_ref was NULL, otherwise, start whining
    if (expression->name_ref.name != NULL)
    {      
      DIE("function name was already set. WTF ARE YOU DOING?");
      DBUG(expression->name_ref.name);
      return FUNC_NOT_OK;
    }

    //expression->name_ref = token;
    expression->name_ref.type = token->type;
    symbol_copy_name(&expression->name_ref, token->name);

    if (expression->name_ref.name == NULL)
    {
      DIE("error while copying a function name");
      DBUG(token->name);
    }

    *state = await_argument_list;
    DBUG("await_argument_list");
    return FUNC_OK;
  }

  // beginning the argument list e.g (a,b,a) in :myfunc(a,b,a)
  if (token->type == T_L_PAREN &&
      *state == await_argument_list)
  {
    // TODO: prevent overriding
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  if ((token->type == T_ALNUM || token->type == T_STRING) &&
      *state == await_next_argument_ref)
  {
    // expand array if needed
	ADD_ARG

	*state = wrote_next_argument_ref;
    DBUG("wrote_next_argument_ref");

    if (expression == NULL)
    {
      DIE("OH NO! RESIZING THE expression LENGTH");
      return FUNC_NOT_OK;
    }

    return FUNC_OK;
  }

  if(token->type == T_STRING_IDENTIFIER && 
      (*state == await_next_argument_ref ||
      *state == wrote_next_argument_ref)) {
    return FUNC_OK;
  }

  if (token->type == T_VAR_SEPARATOR &&
      *state != wrote_next_argument_ref)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a variable separator but no argument was given before");
    return FUNC_NOT_OK;
  }

  // write fn argument (2..n)
  if (token->type == T_VAR_SEPARATOR &&
      *state == wrote_next_argument_ref)
  {
    *state = await_next_argument_ref;
    DBUG("await_next_argument_ref");
    return FUNC_OK;
  }

  // this means :a(a,) is totally valid. need to check if this is actually ok
  if (token->type == T_R_PAREN &&
      (*state != await_next_argument_ref &&
       *state != wrote_next_argument_ref))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found right parenthesis but something else was expected. Either no \"(\" was specified OR you ended with a var separator (,)");
    return FUNC_NOT_OK;
  }

  if (token->type == T_R_PAREN &&
      (*state == await_next_argument_ref ||
       *state == wrote_next_argument_ref))
  {
    *state = done;
    DBUG("T_R_PAREN.. we are at the end");
    return FUNC_OK;
  }
    
  BACKTRACE_PRINT
  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;
}

//sets the tag scope to the first function expr found (upwards)
//expression is for now &FLXP(1), which is a pointer in main and later a stack var
//does it need to be freed?
int8_t on_parse_set_tag_scope(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
	DBUG(__func__);
	//max 16 unsigned instructions.... oh oh BUG OR A FEATURE?
	SPACE_SIZE expression_idx = expressions_count[0];
	
  

  	//scope of expr is if owner == -1
  	int8_t owner = 0;
	while((expression_idx -= 1) < expressions_count[0] || expression_idx < 0) {
    
		expression--;
		if(owner == 127) {
		  DIE("ownership underflow. too much scopes within scopes. rewrite your code because it is dirty");
		}
        //changed &expression->name_ref to (see now). if something doesn't work. blame this.
		if(expression == NULL || (&(expression->name_ref) == NULL && expression->value_ref_idx == USHRT_MAX)) {
		  break;
		}
		if(expression->operation_type == P_IF_EQ || 
		  expression->operation_type == P_IF_CONTAINS || 
		  expression->operation_type == P_FUNCTION) {
		  owner -= 1;
		}
		if(owner == -127) {
		  DIE("ownership overflow. too much scopes within scopes. rewrite your code because it is dirty");
		}

		if(owner == -1) {
		  DBUG("-- SCOPE: --");
		  
		  //parser_emit_reconstruct(expression, token, arguments, arguments_count);
		  DBUG("TODO: implement how to get SCOPE");
		  //TODO: Pls remind me again of VALGRIND? Told you so   
		  expression->scope_ref = expression_idx;
		  break;
		}

		if(expression->operation_type == P_ENDIF) {
		  owner += 1;
		  break;
		}
	}
	return FUNC_OK;	
}

int8_t on_parse_set_nr_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    DIE("Error while parsing. Found an expression with NULL value");
  }

  // syntax checking
  if (token->type == T_SET_IDENTIFIER &&
      *state != start &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a set identifier but something else was expected");
    return FUNC_NOT_OK;
  }

  if (*state == start &&
      token->type == T_SET_IDENTIFIER)
  {
    *state = await_tag_type;
    DBUG("await_tag_type");
    return FUNC_OK;
  }

  if (token->type == T_NR_TYPE &&
      *state != await_tag_type &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a number type but something else was expected");
    return FUNC_NOT_OK;
  }

  if (*state == await_tag_type &&
      token->type == T_NR_TYPE)
  {
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      *state != write_name_ref &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a variable/id but something else was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {
    if (expression->name_ref.name != NULL)
    {
      DIE("function name was already set. WTF ARE YOU DOING?");
      DBUG(expression->name_ref.name);
      return FUNC_NOT_OK;
    }
    DBUG("name_ref is NULL. Invoking N");
    // N(expression->name_ref, symbol, 1)
    // expression->name_ref.name = NULL;
    //name_ref = token;
    
    expression->name_ref.type = token->type;
    symbol_copy_name(&expression->name_ref, token->name);

    if (expression->name_ref.name == NULL)
    {
      DIE("error while copying a function name");
      DBUG(token->name);
    }
    // symbol_copy_2(token, expression->name_ref);
    *state = await_assign_type;
    DBUG("await_assign_type");
    return FUNC_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state != await_assign_type)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found \"is\" but something else was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state == await_assign_type)
  {
    *state = await_variable_or_string;
    DBUG("await_variable_or_string");
    return FUNC_OK;
  }

  // string time!!!! assume anything coming from here on must be T_STRING. Else this is a TODO!!!!

  if (token->type == T_STRING_IDENTIFIER &&
      *state == await_variable_or_string)
  {
    DBUG("T_STRING_IDENTIFIER found. await_variable_or_string");
    return FUNC_OK;
  }

  // TODO HERE!!!
  if (token->type == T_STRING_IDENTIFIER &&
      (*state != await_variable_or_string && state[0] != done_string))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a string identifier but something else was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_STRING_IDENTIFIER && *state == done_string)
  {
    *state = done;
    return FUNC_OK;
  }
  
  // this will fall away because we dont have anything here that says "hey. this lex is a string". Should change later on or by the self-written parser
  if (*state != await_variable_or_string)
  {
    DIE("Syntax error. Parser was expecting a variable/id or a string");
    return FUNC_NOT_OK;
  }

  

  if (token->type == T_ALNUM &&
      *state == await_variable_or_string)
  {

    if (expression->value_ref_idx == USHRT_MAX)
    {
      DBUG("writing string and tanga");
	  ADD_ARG

      *state = done;
      HLINE
      DBUG("done");
      HLINE
    }
    return FUNC_OK;
  }

  if (token->type == T_STRING &&
      *state == await_variable_or_string)
  {

    if (expression->value_ref_idx == USHRT_MAX)
    {
      DBUG("writing string and tanga");
	  ADD_ARG

      *state = done;
      return FUNC_OK;
    }
    return FUNC_OK;
  }

  BACKTRACE_PRINT
  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;
}

int8_t on_parse_set_tag_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    BACKTRACE_PRINT
    DIE("Error while parsing. Found an expression with NULL value");
  }

  // syntax checking
  if (token->type == T_SET_IDENTIFIER &&
      *state != start &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found set identifier, but parser is not at a starting place");
    return FUNC_NOT_OK;
  }

  if (*state == start &&
      token->type == T_SET_IDENTIFIER)
  {
    *state = await_tag_type;
    DBUG("await_tag_type");
    return FUNC_OK;
  }

  if (token->type == T_STRING_TYPE &&
      *state != await_tag_type &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a string type declaration (st bx), but it was not expected");
    return FUNC_NOT_OK;
  }

  if (*state == await_tag_type &&
      token->type == T_STRING_TYPE)
  {
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      *state != write_name_ref &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser was awaiting a variable/id name in declaration");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {
    if (expression->name_ref.name != NULL)
    {
      BACKTRACE_PRINT
      DIE("function name was already set. WTF ARE YOU DOING?");
      DBUG(expression->name_ref.name);
      return FUNC_NOT_OK;
    }
    DBUG("name_ref is NULL. Invoking N");
    // N(expression->name_ref, symbol, 1)
    // expression->name_ref.name = NULL;
    //name_ref = token;
    
    expression->name_ref.type = token->type;
    symbol_copy_name(&expression->name_ref, token->name);

    if (expression->name_ref.name == NULL)
    {
      BACKTRACE_PRINT
      DIE("error while copying a function name");
      DBUG(token->name);
    }
    // symbol_copy_2(token, expression->name_ref);
    *state = await_assign_type;
    DBUG("await_assign_type");
    return FUNC_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state != await_assign_type)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found \"is\", but this was not expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state == await_assign_type)
  {
    *state = await_variable_or_string;
    DBUG("await_variable_or_string");
    return FUNC_OK;
  }

  // string time!!!! assume anything coming from here on must be T_STRING. Else this is a TODO!!!!

  if (token->type == T_STRING_IDENTIFIER &&
      *state == await_variable_or_string)
  {
    DBUG("T_STRING_IDENTIFIER found. await_variable_or_string");
    return FUNC_OK;
  }

  // TODO HERE!!!
  if (token->type == T_STRING_IDENTIFIER &&
      (*state != await_variable_or_string && state[0] != done_string))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found a string identifier but no variable/id, string or end of parsing was expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_STRING_IDENTIFIER && *state == done_string)
  {
    *state = done;
    return FUNC_OK;
  }
  
  // this will fall away because we dont have anything here that says "hey. this lex is a string". Should change later on or by the self-written parser
  if (*state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. string or variable/id found, but parser was not expecting that");
    return FUNC_NOT_OK;
  }

  

  if (token->type == T_ALNUM &&
      *state == await_variable_or_string)
  {

    if (expression->value_ref_idx == USHRT_MAX)
    {
      DBUG("writing string and tanga");
	  ADD_ARG

      *state = done;
      HLINE
      DBUG("done");
      HLINE
    }
    return FUNC_OK;
  }

  if (token->type == T_STRING &&
      *state == await_variable_or_string)
  {

    if (expression->value_ref_idx == USHRT_MAX)
    {
      DBUG("writing string and tanga");
	  ADD_ARG

      *state = done;
      return FUNC_OK;
    }
    return FUNC_OK;
  }
  BACKTRACE_PRINT
  DIE("Syntax error while parsing a set declaration");
  return FUNC_NOT_OK;
}

int8_t on_parse_set_ref_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
    SPACE_SIZE** arguments,
    SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    DIE("Error while parsing. Found an expression with NULL value");
  }

  // syntax checking
  if (token->type == T_SET_IDENTIFIER &&
      *state != start &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Set identifier found and parser was not awaiting wether a variable/id nor a string");
    return FUNC_NOT_OK;
  }

  if (*state == start &&
      token->type == T_SET_IDENTIFIER)
  {
    *state = await_tag_type;
    DBUG("await_tag_type");
    return FUNC_OK;
  }

  if (token->type == T_REF_TYPE &&
      *state != await_tag_type &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found reference type but that wasn't expected");
    return FUNC_NOT_OK;
  }

  if (*state == await_tag_type &&
      token->type == T_REF_TYPE)
  {
    *state = write_name_ref;
    DBUG("write_name_ref");
    return FUNC_OK;
  }

  if (token->type == T_ALNUM &&
      *state != write_name_ref &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Found a variable/id but the parser wasn't expecting it");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ALNUM &&
      *state == write_name_ref)
  {
    if (expression->name_ref.name != NULL)
    {        
      DIE("function name was already set. WTF ARE YOU DOING?");
      DBUG(expression->name_ref.name);
      return FUNC_NOT_OK;
    }
    DBUG("name_ref is NULL. Invoking N");
    // N(expression->name_ref, symbol, 1)
    // expression->name_ref.name = NULL;
    //name_ref = token;
    
    expression->name_ref.type = token->type;
    symbol_copy_name(&expression->name_ref, token->name);

    if (expression->name_ref.name == NULL)
    {
      DIE("error while copying a function name");
      DBUG(token->name);
    }
    // symbol_copy_2(token, expression->name_ref);
    *state = await_assign_type;
    DBUG("await_assign_type");
    return FUNC_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state != await_assign_type)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser found \"is\" but it wasn't expected");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ASSIGN_IDENTIFIER &&
      *state == await_assign_type)
  {
    *state = await_variable_or_string;
    DBUG("await_variable_or_string");
    return FUNC_OK;
  }
 
  // this will fall away because we dont have anything here that says "hey. this lex is a string". Should change later on or by the self-written parser
  if (*state != await_variable_or_string)
  {
    DIE("Syntax error. Parser is expecting a either a variable/id or a string");
    return FUNC_NOT_OK;
  }  

  if (token->type == T_ALNUM &&
      *state == await_variable_or_string &&
      expression->value_ref_length != 2)
  {

    DBUG("writing string and tanga");
    ADD_ARG 
    if(expression->value_ref_length == 2) {
	*state = done;
	HLINE
    	DBUG("done");
    	HLINE
    }
    return FUNC_OK;
  }

  BACKTRACE_PRINT
  DIE("WTF ARE U DOING???");
  return FUNC_NOT_OK;
}


int8_t on_parse_if_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state)
{
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  HLINE

  if (expression == NULL)
  {
    BACKTRACE_PRINT
    DIE("Error while parsing. Found an expression with NULL value");
  }

  // syntax checking
  if (token->type == T_IF_IDENTIFIER &&
      *state != start)
  {
    DIE("Syntax error. an if identifier was found, but the parser is not at the start. are you doing something like \"if a eq b if b\" or something like that?");
    return FUNC_NOT_OK;
  }

  if (*state == start &&
      token->type == T_IF_IDENTIFIER)
  {
    *start_symbol = if_start;
    *state = await_cond_left;
    DBUG("if statement found - looking for the left statement");
    return FUNC_OK;
  }

  // check wether left or right declaration are id or string
  if (*state == await_cond_left &&
      (token->type == T_ALNUM || token->type == T_STRING))
  {
    *state = found_cond_left;
    DBUG("found left statement. it is either an id or a string");
  }

  if (*state == await_cond_right &&
      (token->type == T_ALNUM || token->type == T_STRING))
  {
    *state = found_cond_right;
    DBUG("found right statement. it is either an id or a string");
  }

  // left declaration IS STORED IN name_ref, OP WILL BE STORED IN THE EXPRESSION TYPE, RIGHT IN value_ref ... wow...(slow clap)
  if ((token->type == T_ALNUM || token->type == T_STRING) &&
      (*state != found_cond_left && *state != found_cond_right &&
       *state != await_variable_or_string && 
       *state != await_assign_type))
  {
    
    BACKTRACE_PRINT
    DIE("Syntax error in if statement. Parser was awaiting an id (a variable) or a string, but the parser is wether waiting for the left or right statement of condition");
    return FUNC_NOT_OK;
  }

  if ((token->type == T_ALNUM || token->type == T_STRING) &&
      *state == found_cond_left)
  {
	ADD_ARG
	
	if (expression->value_ref_length == 0) {
		DIE("error while adding left statement");
	}

    *state = await_assign_type;
    DBUG("if statement set to lookup for operation type");
    return FUNC_OK;
  }

  if ((token->type == T_EQ_IDENTIFIER || token->type == T_CONTAINS_IDENTIFIER) &&
      *state != await_assign_type)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser is awaiting an equal or contains operator");
    return FUNC_NOT_OK;
  }

  if (token->type == T_EQ_IDENTIFIER &&
      *state == await_assign_type)
  {
    *state = await_variable_or_string;
    expression->operation_type = P_IF_EQ;
    DBUG("if statement found \"eq\" identifier. looking now for either an id or string");
    return FUNC_OK;
  }

  if (token->type == T_CONTAINS_IDENTIFIER &&
      *state == await_assign_type)
  {
    *state = await_variable_or_string;
    expression->operation_type = P_IF_CONTAINS;
    DBUG("if statement found \"contains\" identifier. looking now for either an id or string");
    return FUNC_OK;
  }

  if (token->type == T_STRING_IDENTIFIER &&
      *state == await_variable_or_string)
  {
    DBUG("beginning string identifier found");
    return FUNC_OK;
  }

  // TODO HERE!!!
  if (token->type == T_STRING_IDENTIFIER &&
      (*state != await_variable_or_string && 
       state[0] != done_string))
  {
    BACKTRACE_PRINT
    DIE("Syntax error. String identifier found, but the parser is not awaiting a variable/id, string or the end of parsing the beginning part of the if declaration");
    return FUNC_NOT_OK;
  }

  if (token->type == T_STRING_IDENTIFIER && 
          *state == done_string)
  {
    *state = done;
    return FUNC_OK;
  }
  
  // this will fall away because we dont have anything here that says "hey. this lex is a string". Should change later on or by the self-written parser
  if (expression->operation_type != P_IF_CONTAINS &&
          expression->operation_type != P_IF_EQ &&
      *state != await_variable_or_string)
  {
    BACKTRACE_PRINT
    DIE("Syntax error. Parser has not been identified as either an \"if ... eq ....\" expression or an \"if .... contains .... \" expression");
    return FUNC_NOT_OK;
  }

  if (token->type == T_ALNUM &&
      *state == await_variable_or_string)
  {

    DBUG("found id/variable");
	ADD_ARG

    *state = done;
    return FUNC_OK;
  }

  if (token->type == T_STRING &&
      *state == await_variable_or_string)
  {

    DBUG("found string");
	ADD_ARG

    *state = done;
    return FUNC_OK;
  }

  BACKTRACE_PRINT
  DIE("Syntax error. Could not properly parse if statement");
  return FUNC_NOT_OK;
}

int8_t on_parse_endif_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state) {
  HLINE
  DBUG(__func__);
  lex_print(token[0].type);
  DBUG(token[0].name);
  on_parse_state_print(state);
  expression->name_ref.name = NULL;
  expression->operation_type = P_ENDIF;
  *state = done;
  HLINE
  return FUNC_OK;
}



