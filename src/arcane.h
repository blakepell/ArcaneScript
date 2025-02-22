/*
 * Arcane Script Interpreter
 *
 *         File: arcane.h
 *       Author: Blake Pell
 * Initial Date: 2025-02-08
 * Last Updated: 2025-02-14
 *      License: MIT License
 */

 #ifndef ARCANE_H
 #define ARCANE_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif

/* ============================================================
    Base Data Types
   ============================================================ */

typedef unsigned char bool;

#if !defined(FALSE)
    #define FALSE 0
#endif

#if !defined(false)
    #define false 0
#endif

#if !defined(TRUE)
    #define TRUE 1
#endif

#if !defined(true)
    #define true 1
#endif

typedef struct {
    int month;
    int day;
    int year;
} Date;

 /* ============================================================
     Helper Macros and Constants
    ============================================================ */
 
 #define IS_NULLSTR(str) ((str)==NULL || (str)[0]=='\0')
 #define MAX_STRING_LENGTH 4608
 #define MSL MAX_STRING_LENGTH
 #define MAX_TOKENS 2048
 #define HEADER "+------------------------------------------------------------------------------+\n\r"
 #define DEBUG TRUE

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
     VAL_DOUBLE,
     VAL_DATE,
     VAL_ARRAY,
     VAL_NULL,
     VAL_ERROR
 } ValueType;

 // Forward reference for dependency.
 typedef struct Value Value;

 typedef struct Array {
    Value *items;
    int length;
} Array;

 typedef struct Value
 {
     ValueType type;
     int temp; /* 1 = temporary (caller–owned), 0 = stored in the symbol table */
     union
     {
         int int_val;
         double double_val;
         char *str_val;
         Date date_val;
         Array *array_val;
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
     TOKEN_DOUBLE,
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
     TOKEN_LBRACKET,
     TOKEN_RBRACKET, 
     TOKEN_EOF
 } AstTokenType;
 
 typedef struct
 {
     AstTokenType type;
     char *text; /* For identifiers, literals, or operator text */
 } Token;
 
 typedef struct
 {
     Token tokens[MAX_TOKENS];
     int count;
 } TokenList;
 
 /* ============================================================
     Scripting Language Functions (interop with C)
    ============================================================ */
 
 /* --- C Interop functions --- */
 typedef Value(*InteropFunction)(Value *args, int arg_count);
 
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
 void raise_error(const char *s, ...);
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
 Value make_double(double d);
 Value make_date(Date d);
 Value make_error(const char *s);
 int get_time(struct timeval *tp, void *tzp);
 const char *_list_getarg(const char *argument, char *arg, int length);
 int _list_contains(const char *list, const char *value);
 Value fn_typeof(Value *args, int arg_count);
 Value fn_left(Value *args, int arg_count);
 Value fn_right(Value *args, int arg_count);
 Value fn_sleep(Value *args, int arg_count);
 Value fn_input(Value *args, int arg_count);
 Value fn_is_number(Value *args, int arg_count);
 Value fn_strlen(Value *args, int arg_count);
 Value fn_cint(Value *args, int arg_count);
 Value fn_cstr(Value *args, int arg_count);
 Value fn_cdbl(Value *args, int arg_count);
 Value fn_cbool(Value *args, int arg_count);
 Value fn_print(Value *args, int arg_count);
 Value fn_println(Value *args, int arg_count);
 Value fn_is_interval(Value *args, int arg_count);
 Value fn_substring(Value *args, int arg_count);
 Value fn_list_contains(Value *args, int arg_count);
 Value fn_list_add(Value *args, int arg_count);
 Value fn_list_remove(Value *args, int arg_count);
 Value fn_number_range(Value *args, int arg_count);
 Value fn_chance(Value *args, int arg_count);
 Value fn_replace(Value *args, int arg_count);
 Value fn_trim(Value *args, int arg_count);
 Value fn_trim_start(Value *args, int arg_count);
 Value fn_trim_end(Value *args, int arg_count);
 Value fn_lcase(Value *args, int arg_count);
 Value fn_ucase(Value *args, int arg_count);
 Value fn_umin(Value *args, int arg_count);
 Value fn_umax(Value *args, int arg_count);
 Value fn_timestr(Value *args, int arg_count);
 Value fn_abs(Value *args, int arg_count);
 Value fn_get_terminal_size(Value *args, int arg_count);
 Value fn_set_cursor_position(Value *args, int arg_count);
 Value fn_clear_screen(Value *args, int arg_count);
 Value fn_round(Value *args, int arg_count);
 Value fn_round_up(Value *args, int arg_count);
 Value fn_round_down(Value *args, int arg_count);
 Value fn_sqrt(Value *args, int arg_count);
 Value fn_contains(Value *args, int arg_count);
 Value fn_starts_with(Value *args, int arg_count);
 Value fn_ends_with(Value *args, int arg_count);
 Value fn_index_of(Value *args, int arg_count);
 Value fn_last_index_of(Value *args, int arg_count);
 Value fn_month(Value *args, int arg_count);
 Value fn_day(Value *args, int arg_count);
 Value fn_year(Value *args, int arg_count);
 Value fn_cdate(Value *args, int arg_count);
 Value fn_today(Value *args, int arg_count);
 Value fn_add_days(Value *args, int arg_count);
 Value fn_add_months(Value *args, int arg_count);
 Value fn_add_years(Value *args, int arg_count);
 Value fn_cepoch(Value *args, int arg_count);
 Value fn_terminal_width(Value *args, int arg_count);
 Value fn_terminal_height(Value *args, int arg_count);
 Value fn_chr(Value *args, int arg_count);
 Value fn_asc(Value *args, int arg_count);
 Value fn_upperbound(Value *args, int arg_count);
 Value fn_split(Value *args, int arg_count);
 Value fn_new_array(Value *args, int arg_count);
 Value fn_array_set(Value *args, int arg_count);

 #ifdef __cplusplus
 }
 #endif
 
 #endif // ARCANE_H
