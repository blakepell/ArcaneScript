/*
 * Arcane Script Interpreter
 *
 *         File: arcane.c
 *       Author: Blake Pell
 * Initial Date: 2025-02-08
 * Last Updated: 2025-02-14
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
 #include <math.h>

 #ifdef _WIN32
    #include <time.h>
    #include <windows.h>
    #define strcasecmp _stricmp
 #else
    #include <time.h>
    #include <errno.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
 #endif
 
 extern Value return_value;
 
 /* ============================================================
     Interop Functions
    ============================================================ */
 
    Function interop_functions[] = {
        {"print", fn_print},
        {"println", fn_println},
        {"typeof", fn_typeof},
        {"substring", fn_substring},
        {"left", fn_left},
        {"right", fn_right},
        {"sleep", fn_sleep},
        {"input",  fn_input},
        {"is_number", fn_is_number},
        {"len", fn_strlen},
        {"cint", fn_cint},
        {"cdbl", fn_cdbl},
        {"cstr", fn_cstr},     
        {"cbool", fn_cbool},
        {"cepoch", fn_cepoch},
        {"is_interval", fn_is_interval},
        {"list_contains", fn_list_contains},
        {"list_add", fn_list_add},
        {"list_remove", fn_list_remove},
        {"rnd", fn_number_range},
        {"chance", fn_chance},
        {"replace", fn_replace},
        {"trim", fn_trim },
        {"trim_start", fn_trim_start },
        {"trim_end", fn_trim_end },
        {"lcase", fn_lcase },
        {"ucase", fn_ucase }, 
        {"umin", fn_umin }, 
        {"umax", fn_umax },
        {"timestr", fn_timestr },
        {"abs", fn_abs },
        {"pos", fn_set_cursor_position},
        {"cls", fn_clear_screen},
        {"round", fn_round},
        {"round_up", fn_round_up},
        {"round_down", fn_round_down},
        {"sqrt", fn_sqrt},
        {"contains", fn_contains},
        {"starts_with", fn_starts_with},
        {"ends_with", fn_ends_with},
        {"index_of", fn_index_of},
        {"last_index_of", fn_last_index_of},
        {"month", fn_month},
        {"day", fn_day},
        {"year", fn_year},
        {"cdate", fn_cdate},
        {"today", fn_today},
        {"add_days", fn_add_days},
        {"add_months", fn_add_months},
        {"add_years", fn_add_years},
        {"terminal_width", fn_terminal_width},
        {"terminal_height", fn_terminal_height},
        {"chr", fn_chr},
        {"asc", fn_asc},
        {"ubound", fn_upperbound},
        {"split", fn_split},
        {"new_array", fn_new_array},
        {"array_set", fn_array_set},
        {NULL, NULL} 
     };

 /* ============================================================
     Private Functions: Support for interop script functions.
    ============================================================ */
 
 /**
  *  Extract a single argument (with given max length) from
  *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
  */
 const char *_list_getarg(const char *argument, char *arg, int length)
 {
     while (*argument && isspace(*argument))
     {
         argument++;
     }
 
     if (arg)
     {
         int len = 0;
 
         while (*argument && !isspace(*argument) && len < length - 1)
         {
             *arg++ = *argument++, len++;
         }
     }
     else
     {
         while (*argument && !isspace(*argument))
         {
             argument++;
         }
     }
 
     while (*argument && !isspace(*argument))
     {
         argument++;
     }
 
     while (*argument && isspace(*argument))
     {
         argument++;
     }
 
     if (arg)
     {
         *arg = 0;
     }
 
     return argument;
 }
 
 /**
  * If a list contains a specified value.
  */
 int _list_contains(const char *list, const char *value)
 {
     // We don't deal with nulls, but we don't crash either.  Return
     // false for this case.
     if (list == NULL || value == NULL)
     {
         return 0;
     }
 
     char arg[MSL];
     const char *p = _list_getarg(list, arg, MSL);
 
     while (arg[0])
     {
         if (!strcasecmp(value, arg))
         {
             return 1;
         }
 
         p = _list_getarg(p, arg, MSL);
     }
 
     return 0;
 }
 
 /**
  * Populates a timeval structure with the current time.
  */
 int get_datetime_string(struct timeval *tp, void *tzp)
 {
     tp->tv_sec = time(NULL);
     tp->tv_usec = 0;
 }

 /* ============================================================
     Language Interop Functions
    ============================================================ */

/*
 * Returns a random number between the two given integers.
 */
Value fn_number_range(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: number_range() expects two arguments.\n");
        return return_value;
    }

    if (args[0].type != VAL_INT || args[1].type != VAL_INT)
    {
        raise_error("Runtime error: number_range() expects two integer arguments.\n");
        return return_value;
    }

    int from = args[0].int_val;
    int to = args[1].int_val;         
    static int seeded = 0;

    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }

    long mm = rand() >> 6;

    register int power;
    register int number;

    if (from == 0 && to == 0)
    {
        return make_int(0);
    }

    if ((to = to - from + 1) <= 1)
    {
        return make_int(from);
    }

    for (power = 2; power < to; power <<= 1)
    {
        ;
    }

    while ((number = rand() >> 6 & (power - 1)) >= to)
    {
        ;
    }

    return make_int(from + number); 
}   

