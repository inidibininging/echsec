#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <string.h>
#include <errno.h>
//file pointer to the current src file

void close_file_if_exists(FILE* fp)
{
    if (!(fp == NULL))
    {
        fclose(fp);
		DIE("close_file_if_exists has been terminated");
    }
    
}

void read_file(const char *file, FILE* fp)
{

    if (fp == NULL)
    {
        WARN("WTF. FILE IS NULL");
        ERRO("uups, file pointer to file cannot be opened");
        exit(EXIT_FAILURE);
    }
    if (errno < 0) 
    {
        DIE(strerror(errno));
    }

    if(fseek(fp, 0, SEEK_SET) != 0) {
        ferror(fp);
    }
}
