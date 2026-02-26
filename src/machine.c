#include "machine.h"
#include "config.h"
#include "lex.h"
#include "meta.h"
#include "parser.h"
#include "token_def.h"
#include "types.h"
#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#ifndef IFUNC_ARGS__
#define IFUNC_ARGS__
// for abbreviation purpouses
#define IFUNC_ARGS                                                             \
  expression, first_expression, expr_idx, tokens, variables, variables_count,  \
      arguments, arguments_count
#endif

#ifndef CALL_STACK__H
#define CALL_STACK__H
// this is where the call stack will be stored
uint8_t callstack_binit = 0;
#define CALL_STACK_INIT 1
#define CALL_STACK_UNSET USHRT_MAX
#endif

#define MAX_IFS USHRT_MAX - 1

void machine_print_vars(var_mem **variable_storage,
                        SPACE_SIZE variable_storage_count) {
  HLINE_DFUNC_START
#if MODE == DEBUG_MODE
  printf("%s ------------------------ \n %s", KWHT, KNRM);
  printf("%s | owner | owner (+1) | name | value | \n %s", KWHT, KNRM);
  printf("%s ------------------------ \n %s", KWHT, KNRM);
  for (SPACE_SIZE vsidx = 0; vsidx < variable_storage_count; vsidx++) {
    printf("%s ------------------------ \n %s", KWHT, KNRM);
    printf("%s| %d | %d | %s | %s |\n %s", KWHT,
           (*variable_storage)[vsidx].scope_ref_idx,
           (*variable_storage)[vsidx].scope_ref_idx + 1,
           (*variable_storage)[vsidx].name, (*variable_storage)[vsidx].value,
           KNRM);
  }
  printf("%s ------------------------ \n %s", KWHT, KNRM);
#endif
  HLINE_DFUNC_END
}

void callstack_init(SPACE_SIZE **callstack,
                     const SPACE_SIZE *max_callstack) {
  if (callstack == NULL) {
    DIE("callstack is null");
  }
  if (max_callstack == NULL) {
    DIE("max call stack is null");
  }
  if (callstack_binit != 0) {
    DIE("callstack was already initiated but was called from somewhere");
  }
  for (SPACE_SIZE cidx = 0; cidx < *max_callstack; cidx++) {
    *callstack[cidx] = CALL_STACK_UNSET;
  }
  callstack_binit = CALL_STACK_INIT;
}

void callstack_push(const char *function_name, const var_expr *first_expression,
                     const SPACE_SIZE expression_count, SPACE_SIZE **callstack,
                     SPACE_SIZE *callstack_count,
                     const SPACE_SIZE *max_callstack) {
#if MODE == DEBUG_MODE
  HLINE_DFUNC_START
#endif

  DBUG(function_name);

#if MODE == DEBUG_MODE
  HLINE_SECTION_START("null checks")
#endif
  if (first_expression == NULL) {
    DIE("first_expression ptr given is null");
  }
  if (callstack == NULL) {
    DIE("call stack is null");
  }

  /* //compiler says is never zero
   * if(*callstack_push == NULL) {
          DIE("call stack is zero");
  }*/

  if (callstack_count == NULL) {
    DIE("call stack count is null");
  }
  if (max_callstack == NULL) {
    DIE("max call stack given is null");
  }
  if (*callstack_count == *max_callstack) {
    DIE("echsec - stack overflow. callstack_count equals max call stack");
  }
#if MODE == DEBUG_MODE
  DBUG("checking first_expression name_ref.name...");
  if (first_expression->name_ref.name != NULL) {
    DBUGS1("first_expression name:", first_expression->name_ref.name);
  }
#endif
  SPACE_SIZE fidx = find_index_of_function_forward(
      function_name, first_expression, expression_count);
  if (fidx == CALL_STACK_UNSET) {
    ERRO(function_name);
    DIE("fidx is unset. either function was not found (see name above) or "
        "something else happened");
  }

#if MODE == DEBUG_MODE
  HLINE_SECTION_END("null checks")
#endif

#if MODE == DEBUG_MODE
  HLINE_SECTION_START("call stack push info")
  DBUGI1("function idx to push", fidx);
  DBUGI1("call stack count", (*callstack_count));
  printf("call stack ptr: %p \n", callstack);
#endif

  (*callstack)[(*callstack_count)] = fidx;
  *callstack_count += 1;

#if MODE == DEBUG_MODE
  DBUGI1("function idx to push", fidx);
  DBUGI1("call stack count", (*callstack_count));
  printf("call stack ptr: %p \n", callstack);
  HLINE_SECTION_END("call stack push info")
#endif
}

SPACE_SIZE callstack_pop(SPACE_SIZE **callstack,
                          SPACE_SIZE *callstack_count) {
  HLINE_DFUNC_START
  if (*callstack == NULL) {
    DIE("call stack given is null");
  }
  if (callstack_count == NULL) {
    DIE("call stack count is null");
  }
  if (*callstack_count == 0) {
    DIE("call stack count is 0.. cannot pop");
  }
#if MODE == DEBUG_MODE
  DBUGI1("call stack count for pop", *callstack_count);
#endif

  SPACE_SIZE fidx = (*callstack)[(*callstack_count - 1)];

#if MODE == DEBUG_MODE
  DBUGI1("fidx popped", fidx);
#endif

  if (fidx == CALL_STACK_UNSET) {
    DIE("call stack was popped already (fidx == CALL_STACK_UNSET), but "
        "function index is unset for popped value. wtf are you doing?");
  }
  (*callstack)[(*callstack_count)] = CALL_STACK_UNSET;
  // CHECK FOR PARENTHESIS
  if (*callstack_count == 0) {
    DIE("callstack_count is zero but pop should be enganged");
  }

  *callstack_count -= 1;
  if (fidx == CALL_STACK_UNSET) {
    DIE("call stack was popped already after pop, but function index is unset "
        "for popped value. wtf are you doing?");
  }
  HLINE_DFUNC_END
  return fidx;
}

/// looks for the nearest variable declaration, starting from the function index
/// (function_idx) the function looks forward and looks for any variable, whose
/// distance is at shortest from the expression index given if no variable is
/// found, VAR_MEM_RESET will be returned
SPACE_SIZE find_index_of_var_nearest(const char *name,
                                     SPACE_SIZE expression_idx,
                                     SPACE_SIZE function_idx,
                                     var_mem **variable_storage,
                                     SPACE_SIZE *variable_storage_count) {
  HLINE_DFUNC_START
  DBUGS1("looking for var", name);
  DBUGI1("between", function_idx);
  DBUGI1("and", expression_idx);
  DBUG("(nearest wins)");
  if (name == NULL) {
    DIE("name is null");
  }
  SPACE_SIZE found_index = VAR_MEM_RESET;
  SPACE_SIZE found_distance = VAR_MEM_RESET;
  for (SPACE_SIZE vsidx = 0; vsidx < *variable_storage_count; vsidx++) {
    var_mem *var = &((*variable_storage)[vsidx]);

    if (var->name == NULL) {
      DIE("var->name is null");
      continue;
    }

    SPACE_SIZE distance_to_function = var->scope_ref_idx - function_idx;

    if (var->scope_ref_idx >= function_idx &&
        var->scope_ref_idx <= expression_idx && strcmp(name, var->name) == 0 &&
        distance_to_function >= 0 && distance_to_function < found_distance) {
      DBUGI1("match found", vsidx);
      DBUGS1("the variable value", var->value);
      found_distance = distance_to_function;
      found_index = vsidx;
    }
  }
  HLINE_DFUNC_END
  return found_index;
}