/*
 * Returns a random number between the two given integers.
 */
Value fn_chance(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        raise_error("Runtime error: chance() expects one argument between 1 and 100.\n");
        return return_value;
    }

    if (args[0].type != VAL_INT)
    {
        raise_error("Runtime error: chance() expects one argument between 1 and 100.\n");
        return return_value;
    }

    int from = 1;
    int to = 100;
    static int seeded = 0;

    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }

    long mm = rand() >> 6;

    register int power;
    register int number;

    if (from == 0 && to == 0)
    {
        return make_int(0);
    }

    if ((to = to - from + 1) <= 1)
    {
        return make_int(from);
    }

    for (power = 2; power < to; power <<= 1)
    {
        ;
    }

    while ((number = rand() >> 6 & (power - 1)) >= to)
    {
        ;
    }

    return make_bool(from + number < args[0].int_val);
}   

 /**
  * If a string list contains an element.
  */
 Value fn_list_contains(Value *args, int arg_count)
 {
     if (arg_count != 2)
     {
         raise_error("Runtime error: list_contains() expects two arguments.\n");
         return return_value;
     }
 
     Value list = args[0];
     Value arg = args[1];
 
     return make_bool(_list_contains(list.str_val, arg.str_val));
 }
 
 /**
  * Adds an item to a list if it does not already exist in it.
  */
 Value fn_list_add(Value *args, int arg_count)
 {
     if (arg_count != 2)
     {
         raise_error("Runtime error: list_add() expects two arguments.\n");
         return return_value;
     }
 
     Value list = args[0];
     Value arg = args[1];
 
     if (_list_contains(list.str_val, arg.str_val))
     {
         return make_string(list.str_val);
     }
 
     char *new_list = malloc(strlen(list.str_val) + strlen(arg.str_val) + 2);
 
     if (!new_list)
     {
         raise_error("Runtime error: Memory allocation failed in list_add().\n");
         return return_value;
     }
 
     strcpy(new_list, list.str_val);
     strcat(new_list, " ");
     strcat(new_list, arg.str_val);
 
     Value str = make_string(new_list);
     free(new_list);
 
     return str;
 }
 
 /**
  * Removes an item from a list.
  */
 Value fn_list_remove(Value *args, int arg_count)
 {
     if (arg_count != 2)
     {
         raise_error("Runtime error: list_remove() expects two arguments.\n");
         return return_value;
     }
 
     Value list = args[0];
     Value arg = args[1];
 
     const char *p;
     char arg_buf[MSL];
     char new_list[MSL] = {0};
 
     p = list.str_val;
 
     while (*p)
     {
         p = _list_getarg(p, arg_buf, MSL);
 
         if (strcasecmp(arg_buf, arg.str_val))
         {
             if (new_list[0])
             {
                 strcat(new_list, " ");
             }
 
             strcat(new_list, arg_buf);
         }
     }
 
     return make_string(new_list);
 }
 
 /*
  * Prints a value to the screen.
  */
 Value fn_print(Value *args, int arg_count)
 {
     if (arg_count != 1)
     {
         raise_error("Runtime error: print() expects exactly one argument.\n");
         return return_value;
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
         else if (arg.type == VAL_DOUBLE) 
         {
            printf("%f", arg.double_val);
         }
         else if (arg.type == VAL_BOOL)
         {
             printf(arg.int_val ? "true" : "false");
         }
         else if (arg.type == VAL_DATE)
         {
            printf("%02d/%02d/%04d", arg.date_val.month, arg.date_val.day, arg.date_val.year);
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
         raise_error("Runtime error: println() expects exactly one argument.\n");
         return return_value;
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
         else if (arg.type == VAL_DOUBLE) 
         {
            printf("%f\n", arg.double_val);
         }
         else if (arg.type == VAL_BOOL)
         {
             printf(arg.int_val ? "true\n" : "false\n");
         }
         else if (arg.type == VAL_DATE)
         {
             printf("%02d/%02d/%04d\n", arg.date_val.month, arg.date_val.day, arg.date_val.year);
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
         raise_error("Runtime error: typeof() expects exactly one argument.\n");
         return return_value;
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
         case VAL_DOUBLE:
            type_str = "double";
            break;             
         case VAL_DATE:
            type_str = "date";
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
         raise_error("Runtime error: substring() expects 3 arguments: a string, a start index, and a length.\n");
         return return_value;
     }
 
     if (args[0].type != VAL_STRING)
     {
         raise_error("Runtime error: substring() expects the first argument to be a string.\n");
         return return_value;
     }
 
     if (args[1].type != VAL_INT)
     {
         raise_error("Runtime error: substring() expects the second argument to be an int.\n");
         return return_value;
     }
 
     if (args[2].type != VAL_INT)
     {
         raise_error("Runtime error: substring() expects the third argument to be an int.\n");
         return return_value;
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
         raise_error("Runtime error: Memory allocation failed in substring().\n");
         return return_value;
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
         raise_error("Runtime error: left() expects 2 arguments: a string and an int.\n");
         return return_value;
     }
 
     if (args[0].type != VAL_STRING)
     {
         raise_error("Runtime error: left() expects the first argument to be a string.\n");
         return return_value;
     }
 
     if (args[1].type != VAL_INT)
     {
         raise_error("Runtime error: left() expects the second argument to be an int.\n");
         return return_value;
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
         raise_error("Runtime error: Memory allocation failed in left().\n");
         return return_value;
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
         raise_error("Runtime error: right() expects 2 arguments: a string and an int.\n");
         return return_value;
     }
 
     if (args[0].type != VAL_STRING)
     {
         raise_error("Runtime error: right() expects the first argument to be a string.\n");
         return return_value;
     }
 
     if (args[1].type != VAL_INT)
     {
         raise_error("Runtime error: right() expects the second argument to be an int.\n");
         return return_value;
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
         raise_error("Runtime error: Memory allocation failed in right().\n");
         return return_value;
     }
 
     /* Copy the last result_len characters from s. */
     strncpy(result, s + (len - result_len), result_len);
     result[result_len] = '\0';
 
     Value ret = make_string(result);
     free(result);
     return ret;
 }
 
/**
 * Replaces all occurrences of a substring in a string with another substring.
 */
Value fn_replace(Value *args, int arg_count)
{
    if (arg_count != 3)
    {
        raise_error("Runtime error: replace() expects 3 arguments: a string, a substring to replace, and a substring to replace it with.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: replace() expects the first argument to be a string.\n");
        return return_value;
    }

    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: replace() expects the second argument to be a string.\n");
        return return_value;
    }

    if (args[2].type != VAL_STRING)
    {
        raise_error("Runtime error: replace() expects the third argument to be a string.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *find = args[1].str_val;
    char *replace = args[2].str_val;

    // Allocate a new string that is large enough to hold the result.
    // We'll allocate the maximum size it could be, which is the length of the
    // original string times the length of the replacement string.  This is
    // the worst case scenario where every character in the original string
    // is the substring we are replacing.
    int find_len = strlen(find);
    int replace_len = strlen(replace);
    int s_len = strlen(s);
    int result_len = s_len;
    char *result = malloc(result_len + 1);

    if (!result)
    {
        raise_error("Runtime error: Memory allocation failed in replace().\n");
        return return_value;
    }

    // Loop through the string looking for the substring to replace.
    char *p = s;
    char *q = result;
    while (*p)
    {
        // If the substring is found, copy the replacement string.
        if (strstr(p, find) == p)
        {
            strcpy(q, replace);
            q += replace_len;
            p += find_len;
        }
        else
        {
            // Copy the current character and move to the next.
            *q++ = *p++;
        }
    }

    // Null terminate the result string.
    *q = '\0';

    // Create a Value containing the new string.
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
         raise_error("Runtime error: sleep() expects 1 integer argument (milliseconds).\n");
         return return_value;
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
         raise_error("Runtime error: input() expects 0 or 1 argument.\n");
         return return_value;
     }
 
     char prompt[256] = {0};
     if (arg_count == 1)
     {
         if (args[0].type != VAL_STRING)
         {
             raise_error("Runtime error: input() expects a string as prompt.\n");
             return return_value;
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
         raise_error("Runtime error: is_number() expects exactly one argument.\n");
         return return_value;
     }
     if (args[0].type != VAL_STRING)
     {
         raise_error("Runtime error: is_number() expects a string argument.\n");
         return return_value;
     }
 
     const char *s = args[0].str_val;
 
     // Skip any leading whitespace.
     while (*s && isspace((unsigned char) *s))
     {
         s++;
     }
 
     // Handle an optional '+' or '-' sign.
     if (*s == '+' || *s == '-')
     {
         s++;
     }
 
     // There must be at least one digit.
     if (!isdigit((unsigned char) *s))
     {
         return make_bool(false);
     }
 
     // Check the remainder of the string.
     while (*s)
     {
         if (!isdigit((unsigned char) *s))
         {
             return make_bool(false);
         }
         s++;
     }
 
     return make_bool(true);
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
         raise_error("Runtime error: strlen() expects exactly one argument.\n");
         return return_value;
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
         raise_error("Runtime error: cint() expects 1 argument.\n");
         return return_value;
     }
 
     Value input = args[0];
 
     if (input.type == VAL_STRING)
     {
         int num = atoi(input.str_val);
         return make_int(num);
     }
     else if (input.type == VAL_BOOL)
     {
         return make_int(input.int_val != 0);
     }
     else
     {
         raise_error("Runtime error: cint() expects a string or bool argument.\n");
         return return_value;
     }
 }
 
 /**
  * Converts a string, int, or bool to a double.
  */
 Value fn_cdbl(Value *args, int arg_count)
 {
     if (arg_count != 1)
     {
         raise_error("Runtime error: cdbl() expects 1 argument.\n");
         return return_value;
     }
 
     Value input = args[0];
 
     if (input.type == VAL_DOUBLE)
     {
         return input;
     }
     else if (input.type == VAL_INT)
     {
         return make_double((double) input.int_val);
     }
     else if (input.type == VAL_BOOL)
     {
         return make_double(input.int_val ? 1.0 : 0.0);
     }
     else if (input.type == VAL_STRING)
     {
         double num = atof(input.str_val);
         return make_double(num);
     }
     else
     {
         raise_error("Runtime error: cdbl() expects a string, int, bool, or double argument.\n");
         return return_value;
     }
 }

 /*
  * Converts an int or bool to a string.
  */
 Value fn_cstr(Value *args, int arg_count)
 {
     if (arg_count != 1)
     {
         raise_error("Runtime error: cstr() expects 1 argument.\n");
         return return_value;
     }
 
     Value input = args[0];
     char buffer[64];
 
     if (input.type == VAL_INT)
     {
         snprintf(buffer, sizeof(buffer), "%d", input.int_val);
         return make_string(buffer);
     }
     else if (input.type == VAL_DOUBLE)
     {
         snprintf(buffer, sizeof(buffer), "%f", input.double_val);
         return make_string(buffer);
     }
     else if (input.type == VAL_BOOL)
     {
         return make_string(input.int_val ? "true" : "false");
     }
     else if (input.type == VAL_DATE)
     {
        snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d", input.date_val.month, input.date_val.day, input.date_val.year);
        return make_string(buffer);
     }
     else
     {
         raise_error("Runtime error: cstr() expects an int, double, bool or date argument.\n");
         return return_value;
     }
 }
 
 /*
  * Converts an int to a bool (nonzero true) or a string ("true"/"false" case-insensitive) to a bool.
  */
 Value fn_cbool(Value *args, int arg_count)
 {
     if (arg_count != 1)
     {
         raise_error("Runtime error: cbool() expects 1 argument.\n");
         return return_value;
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
             raise_error("fn_cbool error: memory allocation failed.\n");
             return return_value;
         }
 
         for (char *p = lower; *p; ++p)
         {
             *p = tolower(*p);
         }
 
         if (strcmp(lower, "true") == 0)
         {
             free(lower);
             return make_bool(true);
         }
         else if (strcmp(lower, "false") == 0)
         {
             free(lower);
             return make_bool(false);
         }
         else
         {
             free(lower);
             raise_error("fn_cbool error: unsupported string value '%s'.\n", input.str_val);
             return return_value;
         }
     }
     else
     {
         raise_error("fn_cbool error: unsupported type.\n");
         return return_value;
     }
 }
 
 /**
  *  Returns true if the given value is an interval (mod), false otherwise.
  */
 Value fn_is_interval(Value *args, int arg_count)
 {
     if (arg_count != 2)
     {
         raise_error("Runtime error: is_interval() expects two arguments.\n");
         return return_value;
     }
 
     if (args[0].type != VAL_INT || args[1].type != VAL_INT)
     {
         return make_bool(false);
     }
 
     if (args[1].int_val == 0) {
         return make_bool(false);
     }
 
     return make_bool((args[0].int_val % args[1].int_val) == 0);
 }

 /**
  * Trims whitespace from the beginning and end of a string.
  */
 Value fn_trim(Value *args, int arg_count)
 {
    if (arg_count != 1)
    {
        raise_error("Runtime error: trim() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: trim() expects a string argument.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *start = s;
    char *end = s + strlen(s) - 1;

    while (isspace(*start))
    {
        start++;
    }

    while (end > start && isspace(*end))
    {
        end--;
    }

    int len = end - start + 1;
    char *result = malloc(len + 1);

    if (!result)
    {
        raise_error("Runtime error: Memory allocation failed in trim().\n");
        return return_value;
    }

    strncpy(result, start, len);
    result[len] = '\0';

    Value ret = make_string(result);
    free(result);
    return ret;    
 }

 /**
  * Trims whitespace from the beginning of a string.
  */
 Value fn_trim_start(Value *args, int arg_count)
 {
    if (arg_count != 1)
    {
        raise_error("Runtime error: trim_start() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: trim_start() expects a string argument.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *start = s;
    char *end = s + strlen(s) - 1;

    while (isspace(*start))
    {
        start++;
    }

    int len = end - start + 1;
    char *result = malloc(len + 1);

    if (!result)
    {
        raise_error("Runtime error: Memory allocation failed in trim_start().\n");
        return return_value;
    }

    strncpy(result, start, len);
    result[len] = '\0';

    Value ret = make_string(result);
    free(result);
    return ret;    
 }

 /**
  * Trims whitespace from the end of a string.
  */
 Value fn_trim_end(Value *args, int arg_count)
 {
    if (arg_count != 1)
    {
        raise_error("Runtime error: trim_end() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: trim_end() expects a string argument.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *start = s;
    char *end = s + strlen(s) - 1;

    while (end > start && isspace(*end))
    {
        end--;
    }

    int len = end - start + 1;
    char *result = malloc(len + 1);

    if (!result)
    {
        raise_error("Runtime error: Memory allocation failed in trim_end().\n");
        return return_value;
    }

    strncpy(result, start, len);
    result[len] = '\0';

    Value ret = make_string(result);
    free(result);
    return ret;    
 }

 /**
  * Converts a string to lowercase.
  */
 Value fn_lcase(Value *args, int arg_count)
 {
    if (arg_count != 1)
    {
        raise_error("Runtime error: lcase() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: lcase() expects a string argument.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *p = s;
    while (*p)
    {
        *p = tolower(*p);
        p++;
    }

    return make_string(s);    
 }

 /**
  * Converts a string to uppercase.
  */
 Value fn_ucase(Value *args, int arg_count)
 {
    if (arg_count != 1)
    {
        raise_error("Runtime error: ucase() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: ucase() expects a string argument.\n");
        return return_value;
    }

    char *s = args[0].str_val;
    char *p = s + strlen(s) - 1;
    while (p >= s)
    {
        *p = toupper(*p);
        p--;
    }

    return make_string(s);    
 }

 /**
  * Returns the lower of two int values.
  */
Value fn_umin(Value *args, int arg_count) 
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: umin() expects exactly two arguments.\n");
        return return_value;
    }

    if (args[0].type != VAL_INT || args[1].type != VAL_INT)
    {
        raise_error("Runtime error: umin() expects two integer arguments.\n");
        return return_value;
    }

    return make_int(args[0].int_val < args[1].int_val ? args[0].int_val : args[1].int_val);
}

/**
 * Returns the higher of two int values.
 */
Value fn_umax(Value *args, int arg_count) 
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: umax() expects exactly two arguments.\n");
        return return_value;
    }

    if (args[0].type != VAL_INT || args[1].type != VAL_INT)
    {
        raise_error("Runtime error: umax() expects two integer arguments.\n");
        return return_value;
    }

    return make_int(args[0].int_val > args[1].int_val ? args[0].int_val : args[1].int_val);
}

/**
 * Returns the current date/time as a string.
 */
Value fn_timestr(Value *args, int arg_count) 
{
    char buf[256];
    struct timeval now_time;

    get_datetime_string(&now_time, NULL);
    time_t current_time = (time_t) now_time.tv_sec;
    strcpy(buf, ctime( &current_time ));

    return make_string(buf);
}

/**
 * Returns the absolute value of a number.
 */
Value fn_abs(Value *args, int arg_count) 
{
    if (arg_count != 1)
    {
        raise_error("Runtime error: abs() expects exactly one argument.\n");
        return return_value;
    }

    if (args[0].type != VAL_INT)
    {
        raise_error("Runtime error: abs() expects an integer argument.\n");
        return return_value;
    }

    int num = args[0].int_val;
    int abs_val = num < 0 ? -num : num;
    return make_int(abs_val);
}

/**
 * Sets the cursor position in the terminal.
 */
Value fn_set_cursor_position(Value *args, int arg_count)
{
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT)
    {
        raise_error("Runtime error: set_cursor_position expects two integer arguments.\n");
        return make_null();
    }
    int x = args[0].int_val;
    int y = args[1].int_val;
    printf("\033[%d;%dH", x, y);

    return make_null();
}

/**
 * Clears the terminal screen using ANSI escape codes.
 */
Value fn_clear_screen(Value *args, int arg_count)
{
    if (arg_count != 0)
    {
        raise_error("Runtime error: clear() expects no arguments.\n");
        return return_value;
    }

    // ANSI escape codes to clear the screen and reset the cursor position.
    printf("\033[2J\033[H");
    fflush(stdout);

    return make_null();
}

Value fn_round(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DOUBLE)
    {
        raise_error("Runtime error: round() expects one double argument.\n");
        return return_value;
    }

    double d = args[0].double_val;
    // round() rounds to the nearest integer.
    return make_int((int) round(d));
}

Value fn_round_up(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DOUBLE)
    {
        raise_error("Runtime error: roundup() expects one double argument.\n");
        return return_value;
    }

    double d = args[0].double_val;
    // ceil() returns the smallest integer value not less than d.
    return make_int((int) ceil(d));
}

Value fn_round_down(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DOUBLE)
    {
        raise_error("Runtime error: roundown() expects one double argument.\n");
        return return_value;
    }

    double d = args[0].double_val;
    // floor() returns the largest integer value not greater than d.
    return make_int((int) floor(d));
}

/**
 *  Finds the square root of a number.
 */
Value fn_sqrt(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DOUBLE)
    {
        raise_error("Runtime error: sqrt() expects one double argument.\n");
        return return_value;
    }

    double d = args[0].double_val;
    if (d < 0)
    {
        raise_error("Runtime error: sqrt() domain error, negative value.\n");
        return return_value;
    }

    return make_double(sqrt(d));
}

