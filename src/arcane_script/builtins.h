#ifndef builtins_h
#define builtins_h

#include <stdint.h>

#ifndef ARCANE_AMALGAMATED
#include "common.h"
#include "object.h"
#endif

typedef struct vm vm_t;

ARCANE_INTERNAL int builtins_count(void);
ARCANE_INTERNAL native_fn builtins_get_fn(int ix);
ARCANE_INTERNAL const char *builtins_get_name(int ix);

#endif /* builtins_h */
