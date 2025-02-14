/*
 * Arcane Script Interpreter
 *
 *         File: main.c
 *       Author: Blake Pell
 * Initial Date: 2025-02-08
 * Last Updated: 2025-02-13
 *      License: MIT License
 * 
 * This is the main entry point for the Arcane scripting language when
 * run from the command line.
 */

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "arcane.h"

// #include <crtdbg.h>

// #define _CRTDBG_MAP_ALLOC

/**
 * Main entry point to the Arcane scripting language when run from the command line.
 */
int main(int argc, char *argv[])
{
    //     // Enable run-time memory check for debug builds.
    //     int flags = _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
    //     | _CRTDBG_LEAK_CHECK_DF
    //     | _CRTDBG_ALLOC_MEM_DF);

    // //_CrtSetBreakAlloc(1038);

    // _CrtSetDbgFlag(flags);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <script_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *script = malloc(file_size + 1);
    if (!script) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    size_t bytes_read = fread(script, 1, file_size, file);
    script[bytes_read] = '\0';
    fclose(file);

    Value ret = interpret(script);
    free(script);

    int exit_code = 0;
    
    printf("Script returned: ");

    if (ret.type == VAL_INT)
    {
        printf("%d\n", ret.int_val);
    }
    else if (ret.type == VAL_STRING)
    {
        printf("%s\n", ret.str_val);
    }
    else if (ret.type == VAL_BOOL)
    {
        printf("%d\n", ret.int_val);
    }
    else
    {
        printf("null\n");
    }

    if (ret.type == VAL_STRING && ret.temp)
    {
        free_value(ret);
    }
    else if (ret.type == VAL_ERROR)
    {
        free_value(ret);
        exit_code = 8;
    }

    return 0;
}