/**
 * If one string is contained within another.
 */
Value fn_contains(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: contains() expects exactly 2 arguments.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: contains() expects the first argument to be a string.\n");
        return return_value;
    }

    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: contains() expects the second argument to be a string.\n");
        return return_value;
    }

    // Use strstr to check if the second string is found within the first.
    if (strstr(args[0].str_val, args[1].str_val))
    {
        return make_bool(true);
    }

    return make_bool(false);
}

/**
 * Returns true if the first string starts with the second string.
 */
Value fn_starts_with(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: starts_with() expects exactly 2 arguments.\n");
        return return_value;
    }
    
    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: starts_with() expects the first argument to be a string.\n");
        return return_value;
    }
    
    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: starts_with() expects the second argument to be a string.\n");
        return return_value;
    }
    
    const char *str = args[0].str_val;
    const char *prefix = args[1].str_val;
    size_t prefix_len = strlen(prefix);
    
    if (strncmp(str, prefix, prefix_len) == 0)
    {
        return make_bool(true);
    }
    
    return make_bool(false);
}

/**
 * Returns true if the first string ends with the second string.
 */
Value fn_ends_with(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: ends_with() expects exactly 2 arguments.\n");
        return return_value;
    }
    
    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: ends_with() expects the first argument to be a string.\n");
        return return_value;
    }
    
    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: ends_with() expects the second argument to be a string.\n");
        return return_value;
    }
    
    const char *str = args[0].str_val;
    const char *suffix = args[1].str_val;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len)
    {
        return make_bool(false);
    }
    
    if (strcmp(str + (str_len - suffix_len), suffix) == 0)
    {
        return make_bool(true);
    }
    
    return make_bool(false);
}

