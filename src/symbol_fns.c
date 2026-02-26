#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "meta.h"
#include "file_stuff.h"
#include "types.h"
#include "config.h"
#include "token_def.h"
#include "lex.h"
#include "symbol_fns.h"

#if DEBUG_SYMBOL_FNS == 0
#undef DBUG
#define DBUG(msg) (void)0;
#undef HLINE
#define HLINE 
#endif


symbol *symbol_new(const char *name, int8_t type)
{
	N2(s, symbol, 1);
	s->name = NULL;	
	s->type = type;
	s->size = 0;
	symbol_copy_name(s, name);
	return s;
}

void symbol_destroy(symbol* s) 
{
	if(s->name != NULL)
	{
		free(s->name);
		s->name = NULL;
	}
	free(s);
}

void symbol_trim_name_capacicty(symbol *s)
{
	// s->name = (char *)realloc(s->name, sizeof(char) * s->size);
	RESIZE(s->name, char, s->size);
}

int8_t symbol_copy_2(const symbol *s, symbol *sg)
{
	DBUG(__func__);

	if (sg == NULL)
	{
		DIE("sg is NULL");
	}
	if (s == NULL)
	{
		DIE("given symbol is NULL");
	}
	if (s->name == NULL)
	{
		DIE("given symbol name is NULL");
	}

	if (sg->name == NULL)
	{
		WARN("sg->name is NULL");
		symbol_copy_name(sg, s->name);
	}
	else
	{
		symbol_cat_name(sg, s->name);
	}
	return FUNC_OK;
}

int8_t symbol_copy_name(symbol *s, const char *name)
{
	if(s == NULL) {
		DIE("symbol given is NULL");
	}
	DBUG(__func__);
	// TODO: check this in order to copy string
	if (!(s->name == NULL))
	{
		HLINE
		DBUG("free. free is not null");		
		free(s->name);
		s->name = NULL;
	}	
	s->size = strlen(name) + 1;
	
	s->name = malloc(sizeof(char) * s->size);
	
	s->name = strcpy(s->name, name);

	s->name[s->size - 1] = '\0';
	return FUNC_OK;
}
int8_t symbol_cat_name(symbol *s, const char *name)
{	
	DBUG(__func__);
	s->size = strlen(name);
#if MODE == DEBUG_MODE
	//printf("strlen(name): %ld\n", s->size);
#endif
	//s->size = s->size + strlen(s->name) + 1;
#if MODE == DEBUG_MODE
	//printf("strlen(s->name) + 1: %ld\n", strlen(s->name) + 1);
	//printf("final s->size: %ld\n", s->size);
#endif
	s->size = strlen(name) + strlen(s->name) + 1;
	s->name = (char*)realloc(s->name, sizeof(char) * s->size);
	s->name = strncat(s->name, name, strlen(name) + strlen(s->name) + 1);
	s->name[s->size - 1] = '\0';

	return FUNC_OK;
}