SPACE_SIZE find_index_of_var(const char *name, SPACE_SIZE owner_idx,
                             var_mem **variable_storage,
                             SPACE_SIZE *variable_storage_count) {
  HLINE_DFUNC_START
  DBUGS1("looking for var", name);
  DBUGI1("on", owner_idx);
  if (name == NULL) {
    DIE("name is null");
  }
  // means string with delimiters
  // means string with delimiters
  for (SPACE_SIZE vsidx = 0; vsidx < *variable_storage_count; vsidx++) {
    var_mem *var = &((*variable_storage)[vsidx]);

    // if(var->init == 0) {
    //	WARN("var is uninitialized");
    //	continue;
    // }

    if (var->name == NULL) {
      DIE("var->name is null");
      continue;
    }

    if (owner_idx == var->scope_ref_idx && strcmp(name, var->name) == 0) {
      DBUGI1("got'em !!", vsidx);
      DBUGS1("the variable value", var->value);
      HLINE_DFUNC_END
      return vsidx;
    }
  }
  DBUG("get_var: var not found");
  HLINE_DFUNC_END
  return VAR_MEM_RESET;
}

// variables are kept inside of a scope.
// editing variables outside of the function is not possible
// instead use ref => not implemented yet
// pass by value means the storage contains a variable with the name
void get_set_var(SPACE_SIZE expression_idx, var_expr *expressions,
                 SPACE_SIZE owner_idx, var_mem **variable_storage,
                 SPACE_SIZE *variable_storage_count, SPACE_SIZE **arguments,
                 SPACE_SIZE *arguments_count, symbol *tokens) {
  HLINE_DFUNC_START
  DBUGI1("expression_idx", expression_idx);
  var_expr *expression = &expressions[expression_idx];

  parser_emit(expression, tokens, arguments, arguments_count);
  DBUGI1("value ref idx", expression->value_ref_idx);

  SPACE_SIZE tok_idx = var_expr_value_ref_get_token_index(
      expression, 0, arguments, arguments_count);
  DBUGI1("tok idx", tok_idx);

  symbol *token = &tokens[tok_idx];
  lex_emit(token);
  // if its T_ALNUM -> check for the value somewhere else

  SPACE_SIZE found_var_idx = VAR_MEM_RESET;
  if (token->type == T_ALNUM) {
    // DIE("referencing some variable that is stored somewhere else is not
    // supported for now. go away");
    SPACE_SIZE variable_idx =
        find_index_of_var_nearest(token->name, owner_idx, expression_idx,
                                  variable_storage, variable_storage_count);

    if (variable_idx == VAR_MEM_RESET) {
      ERROS1("you stupid @#$%^. the variable provided is not here",
             token->name);
      DIE("die you piece of rusty crab");
    }
  }

  const char *token_name = token->name;
  SPACE_SIZE token_name_len = strlen(token_name) + 1;

  // set if not present
  found_var_idx = find_index_of_var(expression->name_ref.name, owner_idx,
                                    variable_storage, variable_storage_count);

  // no variable found
  if (found_var_idx == VAR_MEM_RESET) {
    if (*variable_storage_count == 0) {
      *variable_storage_count = (*variable_storage_count) + 1;
      *variable_storage = calloc(*variable_storage_count, sizeof(var_mem));
      if (*variable_storage == NULL) {
        DIE("Failed at a calloc on get_set_var");
      }
      DBUG("bop bop beep. calloc +1");
    } else {
      *variable_storage_count = (*variable_storage_count) + 1;
      *variable_storage = realloc(*variable_storage,
                                  sizeof(var_mem) * (*variable_storage_count));
      if (*variable_storage == NULL) {
        DIE("Failed at a realloc on get_set_var");
      }
      DBUG("beep beep bop. realloc +1");
    }

    DBUGI1("variable_storage_count", *variable_storage_count);
    // variable_storage_count will be count up. so this is the actual index of
    // the array
    found_var_idx = (*variable_storage_count) - 1;

    // HLINE
    DBUGI1("found_var_idx", found_var_idx);
    DBUGI1("owner_idx", owner_idx);
    DBUGSZT1("name_ref", expression->name_ref.size);
    DBUGS1("expr name_ref name", expression->name_ref.name);
    // HLINE

    // DBUGS1("memname", (*variable_storage)[found_var_idx].name);
    // DBUGS1("memvalue", (*variable_storage)[found_var_idx].value);
    (*variable_storage)[found_var_idx].scope_ref_idx = owner_idx;

    // create variable with name
    (*variable_storage)[found_var_idx].name =
        malloc(sizeof(char) * expression->name_ref.size);
    strcpy((*variable_storage)[found_var_idx].name, expression->name_ref.name);
    (*variable_storage)[found_var_idx].name[expression->name_ref.size - 1] =
        '\0';
  } else {
    free((*variable_storage)[found_var_idx].value);
  }

  // set the variable value
  (*variable_storage)[found_var_idx].value =
      malloc(sizeof(char) * token_name_len);
  strcpy((*variable_storage)[found_var_idx].value, token_name);
  (*variable_storage)[found_var_idx].value[token_name_len - 1] = '\0';
  HLINE_DFUNC_END
}

