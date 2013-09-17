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

#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_API.h"

#include "zend_constants.h"

#include "hphp/runtime/base/zend-printf.h"

ZEND_API const char *zend_zval_type_name(const zval *arg) {
  return HPHP::getDataTypeString(Z_TYPE_P(arg))->data();
}

ZEND_API zend_class_entry *zend_get_class_entry(const zval *zobject TSRMLS_DC) {
  return Z_OBJVAL_P(zobject)->getVMClass();
}

static int parse_arg_object_to_string(zval **arg, const char **p, int *pl, int type TSRMLS_DC) {
  HPHP::StringData *sd = tvCastToString((*arg)->tv());
  *p = sd->data();
  *pl = sd->size();
  zval_ptr_dtor(arg);
  return sd->empty();
}

static const char *zend_parse_arg_impl(int arg_num, zval **arg, va_list *va, const char **spec, char **error, int *severity TSRMLS_DC) {
  const char *spec_walk = *spec;
  char c = *spec_walk++;
  int check_null = 0;

  /* scan through modifiers */
  while (1) {
    if (*spec_walk == '/') {
      SEPARATE_ZVAL_IF_NOT_REF(arg);
    } else if (*spec_walk == '!') {
      check_null = 1;
    } else {
      break;
    }
    spec_walk++;
  }

  switch (c) {
    case 'l':
    case 'L':
      {
        long *p = va_arg(*va, long *);

        if (check_null) {
          zend_bool *p = va_arg(*va, zend_bool *);
          *p = (Z_TYPE_PP(arg) == IS_NULL);
        }

        switch (Z_TYPE_PP(arg)) {
          case IS_STRING:
            {
              double d;
              int type;

              if ((type = is_numeric_string(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), p, &d, -1)) == 0) {
                return "long";
              } else if (type == IS_DOUBLE) {
                if (c == 'L') {
                  if (d > LONG_MAX) {
                    *p = LONG_MAX;
                    break;
                  } else if (d < LONG_MIN) {
                    *p = LONG_MIN;
                    break;
                  }
                }

                *p = zend_dval_to_lval(d);
              }
            }
            break;

          case IS_DOUBLE:
            if (c == 'L') {
              if (Z_DVAL_PP(arg) > LONG_MAX) {
                *p = LONG_MAX;
                break;
              } else if (Z_DVAL_PP(arg) < LONG_MIN) {
                *p = LONG_MIN;
                break;
              }
            }
          case IS_NULL:
          case IS_LONG:
          case IS_BOOL:
            convert_to_long_ex(arg);
            *p = Z_LVAL_PP(arg);
            break;

          case IS_ARRAY:
          case IS_OBJECT:
          case IS_RESOURCE:
          default:
            return "long";
        }
      }
      break;

    case 'd':
      {
        double *p = va_arg(*va, double *);

        if (check_null) {
          zend_bool *p = va_arg(*va, zend_bool *);
          *p = (Z_TYPE_PP(arg) == IS_NULL);
        }

        switch (Z_TYPE_PP(arg)) {
          case IS_STRING:
            {
              long l;
              int type;

              if ((type = is_numeric_string(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &l, p, -1)) == 0) {
                return "double";
              } else if (type == IS_LONG) {
                *p = (double) l;
              }
            }
            break;

          case IS_NULL:
          case IS_LONG:
          case IS_DOUBLE:
          case IS_BOOL:
            convert_to_double_ex(arg);
            *p = Z_DVAL_PP(arg);
            break;

          case IS_ARRAY:
          case IS_OBJECT:
          case IS_RESOURCE:
          default:
            return "double";
        }
      }
      break;

    case 'p':
    case 's':
      {
        const char **p = va_arg(*va, const char **);
        int *pl = va_arg(*va, int *);
        switch (Z_TYPE_PP(arg)) {
          case IS_NULL:
            if (check_null) {
              *p = NULL;
              *pl = 0;
              break;
            }
            /* break omitted intentionally */

          case IS_STRING:
          case IS_LONG:
          case IS_DOUBLE:
          case IS_BOOL:
            convert_to_string_ex(arg);
            if (UNEXPECTED(Z_ISREF_PP(arg) != 0)) {
              /* it's dangerous to return pointers to string
                 buffer of referenced variable, because it can
                 be clobbered throug magic callbacks */
              SEPARATE_ZVAL(arg);
            }
            *p = Z_STRVAL_PP(arg);
            *pl = Z_STRLEN_PP(arg);
            if (c == 'p' && CHECK_ZVAL_NULL_PATH(*arg)) {
              return "a valid path";
            }
            break;

          case IS_OBJECT:
            if (parse_arg_object_to_string(arg, p, pl, IS_STRING TSRMLS_CC) == SUCCESS) {
              if (c == 'p' && CHECK_ZVAL_NULL_PATH(*arg)) {
                return "a valid path";
              }
              break;
            }

          case IS_ARRAY:
          case IS_RESOURCE:
          default:
            return c == 's' ? "string" : "a valid path";
        }
      }
      break;

    case 'b':
      {
        zend_bool *p = va_arg(*va, zend_bool *);

        if (check_null) {
          zend_bool *p = va_arg(*va, zend_bool *);
          *p = (Z_TYPE_PP(arg) == IS_NULL);
        }

        switch (Z_TYPE_PP(arg)) {
          case IS_NULL:
          case IS_STRING:
          case IS_LONG:
          case IS_DOUBLE:
          case IS_BOOL:
            convert_to_boolean_ex(arg);
            *p = Z_BVAL_PP(arg);
            break;

          case IS_ARRAY:
          case IS_OBJECT:
          case IS_RESOURCE:
          default:
            return "boolean";
        }
      }
      break;

    case 'r':
      {
        zval **p = va_arg(*va, zval **);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
          break;
        }
        if (Z_TYPE_PP(arg) == IS_RESOURCE) {
          *p = *arg;
        } else {
          return "resource";
        }
      }
      break;
    case 'A':
    case 'a':
      {
        zval **p = va_arg(*va, zval **);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
          break;
        }
        if (Z_TYPE_PP(arg) == IS_ARRAY || (c == 'A' && Z_TYPE_PP(arg) == IS_OBJECT)) {
          *p = *arg;
        } else {
          return "array";
        }
      }
      break;
    case 'H':
    case 'h':
      {
        HashTable **p = va_arg(*va, HashTable **);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
          break;
        }
        if (Z_TYPE_PP(arg) == IS_ARRAY) {
          *p = Z_ARRVAL_PP(arg);
        } else if(c == 'H' && Z_TYPE_PP(arg) == IS_OBJECT) {
          *p = HASH_OF(*arg);
          if(*p == NULL) {
            return "array";
          }
        } else {
          return "array";
        }
      }
      break;

    case 'o':
      {
        zval **p = va_arg(*va, zval **);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
          break;
        }
        if (Z_TYPE_PP(arg) == IS_OBJECT) {
          *p = *arg;
        } else {
          return "object";
        }
      }
      break;

    case 'O':
      {
        zval **p = va_arg(*va, zval **);
        zend_class_entry *ce = va_arg(*va, zend_class_entry *);

        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
          break;
        }
        if (Z_TYPE_PP(arg) == IS_OBJECT &&
            (!ce || instanceof_function(Z_OBJCE_PP(arg), ce TSRMLS_CC))) {
          *p = *arg;
        } else {
          if (ce) {
            return ce->name()->data();
          } else {
            return "object";
          }
        }
      }
      break;

    case 'C':
      {
        zend_class_entry **lookup, **pce = va_arg(*va, zend_class_entry **);
        zend_class_entry *ce_base = *pce;

        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *pce = NULL;
          break;
        }
        convert_to_string_ex(arg);
        if (zend_lookup_class(Z_STRVAL_PP(arg), Z_STRLEN_PP(arg), &lookup TSRMLS_CC) == FAILURE) {
          *pce = NULL;
        } else {
          *pce = *lookup;
        }
        if (ce_base) {
          if ((!*pce || !instanceof_function(*pce, ce_base TSRMLS_CC))) {
            HPHP::spprintf(error, 0, "to be a class name derived from %s, '%s' given",
              ce_base->name()->data(), Z_STRVAL_PP(arg));
            *pce = NULL;
            return "";
          }
        }
        if (!*pce) {
          HPHP::spprintf(error, 0, "to be a valid class name, '%s' given",
            Z_STRVAL_PP(arg));
          return "";
        }
        break;

      }
      break;

    case 'f':
      {
        zend_fcall_info *fci = va_arg(*va, zend_fcall_info *);
        zend_fcall_info_cache *fcc = va_arg(*va, zend_fcall_info_cache *);
        char *is_callable_error = NULL;

        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          fci->size = 0;
          fcc->initialized = 0;
          break;
        }

        if (zend_fcall_info_init(*arg, 0, fci, fcc, NULL, &is_callable_error TSRMLS_CC) == SUCCESS) {
          if (is_callable_error) {
            *severity = E_STRICT;
            HPHP::spprintf(error, 0, "to be a valid callback, %s", is_callable_error);
            efree(is_callable_error);
            *spec = spec_walk;
            return "";
          }
          break;
        } else {
          if (is_callable_error) {
            *severity = E_WARNING;
            HPHP::spprintf(error, 0, "to be a valid callback, %s", is_callable_error);
            efree(is_callable_error);
            return "";
          } else {
            return "valid callback";
          }
        }
      }

    case 'z':
      {
        zval **p = va_arg(*va, zval **);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
        } else {
          *p = *arg;
        }
      }
      break;

    case 'Z':
      {
        zval ***p = va_arg(*va, zval ***);
        if (check_null && Z_TYPE_PP(arg) == IS_NULL) {
          *p = NULL;
        } else {
          not_implemented();
        }
      }
      break;

    default:
      return "unknown";
  }

  *spec = spec_walk;

  return NULL;
}

