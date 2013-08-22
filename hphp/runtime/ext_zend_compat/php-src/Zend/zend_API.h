/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Andrei Zmievski <andrei@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_API_H
#define ZEND_API_H

#include "zend.h"
#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_modules.h"
#include "zend_operators.h"
#include "zend_variables.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/ext/ext_function.h"

BEGIN_EXTERN_C()

typedef struct _zend_function_entry {
  const char *fname;
  void (*handler)(INTERNAL_FUNCTION_PARAMETERS);
  const struct _zend_arg_info *arg_info;
  zend_uint num_args;
  zend_uint flags;
} zend_function_entry;

typedef struct _zend_fcall_info {
  size_t size;
  HashTable *function_table;
  zval *function_name;
  HashTable *symbol_table;
  zval **retval_ptr_ptr;
  zend_uint param_count;
  zval ***params;
  zval *object_ptr;
  zend_bool no_separation;
} zend_fcall_info;

typedef struct _zend_fcall_info_cache {
  zend_bool initialized;
  zend_function *function_handler;
  zend_class_entry *calling_scope;
  zend_class_entry *called_scope;
  zval *object_ptr;
} zend_fcall_info_cache;

#define ZEND_NS_NAME(ns, name)      ns "\\" name

#define ZEND_FN(name) zif_##name
#define ZEND_MN(name) zim_##name
#define ZEND_NAMED_FUNCTION(name)    void name(INTERNAL_FUNCTION_PARAMETERS)
#define ZEND_FUNCTION(name)        ZEND_NAMED_FUNCTION(ZEND_FN(name))
#define ZEND_METHOD(classname, name)  ZEND_NAMED_FUNCTION(ZEND_MN(classname##_##name))

#define ZEND_FENTRY(zend_name, name, arg_info, flags)  { #zend_name, name, arg_info, (zend_uint) (sizeof(arg_info)/sizeof(struct _zend_arg_info)-1), flags },

#define ZEND_RAW_NAMED_FE(zend_name, name, arg_info) ZEND_RAW_FENTRY(#zend_name, name, arg_info, 0)

