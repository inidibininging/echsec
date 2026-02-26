#ifndef META__H
#define META__H

#define FUNC_OK 1
#define FUNC_NOT_OK 0
#define FUNC_TYPE int

//resize array variable A with type T by length of L
//whines if A is NULL
#define RESIZE(A, T, L) \
	if(A == NULL) { \
		DIE("yo. your array is NULL. like wtf are you doing?"); \
	} \
    A = (T*)realloc(A, sizeof(T) * L); \
    if(A == NULL) { \
      DIE("Beep. Cannot reallocate A. Happened with resize. look it up"); \
    }

//pushes an item O to array A of type T in L
#define PUSH(O, A, T, L) \
	RESIZE(A, T, L) \
    A[L - 1] = O;

#define POP(O, A, T, L) \
    O = A[L - 1]; \
    RESIZE(O, A, T, L - 1)

//takes the variable A with type of T times L by not declaring A at the same time
#define N(A, T, L) \
    A = malloc(sizeof(T) * L); \
    if(A == NULL) { \
      DIE("Beep. Cannot allocate A. Happened with malloc. look it up"); \
    }

//takes the variable A with type of T time L by declaring A at the same time
#define N2(A, T, L) \
	T* A = malloc(sizeof(T) * L); \
	if(A == NULL) { \
		printf("oh program's gonna burp in... %s\n", __func__); \
		DIE("Beep. Canno allocate A. Happened in N2"); \
	}

#define F(A) free(A); A = NULL;

#define FE(A, T, L, FN) \
    for(int8_t i = 0; i < L; i++){ \
        FN(A[i]); \
    }

#define FR(A, T, L, FN) \
    for(int8_t i = 0; i < L; i++){ \
        FN(&A[i]); \
    }

#define FUNC(NAME, TARG, TRET) \
    TRET(*NAME)                \
    (TARG)

#define FUNC_2(NAME, TARG, TARG_2, TRET) \
    TRET(*NAME)                          \
    (TARG, TARG_2)

#define FUNC_3(NAME, TARG, TARG_2, TARG_3, TRET) \
    TRET(*NAME)                                  \
    (TARG, TARG_2, TARG_3)

#define FUNC_4(NAME, TARG, TARG_2, TARG_3, TARG_4, TRET) \
    TRET(*NAME)                                          \
    (TARG, TARG_2, TARG_3, TARG_4)

#define FUNC_ARRAY(NAME, TARG, TRET, SIZE) \
    TRET(*NAME[SIZE])                      \
    (TARG)

#define FUNC_ARRAY_2(NAME, TARG, TARG_2, TRET, SIZE) \
    TRET(*NAME[SIZE])                                \
    (TARG, TARG_2)

#define FUNC_ARRAY_3(NAME, TARG, TARG_2, TARG_3, TRET, SIZE) \
    TRET(*NAME[SIZE])                                        \
    (TARG, TARG_2, TARG_3)

//maybe a TODO:  define a tree<T> where left and right are T?


#define META_FUNC_OR(FUNC_NAME, TRET_VAR, FUNC_PREDICATE_IDX, FUNC_PREDICATE_ARG, FUNC_ARRAY_SIZE) \
    while (FUNC_PREDICATE_IDX < FUNC_ARRAY_SIZE)                                                   \
    {                                                                                              \
        if ((FUNC_NAME[FUNC_PREDICATE_IDX])(FUNC_PREDICATE_ARG) == FUNC_OK)                        \
        {                                                                                          \
            TRET_VAR = FUNC_OK;                                                                    \
            DBUG("FUNC_OK break");                                                                 \
            break;                                                                                 \
        }                                                                                          \
        FUNC_PREDICATE_IDX++;                                                                      \
    }                                                                                              \

#define META_FUNC_OR_2(FUNC_NAME, TRET_VAR, FUNC_PREDICATE_IDX, FUNC_PREDICATE_ARG, FUNC_PREDICATE_ARG_2, FUNC_ARRAY_SIZE) \
    while (FUNC_PREDICATE_IDX < FUNC_ARRAY_SIZE)                                                                           \
    {                                                                                                                      \
        if ((FUNC_NAME[FUNC_PREDICATE_IDX])(FUNC_PREDICATE_ARG, FUNC_PREDICATE_ARG_2) == FUNC_OK)                          \
        {                                                                                                                  \
            TRET_VAR = FUNC_OK;                                                                                            \
            DBUG("FUNC_OK break");                                                                                         \
            break;                                                                                                         \
        }                                                                                                                  \
        FUNC_PREDICATE_IDX++;                                                                                              \
    }

#define META_FUNC_OR_3(FUNC_NAME, TRET_VAR, FUNC_PREDICATE_IDX, FUNC_PREDICATE_ARG, FUNC_PREDICATE_ARG_2, FUNC_PREDICATE_ARG_3, FUNC_ARRAY_SIZE) \
    while (FUNC_PREDICATE_IDX < FUNC_ARRAY_SIZE)                                                                                                 \
    {                                                                                                                                            \
        printf("before check %d < %d\n", FUNC_PREDICATE_IDX, FUNC_ARRAY_SIZE);                                                                   \
        if ((FUNC_NAME[FUNC_PREDICATE_IDX])(FUNC_PREDICATE_ARG, FUNC_PREDICATE_ARG_2, FUNC_PREDICATE_ARG_3) == FUNC_OK)                          \
        {                                                                                                                                        \
            TRET_VAR = FUNC_OK;                                                                                                                  \
            DBUG("FUNC_OK break");                                                                                                               \
            break;                                                                                                                               \
        }                                                                                                                                        \
        FUNC_PREDICATE_IDX++;                                                                                                                    \
        printf("after check %d < %d\n", FUNC_PREDICATE_IDX, FUNC_ARRAY_SIZE);                                                                    \
    }
#endif