void get_set_var_kv(const char *variable_name, const char *variable_value,
                    const SPACE_SIZE type, SPACE_SIZE owner_idx,
                    var_mem **variable_storage,
                    SPACE_SIZE *variable_storage_count) {
  HLINE_DFUNC_START

  if (variable_name == NULL) {
    DIE("variable_name is null");
  } else {
    DBUGS1("variable_name", variable_name);
  }

  if (variable_value == NULL) {
    DIE("variable_value is null");
  } else {
    DBUGS1("variable_value", variable_value);
  }

  SPACE_SIZE variable_value_length = strlen(variable_value) + 1;
  SPACE_SIZE variable_name_length = strlen(variable_name) + 1;
  // set if not present
  SPACE_SIZE found_var_idx = find_index_of_var(
      variable_name, owner_idx, variable_storage, variable_storage_count);

  // no variable found
  if (found_var_idx == VAR_MEM_RESET) {
    if (*variable_storage_count == 0) {
      *variable_storage_count = (*variable_storage_count) + 1;
      *variable_storage = calloc(*variable_storage_count, sizeof(var_mem));
      if (*variable_storage == NULL) {
        DIE("Failed at a calloc on get_set_var");
      }
      DBUG("bop bop beep. calloc +1");
    } else {
      *variable_storage_count = (*variable_storage_count) + 1;
      *variable_storage = realloc(*variable_storage,
                                  sizeof(var_mem) * (*variable_storage_count));
      if (*variable_storage == NULL) {
        DIE("Failed at a realloc on get_set_var");
      }
      DBUG("beep beep bop. realloc +1");
    }

    DBUGI1("variable_storage_count", *variable_storage_count);
    // variable_storage_count will be count up. so this is the actual index of
    // the array
    found_var_idx = (*variable_storage_count) - 1;
    DBUGI1("found_var_idx", found_var_idx);

    (*variable_storage)[found_var_idx].scope_ref_idx = owner_idx;
    (*variable_storage)[found_var_idx].name =
        malloc(sizeof(char) * variable_name_length);
    (*variable_storage)[found_var_idx].type = type;
    strcpy((*variable_storage)[found_var_idx].name, variable_name);
    (*variable_storage)[found_var_idx].name[variable_name_length - 1] = '\0';
  } else {
    free((*variable_storage)[found_var_idx].value);
  }
  (*variable_storage)[found_var_idx].value =
      malloc(sizeof(char) * variable_value_length);
  strcpy((*variable_storage)[found_var_idx].value, variable_value);
  (*variable_storage)[found_var_idx].value[variable_value_length - 1] = '\0';

  HLINE_DFUNC_END
}

void insert_at(char *subject, const char *insert, int pos) {
  char buf[100] = {}; // 100 so that it's big enough. fill with zeros
  // or you could use malloc() to allocate sufficient space
  // e.g. char *buf = (char*)malloc(strlen(subject) + strlen(insert) + 2);
  // to fill with zeros: memset(buf, 0, 100);

  strncpy(buf, subject, pos); // copy at most first pos characters
  int len = strlen(buf);
  strcpy(buf + len, insert);        // copy all of insert[] at the end
  len += strlen(insert);            // increase the length by length of insert[]
  strcpy(buf + len, subject + pos); // copy the rest

  strcpy(subject, buf); // copy it back to subject
  // Note that subject[] must be big enough, or else segfault.
  // deallocate buf[] here, if used malloc()
  // e.g. free(buf);
}

void internal_print(const var_expr *expression, const var_expr *first_expression,
                    SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                    SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                    SPACE_SIZE *arguments_count) {
  printf("%s", "ECHSEC: ");
  SPACE_SIZE expr_arg_c = 0;
  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    if (str_or_var->type == T_STRING) {
      printf("%s", str_or_var->name);
    }
    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    else if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      if (m->value == NULL) {
        ERROS1("variable value is null", m->name);
      }
      printf("%s", m->value);
    } else {
      DIE("ok. this error shouldn't happen. this is either a new type used I "
          "haven't implemented (here) or something is REALLY weird");
    }
    free(str_or_var->name);
    free(str_or_var);
    expr_arg_c++;
  }
  printf("%s", "\n");
}

void internal_add(const var_expr *expression, const var_expr *first_expression,
                  SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                  SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                  SPACE_SIZE *arguments_count) {
  // WARN("ANDREAS MACHT MATHE + (a, b, out)");
  SPACE_SIZE expr_arg_c = 0;
  SPACE_SIZE a = 0;
  SPACE_SIZE b = 0;

  if (expression->value_ref_length != 3) {
    DIE("add function has no 3 args (a, b, out))")
  }
  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      if (expr_arg_c == 0) {
        a = atoi(m->value);
      }
      if (expr_arg_c == 1) {
        b = atoi(m->value);
      }
      if (expr_arg_c == 2) {
        free(m->value);
        int length = snprintf(NULL, 0, "%d", a + b);
        m->value = malloc(length + 1);
        snprintf(m->value, length + 1, "%d", a + b);
      }
    } else {
      DIE("ok. this error shouldn't happen. this is either a new type used I "
          "haven't implemented (here) or something is REALLY weird");
    }
    free(str_or_var->name);
    free(str_or_var);
    expr_arg_c++;
  }
  printf("%s", "\n");
}

void internal_sub(const var_expr *expression,const var_expr *first_expression,
                  SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                  SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                  SPACE_SIZE *arguments_count) {
  // WARN("ANDREAS MACHT MATHE - (a, b, out)");
  SPACE_SIZE expr_arg_c = 0;
  SPACE_SIZE a = 0;
  SPACE_SIZE b = 0;

  if (expression->value_ref_length != 3) {
    DIE("add function has no 3 args (a, b, out))")
  }
  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      if (expr_arg_c == 0) {
        a = atoi(m->value);
      }
      if (expr_arg_c == 1) {
        b = atoi(m->value);
      }
      if (expr_arg_c == 2) {
        free(m->value);
        int length = snprintf(NULL, 0, "%d", a - b);
        m->value = malloc(length + 1);
        snprintf(m->value, length + 1, "%d", a - b);
      }
    } else {
      DIE("ok. this error shouldn't happen. this is either a new type used I "
          "haven't implemented (here) or something is REALLY weird");
    }
    free(str_or_var->name);
    free(str_or_var);
    expr_arg_c++;
  }
  printf("%s", "\n");
}
void internal_len(const var_expr *expression, const var_expr *first_expression,
                  SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                  SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                  SPACE_SIZE *arguments_count) {
  // WARN("ANDREAS MACHT TANGAS POS SY");
  SPACE_SIZE expr_arg_c = 0;

  if (expression->value_ref_length != 2) {
    DIE("pos function has no 3 args (src, out))")
  }
  // variable value
  char *srcvalue = NULL; 
  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }
      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      // out
      if (expr_arg_c == 0) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("src parameter for len is not of a string or "
              "alphanumeric type")
        }
        srcvalue = m->value;
      }
      // out
      if (expr_arg_c == 1) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE(" parameter for position is not of a string or alphanumeric "
              "type")
        }
        int outval = strlen(srcvalue);
        free(m->value);
        m->value = malloc(sizeof(char) * strlen(srcvalue)+1);
        snprintf(m->value, 16, "%d", outval);
      }
    } else {
      DIE("ok. this error shouldn't happen. this is either a new type used I "
          "haven't implemented (here) or something is REALLY weird");
    }
    free(str_or_var->name);
    free(str_or_var);
    expr_arg_c++;
  }
  printf("%s", "\n");
}