#define ZEND_NAMED_FE(zend_name, name, arg_info)  ZEND_FENTRY(zend_name, name, arg_info, 0)
#define ZEND_FE(name, arg_info)            ZEND_FENTRY(name, ZEND_FN(name), arg_info, 0)
#define ZEND_DEP_FE(name, arg_info)                 ZEND_FENTRY(name, ZEND_FN(name), arg_info, ZEND_ACC_DEPRECATED)
#define ZEND_FALIAS(name, alias, arg_info)      ZEND_FENTRY(name, ZEND_FN(alias), arg_info, 0)
#define ZEND_DEP_FALIAS(name, alias, arg_info)    ZEND_FENTRY(name, ZEND_FN(alias), arg_info, ZEND_ACC_DEPRECATED)
#define ZEND_NAMED_ME(zend_name, name, arg_info, flags)  ZEND_FENTRY(zend_name, name, arg_info, flags)
#define ZEND_ME(classname, name, arg_info, flags)  ZEND_FENTRY(name, ZEND_MN(classname##_##name), arg_info, flags)
#define ZEND_ABSTRACT_ME(classname, name, arg_info)  ZEND_FENTRY(name, NULL, arg_info, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
#define ZEND_MALIAS(classname, name, alias, arg_info, flags) \
                                                    ZEND_FENTRY(name, ZEND_MN(classname##_##alias), arg_info, flags)
#define ZEND_ME_MAPPING(name, func_name, arg_types, flags) ZEND_NAMED_ME(name, ZEND_FN(func_name), arg_types, flags)

#define ZEND_NS_FENTRY(ns, zend_name, name, arg_info, flags)    ZEND_RAW_FENTRY(ZEND_NS_NAME(ns, #zend_name), name, arg_info, flags)

#define ZEND_NS_RAW_FENTRY(ns, zend_name, name, arg_info, flags)  ZEND_RAW_FENTRY(ZEND_NS_NAME(ns, zend_name), name, arg_info, flags)
#define ZEND_NS_RAW_NAMED_FE(ns, zend_name, name, arg_info)      ZEND_NS_RAW_FENTRY(ns, #zend_name, name, arg_info, 0)

#define ZEND_NS_NAMED_FE(ns, zend_name, name, arg_info)  ZEND_NS_FENTRY(ns, zend_name, name, arg_info, 0)
#define ZEND_NS_FE(ns, name, arg_info)          ZEND_NS_FENTRY(ns, name, ZEND_FN(name), arg_info, 0)
#define ZEND_NS_DEP_FE(ns, name, arg_info)        ZEND_NS_FENTRY(ns, name, ZEND_FN(name), arg_info, ZEND_ACC_DEPRECATED)
#define ZEND_NS_FALIAS(ns, name, alias, arg_info)    ZEND_NS_FENTRY(ns, name, ZEND_FN(alias), arg_info, 0)
#define ZEND_NS_DEP_FALIAS(ns, name, alias, arg_info)  ZEND_NS_FENTRY(ns, name, ZEND_FN(alias), arg_info, ZEND_ACC_DEPRECATED)

#define ZEND_FE_END            { NULL, NULL, NULL, 0, 0 }

#define ZEND_ARG_INFO(pass_by_ref, name)              { #name, sizeof(#name)-1, NULL, 0, 0, 0, pass_by_ref},
#define ZEND_ARG_PASS_INFO(pass_by_ref)                { NULL, 0, NULL, 0, 0, 0, pass_by_ref},
#define ZEND_ARG_OBJ_INFO(pass_by_ref, name, classname, allow_null) { #name, sizeof(#name)-1, #classname, sizeof(#classname)-1, IS_OBJECT, allow_null, pass_by_ref},
#define ZEND_ARG_ARRAY_INFO(pass_by_ref, name, allow_null) { #name, sizeof(#name)-1, NULL, 0, IS_ARRAY, allow_null, pass_by_ref},
#define ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null) { #name, sizeof(#name)-1, NULL, 0, type_hint, allow_null, pass_by_ref},
#define ZEND_BEGIN_ARG_INFO_EX(name, pass_rest_by_reference, return_reference, required_num_args)  \
  static const zend_arg_info name[] = {                                    \
    { NULL, 0, NULL, required_num_args, 0, return_reference, pass_rest_by_reference},
#define ZEND_BEGIN_ARG_INFO(name, pass_rest_by_reference)  \
  ZEND_BEGIN_ARG_INFO_EX(name, pass_rest_by_reference, ZEND_RETURN_VALUE, -1u)
#define ZEND_END_ARG_INFO()    };

/* Name macros */
#define ZEND_MODULE_STARTUP_N(module)       zm_startup_##module
#define ZEND_MODULE_SHUTDOWN_N(module)    zm_shutdown_##module
#define ZEND_MODULE_ACTIVATE_N(module)    zm_activate_##module
#define ZEND_MODULE_DEACTIVATE_N(module)  zm_deactivate_##module
#define ZEND_MODULE_POST_ZEND_DEACTIVATE_N(module)  zm_post_zend_deactivate_##module
#define ZEND_MODULE_INFO_N(module)      zm_info_##module
#define ZEND_MODULE_GLOBALS_CTOR_N(module)  zm_globals_ctor_##module
#define ZEND_MODULE_GLOBALS_DTOR_N(module)  zm_globals_dtor_##module

/* Declaration macros */
#define ZEND_MODULE_STARTUP_D(module)    int ZEND_MODULE_STARTUP_N(module)(INIT_FUNC_ARGS)
#define ZEND_MODULE_SHUTDOWN_D(module)    int ZEND_MODULE_SHUTDOWN_N(module)(SHUTDOWN_FUNC_ARGS)
#define ZEND_MODULE_ACTIVATE_D(module)    int ZEND_MODULE_ACTIVATE_N(module)(INIT_FUNC_ARGS)
#define ZEND_MODULE_DEACTIVATE_D(module)  int ZEND_MODULE_DEACTIVATE_N(module)(SHUTDOWN_FUNC_ARGS)
#define ZEND_MODULE_POST_ZEND_DEACTIVATE_D(module)  int ZEND_MODULE_POST_ZEND_DEACTIVATE_N(module)(void)
#define ZEND_MODULE_INFO_D(module)      void ZEND_MODULE_INFO_N(module)(ZEND_MODULE_INFO_FUNC_ARGS)
#define ZEND_MODULE_GLOBALS_CTOR_D(module)  void ZEND_MODULE_GLOBALS_CTOR_N(module)(zend_##module##_globals *module##_globals TSRMLS_DC)
#define ZEND_MODULE_GLOBALS_DTOR_D(module)  void ZEND_MODULE_GLOBALS_DTOR_N(module)(zend_##module##_globals *module##_globals TSRMLS_DC)

#define ZEND_GET_MODULE(name) \
    BEGIN_EXTERN_C()\
  ZEND_DLEXPORT zend_module_entry *get_module(void) { return &name##_module_entry; }\
    END_EXTERN_C()

#define ZEND_BEGIN_MODULE_GLOBALS(module_name)    \
  typedef struct _zend_##module_name##_globals {
#define ZEND_END_MODULE_GLOBALS(module_name)    \
  } zend_##module_name##_globals;


#define ZEND_DECLARE_MODULE_GLOBALS(module_name)              \
  zend_##module_name##_globals module_name##_globals;
#define ZEND_EXTERN_MODULE_GLOBALS(module_name)                \
  extern zend_##module_name##_globals module_name##_globals;
#define ZEND_INIT_MODULE_GLOBALS(module_name, globals_ctor, globals_dtor)  \
  globals_ctor(&module_name##_globals);

BEGIN_EXTERN_C()

#define zend_parse_parameters_none()                    \
  zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")

/* Parameter parsing API -- andrei */

#define ZEND_PARSE_PARAMS_QUIET 1<<1
ZEND_API int zend_parse_parameters(int num_args TSRMLS_DC, const char *type_spec, ...);
ZEND_API int zend_parse_parameters_ex(int flags, int num_args TSRMLS_DC, const char *type_spec, ...);
ZEND_API const char *zend_zval_type_name(const zval *arg);
ZEND_API int zend_parse_method_parameters(int num_args TSRMLS_DC, zval *this_ptr, const char *type_spec, ...);
ZEND_API int zend_parse_method_parameters_ex(int flags, int num_args TSRMLS_DC, zval *this_ptr, const char *type_spec, ...);

ZEND_API int zend_parse_parameter(int flags, int arg_num TSRMLS_DC, zval **arg, const char *spec, ...);

/* End of parameter parsing API -- andrei */

#define IS_CALLABLE_CHECK_SYNTAX_ONLY (1<<0)
#define IS_CALLABLE_CHECK_NO_ACCESS   (1<<1)
#define IS_CALLABLE_CHECK_IS_STATIC   (1<<2)
#define IS_CALLABLE_CHECK_SILENT      (1<<3)

ZEND_API zend_bool zend_is_callable(zval *callable, uint check_flags, char **callable_name TSRMLS_DC);

ZEND_API zend_class_entry *zend_get_class_entry(const zval *zobject TSRMLS_DC);
ZEND_API int zend_get_object_classname(const zval *object, const char **class_name, zend_uint *class_name_len TSRMLS_DC);

#define ZEND_NUM_ARGS()    (ar->numArgs())

#define array_init(arg)      _array_init((arg), 0 ZEND_FILE_LINE_CC)
#define array_init_size(arg, size) _array_init((arg), (size) ZEND_FILE_LINE_CC)
ZEND_API int _array_init(zval *arg, uint size ZEND_FILE_LINE_DC);

ZEND_API int add_assoc_long_ex(zval *arg, const char *key, uint key_len, long n);
ZEND_API int add_assoc_null_ex(zval *arg, const char *key, uint key_len);
ZEND_API int add_assoc_bool_ex(zval *arg, const char *key, uint key_len, int b);
ZEND_API int add_assoc_resource_ex(zval *arg, const char *key, uint key_len, int r);
ZEND_API int add_assoc_double_ex(zval *arg, const char *key, uint key_len, double d);
ZEND_API int add_assoc_string_ex(zval *arg, const char *key, uint key_len, char *str, int duplicate);
ZEND_API int add_assoc_stringl_ex(zval *arg, const char *key, uint key_len, char *str, uint length, int duplicate);
ZEND_API int add_assoc_zval_ex(zval *arg, const char *key, uint key_len, zval *value);

#define add_assoc_long(__arg, __key, __n) add_assoc_long_ex(__arg, __key, strlen(__key)+1, __n)
#define add_assoc_null(__arg, __key) add_assoc_null_ex(__arg, __key, strlen(__key) + 1)
#define add_assoc_bool(__arg, __key, __b) add_assoc_bool_ex(__arg, __key, strlen(__key)+1, __b)
#define add_assoc_resource(__arg, __key, __r) add_assoc_resource_ex(__arg, __key, strlen(__key)+1, __r)
#define add_assoc_double(__arg, __key, __d) add_assoc_double_ex(__arg, __key, strlen(__key)+1, __d)
#define add_assoc_string(__arg, __key, __str, __duplicate) add_assoc_string_ex(__arg, __key, strlen(__key)+1, __str, __duplicate)
#define add_assoc_stringl(__arg, __key, __str, __length, __duplicate) add_assoc_stringl_ex(__arg, __key, strlen(__key)+1, __str, __length, __duplicate)
#define add_assoc_zval(__arg, __key, __value) add_assoc_zval_ex(__arg, __key, strlen(__key)+1, __value)

/* unset() functions are only suported for legacy modules and null() functions should be used */
#define add_assoc_unset(__arg, __key) add_assoc_null_ex(__arg, __key, strlen(__key) + 1)
#define add_index_unset(__arg, __key) add_index_null(__arg, __key)
#define add_next_index_unset(__arg) add_next_index_null(__arg)
#define add_property_unset(__arg, __key) add_property_null(__arg, __key)

ZEND_API int add_index_long(zval *arg, ulong idx, long n);
ZEND_API int add_index_null(zval *arg, ulong idx);
ZEND_API int add_index_bool(zval *arg, ulong idx, int b);
ZEND_API int add_index_resource(zval *arg, ulong idx, int r);
ZEND_API int add_index_double(zval *arg, ulong idx, double d);
ZEND_API int add_index_string(zval *arg, ulong idx, const char *str, int duplicate);
ZEND_API int add_index_stringl(zval *arg, ulong idx, const char *str, uint length, int duplicate);
ZEND_API int add_index_zval(zval *arg, ulong index, zval *value);

ZEND_API int add_next_index_long(zval *arg, long n);
ZEND_API int add_next_index_null(zval *arg);
ZEND_API int add_next_index_bool(zval *arg, int b);
ZEND_API int add_next_index_resource(zval *arg, int r);
ZEND_API int add_next_index_double(zval *arg, double d);
ZEND_API int add_next_index_string(zval *arg, const char *str, int duplicate);
ZEND_API int add_next_index_stringl(zval *arg, const char *str, uint length, int duplicate);
ZEND_API int add_next_index_zval(zval *arg, zval *value);

ZEND_API int add_get_assoc_string_ex(zval *arg, const char *key, uint key_len, const char *str, void **dest, int duplicate);
ZEND_API int add_get_assoc_stringl_ex(zval *arg, const char *key, uint key_len, const char *str, uint length, void **dest, int duplicate);

#define add_get_assoc_string(__arg, __key, __str, __dest, __duplicate) add_get_assoc_string_ex(__arg, __key, strlen(__key)+1, __str, __dest, __duplicate)
#define add_get_assoc_stringl(__arg, __key, __str, __length, __dest, __duplicate) add_get_assoc_stringl_ex(__arg, __key, strlen(__key)+1, __str, __length, __dest, __duplicate)

ZEND_API int add_get_index_long(zval *arg, ulong idx, long l, void **dest);
ZEND_API int add_get_index_double(zval *arg, ulong idx, double d, void **dest);
ZEND_API int add_get_index_string(zval *arg, ulong idx, const char *str, void **dest, int duplicate);
ZEND_API int add_get_index_stringl(zval *arg, ulong idx, const char *str, uint length, void **dest, int duplicate);

ZEND_API int array_set_zval_key(HashTable *ht, zval *key, zval *value);

ZEND_API int call_user_function_ex(HashTable *function_table, zval **object_pp, zval *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[], int no_separation, HashTable *symbol_table TSRMLS_DC);

/** Build zend_call_info/cache from a zval*
 *
 * Caller is responsible to provide a return value, otherwise the we will crash.
 * fci->retval_ptr_ptr = NULL;
 * In order to pass parameters the following members need to be set:
 * fci->param_count = 0;
 * fci->params = NULL;
 * The callable_name argument may be NULL.
 * Set check_flags to IS_CALLABLE_STRICT for every new usage!
 */
ZEND_API inline int zend_fcall_info_init(zval *callable, uint check_flags, zend_fcall_info *fci, zend_fcall_info_cache *fcc, char **callable_name, char **error TSRMLS_DC) {
  always_assert("does't work");
  return FAILURE;
}
END_EXTERN_C()

#define CHECK_ZVAL_NULL_PATH(p) (Z_STRLEN_P(p) != strlen(Z_STRVAL_P(p)))
#define CHECK_NULL_PATH(p, l) (strlen(p) != l)

#define ZVAL_RESOURCE(z, l) do {  \
    zval *__z = (z);      \
    Z_LVAL_P(__z) = l;      \
    Z_TYPE_P(__z) = IS_RESOURCE;\
  } while (0)

#define ZVAL_BOOL(z, b) do {    \
    zval *__z = (z);      \
    Z_LVAL_P(__z) = ((b) != 0);  \
    Z_TYPE_P(__z) = IS_BOOL;  \
  } while (0)

#define ZVAL_NULL(z) {        \
    Z_TYPE_P(z) = IS_NULL;    \
  }

#define ZVAL_LONG(z, l) {      \
    zval *__z = (z);      \
    Z_LVAL_P(__z) = l;      \
    Z_TYPE_P(__z) = IS_LONG;  \
  }

