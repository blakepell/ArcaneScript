#ifndef common_h
#define common_h

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#if defined(__linux__)
#define ARCANE_LINUX
#define ARCANE_POSIX
#elif defined(_WIN32)
#define ARCANE_WINDOWS
#elif (defined(__APPLE__) && defined(__MACH__))
#define ARCANE_APPLE
#define ARCANE_POSIX
#elif defined(__EMSCRIPTEN__)
#define ARCANE_EMSCRIPTEN
#endif

#if defined(__unix__)
#include <unistd.h>
#if defined(_POSIX_VERSION)
#define ARCANE_POSIX
#endif
#endif

#if defined(ARCANE_POSIX)
#include <sys/time.h>
#elif defined(ARCANE_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(ARCANE_EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#ifndef ARCANE_AMALGAMATED
#include "arcane.h"
#endif

#define ARCANE_STREQ(a, b) (strcmp((a), (b)) == 0)
#define ARCANE_STRNEQ(a, b, n) (strncmp((a), (b), (n)) == 0)
#define ARCANE_ARRAY_LEN(array) ((int)(sizeof(array) / sizeof(array[0])))
#define ARCANE_DBLEQ(a, b) (fabs((a) - (b)) < DBL_EPSILON)

#ifdef ARCANE_DEBUG
#define ARCANE_ASSERT(x) assert((x))
#define ARCANE_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define ARCANE_LOG(...) arcane_log(ARCANE_FILENAME, __LINE__, __VA_ARGS__)
#else
#define ARCANE_ASSERT(x) ((void)0)
#define ARCANE_LOG(...) ((void)0)
#endif

#ifdef ARCANE_AMALGAMATED
#define COLLECTIONS_AMALGAMATED
#define ARCANE_INTERNAL static
#else
#define ARCANE_INTERNAL
#endif

typedef struct compiled_file compiled_file_t;
typedef struct allocator allocator_t;

typedef struct src_pos {
    const compiled_file_t *file;
    int line;
    int column;
} src_pos_t;

typedef struct arcane_config {
    struct {
        struct {
            arcane_stdout_write_fn write;
            void *context;
        } write;
    } stdio;

    struct {
        struct {
            arcane_read_file_fn read_file;
            void *context;
        } read_file;

        struct {
            arcane_write_file_fn write_file;
            void *context;
        } write_file;
    } fileio;

    bool repl_mode; // allows redefinition of symbols

    double max_execution_time_ms;
    bool max_execution_time_set;
} arcane_config_t;

typedef struct arcane_timer {
#if defined(ARCANE_POSIX)
    int64_t start_offset;
#elif defined(ARCANE_WINDOWS)
    double pc_frequency;
#endif
    double start_time_ms;
} arcane_timer_t;

#ifndef ARCANE_AMALGAMATED
extern const src_pos_t src_pos_invalid;
extern const src_pos_t src_pos_zero;
#endif

ARCANE_INTERNAL src_pos_t src_pos_make(const compiled_file_t *file, int line, int column);
ARCANE_INTERNAL char *arcane_stringf(allocator_t *alloc, const char *format, ...);
ARCANE_INTERNAL void arcane_log(const char *file, int line, const char *format, ...);
ARCANE_INTERNAL char *arcane_strndup(allocator_t *alloc, const char *string, size_t n);
ARCANE_INTERNAL char *arcane_strdup(allocator_t *alloc, const char *string);

ARCANE_INTERNAL uint64_t arcane_double_to_uint64(double val);
ARCANE_INTERNAL double arcane_uint64_to_double(uint64_t val);

ARCANE_INTERNAL bool arcane_timer_platform_supported(void);
ARCANE_INTERNAL arcane_timer_t arcane_timer_start(void);
ARCANE_INTERNAL double arcane_timer_get_elapsed_ms(const arcane_timer_t *timer);

#endif /* common_h */
