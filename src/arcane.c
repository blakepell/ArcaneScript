/*
 * Arcane Script Interpreter
 *
 *         File: arcane.c
 *       Author: Blake Pell
 * Initial Date: 2025-02-08
 * Last Updated: 2025-02-11
 *      License: MIT License
 *
 * Provides functionality for tokenizing, parsing, and interpreting
 * the Arcane scripting language.
 */

 #include "arcane.h"
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 #include <stdio.h>
 
 /* ============================================================
     Interop Functions
    ============================================================ */
 
 #define MAX_INTEROP_FUNCTIONS 9
 
 static Function interop_functions[MAX_INTEROP_FUNCTIONS] = {
     {"typeof", fn_typeof},
     {"left", fn_left},
     {"right", fn_right},
     {"sleep", fn_sleep},
     {"input",  fn_input},
     {"is_number", fn_is_number},
     {"len", fn_strlen},
     {"cint", fn_cint},
     {"cstr", fn_cstr}
 };
 
 /* ============================================================
     Utility functions for Value creation and freeing for the
     in language supported types.
    ============================================================ */
 
/*
 * Makes an integer value.
 */
 Value make_int(int x)
 {
     Value v;
     v.type = VAL_INT;
     v.int_val = x;
     v.temp = 1; /* by default, results are temporary */
     return v;
 }
 
 /*
  * Makes a string value.
  */
 Value make_string(const char *s)
 {
     Value v;
     v.type = VAL_STRING;
     v.str_val = _strdup(s);
     v.temp = 1;
     return v;
 }
 
 /*
  * Makes a null value.
  */
 Value make_null()
 {
     Value v;
     v.type = VAL_NULL;
     v.temp = 1;
     return v;
 }
 
 /*
  * Makes a boolean value.
  */
 Value make_bool(int b)
 {
     Value v;
     v.type = VAL_BOOL;
     v.int_val = b; // 0 = false, nonzero = true
     v.temp = 1;
     return v;
 }
 
 /*
  * Frees a values allocated memory.
  */
 void free_value(Value v)
 {
     if (v.type == VAL_STRING && v.str_val)
     {
         free(v.str_val);
         v.str_val = NULL;
     }
 }
 
 /* ============================================================
     Symbol Table (for local script–variables)
    ============================================================ */
 static Variable *local_variables = NULL;
 
 /*
  * Finds a variable by name in the local symbol table.
  */
 Variable *find_variable(const char *name)
 {
     Variable *cur = local_variables;
     while (cur)
     {
         if (strcmp(cur->name, name) == 0)
         {
             return cur;
         }
         cur = cur->next;
     }
     return NULL;
 }
 
 /*
  *  Adds or updates a variable.  The passed-in Value’s "temp" flag is cleared (0) to 
  *  indicate that the variable table now owns it. 
  */
 void set_variable(const char *name, Value v)
 {
     Variable *var = find_variable(name);
     v.temp = 0; /* variable–stored values are not temporary */
     if (var)
     {
         /* free any previous string if needed */
         if (var->value.type == VAL_STRING && var->value.str_val)
         {
             free(var->value.str_val);
             var->value.str_val = NULL;
         }
         var->value = v;
     }
     else
     {
         Variable *newVar = malloc(sizeof(Variable));
         newVar->name = _strdup(name);
         newVar->value = v;
         newVar->next = local_variables;
         local_variables = newVar;
     }
 }
 
 /*
  * Retrieves a variable by name from the local symbol table.
  */
 Value get_variable(const char *name)
 {
     Variable *var = find_variable(name);
 
     if (var)
     {
         return var->value;
     }
 
     fprintf(stderr, "Runtime error: variable \"%s\" not defined.\n", name);
     exit(1);
 }
 
 /*
  * Frees all local variables.
  */
 void free_variables()
 {
     Variable *cur = local_variables;
     while (cur)
     {
         Variable *next = cur->next;
         free(cur->name);
         cur->name = NULL;
 
         if (cur->value.type == VAL_STRING && cur->value.str_val)
         {
             free(cur->value.str_val);
             cur->value.str_val = NULL;
         }
         free(cur);
         cur = next;
     }
     local_variables = NULL;
 }
 
 /*
  * Lookup interop function by name and call it with the given arguments.
  */
 Value call_function(const char *name, Value *args, int arg_count)
 {
     for (int i = 0; i < MAX_INTEROP_FUNCTIONS; i++)
     {
         if (strcmp(interop_functions[i].name, name) == 0)
         {
             return interop_functions[i].func(args, arg_count);
         }
     }
 
     fprintf(stderr, "Runtime error: Unknown function \"%s\".\n", name);
     exit(1);
 }
 
 /* ============================================================
     Tokenizer
    ============================================================ */
 
 /*
  * Adds a token to the token list for the script.
  */
 void add_token(TokenList *list, AstTokenType type, const char *text)
 {
     if (list->count >= MAX_TOKENS)
     {
         fprintf(stderr, "Tokenizer error: too many tokens.\n");
         exit(1);
     }
     list->tokens[list->count].type = type;
     list->tokens[list->count].text = _strdup(text);
     list->count++;
 }
 
 /*
  * Tokenizes the input script and populates the token list.
  */
 void tokenize(const char *src, TokenList *list)
 {
     list->count = 0;
     const char *p = src;
     while (*p)
     {
         // Skip whitespace
         if (isspace(*p))
         {
             p++;
             continue;
         }
 
         // Skip single-line comments that start with "//"
         if (p[0] == '/' && p[1] == '/')
         {
             p += 2; // Skip the "//"
             while (*p && *p != '\n')
             {
                 p++;
             }
             continue; // Continue with the next character after the comment
         }
 
         // Check for numeric literals
         if (isdigit(*p))
         {
             const char *start = p;
             while (isdigit(*p))
             {
                 p++;
             }
             int len = p - start;
             char *num_str = malloc(len + 1);
             strncpy(num_str, start, len);
             num_str[len] = '\0';
             add_token(list, TOKEN_INT, num_str);
             free(num_str);
             continue;
         }
 
         // Check for string literals
         if (*p == '"')
         {
             p++; /* skip opening quote */
             const char *start = p;
             while (*p && *p != '"')
             {
                 p++;
             }
             if (*p != '"')
             {
                 fprintf(stderr, "Tokenizer error: Unterminated string literal.\n");
                 exit(1);
             }
             int len = p - start;
             char *str_val = malloc(len + 1);
             strncpy(str_val, start, len);
             str_val[len] = '\0';
             add_token(list, TOKEN_STRING, str_val);
             free(str_val);
             p++; /* skip closing quote */
             continue;
         }
 
         // Check for identifiers or keywords
         if (isalpha(*p) || *p == '_')
         {
             const char *start = p;
             while (isalnum(*p) || *p == '_')
             {
                 p++;
             }
             int len = p - start;
             char *id = malloc(len + 1);
             strncpy(id, start, len);
             id[len] = '\0';
             if (strcmp(id, "if") == 0)
             {
                 add_token(list, TOKEN_IF, id);
             }
             else if (strcmp(id, "else") == 0)
             {
                 add_token(list, TOKEN_ELSE, id);
             }
             else if (strcmp(id, "for") == 0)
             {
                 add_token(list, TOKEN_FOR, id);
             }
             else if (strcmp(id, "while") == 0)
             {
                 add_token(list, TOKEN_WHILE, id);
             }
             else if (strcmp(id, "return") == 0)
             {
                 add_token(list, TOKEN_RETURN, id);
             }
             else if (strcmp(id, "print") == 0)
             {
                 add_token(list, TOKEN_PRINT, id);
             }
             else if (strcmp(id, "continue") == 0)
             {
                 add_token(list, TOKEN_CONTINUE, id);
             }
             else if (strcmp(id, "break") == 0)
             {
                 add_token(list, TOKEN_BREAK, id);
             }
             else if (strcmp(id, "true") == 0 || strcmp(id, "false") == 0)
             {
                 add_token(list, TOKEN_BOOL, id); // add TOKEN_BOOL for boolean literals
             }
             else
             {
                 add_token(list, TOKEN_IDENTIFIER, id);
             }
             free(id);
             continue;
         }
 
         // Check for two-character operators for logical && and ||
         if ((p[0] == '&' && p[1] == '&') || (p[0] == '|' && p[1] == '|'))
         {
             char op[3];
             op[0] = p[0];
             op[1] = p[1];
             op[2] = '\0';
             add_token(list, TOKEN_OPERATOR, op);
             p += 2;
             continue;
         }
 
         // Check for two-character operators: "++" and "--"
         if ((p[0] == '+' && p[1] == '+') || (p[0] == '-' && p[1] == '-'))
         {
             char op[3];
             op[0] = p[0];
             op[1] = p[1];
             op[2] = '\0';
             add_token(list, TOKEN_OPERATOR, op);
             p += 2;
             continue;
         }
 
         // Check for two-character operators: "==", "+=", "!=", ">=", "<="
         if ((p[0] == '=' && p[1] == '=') ||
             (p[0] == '+' && p[1] == '=') ||
             (p[0] == '!' && p[1] == '=') ||
             (p[0] == '>' && p[1] == '=') ||
             (p[0] == '<' && p[1] == '='))
         {
             char op[3];
             op[0] = p[0];
             op[1] = p[1];
             op[2] = '\0';
             add_token(list, TOKEN_OPERATOR, op);
             p += 2;
             continue;
         }
 
         /* Single-character tokens */
         switch (*p)
         {
             case '=':
             case '+':
             case '-':
             case '*':
             case '/':
             {
                 char op[2];
                 op[0] = *p;
                 op[1] = '\0';
                 add_token(list, TOKEN_OPERATOR, op);
                 p++;
                 break;
             }
             case '!':
             {
                 char op[2];
                 op[0] = *p;
                 op[1] = '\0';
                 add_token(list, TOKEN_OPERATOR, op);
                 p++;
                 break;
             }
             case ';':
                 add_token(list, TOKEN_SEMICOLON, ";");
                 p++;
                 break;
             case '(':
                 add_token(list, TOKEN_LPAREN, "(");
                 p++;
                 break;
             case ')':
                 add_token(list, TOKEN_RPAREN, ")");
                 p++;
                 break;
             case '{':
                 add_token(list, TOKEN_LBRACE, "{");
                 p++;
                 break;
             case '}':
                 add_token(list, TOKEN_RBRACE, "}");
                 p++;
                 break;
             case ',':
                 add_token(list, TOKEN_COMMA, ",");
                 p++;
                 break;
             case '>':
             case '<':
             {
                 char op[2];
                 op[0] = *p;
                 op[1] = '\0';
                 add_token(list, TOKEN_OPERATOR, op);
                 p++;
                 break;
             }
             default:
                 fprintf(stderr, "Tokenizer error: Unexpected character '%c'\n", *p);
                 exit(1);
         }
     }
     add_token(list, TOKEN_EOF, "EOF");
 }
 
 /* ============================================================
     Parser and Interpreter
    ============================================================ */

