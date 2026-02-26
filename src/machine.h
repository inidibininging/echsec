#ifndef MACHINE__H
#define MACHINE__H
#include "parser.h"
#include "types.h"
#define VAR_MEM_RESET USHRT_MAX

SPACE_SIZE find_index_of_var(
		const char* name, 
		SPACE_SIZE owner_idx, 
		var_mem** variable_storage, 
		SPACE_SIZE* variable_storage_count);

void get_set_var(
			SPACE_SIZE expression_idx,
			var_expr* expressions, 
            SPACE_SIZE owner_idx,
            var_mem** variable_storage, 
            SPACE_SIZE* variable_storage_count,
			SPACE_SIZE** arguments,
			SPACE_SIZE* arguments_count,
			symbol* tokens);

void get_set_var_kv(const char* variable_name,
				const char* variable_value,
                const SPACE_SIZE type,
				SPACE_SIZE owner_idx,
				var_mem** variable_storage, 
				SPACE_SIZE* variable_storage_count);

uint8_t if_is_true(const var_expr* if_eq, 
					symbol* tokens, 
					SPACE_SIZE** arguments, 
					SPACE_SIZE* arguments_count, 
					var_mem** variable_storage, 
					SPACE_SIZE variable_storage_count,
					SPACE_SIZE expression_idx,
					SPACE_SIZE function_idx);

SPACE_SIZE get_scope_ref_idx(
		SPACE_SIZE fn_seek_idx, 
		const var_expr* first_expression);
	
//void free_var(const char* name, const char* function_name);
void internal_function(
		const var_expr* expression,
	 	const var_expr* first_expression,
	 	SPACE_SIZE expr_idx,
	 	symbol* tokens,
	 	var_mem** variables,
	 	SPACE_SIZE* variables_count, 
	 	SPACE_SIZE **arguments, 
	 	SPACE_SIZE* arguments_count);

SPACE_SIZE find_index_of_function(
    const char* function_name,
    var_expr* seek_expression_ptr,
    const SPACE_SIZE expression_count);

SPACE_SIZE find_index_of_function_forward(
		const char* function_name,
		const var_expr* first_expression,
		const SPACE_SIZE expression_count);

var_expr* find_function_of_execution(
    const char* function_name,
    var_expr* seek_expression_ptr,
    const SPACE_SIZE* expression_count,
    var_mem* variable_storage,
    SPACE_SIZE* variable_storage_count);

void run(
	const char* start_function,
	const var_expr* expression,
	const var_expr* first_expression,
    const var_expr* last_expression,
    const SPACE_SIZE expression_count,
    var_mem** variables,
    SPACE_SIZE* variables_count,
	SPACE_SIZE** arguments,
	SPACE_SIZE* arguments_count,
	SPACE_SIZE** call_stack,
	SPACE_SIZE* call_stack_count,
	const SPACE_SIZE* max_call_stack,
	symbol* tokens);
#endif