/*
 * Searchs for the first occurrence of a substring in a string.  Returns -1
 * if no matches are found.
 */
Value fn_index_of(Value *args, int arg_count)
{
    if (arg_count != 3)
    {
        raise_error("Runtime error: index_of() expects 3 arguments: a string, a substring, and a starting index.\n");
        return return_value;
    }

    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: index_of() expects the first argument to be a string.\n");
        return return_value;
    }

    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: index_of() expects the second argument to be a string.\n");
        return return_value;
    }

    if (args[2].type != VAL_INT)
    {
        raise_error("Runtime error: index_of() expects the third argument to be an int.\n");
        return return_value;
    }

    const char *str = args[0].str_val;
    const char *substr = args[1].str_val;
    int start = args[2].int_val;
    size_t str_len = strlen(str);

    if (start < 0 || (size_t)start >= str_len)
    {
        return make_int(-1);
    }

    const char *found = strstr(str + start, substr);
    if (found)
    {
        int index = found - str;
        return make_int(index);
    }

    return make_int(-1);
}

/**
 * Searchs for the last occurrence of a substring in a string.  Returns -1
 * if no matches are found.
 */
Value fn_last_index_of(Value *args, int arg_count)
{
    if (arg_count < 2 || arg_count > 3)
    {
        raise_error("Runtime error: last_index_of() expects 2 or 3 arguments: a string, a substring, and an optional starting index.\n");
        return return_value;
    }
    
    if (args[0].type != VAL_STRING)
    {
        raise_error("Runtime error: last_index_of() expects the first argument to be a string.\n");
        return return_value;
    }
    
    if (args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: last_index_of() expects the second argument to be a string.\n");
        return return_value;
    }
    
    const char *str = args[0].str_val;
    const char *substr = args[1].str_val;
    size_t str_len = strlen(str);
    size_t substr_len = strlen(substr);
    int start;
    
    if (arg_count == 3)
    {
        if (args[2].type != VAL_INT)
        {
            raise_error("Runtime error: last_index_of() expects the third argument to be an int.\n");
            return return_value;
        }
        start = args[2].int_val;
    }
    else
    {
        // Default starting index: from the end of the string.
        start = (int)str_len - 1;
    }
    
    // If start is negative, return -1.
    if (start < 0)
    {
        return make_int(-1);
    }
    
    // Clamp start to the last valid index.
    if ((size_t)start >= str_len)
    {
        start = (int)str_len - 1;
    }
    
    // Search backwards from the start position.
    for (int i = start; i >= 0; i--)
    {
        // Ensure there is enough room for substr to match.
        if ((size_t)(i + substr_len) > str_len)
        {
            continue;
        }
        
        if (strncmp(str + i, substr, substr_len) == 0)
        {
            return make_int(i);
        }
    }
    
    return make_int(-1);
}