void internal_pos(const var_expr *expression, const var_expr *first_expression,
                  SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                  SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                  SPACE_SIZE *arguments_count) {
  // WARN("ANDREAS MACHT TANGAS POS SY");
  SPACE_SIZE expr_arg_c = 0;

  if (expression->value_ref_length != 3) {
    DIE("pos function has no 3 args (needle, txt, out))")
  }
  char *needle = NULL;
  char *haystack = NULL;

  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      // needle
      if (expr_arg_c == 0) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("needle parameter for position is not of a string or "
              "alphanumeric type")
        }
        needle = m->value;
      }
      // txt
      if (expr_arg_c == 1) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("txt parameter for position is not of a string or alphanumeric "
              "type")
        }
        haystack = m->value;
      }
      // out
      if (expr_arg_c == 2) {
        // T_ALNUM is always a nr in this case because it refers to something
        // that can never be truly unrefered
        if (m->type != T_ALNUM) {
          DIE("out parameter for position is not of an alphanumeric type")
        }

        char *p = strstr(haystack, needle);
        int pos = -1;
        if (p) {
          pos = p - haystack;
        }
        // free(p);
        free(m->value);
        m->value = malloc(sizeof(char) * 10);
        snprintf(m->value, 10, "%d", pos);
        // free(needle);
        // free(haystack);
      }

    } else {
      DIE("ok. this error shouldn't happen. this is either a new type used I "
          "haven't implemented (here) or something is REALLY weird");
    }
    free(str_or_var->name);
    free(str_or_var);
    expr_arg_c++;
  }
  printf("%s", "\n");
}

void internal_insert(const var_expr *expression, const var_expr *first_expression,
                     SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                     SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                     SPACE_SIZE *arguments_count) {
  // WARN("ANDREAS MACHT TANGAS REIN");
  SPACE_SIZE expr_arg_c = 0;

  if (expression->value_ref_length != 3) {
    DIE("insert function has no 3 args (needle, txt, out))")
  }
  char *src = NULL;
  char *dst = NULL;
  char *dstname = NULL;
  char *atposchar = NULL;
  int atpos = 0;

  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere in or
    // into other functions like in anonymous functions, delegates/function
    // pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      // what
      if (expr_arg_c == 0) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("what parameter for position is not of a string or "
              "alphanumeric type")
        }
        src = m->value;
        DBUGS1("======> arg 0", str_or_var->name);
      }
      // dst
      if (expr_arg_c == 1) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("dst parameter for position is not of a string or alphanumeric "
              "type")
        }
        dst = m->value;
        // dst[strlen(m->value)] = '\0';
        dstname = m->name;
        DBUGS1("======> arg 1", str_or_var->name);
        DBUGS1("======> arg 1 str", dst);
      }
      // out
      if (expr_arg_c == 2) {
        // T_ALNUM is always a nr in this case because it refers to something
        // that can never be truly unrefered
        if (m->type != T_ALNUM) {
          DIE("out parameter for position is not of an alphanumeric type")
        }
        atposchar = m->value;
        atpos = atoi(atposchar);
        int whatlen = strlen(src);
        int dstlen = strlen(dst);
        int appendsize = dstlen + whatlen;

        DBUGS1("======> arg 2", str_or_var->name);
        DBUGS1("what", src);
        DBUGS1("dst", dst);
        DBUGI1("dst-len", dstlen);
        DBUGS1("atposchar", atposchar);
        DBUGI1("appendsize", appendsize);

        SPACE_SIZE var_idx_src =
            find_index_of_var(dstname, fn_seek_idx, variables, variables_count);

        // do realloc action or something else if same
        char *insert_str = malloc(sizeof(char) * appendsize + 1);
        memset(insert_str, ' ', appendsize);
        if (dstlen - atpos <= 0) {
          DIE("error in insert function. destination length is less than at "
              "position");
        }
        // start copying from left till we hit at the "at position"
        for (int i = 0; i < atpos; i++) {
          insert_str[i] = dst[i];
        }
        // start copying from the "at position" on
        SPACE_SIZE whatlen_endpos = atpos + whatlen;
        for (int i = atpos; i < whatlen_endpos; i++) {
          insert_str[i] = src[i - atpos];
        }

        // start copying from the end of the string to be copied + the rest
        for (int i = atpos; i < dstlen; i++) {
          insert_str[whatlen_endpos + (i - atpos)] = dst[atpos + (i - atpos)];
        }

        insert_str[appendsize] = '\0';
        DBUGS1("dst", dst);

        var_mem *momo = &((*variables)[var_idx_src]);
        free(momo->value);
        momo->value = insert_str;
      }
    }
  }
}

void internal_substr(const var_expr *expression, const var_expr *first_expression,
                     SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                     SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                     SPACE_SIZE *arguments_count) {
  SPACE_SIZE expr_arg_c = 0;
  DBUG("substr enganged (start, end, src, out)");
  if (expression->value_ref_length != 4) {
    DIE("substr function has no 4 args (startpos, endpos, src, dst))")
  }
  char *src = NULL;
//  char *dst = NULL;
  char *dstname = NULL;
  char *startposchar = NULL;
  char *endposchar = NULL;
  int startpos = 0;
  int endpos = 0;

  while (expr_arg_c != expression->value_ref_length) {

    // get the scope idx (function index)
    SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

    symbol *str_or_var = parser_value_ref_copy(expression, expr_arg_c, tokens,
                                               arguments, arguments_count);

    // we start looking for the referenced variable.
    // because anything that comes now must have been declared BEFORE
    // this will be true, till we introduce any random jumps somewhere
    // in or into other functions like in anonymous functions,
    // delegates/function pointers, gotos, reference assignments etc.
    if (str_or_var->type == T_ALNUM) {
      // first, assume that the variable
      // due to the uin16_t nature,
      // we cannot assess if we found something if the value is e.g -1
      SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                             variables, variables_count);

      if (var_idx == VAR_MEM_RESET) {
        ERROS1("variable value not found", str_or_var->name);
        DIE("die die die miss.");
      }
      var_mem *m = &((*variables)[var_idx]);
      if (m->name == NULL) {
        DIE("var name is null");
      } else {
        DBUGS1("var name", m->name);
      }

      if (m->value == NULL) {
        DIE("var value is null");
      } else {
        DBUGS1("var value", m->value);
      }
      if (expr_arg_c == 0) {
        // T_ALNUM is always a nr in this case because it refers to
        // something that can never be truly unrefered
        if (m->type != T_ALNUM) {
          DIE("start position parameter is not of an alphanumeric "
              "type")
        }
        startposchar = m->value;
        startpos = atoi(startposchar);
      }
      if (expr_arg_c == 1) {
        // T_ALNUM is always a nr in this case because it refers to
        // something that can never be truly unrefered
        if (m->type != T_ALNUM) {
          DIE("end position parameter is not of an alphanumeric type")
        }
        endposchar = m->value;
        endpos = atoi(endposchar);
      }

      // src
      if (expr_arg_c == 2) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("src parameter for position is not of a string or "
              "alphanumeric type")
        }
        src = m->value;
        DBUGS1("src", src);
        int srclen = strlen(src);
        if (startpos > srclen) {
          DIE("error in substr function. startpos is outside of src "
              "length");
        }
        if (endpos > srclen) {
          DIE("error in substr function. endpos is outside of end "
              "length");
        }
      }
      // dst
      if (expr_arg_c == 3) {
        if (m->type != T_STRING && m->type != T_ALNUM) {
          lex_print(m->type);
          DIE("dst parameter for position is not of a string or "
              "alphanumeric type")
        }
        //dst = m->value;
        dstname = m->name;

        SPACE_SIZE var_idx_src =
            find_index_of_var(dstname, fn_seek_idx, variables, variables_count);        
		if (startpos >= endpos) {
          DIE("error in substr function. startpos is bigger or equal than startpos");
        }

        SPACE_SIZE len = endpos - startpos;
        char *substr = malloc(sizeof(char) * (len + 1));
        memset(substr, ' ', len + 1);
        substr[len] = '\0';
        
        DBUGS1("src", src);
        DBUGS1("substr", substr);
        DBUGI1("src len", (int)strlen(src));
        for (int i = startpos; i < endpos; i++) {
          //printf("%s", substr);
          substr[i - startpos] = src[i];
        }

        var_mem *momo = &((*variables)[var_idx_src]);
        free(momo->value);
        momo->value = substr;
        DBUGS1("momo value", substr);
      }
    }
	expr_arg_c++;
	free(str_or_var->name);
	free(str_or_var);
  }
}