#define ZVAL_DOUBLE(z, d) {      \
    zval *__z = (z);      \
    Z_DVAL_P(__z) = d;      \
    Z_TYPE_P(__z) = IS_DOUBLE;  \
  }

#define ZVAL_STRINGL(z, s, l, duplicate) do {                       \
    const char *__s=(s); int __l=(l);                               \
    zval *__z = (z);                                                \
    __z->m_data.pstr = HPHP::String(__s, __l, HPHP::CopyString).detach(); \
    if (!duplicate) { efree(s); }                                   \
    Z_TYPE_P(__z) = IS_STRING;                                      \
  } while (0)

#define ZVAL_STRING(z, s, duplicate) ZVAL_STRINGL(z, s, strlen(s), duplicate)

#define ZVAL_EMPTY_STRING(z) do {  \
    zval *__z = (z);      \
    Z_STRLEN_P(__z) = 0;    \
    Z_STRVAL_P(__z) = STR_EMPTY_ALLOC();\
    Z_TYPE_P(__z) = IS_STRING;  \
  } while (0)

#define ZVAL_ZVAL(z, zv, copy, dtor) {      \
    zend_uchar is_ref = Z_ISREF_P(z);    \
    zend_uint refcount = Z_REFCOUNT_P(z);  \
    ZVAL_COPY_VALUE(z, zv);          \
    if (copy) {                \
      zval_copy_ctor(z);          \
      }                    \
    if (dtor) {                \
      if (!copy) {            \
        ZVAL_NULL(zv);          \
      }                  \
      zval_ptr_dtor(&zv);          \
      }                    \
    Z_SET_ISREF_TO_P(z, is_ref);      \
    Z_SET_REFCOUNT_P(z, refcount);      \
  }