/**
 * Returns the month of a date.
 */
Value fn_month(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DATE)
    {
        raise_error("month() requires one date argument.\n");
        return make_null();
    }
    return make_int(args[0].date_val.month);
}

/**
 * Returns the day of a date.
 */
Value fn_day(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DATE)
    {
        raise_error("day() requires one date argument.\n");
        return make_null();
    }
    return make_int(args[0].date_val.day);
}

/**
 * Returns the year of a date.
 */
Value fn_year(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DATE)
    {
        raise_error("year() requires one date argument.\n");
        return make_null();
    }
    return make_int(args[0].date_val.year);
}

/**
 * Converts a string or an int (as an epoch) to a Date value.
 */
Value fn_cdate(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        raise_error("cdate() requires one argument.\n");
        return make_null();
    }

    if (args[0].type == VAL_STRING)
    {
        // Supported formats: MM/DD/YYYY or YYYY/MM/DD
        int m, d, y;
        if (sscanf(args[0].str_val, "%d/%d/%d", &m, &d, &y) == 3)
        {
            // If the first number is greater than 12, assume format is YYYY/MM/DD.
            if (m > 12)
            {
                int temp = m;
                m = d;
                d = y;
                y = temp;
            }
            Date date = { m, d, y };
            return make_date(date);
        }
        else
        {
            raise_error("cdate() could not parse date from string: %s\n", args[0].str_val);
            return make_null();
        }
    }
    else if (args[0].type == VAL_INT)
    {
        time_t epoch = args[0].int_val;
        struct tm *tm_date = localtime(&epoch);
        if (!tm_date)
        {
            raise_error("cdate() failed to convert epoch %d to date.\n", args[0].int_val);
            return make_null();
        }
        Date date = { tm_date->tm_mon + 1, tm_date->tm_mday, tm_date->tm_year + 1900 };
        return make_date(date);
    }
    else
    {
        raise_error("cdate() expects a string or integer argument.\n");
        return make_null();
    }
}

