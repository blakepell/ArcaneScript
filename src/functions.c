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
 #ifdef _WIN32
 #include <windows.h>
 #else
 #include <time.h>
 #include <errno.h>
 #endif


/*
 * Prints a value to the screen.
 */
Value fn_print(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "Runtime error: print() expects exactly one argument.\n");
        exit(1);
    }

    for (int i = 0; i < arg_count; i++)
    {
        Value arg = args[i];
        if (arg.type == VAL_INT)
        {
            printf("%d", arg.int_val);
        }
        else if (arg.type == VAL_STRING)
        {
            if (!IS_NULLSTR(arg.str_val))
            {
                printf("%s", arg.str_val);
            }
        }
        else if (arg.type == VAL_BOOL)
        {
            printf(arg.int_val ? "true" : "false");
        }
    }

    return make_null();
}

/*
 * Prints a value to the screen with a trailing line ending.  Null values do not
 * print to the sreeen.
 */
Value fn_println(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "Runtime error: print() expects exactly one argument.\n");
        exit(1);
    }

    for (int i = 0; i < arg_count; i++)
    {
        Value arg = args[i];
        if (arg.type == VAL_INT)
        {
            printf("%d\n", arg.int_val);
        }
        else if (arg.type == VAL_STRING)
        {
            if (!IS_NULLSTR(arg.str_val))
            {
                printf("%s\n", arg.str_val);
            }
        }
        else if (arg.type == VAL_BOOL)
        {
            printf(arg.int_val ? "true\n" : "false\n");
        }
    }

    return make_null();
}

/**
 * Returns the type of the given value as a string.
 */
Value fn_typeof(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "Runtime error: typeof() expects exactly one argument.\n");
        exit(1);
    }

    Value arg = args[0];
    const char *type_str = "unknown";
    switch (arg.type)
    {
        case VAL_INT:
            type_str = "int";
            break;
        case VAL_STRING:
            type_str = "string";
            break;
        case VAL_BOOL:
            type_str = "bool";
            break;
        case VAL_NULL:
            type_str = "null";
            break;
        default:
            type_str = "unknown";
            break;
    }

    return make_string(type_str);
}

/**
 * Returns the substring of the given string starting at the given index and
 * continuing for the given length.
 */
Value fn_substring(Value *args, int arg_count)
{
    if (arg_count != 3)
    {
        fprintf(stderr, "Runtime error: substring() expects 3 arguments: a string, a start index, and a length.\n");
        exit(1);
    }

    if (args[0].type != VAL_STRING)
    {
        fprintf(stderr, "Runtime error: substring() expects the first argument to be a string.\n");
        exit(1);
    }

    if (args[1].type != VAL_INT)
    {
        fprintf(stderr, "Runtime error: substring() expects the second argument to be an int.\n");
        exit(1);
    }

    if (args[2].type != VAL_INT)
    {
        fprintf(stderr, "Runtime error: substring() expects the third argument to be an int.\n");
        exit(1);
    }

    char *s = args[0].str_val;
    int start = args[1].int_val;
    int len = args[2].int_val;

    if (start < 0)
    {
        start = 0;
    }

    if (len < 0)
    {
        len = 0;
    }

    int s_len = strlen(s);
    if (start >= s_len)
    {
        return make_string("");
    }

    int end = start + len;
    if (end > s_len)
    {
        end = s_len;
    }

    int result_len = end - start;
    char *result = malloc(result_len + 1);

    if (!result)
    {
        fprintf(stderr, "Runtime error: Memory allocation failed in substring().\n");
        exit(1);
    }

    strncpy(result, s + start, result_len);
    result[result_len] = '\0';

    Value ret = make_string(result);
    free(result);
    return ret;
}

/**
 * Returns the first n characters of the given string.
 *
 * @param args Expects a string and an int.
 * @param arg_count Expects 2 arguments: a string and an int.
 * @return A string containing the first n characters of the input string.  If n exceeds the string length then the entire string is returned.
 */
