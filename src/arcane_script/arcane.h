/*
SPDX-License-Identifier: MIT

arcane
https://github.com/kgabis/arcane
Copyright (c) 2023 Krzysztof Gabis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef arcane_h
#define arcane_h

#ifdef __cplusplus
extern "C"
{
#endif
#if 0
} // unconfuse xcode
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _MSC_VER
#define __attribute__(x)
#endif

#define ARCANE_VERSION_MAJOR 0
#define ARCANE_VERSION_MINOR 15
#define ARCANE_VERSION_PATCH 0

#define ARCANE_VERSION_STRING "0.15.0"
#define HEADER "--------------------------------------------------------------------------------\r\n"

#define IS_NULLSTR(str)      ((str) == NULL || (str)[0] == '\0')
#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit) ((var) ^= (bit))
#define UMIN(a, b)           ((a) < (b) ? (a) : (b))
#define UMAX(a, b)           ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)      ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)             ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)             ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))

/**
 * \brief The execution environment for an instance of the script engine.
 */
typedef struct arcane_engine arcane_engine_t;

typedef struct arcane_object {
    uint64_t _internal;
} arcane_object_t;
typedef struct arcane_error arcane_error_t;
typedef struct arcane_program arcane_program_t;
typedef struct arcane_traceback arcane_traceback_t;

typedef enum arcane_error_type {
    ARCANE_ERROR_NONE = 0,
    ARCANE_ERROR_PARSING,
    ARCANE_ERROR_COMPILATION,
    ARCANE_ERROR_RUNTIME,
    ARCANE_ERROR_TIMEOUT,
    ARCANE_ERROR_ALLOCATION,
    ARCANE_ERROR_USER, // from arcane_add_error() or arcane_add_errorf()
} arcane_error_type_t;

typedef enum arcane_object_type {
    ARCANE_OBJECT_NONE = 0,
    ARCANE_OBJECT_ERROR = 1 << 0,
    ARCANE_OBJECT_NUMBER = 1 << 1,
    ARCANE_OBJECT_BOOL = 1 << 2,
    ARCANE_OBJECT_STRING = 1 << 3,
    ARCANE_OBJECT_NULL = 1 << 4,
    ARCANE_OBJECT_NATIVE_FUNCTION = 1 << 5,
    ARCANE_OBJECT_ARRAY = 1 << 6,
    ARCANE_OBJECT_MAP = 1 << 7,
    ARCANE_OBJECT_FUNCTION = 1 << 8,
    ARCANE_OBJECT_EXTERNAL = 1 << 9,
    ARCANE_OBJECT_FREED = 1 << 10,
    ARCANE_OBJECT_ANY = 0xffff, // for checking types with &
} arcane_object_type_t;

typedef arcane_object_t(*arcane_native_fn)(arcane_engine_t *arcane, void *data, int argc, arcane_object_t *args);
typedef void *(*arcane_malloc_fn)(void *ctx, size_t size);
typedef void (*arcane_free_fn)(void *ctx, void *ptr);
typedef void (*arcane_data_destroy_fn)(void *data);
typedef void *(*arcane_data_copy_fn)(void *data);

typedef size_t(*arcane_stdout_write_fn)(void *context, const void *data, size_t data_size);
typedef char *(*arcane_read_file_fn)(void *context, const char *path);
typedef size_t(*arcane_write_file_fn)(void *context, const char *path, const char *string, size_t string_size);

//-----------------------------------------------------------------------------
// Ape API
//-----------------------------------------------------------------------------
arcane_engine_t *arcane_make(void);
arcane_engine_t *arcane_make_ex(arcane_malloc_fn malloc_fn, arcane_free_fn free_fn, void *ctx);
void   arcane_destroy(arcane_engine_t *arcane);

void   arcane_free_allocated(arcane_engine_t *arcane, void *ptr);

void arcane_set_repl_mode(arcane_engine_t *arcane, bool enabled);

// -1 to disable, returns false if it can't be set for current platform (otherwise true).
// If execution time exceeds given limit an ARCANE_ERROR_TIMEOUT error is set.
// Precision is not guaranteed because time can't be checked every VM tick
// but expect it to be submilisecond.
bool arcane_set_timeout(arcane_engine_t *arcane, double max_execution_time_ms);