/**
 * Returns today's date as a Date value.
 */
Value fn_today(Value *args, int arg_count)
{
    if (arg_count != 0)
    {
        raise_error("today() expects no arguments.\n");
        return make_null();
    }

    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    if (!local)
    {
        raise_error("today() failed to retrieve local time.\n");
        return make_null();
    }

    // struct tm: tm_mon is 0-indexed and tm_year is years since 1900.
    Date date = { local->tm_mon + 1, local->tm_mday, local->tm_year + 1900 };
    return make_date(date);
}

/**
 * Adds a given number of days to a date.
 */
Value fn_add_days(Value *args, int arg_count)
{
    if (arg_count != 2 || args[0].type != VAL_DATE || args[1].type != VAL_INT)
    {
        raise_error("add_days() expects a date and an integer.\n");
        return make_null();
    }

    Date old = args[0].date_val;
    int days = args[1].int_val;

    struct tm tm_date = {0};
    tm_date.tm_year = old.year - 1900;
    tm_date.tm_mon  = old.month - 1;
    tm_date.tm_mday = old.day;
    tm_date.tm_isdst = -1;

    time_t t = mktime(&tm_date);
    if (t == -1)
    {
        raise_error("add_days() failed to convert date.\n");
        return make_null();
    }

    t += days * 86400; // 86400 seconds in a day
    struct tm *new_tm = localtime(&t);
    if (!new_tm)
    {
        raise_error("add_days() failed to compute new date.\n");
        return make_null();
    }

    Date result = { new_tm->tm_mon + 1, new_tm->tm_mday, new_tm->tm_year + 1900 };
    return make_date(result);
}

