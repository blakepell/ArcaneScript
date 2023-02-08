#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#include "arcane.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define ARCANE_IMPL_VERSION_MAJOR 0
#define ARCANE_IMPL_VERSION_MINOR 15
#define ARCANE_IMPL_VERSION_PATCH 0

#if (ARCANE_VERSION_MAJOR != ARCANE_IMPL_VERSION_MAJOR)\
 || (ARCANE_VERSION_MINOR != ARCANE_IMPL_VERSION_MINOR)\
 || (ARCANE_VERSION_PATCH != ARCANE_IMPL_VERSION_PATCH)
#error "Version mismatch"
#endif

#ifndef ARCANE_AMALGAMATED
#include "arcane.h"
#include "gc.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include "errors.h"
#include "symbol_table.h"
#include "traceback.h"
#include "global_store.h"
#endif

typedef struct native_fn_wrapper
{
	arcane_native_fn fn;
	arcane_engine_t* arcane;
	void* data;
} native_fn_wrapper_t;

typedef struct arcane_program
{
	arcane_engine_t* arcane;
	compilation_result_t* comp_res;
} arcane_program_t;

typedef struct arcane_engine
{
	allocator_t alloc;
	gcmem_t* mem;
	ptrarray(compiled_file_t)* files;
	global_store_t* global_store;
	compiler_t* compiler;
	vm_t* vm;
	errors_t errors;
	arcane_config_t config;

	allocator_t custom_allocator;
} arcane_engine_t;

static void arcane_deinit(arcane_engine_t* arcane);
static object_t arcane_native_fn_wrapper(vm_t* vm, void* data, int argc, object_t* args);
static object_t arcane_object_to_object(arcane_object_t obj);
static arcane_object_t object_to_arcane_object(object_t obj);
static arcane_object_t arcane_object_make_native_function_with_name(arcane_engine_t* arcane, const char* name,
                                                                    arcane_native_fn fn, void* data);

static void reset_state(arcane_engine_t* arcane);
static void set_default_config(arcane_engine_t* arcane);
static char* read_file_default(void* ctx, const char* filename);
static size_t write_file_default(void* context, const char* path, const char* string, size_t string_size);
static size_t stdout_write_default(void* context, const void* data, size_t size);

static void* arcane_malloc(void* ctx, size_t size);
static void arcane_free(void* ctx, void* ptr);

//-----------------------------------------------------------------------------
// Ape
//-----------------------------------------------------------------------------
arcane_engine_t* arcane_make(void)
{
	return arcane_make_ex(NULL, NULL, NULL);
}

arcane_engine_t* arcane_make_ex(arcane_malloc_fn malloc_fn, arcane_free_fn free_fn, void* ctx)
{
	allocator_t custom_alloc = allocator_make(malloc_fn, free_fn, ctx);

	arcane_engine_t* arcane = allocator_malloc(&custom_alloc, sizeof(arcane_engine_t));
	if (!arcane)
	{
		return NULL;
	}

	memset(arcane, 0, sizeof(arcane_engine_t));
	arcane->alloc = allocator_make(arcane_malloc, arcane_free, arcane);
	arcane->custom_allocator = custom_alloc;

	set_default_config(arcane);

	errors_init(&arcane->errors);

	arcane->mem = gcmem_make(&arcane->alloc);
	if (!arcane->mem)
	{
		goto err;
	}

	arcane->files = ptrarray_make(&arcane->alloc);
	if (!arcane->files)
	{
		goto err;
	}

	arcane->global_store = global_store_make(&arcane->alloc, arcane->mem);
	if (!arcane->global_store)
	{
		goto err;
	}

	arcane->compiler = compiler_make(&arcane->alloc, &arcane->config, arcane->mem, &arcane->errors, arcane->files,
	                                 arcane->global_store);
	if (!arcane->compiler)
	{
		goto err;
	}

	arcane->vm = vm_make(&arcane->alloc, &arcane->config, arcane->mem, &arcane->errors, arcane->global_store);
	if (!arcane->vm)
	{
		goto err;
	}

	return arcane;
err:
	arcane_deinit(arcane);
	allocator_free(&custom_alloc, arcane);
	return NULL;
}

void arcane_destroy(arcane_engine_t* arcane)
{
	if (!arcane)
	{
		return;
	}
	arcane_deinit(arcane);
	allocator_t alloc = arcane->alloc;
	allocator_free(&alloc, arcane);
}

void arcane_free_allocated(arcane_engine_t* arcane, void* ptr)
{
	allocator_free(&arcane->alloc, ptr);
}

void arcane_set_repl_mode(arcane_engine_t* arcane, bool enabled)
{
	arcane->config.repl_mode = enabled;
}