void internal_srv(const var_expr *expression, const var_expr *first_expression,
                       SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                       SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                       SPACE_SIZE *arguments_count) {
  struct sockaddr_in si_me, si_other;
  int8_t s, i, slen = sizeof(si_other) , recv_len;
	
  char buf[512];
	int8_t port;
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		DIE("socket cannot");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//put this into thread until close
	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, 512, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			DIE("recvfrom()");
		}
		
		//print8_t details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);
		
		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			DIE("sendto()");
		}
	}

	close(s);
  DIE("internal srv not implemented yet"); 
}

void internal_function(const var_expr *expression, const var_expr *first_expression,
                       SPACE_SIZE expr_idx, symbol *tokens, var_mem **variables,
                       SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
                       SPACE_SIZE *arguments_count) {
  HLINE_DFUNC_START
  DBUG("emulating a call to the internal system_table for the current "
       "expression:");
  DBUGS1("checking call", expression->name_ref.name);
  if (strcmp(expression->name_ref.name, "print") == 0) {
    internal_print(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "add") == 0) {
    internal_add(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "sub") == 0) {
    internal_sub(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "pos") == 0) {
    internal_pos(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "insert") == 0) {
    internal_insert(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "substr") == 0) {
    internal_substr(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "len") == 0) {
    internal_len(IFUNC_ARGS);
  } else if (strcmp(expression->name_ref.name, "srv") == 0) {
    internal_srv(IFUNC_ARGS);
  } else {
    ERROS1("unsupported method call", expression->name_ref.name);
    DIE("crash >)");
  }
}

// if (strcmp(expression->name_ref.name, "insert") == 0) {
//   // WARN("ANDREAS MACHT TANGAS REIN");
//   SPACE_SIZE expr_arg_c = 0;

//   if (expression->value_ref_length != 3) {
//     DIE("insert function has no 3 args (needle, txt, out))")
//   }
//   char *src = NULL;
//   char *dst = NULL;
//   char *dstname = NULL;
//   char *atposchar = NULL;
//   int atpos = 0;

//   while (expr_arg_c != expression->value_ref_length) {

//     // get the scope idx (function index)
//     SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

//     symbol *str_or_var = parser_value_ref_copy(
//         expression, expr_arg_c, tokens, arguments, arguments_count);

//     // we start looking for the referenced variable.
//     // because anything that comes now must have been declared BEFORE
//     // this will be true, till we introduce any random jumps somewhere in
//     // or into other functions like in anonymous functions,
//     // delegates/function pointers, gotos, reference assignments etc.
//     if (str_or_var->type == T_ALNUM) {
//       // first, assume that the variable
//       // due to the uin16_t nature,
//       // we cannot assess if we found something if the value is e.g -1
//       SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
//                                              variables, variables_count);

//       if (var_idx == VAR_MEM_RESET) {
//         ERROS1("variable value not found", str_or_var->name);
//         DIE("die die die miss.");
//       }
//       var_mem *m = &((*variables)[var_idx]);
//       if (m->name == NULL) {
//         DIE("var name is null");
//       } else {
//         DBUGS1("var name", m->name);
//       }

//       if (m->value == NULL) {
//         DIE("var value is null");
//       } else {
//         DBUGS1("var value", m->value);
//       }
//       // what
//       if (expr_arg_c == 0) {
//         if (m->type != T_STRING && m->type != T_ALNUM) {
//           lex_print(m->type);
//           DIE("what parameter for position is not of a string or "
//               "alphanumeric type")
//         }
//         src = m->value;
//         DBUGS1("======> arg 0", str_or_var->name);
//       }
//       // dst
//       if (expr_arg_c == 1) {
//         if (m->type != T_STRING && m->type != T_ALNUM) {
//           lex_print(m->type);
//           DIE("dst parameter for position is not of a string or "
//               "alphanumeric type")
//         }
//         dst = m->value;
//         // dst[strlen(m->value)] = '\0';
//         dstname = m->name;
//         DBUGS1("======> arg 1", str_or_var->name);
//         DBUGS1("======> arg 1 str", dst);
//       }
//       // out
//       if (expr_arg_c == 2) {
//         // T_ALNUM is always a nr in this case because it refers to
//         // something that can never be truly unrefered
//         if (m->type != T_ALNUM) {
//           DIE("out parameter for position is not of an alphanumeric type")
//         }
//         atposchar = m->value;
//         atpos = atoi(atposchar);
//         int whatlen = strlen(src);
//         int dstlen = strlen(dst);
//         int appendsize = dstlen + whatlen;

//         DBUGS1("======> arg 2", str_or_var->name);
//         DBUGS1("what", src);
//         DBUGS1("dst", dst);
//         DBUGI1("dst-len", dstlen);
//         DBUGS1("atposchar", atposchar);
//         DBUGI1("appendsize", appendsize);

//         SPACE_SIZE var_idx_src = find_index_of_var(
//             dstname, fn_seek_idx, variables, variables_count);

//         char *insert_str = malloc(sizeof(char) * appendsize + 1);
//         memset(insert_str, ' ', appendsize);
//         if (dstlen - atpos <= 0) {
//           DIE("error in insert function. destination length is less than "
//               "at position");
//         }
//         // start copying from left till we hit at the "at position"
//         for (int i = 0; i < atpos; i++) {
//           insert_str[i] = dst[i];
//         }
//         // start copying from the "at position" on
//         SPACE_SIZE whatlen_endpos = atpos + whatlen;
//         for (int i = atpos; i < whatlen_endpos; i++) {
//           insert_str[i] = src[i - atpos];
//         }

//         // start copying from the end of the string to be copied + the
//         // rest
//         for (int i = atpos; i < dstlen; i++) {
//           insert_str[whatlen_endpos + (i - atpos)] =
//               dst[atpos + (i - atpos)];
//         }

//         insert_str[appendsize] = '\0';
//         DBUGS1("dst", dst);