/*
 * The current position in the token list.
 */
Token *current(Parser *p)
{
    if (p->pos >= p->tokens->count) {
        // Return the EOF token which should be the last token.
        return &p->tokens->tokens[p->tokens->count - 1];
    }

    return &p->tokens->tokens[p->pos];
}

/*
 * The next token in the token list.
 */
Token *peek(Parser *p)
{
    if (p->pos + 1 < p->tokens->count)
    {
        return &p->tokens->tokens[p->pos + 1];
    }
    return NULL;
}

/*
 * Advances the current position in the token list.
 */
void advance(Parser *p)
{
    if (p->pos < p->tokens->count)
    {
        p->pos++;
    }
}

/*
 * Expects the current token to be of the given type and advances to the next token.
 */
void expect(Parser *p, AstTokenType type, const char *msg)
{
    if (p->pos >= p->tokens->count || current(p)->type != type)
    {
        fprintf(stderr, "Parser error: %s (got '%s')\n", msg, p->pos < p->tokens->count ? current(p)->text : "EOF");
        exit(1);
    }

    advance(p);
}
 
/*
 * Evaluates a template string by replacing variables with their values.
 */
char *evaluate_template(const char *tpl) {
    // Allocate an initial dynamic buffer.
    size_t buf_size = strlen(tpl) * 2 + 1;
    char *result = malloc(buf_size);
    if (!result) {
        fprintf(stderr, "Memory allocation error in evaluate_template.\n");
        exit(1);
    }
    result[0] = '\0';
    
    const char *p = tpl;
    while (*p) {
        if (p[0] == '$' && p[1] == '{') {
            p += 2; // Skip "${"
            const char *var_start = p;
            while (*p && *p != '}')
                p++;
            if (*p != '}') {
                fprintf(stderr, "Template error: missing '}'\n");
                exit(1);
            }
            size_t var_len = p - var_start;
            char *varName = malloc(var_len + 1);
            strncpy(varName, var_start, var_len);
            varName[var_len] = '\0';
            p++; // Skip "}"
            
            // Look up the variable and convert it to a string.
            Value val = get_variable(varName);
            free(varName);
            char temp[128];
            const char *valStr;
            if (val.type == VAL_INT) {
                sprintf(temp, "%d", val.int_val);
                valStr = temp;
            } else if (val.type == VAL_BOOL) {
                valStr = (val.int_val ? "true" : "false");
            } else if (val.type == VAL_STRING) {
                valStr = val.str_val;
            } else {
                valStr = "null";
            }
            
            // Ensure buffer is large enough.
            if (strlen(result) + strlen(valStr) + 1 > buf_size) {
                buf_size = (strlen(result) + strlen(valStr)) * 2 + 1;
                result = realloc(result, buf_size);
                if (!result) {
                    fprintf(stderr, "Memory reallocation error in evaluate_template.\n");
                    exit(1);
                }
            }
            strcat(result, valStr);
        } else {
            // Append regular character.
            size_t len = strlen(result);
            if (len + 2 > buf_size) {
                buf_size *= 2;
                result = realloc(result, buf_size);
                if (!result) {
                    fprintf(stderr, "Memory reallocation error in evaluate_template.\n");
                    exit(1);
                }
            }
            result[len] = *p;
            result[len + 1] = '\0';
            p++;
        }
    }
    return result;
}