bool arcane_set_timeout(arcane_engine_t* arcane, double max_execution_time_ms)
{
	if (!arcane_timer_platform_supported())
	{
		arcane->config.max_execution_time_ms = 0;
		arcane->config.max_execution_time_set = false;
		return false;
	}

	if (max_execution_time_ms >= 0)
	{
		arcane->config.max_execution_time_ms = max_execution_time_ms;
		arcane->config.max_execution_time_set = true;
	}
	else
	{
		arcane->config.max_execution_time_ms = 0;
		arcane->config.max_execution_time_set = false;
	}
	return true;
}

void arcane_set_stdout_write_function(arcane_engine_t* arcane, arcane_stdout_write_fn stdout_write, void* context)
{
	arcane->config.stdio.write.write = stdout_write;
	arcane->config.stdio.write.context = context;
}

void arcane_set_file_write_function(arcane_engine_t* arcane, arcane_write_file_fn file_write, void* context)
{
	arcane->config.fileio.write_file.write_file = file_write;
	arcane->config.fileio.write_file.context = context;
}

void arcane_set_file_read_function(arcane_engine_t* arcane, arcane_read_file_fn file_read, void* context)
{
	arcane->config.fileio.read_file.read_file = file_read;
	arcane->config.fileio.read_file.context = context;
}

