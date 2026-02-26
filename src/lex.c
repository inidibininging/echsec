#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "token_def.h"
#include "config.h"
#include "meta.h"
#include "types.h"

#if DEBUG_LEX == 0
#undef DBUG
#define DBUG(msg) (void)0;
#undef HLINE
#define HLINE (void)0;
#endif
void lex_emit(const symbol* token) {
#if MODE == DEBUG_MODE
  	if(token == NULL || 
	  token->name == NULL) return;
	printf("{ \"type\" : %i, \"name\": \"%s\" }\n",token->type, token->name);
#endif
}


void lex_print(int8_t token)
{
#if MODE != DEBUG_MODE
	return;
#endif
	if (token == T_UNKNOWN_IDENTIFIER)
	{
		DBUG("T_UNKNOWN_IDENTIFIER");
	}
	if (token == T_SPACE)
	{
		DBUG("T_SPACE");
	}
	if (token == T_FUNCTION_IDENTIFIER)
	{
		DBUG("T_FUNCTION_IDENTIFIER");
	}
	if (token == T_L_PAREN)
	{
		DBUG("T_L_PAREN");
	}
	if (token == T_R_PAREN)
	{
		DBUG("T_R_PAREN");
	}
	if (token == T_VAR_SEPARATOR)
	{
		DBUG("T_VAR_SEPARATOR");
	}
	if (token == T_ASSIGN_IDENTIFIER)
	{
		DBUG("T_ASSIGN_IDENTIFIER");
	}
	if (token == T_SET_IDENTIFIER)
	{
		DBUG("T_SET_IDENTIFIER");
	}
  	if (token == T_STRING) 
  	{
    	DBUG("T_STRING");
  	}
	if (token == T_STRING_IDENTIFIER)
	{
		DBUG("T_STRING_IDENTIFIER");
	}
	if (token == T_EXECUTION_IDENTIFIER)
	{
		DBUG("T_EXECUTION_IDENTIFIER");
	}
	if (token == T_INTERNAL_FUNCTION_IDENTIFIER)
	{
		DBUG("T_INTERNAL_FUNCTION_IDENTIFIER");
	}
	if (token == T_STRING_TYPE)
	{
		DBUG("T_STRING_TYPE");
	}
    if (token == T_NR_TYPE)
	{
		DBUG("T_NR_TYPE)");
	}
	if (token == T_REF_TYPE)
	{
		DBUG("T_REF_TYPE");
	}
	if (token == T_INT_TYPE)
	{
		DBUG("T_INT_TYPE");
	}
	if (token == T_ALNUM)
	{
		DBUG("T_ALNUM");
	}
	if (token == T_IF_IDENTIFIER)
	{
		DBUG("T_IF_IDENTIFIER");
	}
	if (token == T_ENDIF_IDENTIFIER)
	{
		DBUG("T_ENDIF_IDENTIFIER");
	}
	if (token == T_EQ_IDENTIFIER)
	{
		DBUG("T_EQ_IDENTIFIER");
	}
    if (token == T_CONTAINS_IDENTIFIER)
	{
		DBUG("T_CONTAINS_IDENTIFIER");
	}

	if (token == T_RETURN_IDENTIFIER)
	{
		DBUG("T_RETURN_IDENTIFIER");
	}

  /*
	if (token == P_OPTIONAL_START)
	{
		DBUG("P_OPTIONAL_START");
	}
	if (token == P_OPTIONAL_END)
	{
		DBUG("P_OPTIONAL_END");
	}
	if (token == P_REPEAT_START)
	{
		DBUG("P_REPEAT_START");
	}
	if (token == P_REPEAT_END)
	{
		DBUG("P_REPEAT_END");
	}
  */
}

// compares LEX with TOKEN. If true, T_LEX is returned
// LEX stands for an arbitrary const defined through #define
#define TOKEN_CHECK(LEX, TOKEN)  \
	if (strcmp(LEX, TOKEN) == 0) \
	{                            \
		DBUG("FOUND LEX");       \
		return T_##LEX;          \
	}                            \
	else                         \
		(void)0

