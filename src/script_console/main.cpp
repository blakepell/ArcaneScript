#include <cstdio>
#include <ape.h>
#include <crtdbg.h>
#include <Windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define HEADER "--------------------------------------------------------------------------------\r\n"

static ape_object_t debug_fn(ape_t *ape, void *data, int argc, ape_object_t *args);

int main()
{
    printf("Arcane Script: Console\r\n");
    printf(HEADER);

    char code[4096];
    snprintf(code, 4096, "var name = \"Rhien\"\nprintln(`Hello ${name}`)\ndebug(\"Word\")\nname = \"Blake Pell\"\nprint(`${left(name,5)} ${right(name,4)}`)");
    printf("%s\r\n", code);

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