arcane_program_t* arcane_compile(arcane_engine_t* arcane, const char* code)
{
	arcane_clear_errors(arcane);

	compilation_result_t* comp_res = compiler_compile(arcane->compiler, code);

	if (!comp_res || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	arcane_program_t* program = allocator_malloc(&arcane->alloc, sizeof(arcane_program_t));
	if (!program)
	{
		goto err;
	}
	program->arcane = arcane;
	program->comp_res = comp_res;
	return program;

err:
	compilation_result_destroy(comp_res);
	return NULL;
}

arcane_program_t* arcane_compile_file(arcane_engine_t* arcane, const char* path)
{
	arcane_clear_errors(arcane);

	compilation_result_t* comp_res = compiler_compile_file(arcane->compiler, path);

	if (!comp_res || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	arcane_program_t* program = allocator_malloc(&arcane->alloc, sizeof(arcane_program_t));
	if (!program)
	{
		goto err;
	}

	program->arcane = arcane;
	program->comp_res = comp_res;
	return program;

err:
	compilation_result_destroy(comp_res);
	return NULL;
}

arcane_object_t arcane_execute_program(arcane_engine_t* arcane, const arcane_program_t* program)
{
	if (program == NULL)
	{
		errors_add_error(&arcane->errors, ERROR_USER, src_pos_invalid,
			"program passed to execute was null.");
		return arcane_object_make_null();
	}

	reset_state(arcane);

	if (arcane != program->arcane)
	{
		errors_add_error(&arcane->errors, ERROR_USER, src_pos_invalid,
		                 "arcane program was compiled with a different arcane instance");
		return arcane_object_make_null();
	}

	bool ok = vm_run(arcane->vm, program->comp_res, compiler_get_constants(arcane->compiler));
	if (!ok || errors_get_count(&arcane->errors) > 0)
	{
		return arcane_object_make_null();
	}

	ARCANE_ASSERT(arcane->vm->sp == 0);

	object_t res = vm_get_last_popped(arcane->vm);
	if (object_get_type(res) == OBJECT_NONE)
	{
		return arcane_object_make_null();
	}

	return object_to_arcane_object(res);
}

void arcane_program_destroy(arcane_program_t* program)
{
	if (!program)
	{
		return;
	}
	compilation_result_destroy(program->comp_res);
	allocator_free(&program->arcane->alloc, program);
}

arcane_object_t arcane_execute(arcane_engine_t* arcane, const char* code)
{
	reset_state(arcane);

	compilation_result_t* comp_res = compiler_compile(arcane->compiler, code);

	if (!comp_res || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	bool ok = vm_run(arcane->vm, comp_res, compiler_get_constants(arcane->compiler));
	if (!ok || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	ARCANE_ASSERT(arcane->vm->sp == 0);

	object_t res = vm_get_last_popped(arcane->vm);
	if (object_get_type(res) == OBJECT_NONE)
	{
		goto err;
	}

	compilation_result_destroy(comp_res);

	return object_to_arcane_object(res);

err:
	compilation_result_destroy(comp_res);
	return arcane_object_make_null();
}

arcane_object_t arcane_execute_file(arcane_engine_t* arcane, const char* path)
{
	reset_state(arcane);

	compilation_result_t* comp_res = compiler_compile_file(arcane->compiler, path);

	if (!comp_res || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	bool ok = vm_run(arcane->vm, comp_res, compiler_get_constants(arcane->compiler));
	if (!ok || errors_get_count(&arcane->errors) > 0)
	{
		goto err;
	}

	ARCANE_ASSERT(arcane->vm->sp == 0);

	object_t res = vm_get_last_popped(arcane->vm);
	if (object_get_type(res) == OBJECT_NONE)
	{
		goto err;
	}

	compilation_result_destroy(comp_res);

	return object_to_arcane_object(res);

err:
	compilation_result_destroy(comp_res);
	return arcane_object_make_null();
}

arcane_object_t arcane_call(arcane_engine_t* arcane, const char* function_name, int argc, arcane_object_t* args)
{
	reset_state(arcane);

	object_t callee = arcane_object_to_object(arcane_get_object(arcane, function_name));
	if (object_get_type(callee) == OBJECT_NULL)
	{
		return arcane_object_make_null();
	}
	object_t res = vm_call(arcane->vm, compiler_get_constants(arcane->compiler), callee, argc, (object_t*)args);
	if (errors_get_count(&arcane->errors) > 0)
	{
		return arcane_object_make_null();
	}
	return object_to_arcane_object(res);
}

bool arcane_has_errors(const arcane_engine_t* arcane)
{
	return arcane_errors_count(arcane) > 0;
}

int arcane_errors_count(const arcane_engine_t* arcane)
{
	return errors_get_count(&arcane->errors);
}

void arcane_clear_errors(arcane_engine_t* arcane)
{
	errors_clear(&arcane->errors);
}

const arcane_error_t* arcane_get_error(const arcane_engine_t* arcane, int index)
{
	return (const arcane_error_t*)errors_getc(&arcane->errors, index);
}

bool arcane_set_native_function(arcane_engine_t* arcane, const char* name, arcane_native_fn fn, void* data)
{
	arcane_object_t obj = arcane_object_make_native_function_with_name(arcane, name, fn, data);
	if (arcane_object_is_null(obj))
	{
		return false;
	}
	return arcane_set_global_constant(arcane, name, obj);
}

bool arcane_set_global_constant(arcane_engine_t* arcane, const char* name, arcane_object_t obj)
{
	return global_store_set(arcane->global_store, name, arcane_object_to_object(obj));
}

arcane_object_t arcane_get_object(arcane_engine_t* arcane, const char* name)
{
	symbol_table_t* st = compiler_get_symbol_table(arcane->compiler);
	const symbol_t* symbol = symbol_table_resolve(st, name);
	if (!symbol)
	{
		errors_add_errorf(&arcane->errors, ERROR_USER, src_pos_invalid, "Symbol \"%s\" is not defined", name);
		return arcane_object_make_null();
	}
	object_t res = object_make_null();
	if (symbol->type == SYMBOL_MODULE_GLOBAL)
	{
		res = vm_get_global(arcane->vm, symbol->index);
	}
	else if (symbol->type == SYMBOL_ARCANE_GLOBAL)
	{
		bool ok = false;
		res = global_store_get_object_at(arcane->global_store, symbol->index, &ok);
		if (!ok)
		{
			errors_add_errorf(&arcane->errors, ERROR_USER, src_pos_invalid, "Failed to get global object at %d",
			                  symbol->index);
			return arcane_object_make_null();
		}
	}
	else
	{
		errors_add_errorf(&arcane->errors, ERROR_USER, src_pos_invalid,
		                  "Value associated with symbol \"%s\" could not be loaded", name);
		return arcane_object_make_null();
	}
	return object_to_arcane_object(res);
}

bool arcane_check_args(arcane_engine_t* arcane, bool generate_error, int argc, arcane_object_t* args, int expected_argc,
                       int* expected_types)
{
	if (argc != expected_argc)
	{
		if (generate_error)
		{
			arcane_set_runtime_errorf(arcane, "Invalid number or arguments, got %d instead of %d", argc, expected_argc);
		}
		return false;
	}

	for (int i = 0; i < argc; i++)
	{
		arcane_object_t arg = args[i];
		arcane_object_type_t type = arcane_object_get_type(arg);
		arcane_object_type_t expected_type = expected_types[i];
		if (!(type & expected_type))
		{
			if (generate_error)
			{
				const char* type_str = arcane_object_get_type_name(type);
				const char* expected_type_str = arcane_object_get_type_name(expected_type);
				arcane_set_runtime_errorf(arcane, "Invalid argument type, got %s, expected %s", type_str,
				                          expected_type_str);
			}
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Ape object
//-----------------------------------------------------------------------------

arcane_object_t arcane_object_make_number(double val)
{
	return object_to_arcane_object(object_make_number(val));
}

arcane_object_t arcane_object_make_bool(bool val)
{
	return object_to_arcane_object(object_make_bool(val));
}

arcane_object_t arcane_object_make_string(arcane_engine_t* arcane, const char* str)
{
	return object_to_arcane_object(object_make_string(arcane->mem, str));
}

arcane_object_t arcane_object_make_stringf(arcane_engine_t* arcane, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int to_write = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	va_start(args, fmt);
	object_t res = object_make_string_with_capacity(arcane->mem, to_write);
	if (object_is_null(res))
	{
		return arcane_object_make_null();
	}
	char* res_buf = object_get_mutable_string(res);
	int written = vsprintf(res_buf, fmt, args);
	(void)written;
	ARCANE_ASSERT(written == to_write);
	va_end(args);
	object_set_string_length(res, to_write);
	return object_to_arcane_object(res);
}

arcane_object_t arcane_object_make_null()
{
	return object_to_arcane_object(object_make_null());
}

arcane_object_t arcane_object_make_array(arcane_engine_t* arcane)
{
	return object_to_arcane_object(object_make_array(arcane->mem));
}

arcane_object_t arcane_object_make_map(arcane_engine_t* arcane)
{
	return object_to_arcane_object(object_make_map(arcane->mem));
}

arcane_object_t arcane_object_make_native_function(arcane_engine_t* arcane, arcane_native_fn fn, void* data)
{
	return arcane_object_make_native_function_with_name(arcane, "", fn, data);
}

arcane_object_t arcane_object_make_error(arcane_engine_t* arcane, const char* msg)
{
	return object_to_arcane_object(object_make_error(arcane->mem, msg));
}

arcane_object_t arcane_object_make_errorf(arcane_engine_t* arcane, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int to_write = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	va_start(args, fmt);
	char* res = allocator_malloc(&arcane->alloc, to_write + 1);
	if (!res)
	{
		return arcane_object_make_null();
	}
	int written = vsprintf(res, fmt, args);
	(void)written;
	ARCANE_ASSERT(written == to_write);
	va_end(args);
	return object_to_arcane_object(object_make_error_no_copy(arcane->mem, res));
}

arcane_object_t arcane_object_make_external(arcane_engine_t* arcane, void* data)
{
	object_t res = object_make_external(arcane->mem, data);
	return object_to_arcane_object(res);
}

char* arcane_object_serialize(arcane_engine_t* arcane, arcane_object_t obj)
{
	return object_serialize(&arcane->alloc, arcane_object_to_object(obj));
}

bool arcane_object_disable_gc(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	return gc_disable_on_object(obj);
}

void arcane_object_enable_gc(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	gc_enable_on_object(obj);
}

bool arcane_object_equals(arcane_object_t arcane_a, arcane_object_t arcane_b)
{
	object_t a = arcane_object_to_object(arcane_a);
	object_t b = arcane_object_to_object(arcane_b);
	return object_equals(a, b);
}

bool arcane_object_is_null(arcane_object_t obj)
{
	return arcane_object_get_type(obj) == ARCANE_OBJECT_NULL;
}

arcane_object_t arcane_object_copy(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	gcmem_t* mem = object_get_mem(obj);
	object_t res = object_copy(mem, obj);
	return object_to_arcane_object(res);
}

arcane_object_t arcane_object_deep_copy(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	gcmem_t* mem = object_get_mem(obj);
	object_t res = object_deep_copy(mem, obj);
	return object_to_arcane_object(res);
}

void arcane_set_runtime_error(arcane_engine_t* arcane, const char* message)
{
	errors_add_error(&arcane->errors, ERROR_RUNTIME, src_pos_invalid, message);
}

void arcane_set_runtime_errorf(arcane_engine_t* arcane, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int to_write = vsnprintf(NULL, 0, fmt, args);
	(void)to_write;
	va_end(args);
	va_start(args, fmt);
	char res[ERROR_MESSAGE_MAX_LENGTH];
	int written = vsnprintf(res, ERROR_MESSAGE_MAX_LENGTH, fmt, args);
	(void)written;
	ARCANE_ASSERT(to_write == written);
	va_end(args);
	errors_add_error(&arcane->errors, ERROR_RUNTIME, src_pos_invalid, res);
}

arcane_object_type_t arcane_object_get_type(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	switch (object_get_type(obj))
	{
		case OBJECT_NONE:
			return ARCANE_OBJECT_NONE;
		case OBJECT_ERROR:
			return ARCANE_OBJECT_ERROR;
		case OBJECT_NUMBER:
			return ARCANE_OBJECT_NUMBER;
		case OBJECT_BOOL:
			return ARCANE_OBJECT_BOOL;
		case OBJECT_STRING:
			return ARCANE_OBJECT_STRING;
		case OBJECT_NULL:
			return ARCANE_OBJECT_NULL;
		case OBJECT_NATIVE_FUNCTION:
			return ARCANE_OBJECT_NATIVE_FUNCTION;
		case OBJECT_ARRAY:
			return ARCANE_OBJECT_ARRAY;
		case OBJECT_MAP:
			return ARCANE_OBJECT_MAP;
		case OBJECT_FUNCTION:
			return ARCANE_OBJECT_FUNCTION;
		case OBJECT_EXTERNAL:
			return ARCANE_OBJECT_EXTERNAL;
		case OBJECT_FREED:
			return ARCANE_OBJECT_FREED;
		case OBJECT_ANY:
			return ARCANE_OBJECT_ANY;
		default:
			return ARCANE_OBJECT_NONE;
	}
}

const char* arcane_object_get_type_string(arcane_object_t obj)
{
	return arcane_object_get_type_name(arcane_object_get_type(obj));
}

const char* arcane_object_get_type_name(arcane_object_type_t type)
{
	switch (type)
	{
		case ARCANE_OBJECT_NONE:
			return "NONE";
		case ARCANE_OBJECT_ERROR:
			return "ERROR";
		case ARCANE_OBJECT_NUMBER:
			return "NUMBER";
		case ARCANE_OBJECT_BOOL:
			return "BOOL";
		case ARCANE_OBJECT_STRING:
			return "STRING";
		case ARCANE_OBJECT_NULL:
			return "NULL";
		case ARCANE_OBJECT_NATIVE_FUNCTION:
			return "NATIVE_FUNCTION";
		case ARCANE_OBJECT_ARRAY:
			return "ARRAY";
		case ARCANE_OBJECT_MAP:
			return "MAP";
		case ARCANE_OBJECT_FUNCTION:
			return "FUNCTION";
		case ARCANE_OBJECT_EXTERNAL:
			return "EXTERNAL";
		case ARCANE_OBJECT_FREED:
			return "FREED";
		case ARCANE_OBJECT_ANY:
			return "ANY";
		default:
			return "NONE";
	}
}

double arcane_object_get_number(arcane_object_t obj)
{
	return object_get_number(arcane_object_to_object(obj));
}

bool arcane_object_get_bool(arcane_object_t obj)
{
	return object_get_bool(arcane_object_to_object(obj));
}

const char* arcane_object_get_string(arcane_object_t obj)
{
	return object_get_string(arcane_object_to_object(obj));
}

const char* arcane_object_get_error_message(arcane_object_t obj)
{
	return object_get_error_message(arcane_object_to_object(obj));
}

const arcane_traceback_t* arcane_object_get_error_traceback(arcane_object_t arcane_obj)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	return (const arcane_traceback_t*)object_get_error_traceback(obj);
}

bool arcane_object_set_external_destroy_function(arcane_object_t object, arcane_data_destroy_fn destroy_fn)
{
	return object_set_external_destroy_function(arcane_object_to_object(object), destroy_fn);
}

bool arcane_object_set_external_copy_function(arcane_object_t object, arcane_data_copy_fn copy_fn)
{
	return object_set_external_copy_function(arcane_object_to_object(object), copy_fn);
}

//-----------------------------------------------------------------------------
// Ape object array
//-----------------------------------------------------------------------------

int arcane_object_get_array_length(arcane_object_t obj)
{
	return object_get_array_length(arcane_object_to_object(obj));
}

arcane_object_t arcane_object_get_array_value(arcane_object_t obj, int ix)
{
	object_t res = object_get_array_value_at(arcane_object_to_object(obj), ix);
	return object_to_arcane_object(res);
}

const char* arcane_object_get_array_string(arcane_object_t obj, int ix)
{
	arcane_object_t object = arcane_object_get_array_value(obj, ix);
	if (arcane_object_get_type(object) != ARCANE_OBJECT_STRING)
	{
		return NULL;
	}
	return arcane_object_get_string(object);
}

double arcane_object_get_array_number(arcane_object_t obj, int ix)
{
	arcane_object_t object = arcane_object_get_array_value(obj, ix);
	if (arcane_object_get_type(object) != ARCANE_OBJECT_NUMBER)
	{
		return 0;
	}
	return arcane_object_get_number(object);
}

bool arcane_object_get_array_bool(arcane_object_t obj, int ix)
{
	arcane_object_t object = arcane_object_get_array_value(obj, ix);
	if (arcane_object_get_type(object) != ARCANE_OBJECT_BOOL)
	{
		return 0;
	}
	return arcane_object_get_bool(object);
}

bool arcane_object_set_array_value(arcane_object_t arcane_obj, int ix, arcane_object_t arcane_value)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	object_t value = arcane_object_to_object(arcane_value);
	return object_set_array_value_at(obj, ix, value);
}

bool arcane_object_set_array_string(arcane_object_t obj, int ix, const char* string)
{
	gcmem_t* mem = object_get_mem(arcane_object_to_object(obj));
	if (!mem)
	{
		return false;
	}
	object_t new_value = object_make_string(mem, string);
	return arcane_object_set_array_value(obj, ix, object_to_arcane_object(new_value));
}

bool arcane_object_set_array_number(arcane_object_t obj, int ix, double number)
{
	object_t new_value = object_make_number(number);
	return arcane_object_set_array_value(obj, ix, object_to_arcane_object(new_value));
}

bool arcane_object_set_array_bool(arcane_object_t obj, int ix, bool value)
{
	object_t new_value = object_make_bool(value);
	return arcane_object_set_array_value(obj, ix, object_to_arcane_object(new_value));
}

bool arcane_object_add_array_value(arcane_object_t arcane_obj, arcane_object_t arcane_value)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	object_t value = arcane_object_to_object(arcane_value);
	return object_add_array_value(obj, value);
}

bool arcane_object_add_array_string(arcane_object_t obj, const char* string)
{
	gcmem_t* mem = object_get_mem(arcane_object_to_object(obj));
	if (!mem)
	{
		return false;
	}
	object_t new_value = object_make_string(mem, string);
	return arcane_object_add_array_value(obj, object_to_arcane_object(new_value));
}

bool arcane_object_add_array_number(arcane_object_t obj, double number)
{
	object_t new_value = object_make_number(number);
	return arcane_object_add_array_value(obj, object_to_arcane_object(new_value));
}

bool arcane_object_add_array_bool(arcane_object_t obj, bool value)
{
	object_t new_value = object_make_bool(value);
	return arcane_object_add_array_value(obj, object_to_arcane_object(new_value));
}

//-----------------------------------------------------------------------------
// Ape object map
//-----------------------------------------------------------------------------

int arcane_object_get_map_length(arcane_object_t obj)
{
	return object_get_map_length(arcane_object_to_object(obj));
}

arcane_object_t arcane_object_get_map_key_at(arcane_object_t arcane_obj, int ix)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	return object_to_arcane_object(object_get_map_key_at(obj, ix));
}

arcane_object_t arcane_object_get_map_value_at(arcane_object_t arcane_obj, int ix)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	object_t res = object_get_map_value_at(obj, ix);
	return object_to_arcane_object(res);
}

bool arcane_object_set_map_value_at(arcane_object_t arcane_obj, int ix, arcane_object_t arcane_val)
{
	object_t obj = arcane_object_to_object(arcane_obj);
	object_t val = arcane_object_to_object(arcane_val);
	return object_set_map_value_at(obj, ix, val);
}

bool arcane_object_set_map_value_with_value_key(arcane_object_t obj, arcane_object_t key, arcane_object_t val)
{
	return object_set_map_value(arcane_object_to_object(obj), arcane_object_to_object(key),
	                            arcane_object_to_object(val));
}

bool arcane_object_set_map_value(arcane_object_t obj, const char* key, arcane_object_t value)
{
	gcmem_t* mem = object_get_mem(arcane_object_to_object(obj));
	if (!mem)
	{
		return false;
	}
	object_t key_object = object_make_string(mem, key);
	if (object_is_null(key_object))
	{
		return false;
	}
	return arcane_object_set_map_value_with_value_key(obj, object_to_arcane_object(key_object), value);
}

bool arcane_object_set_map_string(arcane_object_t obj, const char* key, const char* string)
{
	gcmem_t* mem = object_get_mem(arcane_object_to_object(obj));
	if (!mem)
	{
		return false;
	}
	object_t string_object = object_make_string(mem, string);
	if (object_is_null(string_object))
	{
		return false;
	}
	return arcane_object_set_map_value(obj, key, object_to_arcane_object(string_object));
}

bool arcane_object_set_map_number(arcane_object_t obj, const char* key, double number)
{
	object_t number_object = object_make_number(number);
	return arcane_object_set_map_value(obj, key, object_to_arcane_object(number_object));
}

bool arcane_object_set_map_bool(arcane_object_t obj, const char* key, bool value)
{
	object_t bool_object = object_make_bool(value);
	return arcane_object_set_map_value(obj, key, object_to_arcane_object(bool_object));
}

arcane_object_t arcane_object_get_map_value_with_value_key(arcane_object_t obj, arcane_object_t key)
{
	return object_to_arcane_object(object_get_map_value(arcane_object_to_object(obj), arcane_object_to_object(key)));
}

arcane_object_t arcane_object_get_map_value(arcane_object_t object, const char* key)
{
	gcmem_t* mem = object_get_mem(arcane_object_to_object(object));
	if (!mem)
	{
		return arcane_object_make_null();
	}
	object_t key_object = object_make_string(mem, key);
	if (object_is_null(key_object))
	{
		return arcane_object_make_null();
	}
	arcane_object_t res = arcane_object_get_map_value_with_value_key(object, object_to_arcane_object(key_object));
	return res;
}

const char* arcane_object_get_map_string(arcane_object_t object, const char* key)
{
	arcane_object_t res = arcane_object_get_map_value(object, key);
	return arcane_object_get_string(res);
}

double arcane_object_get_map_number(arcane_object_t object, const char* key)
{
	arcane_object_t res = arcane_object_get_map_value(object, key);
	return arcane_object_get_number(res);
}

bool arcane_object_get_map_bool(arcane_object_t object, const char* key)
{
	arcane_object_t res = arcane_object_get_map_value(object, key);
	return arcane_object_get_bool(res);
}

bool arcane_object_map_has_key(arcane_object_t arcane_object, const char* key)
{
	object_t object = arcane_object_to_object(arcane_object);
	gcmem_t* mem = object_get_mem(object);
	if (!mem)
	{
		return false;
	}
	object_t key_object = object_make_string(mem, key);
	if (object_is_null(key_object))
	{
		return false;
	}
	return object_map_has_key(object, key_object);
}

//-----------------------------------------------------------------------------
// Ape error
//-----------------------------------------------------------------------------

const char* arcane_error_get_message(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	return error->message;
}

const char* arcane_error_get_filepath(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	if (!error->pos.file)
	{
		return NULL;
	}
	return error->pos.file->path;
}

const char* arcane_error_get_line(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	if (!error->pos.file)
	{
		return NULL;
	}
	ptrarray(char *)* lines = error->pos.file->lines;
	if (error->pos.line >= ptrarray_count(lines))
	{
		return NULL;
	}
	const char* line = ptrarray_get(lines, error->pos.line);
	return line;
}

int arcane_error_get_line_number(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	if (error->pos.line < 0)
	{
		return -1;
	}
	return error->pos.line + 1;
}

int arcane_error_get_column_number(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	if (error->pos.column < 0)
	{
		return -1;
	}
	return error->pos.column + 1;
}

arcane_error_type_t arcane_error_get_type(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	switch (error->type)
	{
		case ERROR_NONE:
			return ARCANE_ERROR_NONE;
		case ERROR_PARSING:
			return ARCANE_ERROR_PARSING;
		case ERROR_COMPILATION:
			return ARCANE_ERROR_COMPILATION;
		case ERROR_RUNTIME:
			return ARCANE_ERROR_RUNTIME;
		case ERROR_TIME_OUT:
			return ARCANE_ERROR_TIMEOUT;
		case ERROR_ALLOCATION:
			return ARCANE_ERROR_ALLOCATION;
		case ERROR_USER:
			return ARCANE_ERROR_USER;
		default:
			return ARCANE_ERROR_NONE;
	}
}

const char* arcane_error_get_type_string(const arcane_error_t* error)
{
	return arcane_error_type_to_string(arcane_error_get_type(error));
}

const char* arcane_error_type_to_string(arcane_error_type_t type)
{
	switch (type)
	{
		case ARCANE_ERROR_PARSING:
			return "PARSING";
		case ARCANE_ERROR_COMPILATION:
			return "COMPILATION";
		case ARCANE_ERROR_RUNTIME:
			return "RUNTIME";
		case ARCANE_ERROR_TIMEOUT:
			return "TIMEOUT";
		case ARCANE_ERROR_ALLOCATION:
			return "ALLOCATION";
		case ARCANE_ERROR_USER:
			return "USER";
		default:
			return "NONE";
	}
}

char* arcane_error_serialize(arcane_engine_t* arcane, const arcane_error_t* err)
{
	const char* type_str = arcane_error_get_type_string(err);
	const char* filename = arcane_error_get_filepath(err);
	const char* line = arcane_error_get_line(err);
	int line_num = arcane_error_get_line_number(err);
	int col_num = arcane_error_get_column_number(err);
	strbuf_t* buf = strbuf_make(&arcane->alloc);
	if (!buf)
	{
		return NULL;
	}
	if (line)
	{
		strbuf_append(buf, line);
		strbuf_append(buf, "\n");
		if (col_num >= 0)
		{
			for (int j = 0; j < (col_num - 1); j++)
			{
				strbuf_append(buf, " ");
			}
			strbuf_append(buf, "^\n");
		}
	}
	strbuf_appendf(buf, "%s ERROR in \"%s\" on %d:%d: %s\n", type_str,
	               filename, line_num, col_num, arcane_error_get_message(err));
	const arcane_traceback_t* traceback = arcane_error_get_traceback(err);
	if (traceback)
	{
		strbuf_appendf(buf, "Traceback:\n");
		traceback_to_string((const traceback_t*)arcane_error_get_traceback(err), buf);
	}
	if (strbuf_failed(buf))
	{
		strbuf_destroy(buf);
		return NULL;
	}
	return strbuf_get_string_and_destroy(buf);
}

const arcane_traceback_t* arcane_error_get_traceback(const arcane_error_t* arcane_error)
{
	const error_t* error = (const error_t*)arcane_error;
	return (const arcane_traceback_t*)error->traceback;
}

//-----------------------------------------------------------------------------
// Ape traceback
//-----------------------------------------------------------------------------

int arcane_traceback_get_depth(const arcane_traceback_t* arcane_traceback)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	return array_count(traceback->items);
}

const char* arcane_traceback_get_filepath(const arcane_traceback_t* arcane_traceback, int depth)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	traceback_item_t* item = array_get(traceback->items, depth);
	if (!item)
	{
		return NULL;
	}
	return traceback_item_get_filepath(item);
}

const char* arcane_traceback_get_line(const arcane_traceback_t* arcane_traceback, int depth)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	traceback_item_t* item = array_get(traceback->items, depth);
	if (!item)
	{
		return NULL;
	}
	return traceback_item_get_line(item);
}

