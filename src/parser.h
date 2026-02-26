#ifndef PARSER__H
#define PARSER__H
#include "token_def.h"
#include "types.h"
#include "meta.h"

// ==============================
// PARSER DEFINITIONS
// ==============================
#define PARSER_L 0
#define PARSER_R 1
#define PARSER_DONE 2
#define PARSER_ERROR 3
#define PARSER_RULE_LENGTH_REACHED 4

#define P_OPTIONAL_START 24
#define P_OPTIONAL_END 25

#define P_REPEAT_START 26
#define P_REPEAT_END 27

#define P_TREE_LEFT_START 28
#define P_TREE_LEFT_END 29

#define P_TREE_RIGHT_START 30
#define P_TRE_RIGHT_END 31

#define P_UNKNOWN 0
#define P_SET_TAG 1
#define P_SET_REF 2
#define P_SET_NR 3
#define P_FUNCTION 4
#define P_EXECUTION 5
#define P_INTERNAL 6
#define P_IF 7
#define P_IF_EQ 8
#define P_IF_CONTAINS 9
#define P_ENDIF 10
#define P_RETURN 11

// these are the args for the parse function applications
// used because of laziness, same as above (PARSE_FN_ARGS)
#define ECPARSE_ARGS_A token, \
	token_index, \
	expression, \
	expressions_count, \
	arguments, \
	arguments_count, \
	start_symbol, \
	state
	
// shortcut name writing 
#define ECPARSE_APPLY(PARSE_FN) PARSE_FN(EC_PARSE_ARGS_A)

// #define && &&
// #define  ||||
//TODO:make tokis great again


void print_op_types();
void parser_print(const var_expr* expression);
void parser_print_op_type(const var_expr* expression);

void parser_emit(const var_expr *expression, symbol *tokens, SPACE_SIZE **arguments, const SPACE_SIZE *arguments_count);

symbol* parser_value_ref_copy(const var_expr* expression,
                                  SPACE_SIZE index,
                                  symbol* tokens,
                                  SPACE_SIZE **arguments,
                                  const SPACE_SIZE *arguments_count);

symbol parser_get_cat_values(const char* separator,
								const var_expr* expression,
								symbol* tokens,
								SPACE_SIZE **arguments,
								const SPACE_SIZE *arguments_count);
int8_t parser_print_error(const int8_t current, const int8_t expected);

void var_expression_value_ref_add(
		var_expr* expression, 		// ptr to the expr 		
		SPACE_SIZE token_index, 		// token_index inside of main.c's symbols
		SPACE_SIZE** arguments, 		// ptr to arguments list
		SPACE_SIZE* arguments_count); 	// ptr to arguments list count 

SPACE_SIZE var_expr_value_ref_get_token_index(
		const var_expr *expression,
		SPACE_SIZE item_index,
		SPACE_SIZE **arguments,
		const SPACE_SIZE *arguments_count); 

enum on_parse_state {
    start,
    done,

    write_name_ref,
    await_argument_list,
    wrote_next_argument_ref,
    await_next_argument_ref,
   
    await_cond_left,
    found_cond_left,

    await_op,
    found_op,

    await_cond_right,
    found_cond_right,

    await_tag_type,
    found_tag_type,

    await_assign_type,

    await_variable_or_string,
    // write_variable,
    // write_string,
    // await_string,
    done_string
    
};

enum on_parse_start_symbol {
  none,
  fn_decl_start,
  exec_decl_start,
  ifn_decl_start,
  set_tag_decl_start,
  set_ref_decl_start,
  return_decl_start,
  if_start,
  endif_start,
//   if_eq_decl_start,
//   if_contains_decl_start,
  done_with_decl
};

//#openration_type# name_ref value_ref
//#SET_REF# name_ref = &value_ref
//#SET_TAG# name_ref = 'value_ref'
//#FUNCTION# name_ref (value_ref*)
//#EXECUTE# name_ref (value_ref*)
//#INTERNAL# name_ref value_ref*@
//#IF_EQ# | #IF_CONTAINS# value_ref value_ref value_ref 
//...
//#END_IF#


void parser_emit_reconstruct(const var_expr* expression, 
		symbol *tokens,
		SPACE_SIZE **arguments, 
		const SPACE_SIZE *arguments_count);

// var_expr* var_expression_new();

void var_expression_init(var_expr* expression);

void var_expression_destroy(
    var_expr* expression,
    const SPACE_SIZE expressions_count
);

int8_t on_parse(symbol* token,
	SPACE_SIZE token_index,
    var_expr* expression,
    SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_if_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_endif_declaration(symbol* token,		
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);


int8_t on_parse_return_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_function_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);


int8_t on_parse_execution_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_internal_execution_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_set_tag_scope(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_set_tag_declaration(symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_set_nr_declaration(symbol* token,
    const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
    SPACE_SIZE** arguments,
    SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);

int8_t on_parse_set_ref_declaration(
	symbol* token,
	const SPACE_SIZE token_index,
    var_expr* expression,
    const SPACE_SIZE* expressions_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
    enum on_parse_start_symbol* start_symbol,
    enum on_parse_state* state);


#endif


