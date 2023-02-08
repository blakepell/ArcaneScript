#ifndef object_h
#define object_h

#include <stdint.h>

#ifndef ARCANE_AMALGAMATED
#include "common.h"
#include "collections.h"
#include "ast.h"
#endif

typedef struct compilation_result compilation_result_t;
typedef struct traceback traceback_t;
typedef struct vm vm_t;
typedef struct gcmem gcmem_t;

#define OBJECT_STRING_BUF_SIZE 24

typedef enum
{
	OBJECT_NONE = 0,
	OBJECT_ERROR = 1 << 0,
	OBJECT_NUMBER = 1 << 1,
	OBJECT_BOOL = 1 << 2,
	OBJECT_STRING = 1 << 3,
	OBJECT_NULL = 1 << 4,
	OBJECT_NATIVE_FUNCTION = 1 << 5,
	OBJECT_ARRAY = 1 << 6,
	OBJECT_MAP = 1 << 7,
	OBJECT_FUNCTION = 1 << 8,
	OBJECT_EXTERNAL = 1 << 9,
	OBJECT_FREED = 1 << 10,
	OBJECT_ANY = 0xffff,
} object_type_t;

typedef struct object
{
	union
	{
		uint64_t handle;
		double number;
	};
} object_t;

typedef struct function
{
	union
	{
		object_t* free_vals_allocated;
		object_t free_vals_buf[2];
	};

	union
	{
		char* name;
		const char* const_name;
	};

	compilation_result_t* comp_result;
	int num_locals;
	int num_args;
	int free_vals_count;
	bool owns_data;
} function_t;

#define NATIVE_FN_MAX_DATA_LEN 24
typedef object_t (*native_fn)(vm_t* vm, void* data, int argc, object_t* args);

typedef struct native_function
{
	char* name;
	native_fn fn;
	uint8_t data[NATIVE_FN_MAX_DATA_LEN];
	int data_len;
} native_function_t;

typedef void (*external_data_destroy_fn)(void* data);
typedef void*(*external_data_copy_fn)(void* data);

typedef struct external_data
{
	void* data;
	external_data_destroy_fn data_destroy_fn;
	external_data_copy_fn data_copy_fn;
} external_data_t;

typedef struct object_error
{
	char* message;
	traceback_t* traceback;
} object_error_t;

typedef struct object_string
{
	union
	{
		char* value_allocated;
		char value_buf[OBJECT_STRING_BUF_SIZE];
	};

	unsigned long hash;
	bool is_allocated;
	int capacity;
	int length;
} object_string_t;

typedef struct object_data
{
	gcmem_t* mem;

	union
	{
		object_string_t string;
		object_error_t error;
		array(object_t)* array;
		valdict(object_t, object_t)* map;
		function_t function;
		native_function_t native_function;
		external_data_t external;
	};

	bool gcmark;
	object_type_t type;
} object_data_t;

ARCANE_INTERNAL object_t object_make_from_data(object_type_t type, object_data_t* data);
ARCANE_INTERNAL object_t object_make_number(double val);
ARCANE_INTERNAL object_t object_make_bool(bool val);
ARCANE_INTERNAL object_t object_make_null(void);
ARCANE_INTERNAL object_t object_make_string(gcmem_t* mem, const char* string);
ARCANE_INTERNAL object_t object_make_string_with_capacity(gcmem_t* mem, int capacity);
ARCANE_INTERNAL object_t object_make_native_function(gcmem_t* mem, const char* name, native_fn fn, void* data,
                                                     int data_len);
