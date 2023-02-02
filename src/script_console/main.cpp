#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <ape.h>
#include <arcane.h>
#include <crtdbg.h>
#include <Windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

char *read_file(const char *file_name);

static ape_object_t debug_fn(ape_t *ape, void *data, int argc, ape_object_t *args);

int main()
{
    printf(HEADER);
    printf("Arcane Script Console: %s\r\n", ARCANE_VERSION_STRING);
    printf(HEADER);

    char *code = read_file("C:\\Git\\ArcaneScript\\src\\examples\\test.arc");

    //printf("[C:\\Git\\ArcaneScript\\src\\examples\\test.arc]\n");
    //printf(HEADER);

    //printf("%s\r\n", code);
    //printf(HEADER);

    ape_t *ape = ape_make();

    ape_set_native_function(ape, "debug", debug_fn, NULL);

    ape_execute(ape, code);

    ape_destroy(ape);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();

    //Sleep(500);
}

static ape_object_t debug_fn(ape_t *ape, void *data, int argc, ape_object_t *args)
{
    if (argc == 1 && ape_object_get_type(args[0]) == APE_OBJECT_STRING)
    {
        const char *msg = ape_object_get_string(args[0]);
        printf("Debug :: %s\r\n", msg);
        return ape_object_make_string(ape, msg);
    }

    return ape_object_make_string(ape, "");
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory for the file contents
    char *file_contents = (char *) calloc(file_size, sizeof(char));
    //char *file_contents = (char *) malloc(file_size * sizeof(char));
    if (file_contents == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the file into the allocated memory
    size_t result = fread(file_contents, sizeof(char), (size_t) file_size, file);
    file_contents[file_size] = '\0';

    if (result != file_size) {
        perror("Error reading file");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return file_contents;
}