/*
 * Parse a primary expression.
 */
Value parse_primary(Parser *p)
{
    Token *tok = current(p);
    if (tok->type == TOKEN_INT)
    {
        int num = atoi(tok->text);
        advance(p);
        return make_int(num);
    }
    
    if (tok->type == TOKEN_STRING)
    {
        char *processed;
        // Check if the string literal contains a template pattern.
        if (strstr(tok->text, "${") != NULL)
        {
            processed = evaluate_template(tok->text);
        }
        else
        {
            processed = _strdup(tok->text);
        }
        Value v = make_string(processed);
        free(processed);
        advance(p);
        return v;
    }
    
    if (tok->type == TOKEN_BOOL)
    {
        // Create a boolean value based on the literal text.
        int b = (strcmp(tok->text, "true") == 0) ? 1 : 0;
        Value v = make_bool(b);
        advance(p);
        return v;
    }
 
     if (tok->type == TOKEN_IDENTIFIER)
     {
         char *id = _strdup(tok->text);
         advance(p);
         if (current(p)->type == TOKEN_LPAREN)
         {
             /* Function call */
             advance(p); // consume '('
             Value args[16];
             int arg_count = 0;
             if (current(p)->type != TOKEN_RPAREN)
             {
                 while (1)
                 {
                     args[arg_count] = parse_assignment(p);
                     arg_count++;
                     if (current(p)->type == TOKEN_COMMA)
                     {
                         advance(p);
                     }
                     else
                     {
                         break;
                     }
                 }
             }
 
             expect(p, TOKEN_RPAREN, "Expected ')' after function arguments");
             Value ret = call_function(id, args, arg_count);
             /* Free temporary argument values */
             for (int i = 0; i < arg_count; i++)
             {
                 if (args[i].type == VAL_STRING && args[i].temp)
                 {
                     free_value(args[i]);
                 }
             }
             free(id);
             return ret;
         }
 
         if (current(p)->type == TOKEN_OPERATOR &&
             (strcmp(current(p)->text, "++") == 0 ||
                 strcmp(current(p)->text, "--") == 0))
         {
             char op[3];
             strcpy(op, current(p)->text);
             advance(p);
             Value orig = get_variable(id);
             if (orig.type != VAL_INT)
             {
                 fprintf(stderr, "Runtime error: %s operator only valid for ints.\n", op);
                 exit(1);
             }
             int oldVal = orig.int_val;
             if (strcmp(op, "++") == 0)
             {
                 orig.int_val++;
             }
             else
             {
                 orig.int_val--;
             }
             set_variable(id, orig);
             free(id);
             return make_int(oldVal); // Postfix returns original value.        
         }
         /* Variable reference */
         Value v = get_variable(id);
         free(id);
         return v;
     }
     if (tok->type == TOKEN_LPAREN)
     {
         advance(p); // consume '('
         Value v = parse_assignment(p);
         expect(p, TOKEN_RPAREN, "Expected ')' after expression");
         return v;
     }
     fprintf(stderr, "Parser error: Unexpected token '%s'\n", tok->text);
     exit(1);
 }
 
 /*
  * Parse a unary expression.
  */
 Value parse_unary(Parser *p)
 {
     // Handle the '!' operator for logical negation
     if (current(p)->type == TOKEN_OPERATOR && strcmp(current(p)->text, "!") == 0)
     {
         advance(p); // consume '!'
         Value operand = parse_unary(p);
         if (operand.type != VAL_BOOL && operand.type != VAL_INT)
         {
             fprintf(stderr, "Runtime error: ! operator only works on bools or ints.\n");
             exit(1);
         }
         int result = !(operand.int_val); // works for both VAL_INT and VAL_BOOL
         return make_bool(result);
     }
 
     if (current(p)->type == TOKEN_OPERATOR &&
         (strcmp(current(p)->text, "++") == 0 || strcmp(current(p)->text, "--") == 0))
     {
         char op[3];
         strcpy(op, current(p)->text);
         advance(p);
         // The next token must be an identifier
         if (current(p)->type != TOKEN_IDENTIFIER)
         {
             fprintf(stderr, "Parser error: Expected identifier after unary %s\n", op);
             exit(1);
         }
         char *id = _strdup(current(p)->text);
         advance(p);
         Value v = get_variable(id);
         if (v.type != VAL_INT)
         {
             fprintf(stderr, "Runtime error: %s operator only valid for ints.\n", op);
             exit(1);
         }
         // For prefix, update before returning.
         if (strcmp(op, "++") == 0)
         {
             v.int_val++;
         }
         else
         {
             v.int_val--;
         }
         set_variable(id, v);
         free(id);
         return v;
     }
     // No prefix operator, so delegate to parse_primary.
     Value v = parse_primary(p);
 
     return v;
 }
 
 /*
  * Parse the And operator.
  */
 Value parse_logical_and(Parser *p)
 {
     Value left = parse_equality(p);
 
     while (current(p)->type == TOKEN_OPERATOR && strcmp(current(p)->text, "&&") == 0)
     {
         advance(p); // consume "&&"
         Value right = parse_equality(p);
         int result = ((left.int_val != 0) && (right.int_val != 0));
         left = make_bool(result);
     }
 
     return left;
 }
 
 /*
  * Parse the Or operator.
  */
 Value parse_logical(Parser *p)
 {
     Value left = parse_logical_and(p);
     while (current(p)->type == TOKEN_OPERATOR && strcmp(current(p)->text, "||") == 0)
     {
         advance(p); // consume "||"
         Value right = parse_logical_and(p);
         int result = ((left.int_val != 0) || (right.int_val != 0));
         left = make_bool(result);
     }
     return left;
 }
 
 /*
  * Parse relational: Calls into parse_term and handles relational operators.
  */
 Value parse_relational(Parser *p)
 {
     Value left = parse_term(p);
     while (current(p)->type == TOKEN_OPERATOR &&
         (strcmp(current(p)->text, ">") == 0 ||
             strcmp(current(p)->text, "<") == 0 ||
             strcmp(current(p)->text, ">=") == 0 ||
             strcmp(current(p)->text, "<=") == 0))
     {
         char op[3];
         strcpy(op, current(p)->text);
         advance(p);
         Value right = parse_term(p);
         if (left.type != VAL_INT || right.type != VAL_INT)
         {
             fprintf(stderr, "Runtime error: Relational operators only support ints.\n");
             exit(1);
         }
         int result = 0;
         if (strcmp(op, ">") == 0)
         {
             result = (left.int_val > right.int_val);
         }
         else if (strcmp(op, "<") == 0)
         {
             result = (left.int_val < right.int_val);
         }
         else if (strcmp(op, ">=") == 0)
         {
             result = (left.int_val >= right.int_val);
         }
         else if (strcmp(op, "<=") == 0)
         {
             result = (left.int_val <= right.int_val);
         }
         left = make_int(result);
         if (right.type == VAL_STRING && right.temp)
         {
             free_value(right);
         }
     }
     return left;
 }
 
 /*
  * Parse a factor (a term that can be multiplied or divided).
  */
 Value parse_factor(Parser *p)
 {
     Value left = parse_unary(p);
     while (current(p)->type == TOKEN_OPERATOR &&
         (strcmp(current(p)->text, "*") == 0 || strcmp(current(p)->text, "/") == 0))
     {
         char op[3];
         strcpy(op, current(p)->text);
         advance(p);
         Value right = parse_primary(p);
         if (strcmp(op, "*") == 0)
         {
             if (left.type != VAL_INT || right.type != VAL_INT)
             {
                 fprintf(stderr, "Runtime error: '*' supports only ints.\n");
                 exit(1);
             }
             int res = left.int_val * right.int_val;
             left = make_int(res);
         }
         else if (strcmp(op, "/") == 0)
         {
             if (left.type != VAL_INT || right.type != VAL_INT)
             {
                 fprintf(stderr, "Runtime error: '/' supports only ints.\n");
                 exit(1);
             }
             if (right.int_val == 0)
             {
                 fprintf(stderr, "Runtime error: Division by zero.\n");
                 exit(1);
             }
             int res = left.int_val / right.int_val;
             left = make_int(res);
         }
         /* (Integers do not need freeing.) */
     }
     return left;
 }
 
 /*
  * Terms (handle +, -; note: '+' is also used for string concatenation)
  */
 Value parse_term(Parser *p)
 {
     Value left = parse_factor(p);
     while (current(p)->type == TOKEN_OPERATOR &&
         (strcmp(current(p)->text, "+") == 0 || strcmp(current(p)->text, "-") == 0))
     {
         char op[3];
         strcpy(op, current(p)->text);
         advance(p);
         Value right = parse_factor(p);
         if (strcmp(op, "+") == 0)
         {
             if (left.type == VAL_STRING || right.type == VAL_STRING)
             {
                 /* String concatenation (convert ints to strings if needed) */
                 char *s1, *s2;
 
                 if (left.type == VAL_STRING)
                 {
                     s1 = left.str_val;
                 }
                 else
                 {
                     char buffer1[64];
                     sprintf(buffer1, "%d", left.int_val);
                     s1 = buffer1;
                 }
 
                 if (right.type == VAL_STRING)
                 {
                     s2 = right.str_val;
                 }
                 else
                 {
                     char buffer2[64];
                     sprintf(buffer2, "%d", right.int_val);
                     s2 = buffer2;
                 }
 
                 char *concat = malloc(strlen(s1) + strlen(s2) + 1);
                 strcpy(concat, s1);
                 strcat(concat, s2);
                 Value temp = make_string(concat);
                 free(concat);
                 if (left.type == VAL_STRING && left.temp)
                 {
                     free_value(left);
                 }
                 left = temp;
             }
             else
             {
                 left = make_int(left.int_val + right.int_val);
             }
         }
         else if (strcmp(op, "-") == 0)
         {
             if (left.type != VAL_INT || right.type != VAL_INT)
             {
                 fprintf(stderr, "Runtime error: '-' supports only ints.\n");
                 exit(1);
             }
             left = make_int(left.int_val - right.int_val);
         }
 
         if (right.type == VAL_STRING && right.temp)
         {
             free_value(right);
         }
     }
     return left;
 }
 
 /*
  * Compares two values for equality.
  */
 int values_equal(const Value a, const Value b)
 {
     if (a.type != b.type)
     {
         return 0;
     }
 
     if (a.type == VAL_INT)
     {
         return a.int_val == b.int_val;
     }
 
     if (a.type == VAL_BOOL)
     {
         return a.int_val == b.int_val;
     }
 
     if (a.type == VAL_STRING)
     {
         return strcmp(a.str_val, b.str_val) == 0;
     }
 
     return 0;
 }
 
 /*
  * Parse equality (handles == and !=)
  */
 Value parse_equality(Parser *p)
 {
     Value left = parse_relational(p);
 
     while (current(p)->type == TOKEN_OPERATOR &&
         (strcmp(current(p)->text, "==") == 0 || strcmp(current(p)->text, "!=") == 0))
     {
         int isNotEqual = 0;
         if (strcmp(current(p)->text, "!=") == 0)
         {
             isNotEqual = 1;
         }
 
         advance(p); // skip '==' or '!='
         Value right = parse_relational(p);
         int eq = values_equal(left, right);
 
         if (isNotEqual)
         {
             eq = !eq;
         }
 
         left = make_int(eq);
 
         if (right.type == VAL_STRING && right.temp)
         {
             free_value(right);
         }
     }
 
     return left;
 }
 
 /*
  * Parse an assignment expression.  (handles x = expr; and x += expr;)
  */
 Value parse_assignment(Parser *p)
 {
     if (current(p)->type == TOKEN_IDENTIFIER && peek(p) &&
         peek(p)->type == TOKEN_OPERATOR &&
         (strcmp(peek(p)->text, "=") == 0 || strcmp(peek(p)->text, "+=") == 0))
     {
         char *varName = _strdup(current(p)->text);
         advance(p); // consume identifier
         char *assign_op = _strdup(current(p)->text);
         advance(p); // consume '=' or '+='
         Value right = parse_assignment(p);
 
         if (strcmp(assign_op, "=") == 0)
         {
             set_variable(varName, right);
             right.temp = 0; /* <--- FIX: mark returned value as non-temporary */
         }
         else if (strcmp(assign_op, "+=") == 0)
         {
             Value currentVal = get_variable(varName);
             Value newVal;
 
             if (currentVal.type == VAL_STRING || right.type == VAL_STRING)
             {
                 char buffer1[64], buffer2[64];
                 char *s1, *s2;
 
                 if (currentVal.type == VAL_STRING)
                 {
                     s1 = currentVal.str_val;
                 }
                 else
                 {
                     sprintf(buffer1, "%d", currentVal.int_val);
                     s1 = buffer1;
                 }
 
                 if (right.type == VAL_STRING)
                 {
                     s2 = right.str_val;
                 }
                 else
                 {
                     sprintf(buffer2, "%d", right.int_val);
                     s2 = buffer2;
                 }
 
                 char *concat = malloc(strlen(s1) + strlen(s2) + 1);
                 strcpy(concat, s1);
                 strcat(concat, s2);
                 newVal = make_string(concat);
                 free(concat);
                 // Free the temporary right value if needed.
                 if (right.type == VAL_STRING && right.temp)
                 {
                     free_value(right);
                 }
             }
             else
             {
                 newVal = make_int(currentVal.int_val + right.int_val);
             }
 
             set_variable(varName, newVal);
             right = newVal;
             right.temp = 0; /* mark returned value as non-temporary */
         }
 
         free(varName);
         free(assign_op);
         return right;
     }
     return parse_logical(p);
 }
 
  /* ============================================================
      Statements
     ============================================================ */

 static int return_flag = 0;
 static Value return_value;
 static int continue_flag = 0;
 static int break_flag = 0;
 
 /*
  * Parses a block of statements.
  */
 void parse_block(Parser *p)
 {
     expect(p, TOKEN_LBRACE, "Expected '{' to start block");
 
     while (current(p)->type != TOKEN_RBRACE && current(p)->type != TOKEN_EOF && !return_flag)
     {
         parse_statement(p);
     }
 
     expect(p, TOKEN_RBRACE, "Expected '}' to end block");
 }
 
 /*
  * Parses a statement.
  */
 void parse_statement(Parser *p)
 {
     Token *tok = current(p);
     if (tok->type == TOKEN_PRINT)
     {
         advance(p); // consume 'print'
         Value v = parse_assignment(p);
         expect(p, TOKEN_SEMICOLON, "Expected ';' after print statement");
         if (v.type == VAL_INT)
         {
             printf("%d\n", v.int_val);
         }
         else if (v.type == VAL_STRING)
         {
             printf("%s\n", v.str_val);
         }
         else if (v.type == VAL_BOOL)
         {
             printf("%s\n", (v.int_val ? "true" : "false"));
         }
         else
         {
             printf("null\n");
         }
 
         if (v.type == VAL_STRING && v.temp)
         {
             free_value(v);
         }
     }
     else if (tok->type == TOKEN_RETURN)
     {
         advance(p); // consume 'return'
         Value v = parse_assignment(p);
         expect(p, TOKEN_SEMICOLON, "Expected ';' after return statement");
         return_flag = 1;
         return_value = v;
     }
     else if (tok->type == TOKEN_IF)
     {
         advance(p); // consume "if"
         expect(p, TOKEN_LPAREN, "Expected '(' after if");
         Value cond = parse_assignment(p);
         expect(p, TOKEN_RPAREN, "Expected ')' after if condition");
 
         int condition_true = 0;
         if (cond.type == VAL_INT || cond.type == VAL_BOOL)
         {
             condition_true = (cond.int_val != 0);
         }
         else
         {
             fprintf(stderr, "Runtime error: if condition must be int or bool.\n");
             exit(1);
         }
 
         if (condition_true)
         {
             parse_block(p);
             // Skip any trailing else/else if blocks.
             while (current(p)->type == TOKEN_ELSE)
             {
                 advance(p); // consume 'else'
                 if (current(p)->type == TOKEN_IF)
                 {
                     // Consume "else if" condition tokens without evaluating.
                     advance(p); // consume 'if'
                     expect(p, TOKEN_LPAREN, "Expected '(' after else if");
                     while (current(p)->type != TOKEN_RPAREN && current(p)->type != TOKEN_EOF)
                     {
                         advance(p);
                     }
                     expect(p, TOKEN_RPAREN, "Expected ')' after else if condition");
                 }
                 if (current(p)->type == TOKEN_LBRACE)
                 {
                     // Skip the block.
                     int brace_count = 1;
                     advance(p); // consume '{'
                     while (brace_count > 0 && current(p)->type != TOKEN_EOF)
                     {
                         if (current(p)->type == TOKEN_LBRACE)
                         {
                             brace_count++;
                         }
                         else if (current(p)->type == TOKEN_RBRACE)
                         {
                             brace_count--;
                         }
                         advance(p);
                     }
                 }
             }
         }
         else
         {
             // If false, process the if's block as before.
             if (current(p)->type == TOKEN_LBRACE)
             {
                 int brace_count = 1;
                 advance(p);
                 while (brace_count > 0 && current(p)->type != TOKEN_EOF)
                 {
                     if (current(p)->type == TOKEN_LBRACE)
                         brace_count++;
                     else if (current(p)->type == TOKEN_RBRACE)
                         brace_count--;
                     advance(p);
                 }
             }
             // Process subsequent else if/else clauses normally.
             while (current(p)->type == TOKEN_ELSE)
             {
                 advance(p); // consume 'else'
                 if (current(p)->type == TOKEN_IF)
                 {
                     advance(p); // consume 'if'
                     expect(p, TOKEN_LPAREN, "Expected '(' after else if");
                     Value cond2 = parse_assignment(p);
                     expect(p, TOKEN_RPAREN, "Expected ')' after else if condition");
 
                     int condition_true2 = 0;
                     if (cond2.type == VAL_INT || cond2.type == VAL_BOOL)
                     {
                         condition_true2 = (cond2.int_val != 0);
                     }
                     else
                     {
                         fprintf(stderr, "Runtime error: else if condition must be int or bool.\n");
                         exit(1);
                     }
 
                     if (condition_true2)
                     {
                         parse_block(p);
                         break;
                     }
                     else
                     {
                         // Skip the else if block.
                         if (current(p)->type == TOKEN_LBRACE)
                         {
                             int brace_count = 1;
                             advance(p);
                             while (brace_count > 0 && current(p)->type != TOKEN_EOF)
                             {
                                 if (current(p)->type == TOKEN_LBRACE)
                                     brace_count++;
                                 else if (current(p)->type == TOKEN_RBRACE)
                                     brace_count--;
                                 advance(p);
                             }
                         }
                         // Continue checking further else clauses.
                     }
                 }
                 else
                 {
                     // Else branch.
                     if (current(p)->type == TOKEN_LBRACE)
                     {
                         parse_block(p);
                     }
                     break;
                 }
             }
         }
     }
     else if (tok->type == TOKEN_FOR)
     {
         advance(p); // consume "for"
         expect(p, TOKEN_LPAREN, "Expected '(' after for");
 
         /* --- Parse the initializer expression (if any) --- */
         if (current(p)->type != TOKEN_SEMICOLON)
         {
             parse_assignment(p);
         }
         expect(p, TOKEN_SEMICOLON, "Expected ';' after for-loop initializer");
 
         /* --- Capture the condition expression range --- */
         int cond_start = p->pos;
         int cond_end = cond_start;
         while (p->pos < p->tokens->count && current(p)->type != TOKEN_SEMICOLON)
         {
             cond_end++;
             advance(p);
         }
         expect(p, TOKEN_SEMICOLON, "Expected ';' after for-loop condition");
 
         /* --- Capture the post expression range --- */
         int post_start = p->pos;
         int post_end = post_start;
         while (p->pos < p->tokens->count && current(p)->type != TOKEN_RPAREN)
         {
             post_end++;
             advance(p);
         }
         expect(p, TOKEN_RPAREN, "Expected ')' after for-loop post expression");
 
         /* --- Parse the loop body --- */
         expect(p, TOKEN_LBRACE, "Expected '{' to start for-loop body");
         int block_start = p->pos; // body starts after '{'
 
         // Scan to find the matching '}'
         int brace_count = 1;
         int i = p->pos;
         while (i < p->tokens->count && brace_count > 0)
         {
             if (p->tokens->tokens[i].type == TOKEN_LBRACE)
             {
                 brace_count++;
             }
             else if (p->tokens->tokens[i].type == TOKEN_RBRACE)
             {
                 brace_count--;
             }
             i++;
         }
         int block_end = i; // token position just after the matching '}'
 
         /* --- Execute the for loop --- */
         while (1)
         {
             /* --- Evaluate the condition --- */
             {
                 Parser condParser;
                 condParser.tokens = p->tokens;
                 condParser.pos = cond_start;
                 // Temporarily restrict the parser to the condition tokens.
                 int original_count = condParser.tokens->count;
                 condParser.tokens->count = cond_end;
                 Value cond_val = parse_assignment(&condParser);
                 condParser.tokens->count = original_count;
                 if (cond_val.type != VAL_INT || cond_val.int_val == 0)
                 {
                     if (cond_val.type == VAL_STRING && cond_val.temp)
                     {
                         free_value(cond_val);
                     }
                     break; // condition false: exit loop
                 }
                 if (cond_val.type == VAL_STRING && cond_val.temp)
                 {
                     free_value(cond_val);
                 }
             }
 
             /* --- Execute the loop body --- */
             {
                 Parser bodyParser;
                 bodyParser.tokens = p->tokens;
                 bodyParser.pos = block_start;
                 while (bodyParser.pos < block_end &&
                     current(&bodyParser)->type != TOKEN_RBRACE &&
                     !return_flag && !continue_flag && !break_flag)
                 {
                     parse_statement(&bodyParser);
                 }
             }
 
             if (break_flag)
             {
                 break_flag = 0;
                 break;
             }
             if (continue_flag)
             {
                 continue_flag = 0;
             }
             if (return_flag)
             {
                 break;
             }
 
             /* --- Execute the post expression --- */
             {
                 Parser postParser;
                 postParser.tokens = p->tokens;
                 postParser.pos = post_start;
                 int original_post_count = postParser.tokens->count;
                 postParser.tokens->count = post_end;
                 Value post_val = parse_assignment(&postParser);
                 postParser.tokens->count = original_post_count;
                 post_val.temp = 0; // mark as non-temporary
                 if (post_val.type == VAL_STRING && post_val.temp)
                 {
                     free_value(post_val);
                 }
             }
         }
 
         // Skip the entire for-loop block.
         p->pos = block_end;
     }
     else if (tok->type == TOKEN_WHILE)
     {
         // Consume the "while" keyword.
         advance(p);
         expect(p, TOKEN_LPAREN, "Expected '(' after while");
 
         // Save the starting position of the condition.
         int cond_start = p->pos;
 
         // Parse the condition once (for syntax checking).
         {
             Value dummy_cond = parse_assignment(p);
 
             if (dummy_cond.type == VAL_STRING && dummy_cond.temp)
             {
                 free_value(dummy_cond);
             }
         }
         expect(p, TOKEN_RPAREN, "Expected ')' after while condition");
 
         // Expect a block for the loop body.
         expect(p, TOKEN_LBRACE, "Expected '{' to start while-loop body");
 
         // Consume the '{' and record the start of the body.
         int block_start = p->pos;
 
         // Find the matching '}' to determine the end of the block.
         int brace_count = 1;
         int i = p->pos;
 
         while (i < p->tokens->count && brace_count > 0)
         {
             if (p->tokens->tokens[i].type == TOKEN_LBRACE)
             {
                 brace_count++;
             }
             else if (p->tokens->tokens[i].type == TOKEN_RBRACE)
             {
                 brace_count--;
             }
 
             i++;
         }
         int block_end = i; // token index just after the matching '}'
 
         // --- Execute the while loop ---
         while (1)
         {
             // Create a temporary parser for the condition, starting at cond_start.
             Parser condParser;
             condParser.tokens = p->tokens;
             condParser.pos = cond_start;
             Value cond_val = parse_assignment(&condParser);
             // Determine truth: accept both int and bool.
             int loop_true = 0;
             if (cond_val.type == VAL_INT || cond_val.type == VAL_BOOL)
             {
                 loop_true = (cond_val.int_val != 0);
             }
             else
             {
                 fprintf(stderr, "Runtime error: while condition must be int or bool.\n");
                 exit(1);
             }
             if (cond_val.type == VAL_STRING && cond_val.temp)
             {
                 free_value(cond_val);
             }
             if (!loop_true)
             {
                 break; // exit loop if condition is false
             }
 
             // Execute the loop body using a temporary parser.
             {
                 Parser bodyParser;
                 bodyParser.tokens = p->tokens;
                 bodyParser.pos = block_start;
                 // Execute until we reach the token just before the closing '}'.
                 while (bodyParser.pos < block_end &&
                     current(&bodyParser)->type != TOKEN_RBRACE &&
                     current(&bodyParser)->type != TOKEN_EOF &&
                     !return_flag &&
                     !continue_flag &&
                     !break_flag)
                 {
                     parse_statement(&bodyParser);
                 }
             }
 
             // If a break was executed, reset the flag and exit the loop.
             if (break_flag)
             {
                 break_flag = 0;
                 break;
             }
 
             // If a continue was executed in the loop body, reset the flag.
             if (continue_flag)
             {
                 continue_flag = 0;
             }
         }
 
         // Advance the main parser position to after the loop block.
         p->pos = block_end;
     }
     else if (tok->type == TOKEN_CONTINUE)
     {
         advance(p);
         expect(p, TOKEN_SEMICOLON, "Expected ';' after continue statement");
         continue_flag = 1;
         return;
     }
     else if (tok->type == TOKEN_BREAK)
     {
         advance(p); // consume 'break'
         expect(p, TOKEN_SEMICOLON, "Expected ';' after break statement");
         break_flag = 1;
         return;
     }
     else
     {
         /* Expression statement */
         Value v = parse_assignment(p);
         expect(p, TOKEN_SEMICOLON, "Expected ';' after expression statement");
         if (v.type == VAL_STRING && v.temp)
         {
             free_value(v);
         }
     }
 }
 
 /*
  * The main interpreter.  This is the entry point for a script to begin
  * execution.
  */
 Value interpret(const char *src)
 {
     TokenList tokens;
     tokens.count = 0;
     tokenize(src, &tokens);
     Parser parser;
     parser.tokens = &tokens;
     parser.pos = 0;
     return_flag = 0;
     return_value = make_null();
 
     while (current(&parser)->type != TOKEN_EOF && !return_flag)
     {
         parse_statement(&parser);
     }
 
     /* Free all tokens */
     for (int i = 0; i < tokens.count; i++)
     {
         free(tokens.tokens[i].text);
     }

     Value ret = return_value;
     free_variables();
     return ret;
 }