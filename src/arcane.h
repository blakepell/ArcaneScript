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
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
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

#ifdef __cplusplus
}
#endif

#endif // ARCANE_H