static int zend_parse_arg(int arg_num, zval **arg, va_list *va, const char **spec, int quiet TSRMLS_DC) {
  const char *expected_type = NULL;
  char *error = NULL;
  int severity = E_WARNING;

  expected_type = zend_parse_arg_impl(arg_num, arg, va, spec, &error, &severity TSRMLS_CC);
  if (expected_type) {
    if (!quiet && (*expected_type || error)) {
      const char *space;
      const char *class_name = get_active_class_name(&space TSRMLS_CC);

      if (error) {
        zend_error(severity, "%s%s%s() expects parameter %d %s",
            class_name, space, get_active_function_name(TSRMLS_C), arg_num, error);
        efree(error);
      } else {
        zend_error(severity, "%s%s%s() expects parameter %d to be %s, %s given",
            class_name, space, get_active_function_name(TSRMLS_C), arg_num, expected_type,
            zend_zval_type_name(*arg));
      }
    }
    if (severity != E_STRICT) {
      return FAILURE;
    }
  }

  return SUCCESS;
}

static int zend_parse_va_args(int num_args, const char *type_spec, va_list *va, int flags TSRMLS_DC) {
  const char *spec_walk;
  int c, i;
  int min_num_args = -1;
  int max_num_args = 0;
  int post_varargs = 0;
  zval **arg;
  int arg_count;
  int quiet = flags & ZEND_PARSE_PARAMS_QUIET;
  zend_bool have_varargs = 0;
  zval ****varargs = NULL;
  int *n_varargs = NULL;

  for (spec_walk = type_spec; *spec_walk; spec_walk++) {
    c = *spec_walk;
    switch (c) {
      case 'l': case 'd':
      case 's': case 'b':
      case 'r': case 'a':
      case 'o': case 'O':
      case 'z': case 'Z':
      case 'C': case 'h':
      case 'f': case 'A':
      case 'H': case 'p':
        max_num_args++;
        break;

      case '|':
        min_num_args = max_num_args;
        break;

      case '/':
      case '!':
        /* Pass */
        break;

      case '*':
      case '+':
        if (have_varargs) {
          if (!quiet) {
            zend_error(E_WARNING, "%s(): only one varargs specifier (* or +) is permitted",
                get_active_function_name(TSRMLS_C));
          }
          return FAILURE;
        }
        have_varargs = 1;
        /* we expect at least one parameter in varargs */
        if (c == '+') {
          max_num_args++;
        }
        /* mark the beginning of varargs */
        post_varargs = max_num_args;
        break;

      default:
        if (!quiet) {
          zend_error(E_WARNING, "%s(): bad type specifier while parsing parameters",
              get_active_function_name(TSRMLS_C));
        }
        return FAILURE;
    }
  }

  if (min_num_args < 0) {
    min_num_args = max_num_args;
  }

  if (have_varargs) {
    /* calculate how many required args are at the end of the specifier list */
    post_varargs = max_num_args - post_varargs;
    max_num_args = -1;
  }

  if (num_args < min_num_args || (num_args > max_num_args && max_num_args > 0)) {
    if (!quiet) {
      zend_error(E_WARNING, "%s() expects %s %d parameter%s, %d given",
          get_active_function_name(TSRMLS_C),
          min_num_args == max_num_args ? "exactly" : num_args < min_num_args ? "at least" : "at most",
          num_args < min_num_args ? min_num_args : max_num_args,
          (num_args < min_num_args ? min_num_args : max_num_args) == 1 ? "" : "s",
          num_args);
    }
    return FAILURE;
  }

  arg_count = HPHP::liveFrame()->numArgs();

  if (num_args > arg_count) {
    zend_error(E_WARNING, "%s(): could not obtain parameters for parsing",
      get_active_function_name(TSRMLS_C));
    return FAILURE;
  }

  i = 0;
  while (num_args-- > 0) {
    if (*type_spec == '|') {
      type_spec++;
    }

    if (*type_spec == '*' || *type_spec == '+') {
      int num_varargs = num_args + 1 - post_varargs;

      /* eat up the passed in storage even if it won't be filled in with varargs */
      varargs = va_arg(*va, zval ****);
      n_varargs = va_arg(*va, int *);
      type_spec++;

      if (num_varargs > 0) {
        int iv = 0;
        HPHP::TypedValue *top = HPHP::vmfp() - (i + 1);
        // zPrepArgs should take care of this for us
        assert(top->m_type == HPHP::KindOfRef);
        zval **p = &top->m_data.pref;

        *n_varargs = num_varargs;

        /* allocate space for array and store args */
        *varargs = (zval ***) safe_emalloc(num_varargs, sizeof(zval **), 0);
        while (num_varargs-- > 0) {
          (*varargs)[iv++] = p++;
        }

        /* adjust how many args we have left and restart loop */
        num_args = num_args + 1 - iv;
        i += iv;
        continue;
      } else {
        *varargs = NULL;
        *n_varargs = 0;
      }
    }

    HPHP::TypedValue *top = HPHP::vmfp() - (i + 1);
    assert(top->m_type == HPHP::KindOfRef);
    arg = &top->m_data.pref;

    if (zend_parse_arg(i+1, arg, va, &type_spec, quiet TSRMLS_CC) == FAILURE) {
      /* clean up varargs array if it was used */
      if (varargs && *varargs) {
        efree(*varargs);
        *varargs = NULL;
      }
      return FAILURE;
    }

    i++;
  }

  return SUCCESS;
}

