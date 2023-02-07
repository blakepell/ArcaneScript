#ifndef traceback_h
#define traceback_h

#ifndef ARCANE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

typedef struct vm vm_t;

typedef struct traceback_item {
    char *function_name;
    src_pos_t pos;
} traceback_item_t;

typedef struct traceback {
    allocator_t *alloc;
    array(traceback_item_t) *items;
} traceback_t;

ARCANE_INTERNAL traceback_t *traceback_make(allocator_t *alloc);
ARCANE_INTERNAL void traceback_destroy(traceback_t *traceback);
ARCANE_INTERNAL bool traceback_append(traceback_t *traceback, const char *function_name, src_pos_t pos);
ARCANE_INTERNAL bool traceback_append_from_vm(traceback_t *traceback, vm_t *vm);
ARCANE_INTERNAL bool traceback_to_string(const traceback_t *traceback, strbuf_t *buf);
ARCANE_INTERNAL const char *traceback_item_get_line(traceback_item_t *item);
ARCANE_INTERNAL const char *traceback_item_get_filepath(traceback_item_t *item);

#endif /* traceback_h */