Value fn_left(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        fprintf(stderr, "Runtime error: left() expects 2 arguments: a string and an int.\n");
        exit(1);
    }

    if (args[0].type != VAL_STRING)
    {
        fprintf(stderr, "Runtime error: left() expects the first argument to be a string.\n");
        exit(1);
    }

    if (args[1].type != VAL_INT)
    {
        fprintf(stderr, "Runtime error: left() expects the second argument to be an int.\n");
        exit(1);
    }

    char *s = args[0].str_val;
    int n = args[1].int_val;

    if (n < 0)
    {
        n = 0;
    }

    int len = strlen(s);
    int result_len = (n < len) ? n : len;
    char *result = malloc(result_len + 1);

    if (!result)
    {
        fprintf(stderr, "Runtime error: Memory allocation failed in left().\n");
        exit(1);
    }

    strncpy(result, s, result_len);
    result[result_len] = '\0';

    // Create a Value containing the new string.
    Value ret = make_string(result);
    free(result);
    return ret;
}

/**
 * Returns the right n characters of a string.
 * @param args Expects 2 arguments, a string and an int.
 * @param arg_count 2
 * @return Returns the right n characters of a string.  If n is greater than the string length the entire string is returned.
 */
Value fn_right(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        fprintf(stderr, "Runtime error: right() expects 2 arguments: a string and an int.\n");
        exit(1);
    }

    if (args[0].type != VAL_STRING)
    {
        fprintf(stderr, "Runtime error: right() expects the first argument to be a string.\n");
        exit(1);
    }

    if (args[1].type != VAL_INT)
    {
        fprintf(stderr, "Runtime error: right() expects the second argument to be an int.\n");
        exit(1);
    }

    char *s = args[0].str_val;
    int n = args[1].int_val;

    if (n < 0)
    {
        n = 0;
    }

    int len = strlen(s);
    int result_len = (n < len) ? n : len;
    char *result = malloc(result_len + 1);

    if (!result)
    {
        fprintf(stderr, "Runtime error: Memory allocation failed in right().\n");
        exit(1);
    }

    /* Copy the last result_len characters from s. */
    strncpy(result, s + (len - result_len), result_len);
    result[result_len] = '\0';

    Value ret = make_string(result);
    free(result);
    return ret;
}

/**
 * Sleeps for the given number of milliseconds.
 * @param args Expects 1 argument, an int.
 * @param arg_count 1
 * @return Null
 */
Value fn_sleep(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_INT) {
        fprintf(stderr, "Runtime error: sleep() expects 1 integer argument (milliseconds).\n");
        exit(1);
    }

    int ms = args[0].int_val;

    if (ms < 0)
    {
        ms = 0;
    }

#ifdef _WIN32
    // Sleep() takes milliseconds.
    Sleep(ms);
#else
    // For POSIX, use nanosleep. Construct a timespec.
    struct timespec req, rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000; // convert ms to ns
    // Loop in case nanosleep is interrupted by a signal.
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
#endif

    // Return null as sleep does not produce a meaningful value.
    return make_null();
}

/**
 * Gets input from the user.
 * @return A string value of the input the user entered.
 */
Value fn_input(Value *args, int arg_count)
{
    // Allow zero or one argument (a prompt message)
    if (arg_count > 1)
    {
        fprintf(stderr, "Runtime error: input() expects 0 or 1 argument.\n");
        exit(1);
    }

    char prompt[256] = {0};
    if (arg_count == 1)
    {
        if (args[0].type != VAL_STRING)
        {
            fprintf(stderr, "Runtime error: input() expects a string as prompt.\n");
            exit(1);
        }
        // Copy the prompt into our buffer (limit its size)
        strncpy(prompt, args[0].str_val, sizeof(prompt) - 1);
    }

    // If a prompt was provided, print it (and flush so the user sees it immediately)
    if (prompt[0] != '\0')
    {
        printf("%s", prompt);
        fflush(stdout);
    }

    // Read a line from stdin
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        // On error or EOF, you might return null
        return make_null();
    }

    // Remove the newline character, if present
    buffer[strcspn(buffer, "\n")] = '\0';

    // Return the input as a string value
    return make_string(buffer);
}

/**
 * is_number: returns true if the string is an integer number, false otherwise.
 * @return A bool value of whether the string is a number.
 */