#define ZVAL_FALSE(z)            ZVAL_BOOL(z, 0)
#define ZVAL_TRUE(z)            ZVAL_BOOL(z, 1)

#define RETVAL_RESOURCE(l)        ZVAL_RESOURCE(return_value, l)
#define RETVAL_BOOL(b)          ZVAL_BOOL(return_value, b)
#define RETVAL_NULL()           ZVAL_NULL(return_value)
#define RETVAL_LONG(l)           ZVAL_LONG(return_value, l)
#define RETVAL_DOUBLE(d)         ZVAL_DOUBLE(return_value, d)
#define RETVAL_STRING(s, duplicate)     ZVAL_STRING(return_value, s, duplicate)
#define RETVAL_STRINGL(s, l, duplicate)   ZVAL_STRINGL(return_value, s, l, duplicate)
#define RETVAL_EMPTY_STRING()       ZVAL_EMPTY_STRING(return_value)
#define RETVAL_ZVAL(zv, copy, dtor)    ZVAL_ZVAL(return_value, zv, copy, dtor)
#define RETVAL_FALSE            ZVAL_BOOL(return_value, 0)
#define RETVAL_TRUE             ZVAL_BOOL(return_value, 1)

#define RETURN_RESOURCE(l)         { RETVAL_RESOURCE(l); return; }
#define RETURN_BOOL(b)           { RETVAL_BOOL(b); return; }
#define RETURN_NULL()           { RETVAL_NULL(); return;}
#define RETURN_LONG(l)           { RETVAL_LONG(l); return; }
#define RETURN_DOUBLE(d)         { RETVAL_DOUBLE(d); return; }
#define RETURN_STRING(s, duplicate)   { RETVAL_STRING(s, duplicate); return; }
#define RETURN_STRINGL(s, l, duplicate) { RETVAL_STRINGL(s, l, duplicate); return; }
#define RETURN_EMPTY_STRING()       { RETVAL_EMPTY_STRING(); return; }
#define RETURN_ZVAL(zv, copy, dtor)    { RETVAL_ZVAL(zv, copy, dtor); return; }
#define RETURN_FALSE            { RETVAL_FALSE; return; }
#define RETURN_TRUE             { RETVAL_TRUE; return; }