//tries to tokenize the token char and returns a token as an int
int8_t lex_find_single_token(const char *possible_token)
{

	if (isspace((unsigned char)possible_token[0]) != 0)
	{
		return T_SPACE;
	}

	
	TOKEN_CHECK(L_PAREN, possible_token);

	TOKEN_CHECK(R_PAREN, possible_token);

	TOKEN_CHECK(VAR_SEPARATOR, possible_token);

	// TOKEN_CHECK(ASSIGN_IDENTIFIER, possible_token);

	TOKEN_CHECK(STRING_IDENTIFIER, possible_token);

	TOKEN_CHECK(REF_IDENTIFIER, possible_token);

    TOKEN_CHECK(INTERNAL_FUNCTION_IDENTIFIER, possible_token);

	return T_UNKNOWN_IDENTIFIER;
}

//tries to tokenize the token string and returns a token as an int
int8_t lex_find_multi_token(const char *possible_token)
{
	DBUG(__func__);
	DBUG(possible_token);

	int8_t idx = 0;
	int8_t max_idx = strlen(possible_token);
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
	printf("%max_idx: %d\n", max_idx);
#endif
	/*
			int8_t is_space = 0;
			while(idx != max_idx) 
			{
				if(isspace((unsigned char)possible_token[idx]) != 0)
				{
					is_space = 1;
					break;
				}
				idx += 1;
			}
			//TODO: refactor this. this is horrible
			if(is_space == 1)
			{
				return T_SPACE;
			}
		*/
	// check tokens > 1
	//
	
	TOKEN_CHECK(FUNCTION_IDENTIFIER, possible_token);

	TOKEN_CHECK(EXECUTION_IDENTIFIER, possible_token);

    //TOKEN_CHECK(INTERNAL_FUNCTION_IDENTIFIER, possible_token);

	//TOKEN_CHECK(L_PAREN, possible_token);

	TOKEN_CHECK(RETURN_IDENTIFIER, possible_token);

	TOKEN_CHECK(SET_IDENTIFIER, possible_token);

	TOKEN_CHECK(ASSIGN_IDENTIFIER, possible_token);

	TOKEN_CHECK(STRING_TYPE, possible_token);
	
    TOKEN_CHECK(NR_TYPE, possible_token);

	TOKEN_CHECK(REF_TYPE, possible_token);

	TOKEN_CHECK(INT_TYPE, possible_token);

	TOKEN_CHECK(IF_IDENTIFIER, possible_token);

    TOKEN_CHECK(CONTAINS_IDENTIFIER, possible_token);

	TOKEN_CHECK(ENDIF_IDENTIFIER, possible_token);

	TOKEN_CHECK(EQ_IDENTIFIER, possible_token);

	TOKEN_CHECK(RETURN_IDENTIFIER, possible_token);

	// check if its a literal
	idx = 0;
	while (idx != max_idx)
	{
		if (isalnum((unsigned char)possible_token[idx]) != 0 
			|| possible_token[idx] == '_'
			|| possible_token[idx] == '.'
			|| possible_token[idx] == ':'
			|| possible_token[idx] == '!'
			|| possible_token[idx] == '?'
			|| possible_token[idx] == '@'
		    || possible_token[idx] == '#'
			|| possible_token[idx] == '$'
			|| possible_token[idx] == '%'
			|| possible_token[idx] == '^'
			|| possible_token[idx] == '&'
			|| possible_token[idx] == '{'
			|| possible_token[idx] == '}'
			|| possible_token[idx] == '['
			|| possible_token[idx] == ']'
			|| possible_token[idx] == '|'
			|| possible_token[idx] == '\\'
			|| possible_token[idx] == '/'
			|| possible_token[idx] == ';'		
			|| possible_token[idx] == '"'
			|| possible_token[idx] == '-'
			|| possible_token[idx] == '<'
			|| possible_token[idx] == '>'
			)
		{
			idx += 1;
			continue;
		}
		else
		{
			ERRO(possible_token);
			DIE("UNKNOWN IDENTIFIER");
			return T_UNKNOWN_IDENTIFIER;
		}
	}
	return T_ALNUM;
}

// doesn't check if the
int8_t lex_function_rule(char *token)
{
	return 0;
}