Value fn_is_number(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "Runtime error: is_number() expects exactly one argument.\n");
        exit(1);
    }
    if (args[0].type != VAL_STRING)
    {
        fprintf(stderr, "Runtime error: is_number() expects a string argument.\n");
        exit(1);
    }

    const char *s = args[0].str_val;

    // Skip any leading whitespace.
    while (*s && isspace((unsigned char)*s))
    {
        s++;
    }

    // Handle an optional '+' or '-' sign.
    if (*s == '+' || *s == '-')
    {
        s++;
    }

    // There must be at least one digit.
    if (!isdigit((unsigned char)*s))
    {
        return make_bool(0);
    }

    // Check the remainder of the string.
    while (*s)
    {
        if (!isdigit((unsigned char)*s))
        {
            return make_bool(0);
        }
        s++;
    }

    return make_bool(1);
}

/**
 * Returns the length of the string if the Value is of type string, or 0 otherwise.
 *
 * @param args Expects a single argument.
 * @param arg_count Expects 1 argument.
 * @return An int Value representing the string length or -1 if not a string.
 */
Value fn_strlen(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "Runtime error: strlen() expects exactly one argument.\n");
        exit(1);
    }

    if (args[0].type != VAL_STRING)
    {
        return make_int(-1);
    }

    int len = strlen(args[0].str_val);
    return make_int(len);
}

/*
 * Converts a string to an int.
 */
Value fn_cint(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "fn_cint error: expected 1 argument.\n");
        exit(1);
    }

    Value input = args[0];

    if (input.type == VAL_STRING) 
    {
        int num = atoi(input.str_val);
        return make_int(num);
    } 
    else if(input.type == VAL_BOOL) 
    {
        return make_int(input.int_val != 0);
    } 
    else 
    {
        fprintf(stderr, "fn_cint error: unsupported type.\n");
        exit(1);
    }
}

/*
 * Converts an int or bool to a string.
 */
Value fn_cstr(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        fprintf(stderr, "fn_cstr error: expected 1 argument.\n");
        exit(1);
    }

    Value input = args[0];
    char buffer[64];

    if (input.type == VAL_INT)
    {
        snprintf(buffer, sizeof(buffer), "%d", input.int_val);
        return make_string(buffer);
    } 
    else if(input.type == VAL_BOOL) 
    {
        return make_string(input.int_val ? "true" : "false");
    } 
    else 
    {
        fprintf(stderr, "fn_cstr error: unsupported type.\n");
        exit(1);
    }
}

/*
 * Converts an int to a bool (nonzero true) or a string ("true"/"false" case-insensitive) to a bool.
 */
Value fn_cbool(Value *args, int arg_count)
{
    if (arg_count != 1) 
    {
        fprintf(stderr, "fn_cbool error: expected 1 argument.\n");
        exit(1);
    }

    Value input = args[0];

    if (input.type == VAL_INT) 
    {
        return make_bool(input.int_val != 0);
    }
    else if (input.type == VAL_STRING)
    {
        char *lower = _strdup(input.str_val);

        if (!lower)
        {
            fprintf(stderr, "fn_cbool error: memory allocation failed.\n");
            exit(1);
        }

        for (char *p = lower; *p; ++p)
        {
            *p = tolower(*p);
        }
        
        if(strcmp(lower, "true") == 0)
        {
            free(lower);
            return make_bool(1);
        }
        else if(strcmp(lower, "false") == 0)
        {
            free(lower);
            return make_bool(0);
        }
        else 
        {
            free(lower);
            fprintf(stderr, "fn_cbool error: unsupported string value '%s'.\n", input.str_val);
            exit(1);
        }
    }
    else 
    {
        fprintf(stderr, "fn_cbool error: unsupported type.\n");
        exit(1);
    }
}

/** 
 *  Returns true if the given value is an interval (mod), false otherwise.
 */
Value fn_is_interval(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        fprintf(stderr, "Runtime error: is_interval() expects two arguments.\n");
        exit(1);
    }

    if (args[0].type != VAL_INT || args[1].type != VAL_INT)
    {
        return make_bool(0);
    }

    if (args[1].int_val == 0) {
        return make_bool(0);
    }

    return make_bool((args[0].int_val % args[1].int_val) == 0);    
}