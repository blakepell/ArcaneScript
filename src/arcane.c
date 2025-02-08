#include "arcane.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================
   Utility functions for Value creation and freeing
   ============================================================ */
 
Value make_int(int x) {
    Value v;
    v.type = VAL_INT;
    v.int_val = x;
    v.temp = 1; /* by default, results are temporary */
    return v;
}

Value make_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.str_val = strdup(s);
    v.temp = 1;
    return v;
}

Value make_null() {
    Value v;
    v.type = VAL_NULL;
    v.temp = 1;
    return v;
}

/* free_value() frees a Value’s allocated memory.
   (Only string values allocate memory.) */
void free_value(Value v) {
    if(v.type == VAL_STRING && v.str_val)
        free(v.str_val);
}

/* ============================================================
   Symbol Table (for local script–variables)
   ============================================================ */

typedef struct Var {
    char *name;
    Value value;
    struct Var *next;
} Var;

static Var *local_vars = NULL;

Var *find_var(const char *name) {
    Var *cur = local_vars;
    while(cur) {
        if(strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* set_var() stores a value under a given name.
   The passed-in Value’s “temp” flag is cleared (0) to indicate that the variable table now owns it. */
void set_var(const char *name, Value v) {
    Var *var = find_var(name);
    v.temp = 0; /* variable–stored values are not temporary */
    if(var) {
        /* free any previous string if needed */
        if(var->value.type == VAL_STRING && var->value.str_val)
            free(var->value.str_val);
        var->value = v;
    } else {
        Var *newVar = malloc(sizeof(Var));
        newVar->name = strdup(name);
        newVar->value = v;
        newVar->next = local_vars;
        local_vars = newVar;
    }
}

Value get_var(const char *name) {
    Var *var = find_var(name);
    if(var)
        return var->value;
    fprintf(stderr, "Runtime error: variable \"%s\" not defined.\n", name);
    exit(1);
}

void free_vars() {
    Var *cur = local_vars;
    while(cur) {
        Var *next = cur->next;
        free(cur->name);
        if(cur->value.type == VAL_STRING && cur->value.str_val)
            free(cur->value.str_val);
        free(cur);
        cur = next;
    }
    local_vars = NULL;
}

/* ============================================================
   Built–in Functions
   ============================================================ */

/* Example: inc(x) returns x+1 */
Value builtin_inc(Value *args, int arg_count) {
    if(arg_count != 1 || args[0].type != VAL_INT) {
        fprintf(stderr, "Runtime error: inc() expects one integer argument.\n");
        exit(1);
    }
    return make_int(args[0].int_val + 1);
}

/* Example: foo() prints a message and returns 42 */
Value builtin_foo(Value *args, int arg_count) {
    if(arg_count != 0) {
        fprintf(stderr, "Runtime error: foo() expects no arguments.\n");
        exit(1);
    }
    printf("[builtin foo called]\n");
    return make_int(42);
}

#define NUM_BUILTINS 2
static Function builtins[NUM_BUILTINS] = {
    { "inc", builtin_inc },
    { "foo", builtin_foo }
};

Value call_function(const char *name, Value *args, int arg_count) {
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if(strcmp(builtins[i].name, name) == 0)
            return builtins[i].func(args, arg_count);
    }
    fprintf(stderr, "Runtime error: Unknown function \"%s\".\n", name);
    exit(1);
}

/* ============================================================
   Tokenizer
   ============================================================ */

typedef enum {
    TOKEN_INT,
    TOKEN_STRING,
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
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;  /* For identifiers, literals, or operator text */
} Token;

#define MAX_TOKENS 1024

typedef struct {
    Token tokens[MAX_TOKENS];
    int count;
} TokenList;

void add_token(TokenList *list, TokenType type, const char *text) {
    if(list->count >= MAX_TOKENS) {
        fprintf(stderr, "Tokenizer error: too many tokens.\n");
        exit(1);
    }
    list->tokens[list->count].type = type;
    list->tokens[list->count].text = strdup(text);
    list->count++;
}

void tokenize(const char *src, TokenList *list) {
    list->count = 0;
    const char *p = src;
    while(*p) {
        // Skip whitespace
        if(isspace(*p)) { p++; continue; }
        
        // Skip single-line comments that start with "//"
        if (p[0] == '/' && p[1] == '/') {
            p += 2; // Skip the "//"
            while (*p && *p != '\n') {
                p++;
            }
            continue;  // Continue with the next character after the comment
        }

        // Check for numeric literals
        if(isdigit(*p)) {
            const char *start = p;
            while(isdigit(*p)) p++;
            int len = p - start;
            char *num_str = malloc(len+1);
            strncpy(num_str, start, len);
            num_str[len] = '\0';
            add_token(list, TOKEN_INT, num_str);
            free(num_str);
            continue;
        }

        // Check for string literals
        if(*p == '"') {
            p++; /* skip opening quote */
            const char *start = p;
            while(*p && *p != '"') p++;
            if(*p != '"') {
                fprintf(stderr, "Tokenizer error: Unterminated string literal.\n");
                exit(1);
            }
            int len = p - start;
            char *str_val = malloc(len+1);
            strncpy(str_val, start, len);
            str_val[len] = '\0';
            add_token(list, TOKEN_STRING, str_val);
            free(str_val);
            p++; /* skip closing quote */
            continue;
        }
        
        // Check for identifiers or keywords
        if(isalpha(*p) || *p == '_') {
            const char *start = p;
            while(isalnum(*p) || *p == '_') p++;
            int len = p - start;
            char *id = malloc(len+1);
            strncpy(id, start, len);
            id[len] = '\0';
            if(strcmp(id, "if") == 0)
                add_token(list, TOKEN_IF, id);
            else if(strcmp(id, "else") == 0)
                add_token(list, TOKEN_ELSE, id);
            else if(strcmp(id, "return") == 0)
                add_token(list, TOKEN_RETURN, id);
            else if(strcmp(id, "print") == 0)
                add_token(list, TOKEN_PRINT, id);
            else
                add_token(list, TOKEN_IDENTIFIER, id);
            free(id);
            continue;
        }

        // Check for two-character operators: "==", "+=", "!=", ">=", "<="
        if ((p[0]=='=' && p[1]=='=') ||
        (p[0]=='+' && p[1]=='=') ||
        (p[0]=='!' && p[1]=='=') ||
        (p[0]=='>' && p[1]=='=') ||
        (p[0]=='<' && p[1]=='=')) {
            char op[3];
            op[0] = p[0];
            op[1] = p[1];
            op[2] = '\0';
            add_token(list, TOKEN_OPERATOR, op);
            p += 2;
            continue;
        }
    
        /* Single-character tokens */
        switch(*p) {
            case '=':
            case '+':
            case '-':
            case '*':
            case '/': {
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
            case '<': {
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

typedef struct {
    TokenList *tokens;
    int pos;
} Parser;

Token *current(Parser *p) {
    return &p->tokens->tokens[p->pos];
}
Token *peek(Parser *p) {
    if(p->pos+1 < p->tokens->count)
        return &p->tokens->tokens[p->pos+1];
    return NULL;
}
void advance(Parser *p) {
    if(p->pos < p->tokens->count)
        p->pos++;
}
void expect(Parser *p, TokenType type, const char *msg) {
    if(current(p)->type != type) {
        fprintf(stderr, "Parser error: %s (got '%s')\n", msg, current(p)->text);
        exit(1);
    }
    advance(p);
}

/* Forward declaration */
Value parse_expression(Parser *p);

/* --- Primary expressions --- */
Value parse_primary(Parser *p) {
    Token *tok = current(p);
    if(tok->type == TOKEN_INT) {
        int num = atoi(tok->text);
        advance(p);
        return make_int(num);
    }
    if(tok->type == TOKEN_STRING) {
        Value v = make_string(tok->text);
        advance(p);
        return v;
    }
    if(tok->type == TOKEN_IDENTIFIER) {
        char *id = strdup(tok->text);
        advance(p);
        if(current(p)->type == TOKEN_LPAREN) {
            /* Function call */
            advance(p); // consume '('
            Value args[16];
            int arg_count = 0;
            if(current(p)->type != TOKEN_RPAREN) {
                while (1) {
                    args[arg_count] = parse_expression(p);
                    arg_count++;
                    if(current(p)->type == TOKEN_COMMA)
                        advance(p);
                    else
                        break;
                }
            }
            expect(p, TOKEN_RPAREN, "Expected ')' after function arguments");
            Value ret = call_function(id, args, arg_count);
            /* Free temporary argument values */
            for (int i = 0; i < arg_count; i++) {
                if(args[i].type == VAL_STRING && args[i].temp)
                    free_value(args[i]);
            }
            free(id);
            return ret;
        } else {
            /* Variable reference */
            Value v = get_var(id);
            free(id);
            return v;
        }
    }
    if(tok->type == TOKEN_LPAREN) {
        advance(p); // consume '('
        Value v = parse_expression(p);
        expect(p, TOKEN_RPAREN, "Expected ')' after expression");
        return v;
    }
    fprintf(stderr, "Parser error: Unexpected token '%s'\n", tok->text);
    exit(1);
}

/* Forward declarations */
Value parse_primary(Parser *p);
Value parse_factor(Parser *p);
Value parse_term(Parser *p);         // Add this forward declaration
Value parse_relational(Parser *p);
Value parse_equality(Parser *p);
Value parse_assignment(Parser *p);
Value parse_expression(Parser *p);

/* Now, define parse_relational which calls parse_term */
Value parse_relational(Parser *p) {
    Value left = parse_term(p);
    while (current(p)->type == TOKEN_OPERATOR &&
          (strcmp(current(p)->text, ">") == 0 ||
           strcmp(current(p)->text, "<") == 0 ||
           strcmp(current(p)->text, ">=") == 0 ||
           strcmp(current(p)->text, "<=") == 0)) {
        char op[3];
        strcpy(op, current(p)->text);
        advance(p);
        Value right = parse_term(p);
        if (left.type != VAL_INT || right.type != VAL_INT) {
            fprintf(stderr, "Runtime error: Relational operators only support ints.\n");
            exit(1);
        }
        int result = 0;
        if (strcmp(op, ">") == 0)
            result = (left.int_val > right.int_val);
        else if (strcmp(op, "<") == 0)
            result = (left.int_val < right.int_val);
        else if (strcmp(op, ">=") == 0)
            result = (left.int_val >= right.int_val);
        else if (strcmp(op, "<=") == 0)
            result = (left.int_val <= right.int_val);
        left = make_int(result);
        if (right.type == VAL_STRING && right.temp)
            free_value(right);
    }
    return left;
}

/* --- Factors (handle * and /) --- */
Value parse_factor(Parser *p) {
    Value left = parse_primary(p);
    while (current(p)->type == TOKEN_OPERATOR &&
           (strcmp(current(p)->text, "*") == 0 || strcmp(current(p)->text, "/") == 0)) {
        char op[3];
        strcpy(op, current(p)->text);
        advance(p);
        Value right = parse_primary(p);
        if(strcmp(op, "*") == 0) {
            if(left.type != VAL_INT || right.type != VAL_INT) {
                fprintf(stderr, "Runtime error: '*' supports only ints.\n");
                exit(1);
            }
            int res = left.int_val * right.int_val;
            left = make_int(res);
        } else if(strcmp(op, "/") == 0) {
            if(left.type != VAL_INT || right.type != VAL_INT) {
                fprintf(stderr, "Runtime error: '/' supports only ints.\n");
                exit(1);
            }
            if(right.int_val == 0) {
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

/* --- Terms (handle +, -; note: '+' is also used for string concatenation) --- */
Value parse_term(Parser *p) {
    Value left = parse_factor(p);
    while (current(p)->type == TOKEN_OPERATOR &&
           (strcmp(current(p)->text, "+") == 0 || strcmp(current(p)->text, "-") == 0)) {
        char op[3];
        strcpy(op, current(p)->text);
        advance(p);
        Value right = parse_factor(p);
        if(strcmp(op, "+") == 0) {
            if(left.type == VAL_STRING || right.type == VAL_STRING) {
                /* String concatenation (convert ints to strings if needed) */
                char buffer1[64], buffer2[64];
                char *s1, *s2;
                if(left.type == VAL_STRING)
                    s1 = left.str_val;
                else {
                    sprintf(buffer1, "%d", left.int_val);
                    s1 = buffer1;
                }
                if(right.type == VAL_STRING)
                    s2 = right.str_val;
                else {
                    sprintf(buffer2, "%d", right.int_val);
                    s2 = buffer2;
                }
                char *concat = malloc(strlen(s1) + strlen(s2) + 1);
                strcpy(concat, s1);
                strcat(concat, s2);
                Value temp = make_string(concat);
                free(concat);
                left = temp;
            } else {
                left = make_int(left.int_val + right.int_val);
            }
        } else if(strcmp(op, "-") == 0) {
            if(left.type != VAL_INT || right.type != VAL_INT) {
                fprintf(stderr, "Runtime error: '-' supports only ints.\n");
                exit(1);
            }
            left = make_int(left.int_val - right.int_val);
        }
        if(right.type == VAL_STRING && right.temp)
            free_value(right);
    }
    return left;
}

/* --- Equality (handles "==") --- */
int values_equal(Value a, Value b) {
    if(a.type != b.type)
        return 0;
    if(a.type == VAL_INT)
        return a.int_val == b.int_val;
    if(a.type == VAL_STRING)
        return strcmp(a.str_val, b.str_val) == 0;
    return 0;
}

Value parse_equality(Parser *p) {
    Value left = parse_relational(p);
    while (current(p)->type == TOKEN_OPERATOR &&
           (strcmp(current(p)->text, "==") == 0 || strcmp(current(p)->text, "!=") == 0)) {
        int isNotEqual = 0;
        if (strcmp(current(p)->text, "!=") == 0)
            isNotEqual = 1;
        advance(p);  // skip '==' or '!='
        Value right = parse_relational(p);
        int eq = values_equal(left, right);
        if(isNotEqual)
            eq = !eq;
        left = make_int(eq);
        if(right.type == VAL_STRING && right.temp)
            free_value(right);
    }
    return left;
}

/* --- Assignment (handles x = expr; and x += expr;) --- */
Value parse_assignment(Parser *p) {
    if(current(p)->type == TOKEN_IDENTIFIER && peek(p) &&
       peek(p)->type == TOKEN_OPERATOR &&
       (strcmp(peek(p)->text, "=") == 0 || strcmp(peek(p)->text, "+=") == 0)) {
        char *varName = strdup(current(p)->text);
        advance(p); // consume identifier
        char *assign_op = strdup(current(p)->text);
        advance(p); // consume '=' or '+='
        Value right = parse_assignment(p);
        if(strcmp(assign_op, "=") == 0) {
            set_var(varName, right);
            right.temp = 0;  /* <--- FIX: mark returned value as non-temporary */
        } else if(strcmp(assign_op, "+=") == 0) {
            Value currentVal = get_var(varName);
            Value newVal;
            if(currentVal.type == VAL_STRING || right.type == VAL_STRING) {
                char buffer1[64], buffer2[64];
                char *s1, *s2;
                if(currentVal.type == VAL_STRING)
                    s1 = currentVal.str_val;
                else {
                    sprintf(buffer1, "%d", currentVal.int_val);
                    s1 = buffer1;
                }
                if(right.type == VAL_STRING)
                    s2 = right.str_val;
                else {
                    sprintf(buffer2, "%d", right.int_val);
                    s2 = buffer2;
                }
                char *concat = malloc(strlen(s1) + strlen(s2) + 1);
                strcpy(concat, s1);
                strcat(concat, s2);
                newVal = make_string(concat);
                free(concat);
            } else {
                newVal = make_int(currentVal.int_val + right.int_val);
            }
            set_var(varName, newVal);
            right = newVal;
            right.temp = 0;  /* <--- FIX: mark returned value as non-temporary */
        }
        free(varName);
        free(assign_op);
        return right;
    }
    return parse_equality(p);
}

/* --- Top-level expression parser --- */
Value parse_expression(Parser *p) {
    return parse_assignment(p);
}

/* --- Statements --- */

static int return_flag = 0;
static Value return_value;

void parse_statement(Parser *p);

void parse_block(Parser *p) {
    expect(p, TOKEN_LBRACE, "Expected '{' to start block");
    while(current(p)->type != TOKEN_RBRACE && current(p)->type != TOKEN_EOF && !return_flag) {
        parse_statement(p);
    }
    expect(p, TOKEN_RBRACE, "Expected '}' to end block");
}

void parse_statement(Parser *p) {
    Token *tok = current(p);
    if(tok->type == TOKEN_PRINT) {
        advance(p); // consume 'print'
        Value v = parse_expression(p);
        expect(p, TOKEN_SEMICOLON, "Expected ';' after print statement");
        if(v.type == VAL_INT)
            printf("%d\n", v.int_val);
        else if(v.type == VAL_STRING)
            printf("%s\n", v.str_val);
        else
            printf("null\n");
        if(v.type == VAL_STRING && v.temp)
            free_value(v);
    }
    else if(tok->type == TOKEN_RETURN) {
        advance(p); // consume 'return'
        Value v = parse_expression(p);
        expect(p, TOKEN_SEMICOLON, "Expected ';' after return statement");
        return_flag = 1;
        return_value = v;
    }
    else if(tok->type == TOKEN_IF) {
        advance(p); // consume 'if'
        expect(p, TOKEN_LPAREN, "Expected '(' after if");
        Value cond = parse_expression(p);
        expect(p, TOKEN_RPAREN, "Expected ')' after if condition");
        int executed = 0;
        if(cond.type == VAL_INT && cond.int_val != 0) {
            parse_block(p);
            executed = 1;
        } else {
            /* Skip block */
            if(current(p)->type == TOKEN_LBRACE) {
                int brace_count = 0;
                brace_count++;
                advance(p);
                while(brace_count > 0 && current(p)->type != TOKEN_EOF) {
                    if(current(p)->type == TOKEN_LBRACE)
                        brace_count++;
                    else if(current(p)->type == TOKEN_RBRACE)
                        brace_count--;
                    advance(p);
                }
            }
        }
        while(current(p)->type == TOKEN_ELSE) {
            advance(p); // consume 'else'
            if(current(p)->type == TOKEN_IF) {
                advance(p); // consume 'if'
                expect(p, TOKEN_LPAREN, "Expected '(' after else if");
                Value cond2 = parse_expression(p);
                expect(p, TOKEN_RPAREN, "Expected ')' after else if condition");
                if(!executed && cond2.type == VAL_INT && cond2.int_val != 0) {
                    parse_block(p);
                    executed = 1;
                } else {
                    if(current(p)->type == TOKEN_LBRACE) {
                        int brace_count = 0;
                        brace_count++;
                        advance(p);
                        while(brace_count > 0 && current(p)->type != TOKEN_EOF) {
                            if(current(p)->type == TOKEN_LBRACE)
                                brace_count++;
                            else if(current(p)->type == TOKEN_RBRACE)
                                brace_count--;
                            advance(p);
                        }
                    }
                }
            } else {
                if(!executed) {
                    parse_block(p);
                    executed = 1;
                } else {
                    if(current(p)->type == TOKEN_LBRACE) {
                        int brace_count = 0;
                        brace_count++;
                        advance(p);
                        while(brace_count > 0 && current(p)->type != TOKEN_EOF) {
                            if(current(p)->type == TOKEN_LBRACE)
                                brace_count++;
                            else if(current(p)->type == TOKEN_RBRACE)
                                brace_count--;
                            advance(p);
                        }
                    }
                }
                break;
            }
        }
    }
    else {
        /* Expression statement */
        Value v = parse_expression(p);
        expect(p, TOKEN_SEMICOLON, "Expected ';' after expression statement");
        if(v.type == VAL_STRING && v.temp)
            free_value(v);
    }
}

/* ============================================================
   The interpret() Function
   ============================================================ */

Value interpret(const char *src) {
    TokenList tokens;
    tokens.count = 0;
    tokenize(src, &tokens);
    Parser parser;
    parser.tokens = &tokens;
    parser.pos = 0;
    return_flag = 0;
    return_value = make_null();
    while(current(&parser)->type != TOKEN_EOF && !return_flag) {
        parse_statement(&parser);
    }
    /* Free all tokens */
    for (int i = 0; i < tokens.count; i++) {
        free(tokens.tokens[i].text);
    }
    Value ret = return_value;
    free_vars();
    return ret;
}