int arcane_traceback_get_line_number(const arcane_traceback_t* arcane_traceback, int depth)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	traceback_item_t* item = array_get(traceback->items, depth);
	if (!item)
	{
		return -1;
	}
	return item->pos.line;
}

int arcane_traceback_get_column_number(const arcane_traceback_t* arcane_traceback, int depth)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	traceback_item_t* item = array_get(traceback->items, depth);
	if (!item)
	{
		return -1;
	}
	return item->pos.column;
}

const char* arcane_traceback_get_function_name(const arcane_traceback_t* arcane_traceback, int depth)
{
	const traceback_t* traceback = (const traceback_t*)arcane_traceback;
	traceback_item_t* item = array_get(traceback->items, depth);
	if (!item)
	{
		return "";
	}
	return item->function_name;
}

//-----------------------------------------------------------------------------
// Ape internal
//-----------------------------------------------------------------------------
static void arcane_deinit(arcane_engine_t* arcane)
{
	vm_destroy(arcane->vm);
	compiler_destroy(arcane->compiler);
	global_store_destroy(arcane->global_store);
	gcmem_destroy(arcane->mem);
	ptrarray_destroy_with_items(arcane->files, compiled_file_destroy);
	errors_deinit(&arcane->errors);
}

static object_t arcane_native_fn_wrapper(vm_t* vm, void* data, int argc, object_t* args)
{
	(void)vm;
	native_fn_wrapper_t* wrapper = data;
	ARCANE_ASSERT(vm == wrapper->arcane->vm);
	arcane_object_t res = wrapper->fn(wrapper->arcane, wrapper->data, argc, (arcane_object_t*)args);
	if (arcane_has_errors(wrapper->arcane))
	{
		return object_make_null();
	}
	return arcane_object_to_object(res);
}