//         var_mem *momo = &((*variables)[var_idx_src]);
//         free(momo->value);
//         momo->value = insert_str;
//       }
//     } else {
//       DIE("ok. this error shouldn't happen. this is either a new type "
//           "used I haven't implemented (here) or something is REALLY "
//           "weird");
//     }
//     free(str_or_var->name);
//     free(str_or_var);
//     expr_arg_c++;
//   }


var_expr *find_function_of_execution(const char *function_name,
                                     var_expr *seek_expression_ptr,
                                     const SPACE_SIZE *expression_count,
                                     var_mem *variable_storage,
                                     SPACE_SIZE *variable_storage_count) {
  HLINE_DFUNC_START
  DBUG("seeking the following function"); // copy of on_parse_set_tag_scope

  DBUG(function_name);
  HLINE
  // max 16 unsigned instructions.... oh oh BUG OR A FEATURE?
  SPACE_SIZE expression_idx = *expression_count;

  if (*expression_count == 0) {
    DIE("expression count is 0. like.. wtf are you ding dong?");
  }
  // scope of expr is if owner == -1
  while ((expression_idx -= 1) < *expression_count) {

    if (seek_expression_ptr->operation_type != P_FUNCTION) {
      if (expression_idx == 0) {
        DIE("error seeking for function. not found");
      }
      seek_expression_ptr--;
      continue;
    }

    if (seek_expression_ptr->name_ref.name == NULL) {
      WARN("seek_expression_ptr->name_ref is null");
      if (expression_idx == 0) {
        DIE("error seeking for function. not found");
      }
      continue;
    }
    DBUG(seek_expression_ptr->name_ref.name);
    if (seek_expression_ptr->name_ref.name != NULL &&
        seek_expression_ptr->name_ref.name != NULL &&
        strcmp(seek_expression_ptr->name_ref.name, function_name) == 0) {

      DBUG("found function name");
      break;
    }
    if (expression_idx == 0) {
      DIE("error seeking for function. not found");
    }
    seek_expression_ptr--;
  }

  return seek_expression_ptr;
}

SPACE_SIZE find_index_of_function_forward(const char *function_name,
                                          const var_expr *first_expression,
                                          const SPACE_SIZE expression_count) {
  DBUG("seeking the following function");
  DBUG(function_name);
  SPACE_SIZE expression_idx = 0;
  SPACE_SIZE expression_last_idx = (SPACE_SIZE)expression_count;

  while (expression_idx != expression_last_idx) {
    const var_expr *e = &first_expression[expression_idx];
    if (e->operation_type != P_FUNCTION) {
      expression_idx += 1;
      continue;
    }
    if (e->name_ref.type == T_ALNUM &&
        strcmp(e->name_ref.name, function_name) == 0) {
      DBUG("found function!");
      break;
    }

    expression_idx += 1;
  }
  return expression_idx;
}

// is this from the top? then name it
// is it not? tell me where you start -> rename this fn
SPACE_SIZE find_index_of_function(const char *function_name,
                                  var_expr *seek_expression_ptr,
                                  const SPACE_SIZE expression_count) {
  HLINE_DFUNC_START
  DBUG("seeking the following function");
  DBUG(function_name);
  // copy of on_parse_set_tag_scope

  // max 16 unsigned instructions.... oh oh BUG OR A FEATURE?
  SPACE_SIZE expression_idx = (SPACE_SIZE)expression_count;
  expression_idx -= 1;

  // scope of expr is if owner == -1
  while (expression_idx >= 0) {

    if (seek_expression_ptr->operation_type != P_FUNCTION) {
      if (expression_idx == 0) {
        DBUG("expression_idx == 0");
        ERRO(function_name);
        DIE("didn't find function");
      }
      if (expression_idx == 0) {
        break;
      }
      expression_idx -= 1;
      seek_expression_ptr--;
      continue;
    }
#if MODE == DEBUG_MODE
    printf("name ref type %i, %li, %s \n", seek_expression_ptr->name_ref.type,
           strlen(seek_expression_ptr->name_ref.name),
           seek_expression_ptr->name_ref.name);
#endif
    if (seek_expression_ptr->name_ref.type == 16 &&
        strcmp(seek_expression_ptr->name_ref.name, function_name) == 0) {
      DBUG("found function!");
      break;
    }

    if (expression_idx == 0) {
      DBUG("expression_idx == 0");
      ERROS1("didn't find function", function_name);
      DIE("didn't find function");
    }
    expression_idx -= 1;
    seek_expression_ptr--;
  }

  DBUG("returning expression_idx");
  return expression_idx;
}

SPACE_SIZE contains_str(char const *str, char const *substr)
{
#if MODE == DEBUG_MODE && DEBUG_MACHINE == 1
    printf("str: %s substr: %s\n",str, substr);
#endif
    char* t = strstr(str, substr);
    return t == NULL ? FUNC_NOT_OK : FUNC_OK;
}

uint8_t if_is_true(const var_expr *if_eq, symbol *tokens, SPACE_SIZE **arguments,
                   SPACE_SIZE *arguments_count, var_mem **variable_storage,
                   SPACE_SIZE variable_storage_count, SPACE_SIZE expression_idx,
                   SPACE_SIZE function_idx) {
  DFUNC
  if (if_eq == NULL) {
    DIE("if_eq is null");
  }

  // TODO: left-side is not name_ref anymore
  //
  // check if left operand is a variable or a string
  //    if(if_eq->name_ref.type == T_ALNUM) {
  //  	DBUG("left variable is T_ALNUM");
  //    }
  //    if(if_eq->name_ref.type == T_STRING) {
  //  	DBUG("left variable is T_STRING");
  //    }

  if (if_eq->value_ref_length == 0) {
    DIE("TODO. value_ref_length = 0");
  }

  if (if_eq->value_ref_length < 2) {
    DIE("TODO");
  }

  if (if_eq->value_ref_length == 2) {
    DBUG("value_ref_length == 2");

    // THE LEFT

    // get token.
    // if ALNUM => check for var name with corresponding value
    // if T_STRING => all good
    symbol *left_side =
        parser_value_ref_copy(if_eq, 0, tokens, arguments, arguments_count);

    if (left_side == NULL) {
      DIE("Something really weird happened while fetching the left side of "
          "if_eq expression");
    }

    SPACE_SIZE left_side_idx = VAR_MEM_RESET;
    char *left_side_val = NULL;
    if (left_side->type == T_ALNUM) {
      lex_emit(left_side);
      left_side_idx = find_index_of_var_nearest(left_side->name, expression_idx,
                                                function_idx, variable_storage,
                                                &variable_storage_count);
      if (left_side_idx == VAR_MEM_RESET) {
        ERROS1("variable not found", left_side->name);
        DIE("variable not found. are you retarded?");
      }
      left_side_val = ((*variable_storage)[left_side_idx]).value;
    }

    if (left_side->type == T_STRING) {
      lex_emit(left_side);
      left_side_val = left_side->name;
    }

    // ALT-RIGHT
    symbol *right_side =
        parser_value_ref_copy(if_eq, 1, tokens, arguments, arguments_count);

    if (right_side == NULL) {
      DIE("Something really weird happened while fetching the right side of "
          "if_eq expression");
    }

    SPACE_SIZE right_side_idx = VAR_MEM_RESET;
    char *right_side_val = NULL;
    if (right_side->type == T_ALNUM) {
      lex_emit(right_side);
      right_side_idx = find_index_of_var_nearest(
          right_side->name, expression_idx, function_idx, variable_storage,
          &variable_storage_count);
      if (right_side_idx == VAR_MEM_RESET) {
        ERROS1("variable not found", right_side->name);
        DIE("variable not found. are you retarded?");
      }
      right_side_val = ((*variable_storage)[right_side_idx]).value;
    }

    if (right_side->type == T_STRING) {
      lex_emit(right_side);
      right_side_val = right_side->name;
    }
    //use either strcmp or strstr, depending on operator
    SPACE_SIZE resultcmp = if_eq->operation_type == P_IF_EQ ?
        (strcmp(left_side_val, right_side_val) == 0 ? FUNC_OK : FUNC_NOT_OK) :
        contains_str(left_side_val, right_side_val);
    //DBUGI1("if eq is contains", if_eq->operation_type == P_IF_CONTAINS);
    SPACE_SIZE result = resultcmp; //== 0 ? FUNC_OK : FUNC_NOT_OK;
    free(left_side->name);
    free(left_side);
    free(right_side->name);
    free(right_side);
    return result;
  } else {
    DBUG("value_ref_length is either 0 or something else");
    // A TODO:
    DBUG("A TODO");
    return FUNC_NOT_OK;
  }
}

