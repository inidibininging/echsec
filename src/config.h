#ifndef LOG__H
#define LOG__H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "file_stuff.h"
#include "fun.h"
/*
 * PROGRAM MODES
 *
 * DEBUG          prints debug lines
 * PRODUCTION     doesn't
 */
#define DEBUG_MODE 0
#define WARN_MODE 2
#define ERROR_MODE 3
#define MAIN_MODE_REAL 0
#define MAIN_MODE_TEST 1
#define MAIN_MODE MAIN_MODE_REAL
#define DEBUG_LEX 0
#define DEBUG_PARSER 1
#define DEBUG_SYMBOL_FNS 0
#define DEBUG_MACHINE 1
#define DEBUG_MAIN 0

#define EMIT_TOKEN 0
#define EMIT_JSON 0
#define MODE WARN_MODE
#define USE_WASM 0



#define DEBUG_MODE_STR "DEBUG"
#define WARN_MODE_STR "WARN"
#define ERRO_MODE_STR "ERROR"

//unsupported for now
#define CAN_MULTIBYTE 0

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define DBUG_COLOR KCYN
#define WARN_COLOR KYEL
#define ERRO_COLOR KRED

#define NOTHINGLOL //(void)0;
#define _OUTPUT_MSG_(color, prefix,text) printf("%s - %s: %s %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_SZT1(color, prefix, text, i1) printf("%s - %s: %s: %lu %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_I1(color, prefix, text, i1) printf("%s - %s: %s: %i %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_I2(color, prefix, text, i1, i2) printf("%s - %s: %s: %i,%i %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, i2, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_I3(color, prefix, text, i1, i2, i3) printf("%s - %s: %s: %i,%i %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, i2, i3, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_S1(color, prefix, text, i1) printf("%s - %s: %s: %s %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_S2(color, prefix, text, i1, i2) printf("%s - %s: %s: %s,%s %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, i2, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_S3(color, prefix, text, i1, i2, i3) printf("%s - %s: %s: %s,%s %s(f:%s fn:%s lnr:%d)\n%s", color,prefix, text, i1, i2, i3, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define _OUTPUT_MSG_P1(color, prefix, text, i1) printf("%s - %s: %s: %p %s(f:%s fn:%s lnr:%d)\n%s", color, prefix, text, i1, KYEL, __BASE_FILE__, __func__, __LINE__, KNRM);
#define DFUNC DBUG(__func__);

#define HLINE printf("%s","  -----------------\n");
#define HLINE_SECTION_START(text) printf("%s =======+ %s START +======\n%s", KGRN, text, KNRM);
#define HLINE_SECTION_END(text) printf("%s =======+ %s END +=======\n%s", KGRN, text, KNRM);
#define HLINE_DFUNC_START HLINE_SECTION_START(__func__)
#define HLINE_DFUNC_END HLINE_SECTION_END(__func__)

#define DBUG(text) _OUTPUT_MSG_(DBUG_COLOR, DEBUG_MODE_STR, text)
#define DBUGSZT1(text, i1) _OUTPUT_MSG_SZT1(DBUG_COLOR, DEBUG_MODE_STR, text, i1)
#define DBUGI1(text, i1) _OUTPUT_MSG_I1(DBUG_COLOR, DEBUG_MODE_STR, text, i1)
#define DBUGI2(text, i1, i2) _OUTPUT_MSG_I2(DBUG_COLOR, DEBUG_MODE_STR, text, i1, i2)
#define DBUGI3(text, i1, i2, i3) _OUTPUT_MSG_I3(DBUG_COLOR, DEBUG_MODE_STR, text, i1, i2, i3)
#define DBUGS1(text, i1) _OUTPUT_MSG_S1(DBUG_COLOR, DEBUG_MODE_STR, text, i1)
#define DBUGS2(text, i1, i2) _OUTPUT_MSG_S2(DBUG_COLOR, DEBUG_MODE_STR, text, i1, i2)
#define DBUGS3(text, i1, i2, i3) _OUTPUT_MSG_S3(DBUG_COLOR, DEBUG_MODE_STR, text, i1, i2, i3)
#define DBUGP1(text, i1) _OUTPUT_MSG_P1(DBUG_COLOR, DEBUG_MODE_STR, text, i1)