void arcane_set_stdout_write_function(arcane_engine_t *arcane, arcane_stdout_write_fn stdout_write, void *context);
void arcane_set_file_write_function(arcane_engine_t *arcane, arcane_write_file_fn file_write, void *context);
void arcane_set_file_read_function(arcane_engine_t *arcane, arcane_read_file_fn file_read, void *context);

arcane_program_t *arcane_compile(arcane_engine_t *arcane, const char *code);
arcane_program_t *arcane_compile_file(arcane_engine_t *arcane, const char *path);
arcane_object_t   arcane_execute_program(arcane_engine_t *arcane, const arcane_program_t *program);
void           arcane_program_destroy(arcane_program_t *program);

arcane_object_t  arcane_execute(arcane_engine_t *arcane, const char *code);
arcane_object_t  arcane_execute_file(arcane_engine_t *arcane, const char *path);

arcane_object_t  arcane_call(arcane_engine_t *arcane, const char *function_name, int argc, arcane_object_t *args);
#define ARCANE_CALL(arcane, function_name, ...) \
    arcane_call(\
        (arcane),\
        (function_name),\
        sizeof((arcane_object_t[]){__VA_ARGS__}) / sizeof(arcane_object_t),\
        (arcane_object_t[]){__VA_ARGS__})

void arcane_set_runtime_error(arcane_engine_t *arcane, const char *message);
void arcane_set_runtime_errorf(arcane_engine_t *arcane, const char *format, ...) __attribute__((format(printf, 2, 3)));
bool arcane_has_errors(const arcane_engine_t *arcane);
int  arcane_errors_count(const arcane_engine_t *arcane);
void arcane_clear_errors(arcane_engine_t *arcane);
const arcane_error_t *arcane_get_error(const arcane_engine_t *arcane, int index);

bool arcane_set_native_function(arcane_engine_t *arcane, const char *name, arcane_native_fn fn, void *data);
bool arcane_set_global_constant(arcane_engine_t *arcane, const char *name, arcane_object_t obj);
arcane_object_t arcane_get_object(arcane_engine_t *arcane, const char *name);

bool arcane_check_args(arcane_engine_t *arcane, bool generate_error, int argc, arcane_object_t *args, int expected_argc, int *expected_types);
#define ARCANE_CHECK_ARGS(arcane, generate_error, argc, args, ...)\
    arcane_check_args(\
        (arcane),\
        (generate_error),\
        (argc),\
        (args),\
        sizeof((int[]){__VA_ARGS__}) / sizeof(int),\
        (int[]){__VA_ARGS__})

//-----------------------------------------------------------------------------
// Ape object
//-----------------------------------------------------------------------------

arcane_object_t arcane_object_make_number(double val);
arcane_object_t arcane_object_make_bool(bool val);
arcane_object_t arcane_object_make_null(void);
arcane_object_t arcane_object_make_string(arcane_engine_t *arcane, const char *str);
arcane_object_t arcane_object_make_stringf(arcane_engine_t *arcane, const char *format, ...) __attribute__((format(printf, 2, 3)));
arcane_object_t arcane_object_make_array(arcane_engine_t *arcane);
arcane_object_t arcane_object_make_map(arcane_engine_t *arcane);
arcane_object_t arcane_object_make_native_function(arcane_engine_t *arcane, arcane_native_fn fn, void *data);
arcane_object_t arcane_object_make_error(arcane_engine_t *arcane, const char *message);
arcane_object_t arcane_object_make_errorf(arcane_engine_t *arcane, const char *format, ...) __attribute__((format(printf, 2, 3)));
arcane_object_t arcane_object_make_external(arcane_engine_t *arcane, void *data);

char *arcane_object_serialize(arcane_engine_t *arcane, arcane_object_t obj);

bool arcane_object_disable_gc(arcane_object_t obj);
void arcane_object_enable_gc(arcane_object_t obj);

bool arcane_object_equals(arcane_object_t a, arcane_object_t b);

bool arcane_object_is_null(arcane_object_t obj);

arcane_object_t arcane_object_copy(arcane_object_t obj);
arcane_object_t arcane_object_deep_copy(arcane_object_t obj);

arcane_object_type_t arcane_object_get_type(arcane_object_t obj);
const char *arcane_object_get_type_string(arcane_object_t obj);
const char *arcane_object_get_type_name(arcane_object_type_t type);

double       arcane_object_get_number(arcane_object_t obj);
bool         arcane_object_get_bool(arcane_object_t obj);
const char *arcane_object_get_string(arcane_object_t obj);