// TODO: make safeguard,
SPACE_SIZE get_scope_ref_idx(SPACE_SIZE fn_seek_idx,
                             const var_expr *first_expression) {
  if (fn_seek_idx == 0) {
    DIE("why whould you ask for get_scope_ref_idx for the first expression?");
  }
  while (fn_seek_idx != 0) {
    if (first_expression[fn_seek_idx].operation_type == P_FUNCTION) {
      break;
    }
    fn_seek_idx -= 1;
  }
  return fn_seek_idx;
}

void run(const char *start_function, const var_expr *expression,
         const var_expr *first_expression, const var_expr *last_expression,
         const SPACE_SIZE expression_count, var_mem **variables,
         SPACE_SIZE *variables_count, SPACE_SIZE **arguments,
         SPACE_SIZE *arguments_count, SPACE_SIZE **callstack,
         SPACE_SIZE *callstack_count, const SPACE_SIZE *max_callstack,
         symbol *tokens) {
  DFUNC
  if (expression->name_ref.name != NULL) {
    DBUG("expr name:");
    DBUG(expression->name_ref.name);
  }
  if (last_expression->name_ref.name != NULL) {
    DBUG("last expr name:");
    DBUG(last_expression->name_ref.name);
  }
  if (start_function == NULL) {
    DIE("start function was not given");
  }

  // stack empty? look for a main function
  // if not found. @#$%^ off
  // if found. put to stack
  DBUG("pushing main / start_function");
  if (*callstack == NULL) {
    DIE("callstack is null...");
  }

  callstack_push(start_function, first_expression, expression_count,
                  callstack, callstack_count, max_callstack);

  HLINE

  // this is so dumb
  DBUG("preserving start expr. this will be used for the searching inside of "
       "the expr array");
  const var_expr *start_expr = expression;
  parser_emit(start_expr, tokens, arguments, arguments_count);

  HLINE
  while (*callstack_count != 0) {

    // DBUGI1("callstack_count", *callstack_count);
    SPACE_SIZE expr_idx = callstack_pop(callstack, callstack_count) + 1;
    // SPACE_SIZE expr_idx_initial = expr_idx;
    if (expr_idx > expression_count) {
      ERROI1("expression index", expr_idx);
      DIE("callstack popped is > expression_count. like.. what are you doing "
          "bro?");
    }
    const var_expr *expr = &first_expression[expr_idx];

    while (expr_idx < expression_count && expr->operation_type != P_FUNCTION) {
      HLINE_SECTION_START("while start inside of function")

      DBUGI1("expr_idx", expr_idx);

      // SEE CODE BELOW!!!! ITS THE SAME!!!!
      parser_emit(expr, tokens, arguments, arguments_count);

      machine_print_vars(variables, *variables_count);

      if (expr->operation_type == P_SET_REF) {

        symbol *fn =
            parser_value_ref_copy(expr, 0, tokens, arguments, arguments_count);

        SPACE_SIZE fidx = find_index_of_function_forward(
            fn->name, first_expression, expression_count);

        symbol *variable_name =
            parser_value_ref_copy(expr, 1, tokens, arguments, arguments_count);

        DBUGS1("ref variable name found is ", variable_name->name);
        SPACE_SIZE var_idx = find_index_of_var(variable_name->name, fidx,
                                               variables, variables_count);

        SPACE_SIZE scope_ref_idx =
            get_scope_ref_idx(expr_idx, first_expression);

        if (var_idx == VAR_MEM_RESET) {
          ERROS1("variable value not found", variable_name->name);
          DIE("die die die miss.");
        }
        var_mem *m = &((*variables)[var_idx]);
        if (m->name == NULL) {
          DIE("var name is null");
        } else {
          DBUGS1("var name", m->name);
        }

        if (m->value == NULL) {
          DIE("var value is null");
        } else {
          DBUGS1("var value", m->value);
        }
        get_set_var_kv(expr->name_ref.name, m->value, m->type, scope_ref_idx,
                       variables, variables_count);

        free(fn->name);
        free(fn);
        free(variable_name->name);
        free(variable_name);
      }

      // TODO: copy me and make the P_SET_REF
      //  VARIABLE DECLARATIONS
      //  OK.. here is the deal
      //  Variables have a scope_ref_idx
      //  scope_ref_idx is the index of the function
      //  means a variable can only exist once per function
      if (expr->operation_type == P_SET_TAG) {
        // get the scope idx (function index)
        SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

        symbol *str_or_var =
            parser_value_ref_copy(expr, 0, tokens, arguments, arguments_count);

        if (str_or_var->type == T_STRING) {
          get_set_var_kv(expr->name_ref.name, str_or_var->name,
                         str_or_var->type, fn_seek_idx, variables,
                         variables_count);
        }
        // we start looking for the referenced variable.
        // because anything that comes now must have been declared BEFORE
        // this will be true, till we introduce any random jumps somewhere in
        // or into other functions like in anonymous functions,
        // delegates/function pointers, gotos, reference assignments etc.
        else if (str_or_var->type == T_ALNUM) {
          // first, assume that the variable
          // due to the uin16_t nature,
          // we cannot assess if we found something if the value is e.g -1
          SPACE_SIZE var_idx = find_index_of_var(str_or_var->name, fn_seek_idx,
                                                 variables, variables_count);

          if (var_idx == VAR_MEM_RESET) {
            ERROS1("variable value not found", str_or_var->name);
            DIE("die die die miss.");
          }
          var_mem *m = &((*variables)[var_idx]);
          if (m->name == NULL) {
            DIE("var name is null");
          } else {
            DBUGS1("var name", m->name);
          }

          if (m->value == NULL) {
            DIE("var value is null");
          } else {
            DBUGS1("var value", m->value);
          }
          get_set_var_kv(expr->name_ref.name, m->value, m->type, fn_seek_idx,
                         variables, variables_count);

          // free(m);
        } else {
          DIE("ok. this error shouldn't happen. this is either a new type "
              "used "
              "I haven't implemented (here) or something is REALLY weird");
        }
        free(str_or_var->name);
        free(str_or_var);
      }
      if (expr->operation_type == P_SET_NR) {
        // get the scope idx (function index)
        SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

        symbol *str_or_var =
            parser_value_ref_copy(expr, 0, tokens, arguments, arguments_count);

        // OK, T_ALNUM must be parsed to a number (if needed)
        if (str_or_var->type == T_ALNUM) {
          // first, assume that the variable
          // due to the uin16_t nature,
          // we cannot assess if we found something if the value is e.g -1
          get_set_var_kv(expr->name_ref.name, str_or_var->name,
                         str_or_var->type, fn_seek_idx, variables,
                         variables_count);

        } else {
          DIE("ok. this error shouldn't happen. this is either a new type "
              "used "
              "I haven't implemented (here) or something is REALLY weird");
        }
        free(str_or_var->name);
        free(str_or_var);
      }
      
      if (expr->operation_type == P_IF_EQ || expr->operation_type == P_IF_CONTAINS) {
        SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);
        SPACE_SIZE stmt_is_true = if_is_true(expr, tokens, arguments, arguments_count, variables,
                       *variables_count, expr_idx, fn_seek_idx);        
        if (stmt_is_true == FUNC_OK) {
          DBUG("statement is true");
        } else {
          // go out of if scope
          DBUG("statement is not true. jumping out of an eastern factory "
               "window");
          // jump to end if
          // look for next end if
          SPACE_SIZE ifs = 0;
          SPACE_SIZE endifs = 0;
          do {
            parser_emit(expr, tokens, arguments, arguments_count);
            if (first_expression[expr_idx].operation_type == P_IF_EQ || first_expression[expr_idx].operation_type == P_IF_CONTAINS)  {
              ifs++;
            }
            if (first_expression[expr_idx].operation_type == P_ENDIF) {
              endifs++;
            }

            if (ifs == endifs) {
              ifs = 0;
              endifs = 0;
              break;
            } else {
              if ((expression_count - 1) - expr_idx <= 0) {
                DIE("you died. ifs and end-ifs don't match and you reached "
                    "the "
                    "last expression");
              }
            }

            expr++;
            expr_idx += 1;
          } while (endifs <= MAX_IFS && (endifs > 0 || ifs > 0));
        }
      }

      if (expr->operation_type == P_INTERNAL) {
        // TODO: add args with execl
        internal_function(expr, first_expression, expr_idx, tokens, variables,
                          variables_count, arguments, arguments_count);
      }

      if (expr->operation_type == P_EXECUTION) {
        // store the last instruction before executing the function. after all
        // is done, callstack is popped + 1, which means it returns on the
        // instruction after the execution
        if (*max_callstack - 1 < *callstack_count) {
          DIE("YEAH SURE! TRY TO FLOOD ME YOU MORON. callstack_count > "
              "max_callstack THIS IS NOT INTENDED FOR THIS KIND OF "
              "BEHAVIOUR. "
              "CHANGE YOUR CODE");
        }
        if (expr_idx + 1 < expression_count &&
            first_expression[expr_idx + 1].operation_type != P_FUNCTION) {
          (*callstack)[(*callstack_count)] = expr_idx;
          *callstack_count += 1;
        } else {
          DBUG("back ptr not added because it is outside of the expr_count "
               "ak. "
               "this would reference an expr that doesn't exist. so why add "
               "it "
               "in the first place?");
        }

        // TODO: pass all variables names via find_index_of_var as the
        // corresponding variable names inside of the function declaration
        // from
        // left to right
        //  		use get_set_var forexpr_arg_c storing values
        SPACE_SIZE expr_arg_c = 0;
        SPACE_SIZE fn_seek_idx = get_scope_ref_idx(expr_idx, first_expression);

        // SPACE_SIZE fn_seek_idx = expr_idx;
        // while(fn_seek_idx != 0) {
        //  	if(first_expression[fn_seek_idx].operation_type == P_FUNCTION) {
        //  		break;
        //  	}
        //  	fn_seek_idx -= 1;
        // }

        // DBUGI1("found fn_seek_idx", fn_seek_idx);

        while (expr_arg_c != expr->value_ref_length) {

          // DBUGI1("found fn_seek_idx", fn_seek_idx);
          symbol *s = parser_value_ref_copy(expr, expr_arg_c, tokens, arguments,
                                            arguments_count);

          SPACE_SIZE fidx = find_index_of_function_forward(
              expr->name_ref.name, first_expression, expression_count);

          SPACE_SIZE var_idx =
              find_index_of_var(s->name, expr_idx, variables, variables_count);

          DBUGI1("var_idx", var_idx);

          if (var_idx == VAR_MEM_RESET) {

            DBUGS1("variable not found", s->name);
            SPACE_SIZE nearest_idx = find_index_of_var_nearest(
                s->name, expr_idx, fn_seek_idx, variables, variables_count);

            if (nearest_idx == VAR_MEM_RESET) {
              ERROS1("variable", s->name);
              DIE("variable not found");
            } else {
              var_idx = nearest_idx;
            }
          }

          var_mem *m = &((*variables)[var_idx]);
          if (m->name == NULL) {
            DIE("var name is null");
          } else {
            DBUGS1("var name", m->name);
          }

          if (m->value == NULL) {
            DIE("var value is null");
          } else {
            DBUGS1("var value", m->value);
          }

          // pass function arg
          symbol *current_function_arg_name =
              parser_value_ref_copy(&first_expression[fidx], expr_arg_c, tokens,
                                    arguments, arguments_count);

          get_set_var_kv(current_function_arg_name->name, m->value, m->type,
                         fidx, variables, variables_count);

          // printf("name: %s, value: %s\n",
          // 		m->name,
          // 		m->value);
          free(s->name);
          free(s);
          free(current_function_arg_name->name);
          free(current_function_arg_name);

          expr_arg_c++;
        }
        machine_print_vars(variables, *variables_count);
        DBUGS1("pushing to callstack", expr->name_ref.name);
        callstack_push(expr->name_ref.name, first_expression, expression_count,
                        callstack, callstack_count, max_callstack);
        DBUG("BREAK");
        break;
      }

      DBUGI1("expr_idx", expr_idx);
      HLINE_SECTION_END("while end inside of function")
      expr_idx += 1;
      expr++;
    }
    machine_print_vars(variables, *variables_count);
    // TODO: check if expr is p_function
    // -> means, next expr is a function
    // -> clear all variables associated with the scope
  }
}