// goes to the next character via fgetc
// c stands for the array the next char will be stored
// idx stands for  the index of c where the next char will be stored
// the function returns either FUNC_OK or LEXT_EOF if the file pointer has reached its end
int8_t lex_next(char *c, int8_t idx, FILE *esrc_file)
{
	DBUG(__func__);
	DBUG(c);
	DBUG("storing at idx ...");
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
	printf("%d\n", idx);
	if (ferror(esrc_file) != 0)
	{
		DIE(strerror(errno));
		//DIE("ferror...");
	}
	//if (!(esrc_file == NULL))
	//{
	//	DIE(strerror(errno));
		//ERRO("esrc_file is null");
	//}
#endif
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
	printf("idx: %d\n", idx);
#endif
	while ((c[idx] = fgetc(esrc_file)) != EOF)
	{
		if (feof(esrc_file))
		{
			break;
		}
		if (c[idx] == '\n')
		{
			//src_line_nr = src_line_nr += 1;
			//src_cursor_nr = 0;
		}
		//++src_cursor_nr;
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
		printf("CURRENT CHAR %c\n", c[idx]);
#endif
		return FUNC_OK;
	}
	DBUG("EOF reached");
	return LEXER_EOF;

	DIE("something horrible happened");
}

// im going to regret this
// char??? really? i don't know what i'm doing
// php iconv drama incoming
char fgetc_wasm(const char* script, SPACE_SIZE* script_pos)
{
	// hack me
	char r = script[*script_pos];
	return r == '\0' ? EOF : r;
}

int8_t ferror_wasm(const char* script, SPACE_SIZE* script_pos)
{
	return strlen(script) >= (unsigned long)script_pos ? 0 : 1;
}

int8_t lex_next_wasm(char *c, int8_t idx, const char* script, SPACE_SIZE* script_pos)
{
	DBUG(__func__);
	DBUG(c);
	DBUG("storing at idx ...");
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
	printf("%d\n", idx);
	if (ferror_wasm(script, script_pos) != 0)
	{
		DIE(strerror(errno));
		//DIE("ferror...");
	}
	//if (!(esrc_file == NULL))
	//{
	//	DIE(strerror(errno));
		//ERRO("esrc_file is null");
	//}
#endif
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
	printf("idx: %d\n", idx);
#endif
	while ((c[idx] = fgetc_wasm(script, script_pos++)) != EOF)
	{
		// if (feof(script))
		// {
		// 	break;
		// }
		if (c[idx] == '\n')
		{
			//src_line_nr = src_line_nr += 1;
			//src_cursor_nr = 0;
		}
		//++src_cursor_nr;
#if MODE == DEBUG_MODE && DEBUG_LEX == 1
		printf("le char %c\n", c[idx]);
#endif
		return FUNC_OK;
	}
	DBUG("EOF reached");
	return LEXER_EOF;

	DIE("something horrible happened");
}


int8_t lex_next_to_buffer_2_wasm(
	FUNC_3(on_token, const char *, int8_t, int16_t, int8_t),
	const char *script,
	SPACE_SIZE* script_pos) {
	int16_t r = 0;
	char buff[MAX_IDENTIFIER_SIZE];
	char single_token[2] = {'\0', '\0'};

	memset(buff, '\0', MAX_IDENTIFIER_SIZE - 1);
	do
	{
		if (lex_next_wasm(&single_token[0], 0, script, script_pos) == LEXER_EOF)
		{
			DBUG("LEXER_EOF found");
			return FUNC_NOT_OK;
		}

		if (r == MAX_IDENTIFIER_SIZE - 1)
		{
#if MODE == DBUG_MODE || MODE == DEBUG_MODE
			ERRO(buff);
#endif
			ERRO("the max identifier size has been reached");
			return FUNC_NOT_OK;
		}

		//check for a single token
		//if token is not identified. the tokens will be aggregated and checked against the multi token function
		//if the token cannot be identified, an error will be thrown
		int8_t found_token = lex_find_single_token(&single_token[0]);

		//omit spaces.commented out because it is not integrated in the function
		//todo: add on / off option for omitting spaces
		/*
		if(found_token == T_SPACE) {
			single_token[0] = '\0';
			continue;
		}
		*/

		if (found_token != T_UNKNOWN_IDENTIFIER)
		{
#if MODE == DEBUG_MODE || MODE == DBUG_MODE
			//lex_print(found_token);
#endif
			/*
			  this is from the token before.
              the token before was a literal
			  this will be triggered after the alnum token is done and the result is not a multi token. 
			  AND a new single_token is found.
			  all this happens because I decided not to use "lookahead"s
			*/
			if (r > 0)
			{
				on_token(&buff[0], T_ALNUM, r);
				r = 0;
				memset(buff, '\0', MAX_IDENTIFIER_SIZE);
			}

			return on_token(&single_token[0], found_token, 0);
		}

		buff[r] = single_token[0];
		found_token = lex_find_multi_token(&buff[0]);
		if (found_token == T_ALNUM)
		{
			single_token[0] = '\0';
			r += 1;
			continue;
		}

		//this if branch will be executed if the found_token is a multi token (If, EnfIf, Tag etc.)
		if (found_token != T_UNKNOWN_IDENTIFIER)
		{
#if MODE == DEBUG_MODE || MODE == DBUG_MODE
			//lex_print(found_token);
#endif
			return on_token(&buff[0], found_token, r);

			/*r = 0;
            memset(buff, '\0', MAX_IDENTIFIER_SIZE - 1);
            continue;*/
		}
		ERRO(buff);
		ERRO("FUNC_NOT_OK");
		return FUNC_NOT_OK;
	} while (1);
}