#define RETURN_IF_ZERO_ARGS(num_args, type_spec, quiet) { \
  int __num_args = (num_args); \
  \
  if (0 == (type_spec)[0] && 0 != __num_args && !(quiet)) { \
    const char *__space; \
    const char * __class_name = get_active_class_name(&__space TSRMLS_CC); \
    zend_error(E_WARNING, "%s%s%s() expects exactly 0 parameters, %d given", \
      __class_name, __space, \
      get_active_function_name(TSRMLS_C), __num_args); \
    return FAILURE; \
  }\
}

ZEND_API int zend_parse_parameters(int num_args TSRMLS_DC, const char *type_spec, ...) {
  va_list va;
  int retval;

  RETURN_IF_ZERO_ARGS(num_args, type_spec, 0);

  va_start(va, type_spec);
  retval = zend_parse_va_args(num_args, type_spec, &va, 0 TSRMLS_CC);
  va_end(va);

  return retval;
}

ZEND_API int add_assoc_long_ex(zval *arg, const char *key, uint key_len, long n) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_LONG(tmp, n);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_null_ex(zval *arg, const char *key, uint key_len) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_NULL(tmp);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_bool_ex(zval *arg, const char *key, uint key_len, int b) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_BOOL(tmp, b);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_resource_ex(zval *arg, const char *key, uint key_len, int r) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_RESOURCE(tmp, r);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_double_ex(zval *arg, const char *key, uint key_len, double d) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_DOUBLE(tmp, d);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_string_ex(zval *arg, const char *key, uint key_len, char *str, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_stringl_ex(zval *arg, const char *key, uint key_len, char *str, uint length, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRINGL(tmp, str, length, duplicate);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_zval_ex(zval *arg, const char *key, uint key_len, zval *value) /* {{{ */
{
  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &value, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_long(zval *arg, ulong index, long n) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_LONG(tmp, n);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_null(zval *arg, ulong index) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_NULL(tmp);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_bool(zval *arg, ulong index, int b) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_BOOL(tmp, b);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_resource(zval *arg, ulong index, int r) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_RESOURCE(tmp, r);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_double(zval *arg, ulong index, double d) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_DOUBLE(tmp, d);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_string(zval *arg, ulong index, const char *str, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_stringl(zval *arg, ulong index, const char *str, uint length, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRINGL(tmp, str, length, duplicate);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_index_zval(zval *arg, ulong index, zval *value) /* {{{ */
{
  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &value, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_long(zval *arg, long n) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_LONG(tmp, n);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_null(zval *arg) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_NULL(tmp);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_bool(zval *arg, int b) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_BOOL(tmp, b);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_resource(zval *arg, int r) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_RESOURCE(tmp, r);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_double(zval *arg, double d) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_DOUBLE(tmp, d);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_string(zval *arg, const char *str, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_stringl(zval *arg, const char *str, uint length, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRINGL(tmp, str, length, duplicate);

  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_next_index_zval(zval *arg, zval *value) /* {{{ */
{
  return zend_hash_next_index_insert(Z_ARRVAL_P(arg), &value, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_get_assoc_string_ex(zval *arg, const char *key, uint key_len, const char *str, void **dest, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int add_get_assoc_stringl_ex(zval *arg, const char *key, uint key_len, const char *str, uint length, void **dest, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRINGL(tmp, str, length, duplicate);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int add_get_index_long(zval *arg, ulong index, long l, void **dest) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_LONG(tmp, l);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int add_get_index_double(zval *arg, ulong index, double d, void **dest) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_DOUBLE(tmp, d);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int add_get_index_string(zval *arg, ulong index, const char *str, void **dest, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int add_get_index_stringl(zval *arg, ulong index, const char *str, uint length, void **dest, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRINGL(tmp, str, length, duplicate);

  return zend_hash_index_update(Z_ARRVAL_P(arg), index, &tmp, sizeof(zval *), dest);
}
/* }}} */

ZEND_API int array_set_zval_key(HashTable *ht, zval *key, zval *value) /* {{{ */
{
  int result;

  switch (Z_TYPE_P(key)) {
    case IS_STRING:
      result = zend_symtable_update(ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, &value, sizeof(zval *), NULL);
      break;
    case IS_NULL:
      result = zend_symtable_update(ht, "", 1, &value, sizeof(zval *), NULL);
      break;
    case IS_RESOURCE:
      zend_error(E_STRICT, "Resource ID#%ld used as offset, casting to integer (%ld)", Z_LVAL_P(key), Z_LVAL_P(key));
      /* break missing intentionally */
    case IS_BOOL:
    case IS_LONG:
      result = zend_hash_index_update(ht, Z_LVAL_P(key), &value, sizeof(zval *), NULL);
      break;
    case IS_DOUBLE:
      result = zend_hash_index_update(ht, zend_dval_to_lval(Z_DVAL_P(key)), &value, sizeof(zval *), NULL);
      break;
    default:
      zend_error(E_WARNING, "Illegal offset type");
      result = FAILURE;
  }

  if (result == SUCCESS) {
    Z_ADDREF_P(value);
  }

  return result;
}
/* }}} */

ZEND_API zend_bool zend_is_callable(zval *callable, uint check_flags, char **callable_name TSRMLS_DC) {
  HPHP::Variant name;
  bool b = f_is_callable(tvAsVariant(callable->tv()), check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY, HPHP::ref(name));
  if (callable_name) {
    HPHP::StringData *sd = name.getStringData();
    *callable_name = (char*) emalloc(sd->size() + 1);
    memcpy(*callable_name, sd->data(), sd->size() + 1);
  }
  return b;
}

ZEND_API int call_user_function_ex(HashTable *function_table, zval **object_pp, zval *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[], int no_separation, HashTable *symbol_table TSRMLS_DC) {
  HPHP::PackedArrayInit ad_params(param_count);
  for (int i = 0; i < param_count; i++) {
    ad_params.append(tvAsVariant((*params[i])->tv()));
  }
  HPHP::Variant retval = vm_call_user_func(tvAsCVarRef(function_name->tv()), ad_params.toArray());
  if (!retval.isInitialized()) {
    return FAILURE;
  }

  MAKE_STD_ZVAL(*retval_ptr_ptr);
  HPHP::cellDup(*retval.asCell(), *(*retval_ptr_ptr)->tv());
  return SUCCESS;
}

ZEND_API int _array_init(zval *arg, uint size ZEND_FILE_LINE_DC) {
  ALLOC_HASHTABLE(Z_ARRVAL_P(arg));
  Z_TYPE_P(arg) = IS_ARRAY;
  Z_ARRVAL_P(arg)->incRefCount();
  return SUCCESS;
}

/* returns 1 if you need to copy result, 0 if it's already a copy */
ZEND_API int zend_get_object_classname(const zval *object, const char **class_name, zend_uint *class_name_len TSRMLS_DC) {
  zend_class_entry* clazz = zend_get_class_entry(object);
  auto* name = clazz->name();
  *class_name_len = name->size();
  *class_name = estrndup(name->data(), *class_name_len);
  return 0;
}