/**
 * Adds a given number of months to a date.
 */
Value fn_add_months(Value *args, int arg_count)
{
    if (arg_count != 2 || args[0].type != VAL_DATE || args[1].type != VAL_INT)
    {
        raise_error("add_months() expects a date and an integer.\n");
        return make_null();
    }

    Date old = args[0].date_val;
    int months_to_add = args[1].int_val;
    int new_month = old.month + months_to_add;
    int new_year = old.year;

    // Normalize month and adjust year accordingly.
    while (new_month > 12)
    {
        new_month -= 12;
        new_year++;
    }
    while (new_month < 1)
    {
        new_month += 12;
        new_year--;
    }

    int new_day = old.day;
    int days_in_month;
    switch(new_month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            days_in_month = 31; break;
        case 4: case 6: case 9: case 11:
            days_in_month = 30; break;
        case 2:
            // Leap year check.
            if ((new_year % 4 == 0 && new_year % 100 != 0) || (new_year % 400 == 0))
                days_in_month = 29;
            else
                days_in_month = 28;
            break;
        default:
            days_in_month = 31; break;
    }
    if (new_day > days_in_month)
    {
        new_day = days_in_month;
    }

    Date result = { new_month, new_day, new_year };
    return make_date(result);
}

/**
 * Adds a given number of years to a date.
 */
Value fn_add_years(Value *args, int arg_count)
{
    if (arg_count != 2 || args[0].type != VAL_DATE || args[1].type != VAL_INT)
    {
        raise_error("add_years() expects a date and an integer.\n");
        return make_null();
    }

    Date old = args[0].date_val;
    int years_to_add = args[1].int_val;
    int new_year = old.year + years_to_add;
    int new_month = old.month;
    int new_day = old.day;

    // Adjust February 29th if new year is not a leap year.
    if (new_month == 2 && new_day == 29)
    {
        if (!((new_year % 4 == 0 && new_year % 100 != 0) || (new_year % 400 == 0)))
        {
            new_day = 28;
        }
    }

    Date result = { new_month, new_day, new_year };
    return make_date(result);
}

/**
 * Converts a date to its Unix epoch equivalent.
 */
Value fn_cepoch(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_DATE)
    {
        raise_error("cepoch() expects a single date argument.\n");
        return make_null();
    }
    
    Date date = args[0].date_val;
    struct tm tm_date = {0};
    tm_date.tm_year = date.year - 1900;
    tm_date.tm_mon  = date.month - 1;
    tm_date.tm_mday = date.day;
    tm_date.tm_hour = 0;
    tm_date.tm_min  = 0;
    tm_date.tm_sec  = 0;
    tm_date.tm_isdst = -1;

    time_t epoch = mktime(&tm_date);
    if (epoch == -1)
    {
        raise_error("cepoch() failed to convert date to epoch.\n");
        return make_null();
    }
    
    return make_int((int)epoch);
}

/**
 * Returns the width of the terminal window.
 */
Value fn_terminal_width(Value *args, int arg_count)
{
    int width;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        // Handle error: default to a safe size or exit.
        width = 80;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        width = w.ws_col;
        height = w.ws_row;
    } else {
        // Handle error: default to a safe size or exit.
        width = 80;
    }
#endif

    return make_int(width);
}

/**
 * Returns the height of the terminal window.
 */