ARCANE_INTERNAL object_t object_make_array(gcmem_t* mem);
ARCANE_INTERNAL object_t object_make_array_with_capacity(gcmem_t* mem, unsigned capacity);
ARCANE_INTERNAL object_t object_make_map(gcmem_t* mem);
ARCANE_INTERNAL object_t object_make_map_with_capacity(gcmem_t* mem, unsigned capacity);
ARCANE_INTERNAL object_t object_make_error(gcmem_t* mem, const char* message);
ARCANE_INTERNAL object_t object_make_error_no_copy(gcmem_t* mem, char* message);
ARCANE_INTERNAL object_t object_make_errorf(gcmem_t* mem, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
ARCANE_INTERNAL object_t object_make_function(gcmem_t* mem, const char* name, compilation_result_t* comp_res,
                                              bool owns_data, int num_locals, int num_args,
                                              int free_vals_count);
ARCANE_INTERNAL object_t object_make_external(gcmem_t* mem, void* data);

ARCANE_INTERNAL void object_deinit(object_t obj);
ARCANE_INTERNAL void object_data_deinit(object_data_t* obj);

ARCANE_INTERNAL bool object_is_allocated(object_t obj);
ARCANE_INTERNAL gcmem_t* object_get_mem(object_t obj);
ARCANE_INTERNAL bool object_is_hashable(object_t obj);
ARCANE_INTERNAL void object_to_string(object_t obj, strbuf_t* buf, bool quote_str);
ARCANE_INTERNAL const char* object_get_type_name(const object_type_t type);
ARCANE_INTERNAL char* object_get_type_union_name(allocator_t* alloc, const object_type_t type);
ARCANE_INTERNAL char* object_serialize(allocator_t* alloc, object_t object);
ARCANE_INTERNAL object_t object_deep_copy(gcmem_t* mem, object_t object);
ARCANE_INTERNAL object_t object_copy(gcmem_t* mem, object_t obj);
ARCANE_INTERNAL double object_compare(object_t a, object_t b, bool* out_ok);
ARCANE_INTERNAL bool object_equals(object_t a, object_t b);

ARCANE_INTERNAL object_data_t* object_get_allocated_data(object_t object);

ARCANE_INTERNAL bool object_get_bool(object_t obj);
ARCANE_INTERNAL double object_get_number(object_t obj);
ARCANE_INTERNAL function_t* object_get_function(object_t obj);
ARCANE_INTERNAL const char* object_get_string(object_t obj);
ARCANE_INTERNAL int object_get_string_length(object_t obj);
ARCANE_INTERNAL void object_set_string_length(object_t obj, int len);
ARCANE_INTERNAL int object_get_string_capacity(object_t obj);
ARCANE_INTERNAL char* object_get_mutable_string(object_t obj);
ARCANE_INTERNAL bool object_string_append(object_t obj, const char* src, int len);
ARCANE_INTERNAL unsigned long object_get_string_hash(object_t obj);
ARCANE_INTERNAL native_function_t* object_get_native_function(object_t obj);
ARCANE_INTERNAL object_type_t object_get_type(object_t obj);

ARCANE_INTERNAL bool object_is_numeric(object_t obj);
ARCANE_INTERNAL bool object_is_null(object_t obj);
ARCANE_INTERNAL bool object_is_callable(object_t obj);

ARCANE_INTERNAL const char* object_get_function_name(object_t obj);
ARCANE_INTERNAL object_t object_get_function_free_val(object_t obj, int ix);
ARCANE_INTERNAL void object_set_function_free_val(object_t obj, int ix, object_t val);
ARCANE_INTERNAL object_t* object_get_function_free_vals(object_t obj);

ARCANE_INTERNAL const char* object_get_error_message(object_t obj);
ARCANE_INTERNAL void object_set_error_traceback(object_t obj, traceback_t* traceback);
ARCANE_INTERNAL traceback_t* object_get_error_traceback(object_t obj);

ARCANE_INTERNAL external_data_t* object_get_external_data(object_t object);
ARCANE_INTERNAL bool object_set_external_destroy_function(object_t object, external_data_destroy_fn destroy_fn);
ARCANE_INTERNAL bool object_set_external_data(object_t object, void* data);
ARCANE_INTERNAL bool object_set_external_copy_function(object_t object, external_data_copy_fn copy_fn);

ARCANE_INTERNAL object_t object_get_array_value_at(object_t array, int ix);
ARCANE_INTERNAL bool object_set_array_value_at(object_t obj, int ix, object_t val);
ARCANE_INTERNAL bool object_add_array_value(object_t array, object_t val);
ARCANE_INTERNAL int object_get_array_length(object_t array);
ARCANE_INTERNAL bool object_remove_array_value_at(object_t array, int ix);

ARCANE_INTERNAL int object_get_map_length(object_t obj);
ARCANE_INTERNAL object_t object_get_map_key_at(object_t obj, int ix);
ARCANE_INTERNAL object_t object_get_map_value_at(object_t obj, int ix);
ARCANE_INTERNAL bool object_set_map_value_at(object_t obj, int ix, object_t val);
ARCANE_INTERNAL object_t object_get_kv_pair_at(gcmem_t* mem, object_t obj, int ix);
ARCANE_INTERNAL bool object_set_map_value(object_t obj, object_t key, object_t val);
ARCANE_INTERNAL object_t object_get_map_value(object_t obj, object_t key);
ARCANE_INTERNAL bool object_map_has_key(object_t obj, object_t key);

#endif /* object_h */