// experimental function for reading tokens. REMINDER: ONLY for ASCII!!! Single tokens are read 1 byte at a time
int8_t lex_next_to_buffer_2(
	FUNC_3(on_token, const char *, int8_t, int16_t, int8_t),
	FILE *esrc_file)
{
	int16_t r = 0;
    int8_t lookahead_token = T_UNKNOWN_IDENTIFIER;
	char buff[MAX_IDENTIFIER_SIZE];
	char single_token[2] = {'\0', '\0'};

	memset(buff, '\0', MAX_IDENTIFIER_SIZE - 1);
	do
	{
		if (lex_next(&single_token[0], 0, esrc_file) == LEXER_EOF)
		{
			DBUG("LEXER_EOF found");
			return FUNC_NOT_OK;
		}

		if (r == MAX_IDENTIFIER_SIZE - 1)
		{
#if MODE == DBUG_MODE || MODE == DEBUG_MODE
			ERRO(buff);
#endif
			ERRO("the max identifier size has been reached");
			return FUNC_NOT_OK;
		}

		//check for a single token
		//if token is not identified. the tokens will be aggregated and checked against the multi token function
		//if the token cannot be identified, an error will be thrown
		int8_t found_token = lex_find_single_token(&single_token[0]);

		if (found_token != T_UNKNOWN_IDENTIFIER)
		{
#if MODE == DEBUG_MODE || MODE == DBUG_MODE
            // WARN("FOUND SINGLE TOKEN");
			lex_print(found_token);
#endif
			/*
			  this is from the token before.
              the token before was a literal
			  this will be triggered after the alnum token is done and the result is not a multi token. 
			  AND a new single_token is found.
			  all this happens because I decided not to use "lookahead"s
			*/
			if (r > 0)
			{
                // WARN("LOOKAHEAD REST FOUND");
                // WARNS1("buff", buff);
                int8_t previous_token = lex_find_multi_token(&buff[0]);
 			    on_token(&buff[0], previous_token, r);                   
				r = 0;
                lookahead_token = T_UNKNOWN_IDENTIFIER;
				memset(buff, '\0', MAX_IDENTIFIER_SIZE);
			}
            //WARN("SINGLE TOKEN FOUND");
            //WARNS1("single_token", buff);
			return on_token(&single_token[0], found_token, 0);
		}
        
        if(lookahead_token != T_UNKNOWN_IDENTIFIER)
        {
            if(r == 0) {
                DIE("lex logic error, lookahead is set, but the lookahead index r is set to 0");
            }
		    found_token = lookahead_token;
            //WARN("lookahead token passed to found_token");
            //WARNS1("buff", buff);
        }

		buff[r] = single_token[0];
		found_token = lex_find_multi_token(&buff[0]);

        // lookahead part
		if (found_token == T_ALNUM)
		{
			single_token[0] = '\0';
			r += 1;
			continue;
		}

		//this if branch will be executed if the found_token is a multi token (If, EnfIf, Tag etc.)
		if (found_token != T_UNKNOWN_IDENTIFIER)
		{
#if MODE == DEBUG_MODE || MODE == DBUG_MODE
			//lex_print(found_token);
#endif
            //lookahead part for other tokens other than T_ALNUM
            lookahead_token = found_token;
            r += 1;
            continue;

		}
		ERROS1("CURRENT buff",buff);
		ERRO("FUNC_NOT_OK");
		return FUNC_NOT_OK;
	} while (1);
}
