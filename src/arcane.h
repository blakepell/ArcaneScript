#ifndef INTERP_H
#define INTERP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* --- Value type --- */
/* The 'temp' field indicates that this Value’s string (if any) is owned by the caller
   and should be freed when no longer needed. If temp==0 the value is stored in the symbol table. */
typedef enum {
    VAL_INT,
    VAL_STRING,
    VAL_NULL
} ValueType;

typedef struct {
    ValueType type;
    int temp;  /* 1 = temporary (caller–owned), 0 = stored in the symbol table */
    union {
        int int_val;
        char *str_val;
    };
} Value;

/* --- Built–in functions --- */
typedef Value (*BuiltinFunc)(Value *args, int arg_count);

typedef struct {
    char *name;
    BuiltinFunc func;
} Function;

/* --- interpret --- */
/* interpret() takes a script source (as a C string) and returns a Value (the result of a return statement)
   from the script. */
Value interpret(const char *src);

/* free_value() frees a Value’s allocated memory (if any). */
void free_value(Value v);

#ifdef __cplusplus
}
#endif

#endif // INTERP_H