static object_t arcane_object_to_object(arcane_object_t obj)
{
	return (object_t){
		.handle = obj._internal
	};
}

static arcane_object_t object_to_arcane_object(object_t obj)
{
	return (arcane_object_t){
		._internal = obj.handle
	};
}

static arcane_object_t arcane_object_make_native_function_with_name(arcane_engine_t* arcane, const char* name,
                                                                    arcane_native_fn fn, void* data)
{
	native_fn_wrapper_t wrapper;
	memset(&wrapper, 0, sizeof(native_fn_wrapper_t));
	wrapper.fn = fn;
	wrapper.arcane = arcane;
	wrapper.data = data;
	object_t wrapper_native_function = object_make_native_function(arcane->mem, name, arcane_native_fn_wrapper,
	                                                               &wrapper, sizeof(wrapper));
	if (object_is_null(wrapper_native_function))
	{
		return arcane_object_make_null();
	}
	return object_to_arcane_object(wrapper_native_function);
}

static void reset_state(arcane_engine_t* arcane)
{
	arcane_clear_errors(arcane);
	vm_reset(arcane->vm);
}

static void set_default_config(arcane_engine_t* arcane)
{
	memset(&arcane->config, 0, sizeof(arcane_config_t));
	arcane_set_repl_mode(arcane, false);
	arcane_set_timeout(arcane, -1);
	arcane_set_file_read_function(arcane, read_file_default, arcane);
	arcane_set_file_write_function(arcane, write_file_default, arcane);
	arcane_set_stdout_write_function(arcane, stdout_write_default, arcane);
}

