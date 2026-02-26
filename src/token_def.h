#ifndef TOKEN_DEF__H
#define TOKEN_DEF__H
// ==============================
// TOKEN DEFINITIONS
// ==============================
// !!! BEWARE OF THE LENGTH OF MULTITOKENS
// MAX_IDENTIFIER_SIZE IS SET TO 64.
// IF ANY BUG OCCURS BLAME CHECK THIS FIRST!
#define FUNCTION_IDENTIFIER "ff"
#define L_PAREN "("
#define R_PAREN ")"
#define VAR_SEPARATOR ","
#define ASSIGN_IDENTIFIER "is"
#define SET_IDENTIFIER "set"
#define STRING_IDENTIFIER "'"
#define EXECUTION_IDENTIFIER "call"
#define RETURN_IDENTIFIER "ret"
#define INTERNAL_FUNCTION_IDENTIFIER "!"
#define REF_IDENTIFIER "&"
#define STRING_TYPE "str"
#define NR_TYPE "nr"
#define REF_TYPE "ref"
#define INT_TYPE "number"
#define IF_IDENTIFIER "if"
#define ENDIF_IDENTIFIER "end"
#define EQ_IDENTIFIER "eq"
#define CONTAINS_IDENTIFIER "contains"

// not implemented
#define GREATER_THAN_IDENTIFIER ">"
#define LESS_THAN_IDENTIFIER "<"
#define NR_OP_ADD "+"
#define NR_OP_SUB "-"
#define NR_OP_MUL "*"
#define NR_OP_DIV "/"
#define NR_OP_MOD "%"

// ==============================
// SYMBOL DEFINITIONS
// ==============================
#define T_UNKNOWN_IDENTIFIER 0
#define T_SPACE 1
#define T_FUNCTION_IDENTIFIER 2
#define T_RETURN_IDENTIFIER 3
#define T_L_PAREN 4
#define T_R_PAREN 5
#define T_VAR_SEPARATOR 6
#define T_ASSIGN_IDENTIFIER 7
#define T_SET_IDENTIFIER 8
#define T_STRING_IDENTIFIER 9
#define T_EXECUTION_IDENTIFIER 10
#define T_INTERNAL_FUNCTION_IDENTIFIER 11
#define T_REF_IDENTIFIER 12
#define T_STRING_TYPE 13
#define T_NR_TYPE 14
#define T_REF_TYPE 15
#define T_INT_TYPE 16
#define T_ALNUM 17
#define T_NR 18
#define T_IF_IDENTIFIER 19
#define T_ENDIF_IDENTIFIER 20
#define T_FUNCTION T_ALNUM
#define T_ARGUMENT_IDENTIFIER 21
#define T_EQ_IDENTIFIER 22
#define T_CONTAINS_IDENTIFIER 23
#define T_STRING 24
#define T_NR_OP_ADD 25
#define T_NR_OP_SUB 26
#define T_NR_OP_MUL 27
#define T_NR_OP_DIV 28
#define T_NR_OP_MOD 29
// #define T_NR_ID 30

//defines the max size of any identifier
#define MAX_IDENTIFIER_SIZE 64
#define MAX_SYMBOL_SIZE 8
#define LEXER_EOF -1

#endif