#define HASH_OF(p) (Z_TYPE_P(p)==IS_ARRAY ? Z_ARRVAL_P(p) : ((Z_TYPE_P(p)==IS_OBJECT ? Z_OBJ_HT_P(p)->o_toArray().detach() : NULL)))
#define ZVAL_IS_NULL(z) (Z_TYPE_P(z)==IS_NULL)

/* For compatibility */
#define ZEND_MINIT      ZEND_MODULE_STARTUP_N
#define ZEND_MSHUTDOWN    ZEND_MODULE_SHUTDOWN_N
#define ZEND_RINIT      ZEND_MODULE_ACTIVATE_N
#define ZEND_RSHUTDOWN    ZEND_MODULE_DEACTIVATE_N
#define ZEND_MINFO      ZEND_MODULE_INFO_N
#define ZEND_GINIT(module)    ((void (*)(void* TSRMLS_DC))(ZEND_MODULE_GLOBALS_CTOR_N(module)))
#define ZEND_GSHUTDOWN(module)  ((void (*)(void* TSRMLS_DC))(ZEND_MODULE_GLOBALS_DTOR_N(module)))

#define ZEND_MINIT_FUNCTION      ZEND_MODULE_STARTUP_D
#define ZEND_MSHUTDOWN_FUNCTION    ZEND_MODULE_SHUTDOWN_D
#define ZEND_RINIT_FUNCTION      ZEND_MODULE_ACTIVATE_D
#define ZEND_RSHUTDOWN_FUNCTION    ZEND_MODULE_DEACTIVATE_D
#define ZEND_MINFO_FUNCTION      ZEND_MODULE_INFO_D
#define ZEND_GINIT_FUNCTION      ZEND_MODULE_GLOBALS_CTOR_D
#define ZEND_GSHUTDOWN_FUNCTION    ZEND_MODULE_GLOBALS_DTOR_D

END_EXTERN_C()

#endif /* ZEND_API_H */