static char* read_file_default(void* ctx, const char* filename)
{
	arcane_engine_t* arcane = ctx;
	FILE* fp = fopen(filename, "r");
	size_t size_to_read = 0;
	size_t size_read = 0;
	long pos;
	char* file_contents;
	if (!fp)
	{
		return NULL;
	}
	fseek(fp, 0L, SEEK_END);
	pos = ftell(fp);
	if (pos < 0)
	{
		fclose(fp);
		return NULL;
	}
	size_to_read = pos;
	rewind(fp);
	file_contents = (char*)allocator_malloc(&arcane->alloc, sizeof(char) * (size_to_read + 1));
	if (!file_contents)
	{
		fclose(fp);
		return NULL;
	}
	size_read = fread(file_contents, 1, size_to_read, fp);
	if (ferror(fp))
	{
		fclose(fp);
		free(file_contents);
		return NULL;
	}
	fclose(fp);
	file_contents[size_read] = '\0';
	return file_contents;
}

static size_t write_file_default(void* ctx, const char* path, const char* string, size_t string_size)
{
	(void)ctx;
	FILE* fp = fopen(path, "w");
	if (!fp)
	{
		return 0;
	}
	size_t written = fwrite(string, 1, string_size, fp);
	fclose(fp);
	return written;
}

static size_t stdout_write_default(void* ctx, const void* data, size_t size)
{
	(void)ctx;
	return fwrite(data, 1, size, stdout);
}

static void* arcane_malloc(void* ctx, size_t size)
{
	arcane_engine_t* arcane = ctx;
	void* res = allocator_malloc(&arcane->custom_allocator, size);
	if (!res)
	{
		errors_add_error(&arcane->errors, ERROR_ALLOCATION, src_pos_invalid, "Allocation failed");
	}
	return res;
}

static void arcane_free(void* ctx, void* ptr)
{
	arcane_engine_t* arcane = ctx;
	allocator_free(&arcane->custom_allocator, ptr);
}