const char *arcane_object_get_error_message(arcane_object_t obj);
const arcane_traceback_t *arcane_object_get_error_traceback(arcane_object_t obj);

bool arcane_object_set_external_destroy_function(arcane_object_t object, arcane_data_destroy_fn destroy_fn);
bool arcane_object_set_external_copy_function(arcane_object_t object, arcane_data_copy_fn copy_fn);

//-----------------------------------------------------------------------------
// Ape object array
//-----------------------------------------------------------------------------

int arcane_object_get_array_length(arcane_object_t obj);

arcane_object_t arcane_object_get_array_value(arcane_object_t object, int ix);
const char *arcane_object_get_array_string(arcane_object_t object, int ix);
double       arcane_object_get_array_number(arcane_object_t object, int ix);
bool         arcane_object_get_array_bool(arcane_object_t object, int ix);

bool arcane_object_set_array_value(arcane_object_t object, int ix, arcane_object_t value);
bool arcane_object_set_array_string(arcane_object_t object, int ix, const char *string);
bool arcane_object_set_array_number(arcane_object_t object, int ix, double number);
bool arcane_object_set_array_bool(arcane_object_t object, int ix, bool value);

bool arcane_object_add_array_value(arcane_object_t object, arcane_object_t value);
bool arcane_object_add_array_string(arcane_object_t object, const char *string);
bool arcane_object_add_array_number(arcane_object_t object, double number);
bool arcane_object_add_array_bool(arcane_object_t object, bool value);

//-----------------------------------------------------------------------------
// Ape object map
//-----------------------------------------------------------------------------

int          arcane_object_get_map_length(arcane_object_t obj);
arcane_object_t arcane_object_get_map_key_at(arcane_object_t object, int ix);
arcane_object_t arcane_object_get_map_value_at(arcane_object_t object, int ix);
bool         arcane_object_set_map_value_at(arcane_object_t object, int ix, arcane_object_t val);

bool arcane_object_set_map_value_with_value_key(arcane_object_t object, arcane_object_t key, arcane_object_t value);
bool arcane_object_set_map_value(arcane_object_t object, const char *key, arcane_object_t value);
bool arcane_object_set_map_string(arcane_object_t object, const char *key, const char *string);
bool arcane_object_set_map_number(arcane_object_t object, const char *key, double number);
bool arcane_object_set_map_bool(arcane_object_t object, const char *key, bool value);

arcane_object_t  arcane_object_get_map_value_with_value_key(arcane_object_t object, arcane_object_t key);
arcane_object_t  arcane_object_get_map_value(arcane_object_t object, const char *key);
const char *arcane_object_get_map_string(arcane_object_t object, const char *key);
double        arcane_object_get_map_number(arcane_object_t object, const char *key);
bool          arcane_object_get_map_bool(arcane_object_t object, const char *key);

bool arcane_object_map_has_key(arcane_object_t object, const char *key);

//-----------------------------------------------------------------------------
// Ape error
//-----------------------------------------------------------------------------
const char *arcane_error_get_message(const arcane_error_t *error);
const char *arcane_error_get_filepath(const arcane_error_t *error);
const char *arcane_error_get_line(const arcane_error_t *error);
int              arcane_error_get_line_number(const arcane_error_t *error);
int              arcane_error_get_column_number(const arcane_error_t *error);
arcane_error_type_t arcane_error_get_type(const arcane_error_t *error);
const char *arcane_error_get_type_string(const arcane_error_t *error);
const char *arcane_error_type_to_string(arcane_error_type_t type);
char *arcane_error_serialize(arcane_engine_t *arcane, const arcane_error_t *error);
const arcane_traceback_t *arcane_error_get_traceback(const arcane_error_t *error);

//-----------------------------------------------------------------------------
// Ape traceback
//-----------------------------------------------------------------------------
int         arcane_traceback_get_depth(const arcane_traceback_t *traceback);
const char *arcane_traceback_get_filepath(const arcane_traceback_t *traceback, int depth);
const char *arcane_traceback_get_line(const arcane_traceback_t *traceback, int depth);
int         arcane_traceback_get_line_number(const arcane_traceback_t *traceback, int depth);
int         arcane_traceback_get_column_number(const arcane_traceback_t *traceback, int depth);
const char *arcane_traceback_get_function_name(const arcane_traceback_t *traceback, int depth);

#ifdef __cplusplus
}
#endif

#endif /* arcane_h */