/*
 * Arcane Script Interpreter
 *
 *         File: arcane.h
 *       Author: Blake Pell
 * Initial Date: 2025-02-08
 * Last Updated: 2025-02-09
 *      License: MIT License
 */

#ifndef ARCANE_H
#define ARCANE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Values and Variables
   ============================================================ */

/* The 'temp' field indicates that this Value’s string (if any) is owned by the caller
   and should be freed when no longer needed. If temp==0 the value is stored in the symbol table. */
typedef enum
{
    VAL_INT,
    VAL_STRING,
    VAL_BOOL,
    VAL_NULL
} ValueType;

typedef struct
{
    ValueType type;
    int temp; /* 1 = temporary (caller–owned), 0 = stored in the symbol table */
    union
    {
        int int_val;
        char *str_val;
    };
} Value;

typedef struct Variable
{
    char *name;
    Value value;
    struct Variable *next;
} Variable;

/* ============================================================
   Tokenizer
   ============================================================ */

typedef enum
{
    TOKEN_INT,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_IDENTIFIER,
    TOKEN_OPERATOR,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_CONTINUE,
    TOKEN_BREAK,
    TOKEN_EOF
} AstTokenType;

typedef struct
{
    AstTokenType type;
    char *text; /* For identifiers, literals, or operator text */
} Token;

#define MAX_TOKENS 1024

typedef struct
{
    Token tokens[MAX_TOKENS];
    int count;
} TokenList;

/* ============================================================
   Scripting Language Functions (interop with C)
   ============================================================ */

/* --- C Interop functions --- */
typedef Value (*InteropFunction)(Value *args, int arg_count);

typedef struct
{
    char *name;
    InteropFunction func;
} Function;

/* ============================================================
   Parser and Interpreter
   ============================================================ */

typedef struct
{
    TokenList *tokens;
    int pos;
} Parser;

/* ============================================================
   Declarations
   ============================================================ */
Value interpret(const char *src);
void free_value(Value v);
void parse_statement(Parser *p);
Value parse_primary(Parser *p);
Value parse_factor(Parser *p);
Value parse_term(Parser *p); // Add this forward declaration
Value parse_relational(Parser *p);
Value parse_equality(Parser *p);
Value parse_assignment(Parser *p);
Value parse_unary(Parser *p);
Value parse_logical_and(Parser *p);
Value parse_logical(Parser *p);
Value make_int(int x);
Value make_string(const char *s);
Value make_null();
Value make_bool(int b);
Value fn_typeof(Value *args, int arg_count);
Value fn_left(Value *args, int arg_count);
Value fn_right(Value *args, int arg_count);
Value fn_sleep(Value *args, int arg_count);
Value fn_input(Value *args, int arg_count);
Value fn_is_number(Value *args, int arg_count);

#ifdef __cplusplus
}
#endif

#endif // ARCANE_H