Value fn_terminal_height(Value *args, int arg_count)
{
    int height;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        // Handle error: default to a safe size or exit.
        height = 25;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        height = w.ws_row;
    } else {
        // Handle error: default to a safe size or exit.
        height = 25;
    }
#endif

    return make_int(height);
}

/**
 * Returns the ASCII character for a given code.
 */
Value fn_chr(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_INT)
    {
        raise_error("asc() expects a single integer argument.\n");
        return make_null();
    }

    int code = args[0].int_val;
    char result[2];
    result[0] = (char) code;
    result[1] = '\0';

    return make_string(result);
}

/**
 * Returns the ASCII code for a given character.
 */
Value fn_asc(Value *args, int arg_count)
{
    if (arg_count != 1 || args[0].type != VAL_STRING)
    {
        raise_error("asc() expects a single character argument.\n");
        return make_null();
    }

    char *str = args[0].str_val;
    return make_int((int) str[0]);
}

Value fn_upperbound(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        raise_error("Runtime error: upperbound() expects one argument (an array).\n");
        return return_value;
    }
    if (args[0].type != VAL_ARRAY)
    {
        raise_error("Runtime error: upperbound() expects an array.\n");
        return return_value;
    }
    Array *arr = args[0].array_val;
    return make_int(arr->length - 1);
}

Value fn_split(Value *args, int arg_count)
{
    if (arg_count != 2)
    {
        raise_error("Runtime error: split() expects two arguments: a string and a delimiter.\n");
        return return_value;
    }
    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING)
    {
        raise_error("Runtime error: split() expects both arguments to be strings.\n");
        return return_value;
    }
    char *str = args[0].str_val;
    char *delim = args[1].str_val;
    
    // First, count the number of tokens
    int count = 0;
    char *copy = strdup(str);
    char *token = strtok(copy, delim);
    while (token != NULL)
    {
        count++;
        token = strtok(NULL, delim);
    }
    free(copy);
    
    // Allocate an array of Value items
    Value *items = malloc(sizeof(Value) * count);
    if (!items)
    {
        raise_error("Runtime error: Memory allocation failed in split().\n");
        return return_value;
    }
    int index = 0;
    copy = strdup(str);
    token = strtok(copy, delim);
    while (token != NULL)
    {
        items[index++] = make_string(token);
        token = strtok(NULL, delim);
    }
    free(copy);
    
    // Create and populate the Array structure
    Array *arr = malloc(sizeof(Array));
    if (!arr)
    {
        free(items);
        raise_error("Runtime error: Memory allocation failed in split().\n");
        return return_value;
    }
    arr->items = items;
    arr->length = count;
    
    Value ret;
    ret.type = VAL_ARRAY;
    ret.array_val = arr;
    ret.temp = 1;
    return ret;
}

Value fn_new_array(Value *args, int arg_count)
{
    if (arg_count != 1)
    {
        raise_error("Runtime error: new_array() expects one argument (the size).\n");
        return return_value;
    }
    if (args[0].type != VAL_INT)
    {
        raise_error("Runtime error: new_array() expects an integer.\n");
        return return_value;
    }
    int size = args[0].int_val;
    if (size < 0)
    {
        raise_error("Runtime error: new_array() expects a non-negative integer.\n");
        return return_value;
    }
    Value *items = malloc(sizeof(Value) * size);
    if (!items)
    {
        raise_error("Runtime error: Memory allocation failed in new_array().\n");
        return return_value;
    }
    for (int i = 0; i < size; i++)
    {
        items[i] = make_null(); // Initialize each element to null
    }
    Array *arr = malloc(sizeof(Array));
    if (!arr)
    {
        free(items);
        raise_error("Runtime error: Memory allocation failed in new_array().\n");
        return return_value;
    }
    arr->items = items;
    arr->length = size;
    
    Value ret;
    ret.type = VAL_ARRAY;
    ret.array_val = arr;
    ret.temp = 1;
    return ret;
}

Value fn_array_set(Value *args, int arg_count)
{
    if (arg_count != 3)
    {
        raise_error("Runtime error: array_set() expects three arguments: an array, an index, and a value.\n");
        return return_value;
    }
    if (args[0].type != VAL_ARRAY)
    {
        raise_error("Runtime error: First argument to array_set() must be an array.\n");
        return return_value;
    }
    if (args[1].type != VAL_INT)
    {
        raise_error("Runtime error: Second argument to array_set() must be an integer index.\n");
        return return_value;
    }
    Array *arr = args[0].array_val;
    int idx = args[1].int_val;
    if (idx < 0 || idx >= arr->length)
    {
        raise_error("Runtime error: Array index out of bounds.\n");
        return return_value;
    }
    /* Free the previous value at that index if necessary */
    free_value(arr->items[idx]);

    Value newVal;
    if (args[2].type == VAL_STRING)
    {
        /* Create a new string so that the array element owns its memory */
        newVal = make_string(args[2].str_val);
        newVal.temp = 0;  // Mark it as owned by the array.
    }
    else
    {
        /* For other types, a shallow copy is sufficient */
        newVal = args[2];
        newVal.temp = 0;
    }
    
    arr->items[idx] = newVal;
    return make_null();
}