#define ERRO(text) _OUTPUT_MSG_(ERRO_COLOR, ERRO_MODE_STR, text)
#define ERROI1(text, i1) _OUTPUT_MSG_I1(ERRO_COLOR, ERRO_MODE_STR, text, i1)
#define ERROI2(text, i1, i2) _OUTPUT_MSG_I2(ERRO_COLOR, ERRO_MODE_STR, text, i1, i2)
#define ERROI3(text, i1, i2, i3) _OUTPUT_MSG_I3(ERRO_COLOR, ERRO_MODE_STR, text, i1, i2, i3)
#define ERROS1(text, i1) _OUTPUT_MSG_S1(ERRO_COLOR, ERRO_MODE_STR, text, i1)
#define ERROS2(text, i1, i2) _OUTPUT_MSG_S2(ERRO_COLOR, ERRO_MODE_STR, text, i1, i2)
#define ERROS3(text, i1, i2, i3) _OUTPUT_MSG_S3(ERRO_COLOR, ERRO_MODE_STR, text, i1, i2, i3)
#define ERROP1(text, i1) _OUTPUT_MSG_P1(ERRO_COLOR, ERRO_MODE_STR, text, i1)

#define WARN(text) _OUTPUT_MSG_(WARN_COLOR, WARN_MODE_STR, text)
#define WARNI1(text, i1) _OUTPUT_MSG_I1(WARN_COLOR, WARN_MODE_STR, text, i1)
#define WARNI2(text, i1, i2) _OUTPUT_MSG_I2(WARN_COLOR, WARN_MODE_STR, text, i1, i2)
#define WARNI3(text, i1, i2, i3) _OUTPUT_MSG_I3(WARN_COLOR, WARN_MODE_STR, text, i1, i2, i3)
#define WARNS1(text, i1) _OUTPUT_MSG_S1(WARN_COLOR, WARN_MODE_STR, text, i1)
#define WARNS2(text, i1, i2) _OUTPUT_MSG_S2(WARN_COLOR, WARN_MODE_STR, text, i1, i2)
#define WARNS3(text, i1, i2, i3) _OUTPUT_MSG_S3(WARN_COLOR, WARN_MODE_STR, text, i1, i2, i3)
#define WARNP1(text, i1) _OUTPUT_MSG_P1(WARN_COLOR, WARN_MODE_STR, text, i1)


#if MODE != DEBUG_MODE
#undef DFUNC
#undef HLINE
#undef HLINE_DFUNC_END
#undef HLINE_SECTION_START
#undef HLINE_SECTION_END
#undef HLINE_DFUNC_START
#undef HLINE_DFUNC_END
#define DFUNC
#define HLINE
#define HLINE_DFUNC_END
#define HLINE_SECTION_START(text)
#define HLINE_SECTION_END(text)
#define HLINE_DFUNC_START
#define HLINE_DFUNC_END

#undef DBUG
#undef DBUGP1
#undef DBUGI1
#undef DBUGSZT1
#undef DBUGI2
#undef DBUGI3
#undef DBUGS1
#undef DBUGS2
#undef DBUGS3

#define DBUG(text)
#define DBUGP1(text, i1)
#define DBUGI1(text, i1)
#define DBUGSZT1(text, i1)
#define DBUGI2(text, i1, i2)
#define DBUGI3(text, i1, i2, i3)
#define DBUGS1(text, i1)
#define DBUGS2(text, i1, i2);
#define DBUGS3(text, i1, i2, i3)

#endif

#if MODE == ERROR_MODE

#undef WARNI
#undef WARNI2
#undef WARNI3
#undef WARNS1
#undef WARNS2
#undef WARNS3
#undef WARNP1

#define WARN(text)
#define WARNI1(text, i1)
#define WARNI2(text, i1, i2)
#define WARNI3(text, i1, i2, i3)
#define WARNS1(text, i1)
#define WARNS2(text, i1, i2)
#define WARNS3(text, i1, i2, i3)
#define WARNP1(text, i1)

#endif 

#define EXIT_FUNCTION(args) exit(EXIT_FAILURE);

#define DIE(text) \
    ERRO(text);   \
    EXIT_FUNCTION(text)

#define DIES1(s1, text) \
    ERROS1(s1, text);   \
    EXIT_FUNCTION(text)

#define DIES2(s1, s2, text) \
    ERROS2(s1, s2, text);   \
    EXIT_FUNCTION(text)

#define DIES3(s1, s2, s3, text) \
    ERROS2(s1, s2, s3, text);   \
    EXIT_FUNCTION(text)

#endif

