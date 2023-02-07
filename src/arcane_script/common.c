#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef ARCANE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

ARCANE_INTERNAL const src_pos_t src_pos_invalid = { NULL, -1, -1 };
ARCANE_INTERNAL const src_pos_t src_pos_zero = { NULL, 0, 0 };

ARCANE_INTERNAL src_pos_t src_pos_make(const compiled_file_t *file, int line, int column) {
    return (src_pos_t) {
        .file = file,
            .line = line,
            .column = column,
    };
}

char *arcane_stringf(allocator_t *alloc, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int to_write = vsnprintf(NULL, 0, format, args);
    va_end(args);
    va_start(args, format);
    char *res = (char *) allocator_malloc(alloc, to_write + 1);
    if (!res) {
        return NULL;
    }
    int written = vsprintf(res, format, args);
    (void) written;
    ARCANE_ASSERT(written == to_write);
    va_end(args);
    return res;
}

void arcane_log(const char *file, int line, const char *format, ...) {
    char msg[4096];
    int written = snprintf(msg, ARCANE_ARRAY_LEN(msg) - 1, "%s:%d: ", file, line);
    (void) written;
    va_list args;
    va_start(args, format);
    int written_msg = vsnprintf(msg + written, ARCANE_ARRAY_LEN(msg) - written - 1, format, args);
    (void) written_msg;
    va_end(args);

    ARCANE_ASSERT(written_msg <= (ARCANE_ARRAY_LEN(msg) - written));

    printf("%s", msg);
}

char *arcane_strndup(allocator_t *alloc, const char *string, size_t n) {
    char *output_string = (char *) allocator_malloc(alloc, n + 1);
    if (!output_string) {
        return NULL;
    }
    output_string[n] = '\0';
    memcpy(output_string, string, n);
    return output_string;
}

char *arcane_strdup(allocator_t *alloc, const char *string) {
    if (!string) {
        return NULL;
    }
    return arcane_strndup(alloc, string, strlen(string));
}

uint64_t arcane_double_to_uint64(double val) {
    union {
        uint64_t val_uint64;
        double val_double;
    } temp = {
        .val_double = val
    };
    return temp.val_uint64;
}

double arcane_uint64_to_double(uint64_t val) {
    union {
        uint64_t val_uint64;
        double val_double;
    } temp = {
        .val_uint64 = val
    };
    return temp.val_double;
}

bool arcane_timer_platform_supported() {
#if defined(ARCANE_POSIX) || defined(ARCANE_EMSCRIPTEN) || defined(ARCANE_WINDOWS)
    return true;
#else
    return false;
#endif
}

arcane_timer_t arcane_timer_start() {
    arcane_timer_t timer;
    memset(&timer, 0, sizeof(arcane_timer_t));
#if defined(ARCANE_POSIX)
    // At some point it should be replaced with more accurate per-platform timers
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    timer.start_offset = start_time.tv_sec;
    timer.start_time_ms = start_time.tv_usec / 1000.0;
#elif defined(ARCANE_WINDOWS)
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li); // not sure what to do if it fails
    timer.pc_frequency = (double) (li.QuadPart) / 1000.0;
    QueryPerformanceCounter(&li);
    timer.start_time_ms = li.QuadPart / timer.pc_frequency;
#elif defined(ARCANE_EMSCRIPTEN)
    timer.start_time_ms = emscripten_get_now();
#endif
    return timer;
}

double arcane_timer_get_elapsed_ms(const arcane_timer_t *timer) {
#if defined(ARCANE_POSIX)
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    int time_s = (int) ((int64_t) current_time.tv_sec - timer->start_offset);
    double current_time_ms = (time_s * 1000) + (current_time.tv_usec / 1000.0);
    return current_time_ms - timer->start_time_ms;
#elif defined(ARCANE_WINDOWS)
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double current_time_ms = li.QuadPart / timer->pc_frequency;
    return current_time_ms - timer->start_time_ms;
#elif defined(ARCANE_EMSCRIPTEN)
    double current_time_ms = emscripten_get_now();
    return current_time_ms - timer->start_time_ms;
#else
    return 0;
#endif
}
