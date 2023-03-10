#define _CRT_SECURE_NO_WARNINGS

#include <crtdbg.h>
#include <Windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include<stdio.h>
#include "arcane.h"

char *read_file(const char *file_name);

static arcane_object_t debug_fn(arcane_engine_t *arcane, void *data, int argc, arcane_object_t *args);
static void print_ape_errors(arcane_engine_t *engine);

int main()
{
    printf(HEADER);
    printf("Arcane Script Console: %s\r\n", ARCANE_VERSION_STRING);

    TCHAR tszBuffer[MAX_PATH];
    DWORD dwRet;

    dwRet = GetCurrentDirectory(MAX_PATH, tszBuffer);
    if (dwRet == 0)
    {
        //TODO: handle error
    }

    printf("Working Folder: %s\r\n", tszBuffer);

    printf(HEADER);

    char *code = read_file("C:\\Git\\ArcaneScript\\src\\examples\\test.arc");

    // Create the scripting environment
    arcane_engine_t *engine = arcane_make();

    // Add the native functions we're adding on before the program is compiled
    arcane_set_native_function(engine, "debug", debug_fn, NULL);

    // Compile the given program
    arcane_program_t *program = arcane_compile(engine, code);

    // Execute the program against it's scripting environment.
    arcane_execute_program(engine, program);

    if (arcane_has_errors(engine)) {
        print_ape_errors(engine);
    }

    //Sleep(1);

// Free the resources for the program and the scripting environment.
    arcane_program_destroy(program);
    arcane_destroy(engine);

    free(code);
    code = NULL;

    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    //_CrtDumpMemoryLeaks();

    //Sleep(500);
}

static void print_ape_errors(arcane_engine_t *engine)
{
    for (int i = 0; i < arcane_errors_count(engine); i++)
    {
        const arcane_error_t *err = arcane_get_error(engine, i);
        char *err_str = arcane_error_serialize(engine, err);
        puts(err_str);
        arcane_free_allocated(engine, err_str);
    }
}

static arcane_object_t debug_fn(arcane_engine_t *arcane, void *data, int argc, arcane_object_t *args)
{
    if (argc == 1 && arcane_object_get_type(args[0]) == ARCANE_OBJECT_STRING)
    {
        const char *msg = arcane_object_get_string(args[0]);
        printf("Debug :: %s\r\n", msg);
        return arcane_object_make_string(arcane, msg);
    }

    return arcane_object_make_string(arcane, "");
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory for the file contents
    char *file_contents = calloc(file_size, sizeof(char) + 1);
    //char *file_contents = (char *) malloc(file_size * sizeof(char));
    if (file_contents == NULL)
    {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the file into the allocated memory
    size_t result = fread(file_contents, sizeof(char), file_size, file);
    file_contents[file_size] = '\0';

    if (result != file_size)
    {
        perror("Error reading file");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return file_contents;
}
