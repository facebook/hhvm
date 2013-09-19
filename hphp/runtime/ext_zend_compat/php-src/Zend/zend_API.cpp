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
#include "hphp/util/thread-local.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend_hphp_class_to_class_entry.h"

ZEND_API const char *zend_zval_type_name(const zval *arg) {
  return HPHP::getDataTypeString(Z_TYPE_P(arg))->data();
}

ZEND_API zend_class_entry *zend_get_class_entry(const zval *zobject TSRMLS_DC) {
  auto* hphp_class = Z_OBJVAL_P(zobject)->getVMClass();
  return zend_hphp_class_to_class_entry(hphp_class);
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
            return ce->hphp_class->name()->data();
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
              ce_base->hphp_class->name()->data(), Z_STRVAL_PP(arg));
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

ZEND_API int zend_parse_method_parameters(int num_args TSRMLS_DC, zval *this_ptr, const char *type_spec, ...) /* {{{ */
{
  va_list va;
  int retval;
  const char *p = type_spec;
  zval **object;
  zend_class_entry *ce;

  if (!this_ptr) {
    RETURN_IF_ZERO_ARGS(num_args, p, 0);

    va_start(va, type_spec);
    retval = zend_parse_va_args(num_args, type_spec, &va, 0 TSRMLS_CC);
    va_end(va);
  } else {
    p++;
    RETURN_IF_ZERO_ARGS(num_args, p, 0);

    va_start(va, type_spec);

    object = va_arg(va, zval **);
    ce = va_arg(va, zend_class_entry *);
    *object = this_ptr;

    if (ce && !instanceof_function(Z_OBJCE_P(this_ptr), ce TSRMLS_CC)) {
      zend_error(E_CORE_ERROR, "%s::%s() must be derived from %s::%s",
        ce->name, get_active_function_name(TSRMLS_C), Z_OBJCE_P(this_ptr)->name, get_active_function_name(TSRMLS_C));
    }

    retval = zend_parse_va_args(num_args, p, &va, 0 TSRMLS_CC);
    va_end(va);
  }
  return retval;
}
/* }}} */

ZEND_API int zend_parse_method_parameters_ex(int flags, int num_args TSRMLS_DC, zval *this_ptr, const char *type_spec, ...) /* {{{ */
{
  va_list va;
  int retval;
  const char *p = type_spec;
  zval **object;
  zend_class_entry *ce;
  int quiet = flags & ZEND_PARSE_PARAMS_QUIET;

  if (!this_ptr) {
    RETURN_IF_ZERO_ARGS(num_args, p, quiet);

    va_start(va, type_spec);
    retval = zend_parse_va_args(num_args, type_spec, &va, flags TSRMLS_CC);
    va_end(va);
  } else {
    p++;
    RETURN_IF_ZERO_ARGS(num_args, p, quiet);

    va_start(va, type_spec);

    object = va_arg(va, zval **);
    ce = va_arg(va, zend_class_entry *);
    *object = this_ptr;

    if (ce && !instanceof_function(Z_OBJCE_P(this_ptr), ce TSRMLS_CC)) {
      if (!quiet) {
        zend_error(E_CORE_ERROR, "%s::%s() must be derived from %s::%s",
          ce->name, get_active_function_name(TSRMLS_C), Z_OBJCE_P(this_ptr)->name, get_active_function_name(TSRMLS_C));
      }
      va_end(va);
      return FAILURE;
    }

    retval = zend_parse_va_args(num_args, p, &va, flags TSRMLS_CC);
    va_end(va);
  }
  return retval;
}
/* }}} */

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

ZEND_API int add_assoc_string_ex(zval *arg, const char *key, uint key_len, const char *str, int duplicate) /* {{{ */
{
  zval *tmp;

  MAKE_STD_ZVAL(tmp);
  ZVAL_STRING(tmp, str, duplicate);

  return zend_symtable_update(Z_ARRVAL_P(arg), key, key_len, &tmp, sizeof(zval *), NULL);
}
/* }}} */

ZEND_API int add_assoc_stringl_ex(zval *arg, const char *key, uint key_len, const char *str, uint length, int duplicate) /* {{{ */
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

ZEND_API zend_bool zend_is_callable_ex(zval *callable, zval *object_ptr, uint check_flags, char **callable_name, int *callable_name_len, zend_fcall_info_cache *fcc, char **error TSRMLS_DC) /* {{{ */
{
  HPHP::Variant name;
  bool b = f_is_callable(tvAsVariant(callable->tv()), check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY, HPHP::ref(name));
  if (callable_name) {
    HPHP::StringData *sd = name.getStringData();
    *callable_name = (char*) emalloc(sd->size() + 1);
    memcpy(*callable_name, sd->data(), sd->size() + 1);
  }
  return b;
}

ZEND_API zend_bool zend_is_callable(zval *callable, uint check_flags, char **callable_name TSRMLS_DC) {
  return zend_is_callable_ex(callable, NULL, check_flags, callable_name, NULL, NULL, NULL TSRMLS_CC);
}

ZEND_API int call_user_function(HashTable *function_table, zval **object_pp, zval *function_name, zval *retval_ptr, zend_uint param_count, zval *params[] TSRMLS_DC) {
  zval ***params_array;
  zend_uint i;
  int ex_retval;
  zval *local_retval_ptr = NULL;

  if (param_count) {
    params_array = (zval ***) emalloc(sizeof(zval **)*param_count);
    for (i=0; i<param_count; i++) {
      params_array[i] = &params[i];
    }
  } else {
    params_array = NULL;
  }
  ex_retval = call_user_function_ex(function_table, object_pp, function_name, &local_retval_ptr, param_count, params_array, 1, NULL TSRMLS_CC);
  if (local_retval_ptr) {
    COPY_PZVAL_TO_ZVAL(*retval_ptr, local_retval_ptr);
  } else {
    INIT_ZVAL(*retval_ptr);
  }
  if (params_array) {
    efree(params_array);
  }
  return ex_retval;
}

ZEND_API int call_user_function_ex(HashTable *function_table, zval **object_pp, zval *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[], int no_separation, HashTable *symbol_table TSRMLS_DC) {
  assert(object_pp == nullptr);

  // mostly from vm_call_user_func
  HPHP::ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  HPHP::Transl::CallerFrame cf;
  HPHP::StringData* invName = nullptr;
  const HPHP::Func* f = vm_decode_function(
    tvAsCVarRef(function_name->tv()), cf(), false, obj, cls, invName
  );
  if (f == nullptr) {
    return FAILURE;
  }

  HPHP::PackedArrayInit ad_params(param_count);
  for (int i = 0; i < param_count; i++) {
    HPHP::Variant v;
    v.asTypedValue()->m_type = HPHP::KindOfRef;
    v.asTypedValue()->m_data.pref = *params[i];
    v.asTypedValue()->m_data.pref->incRefCount();
    if (f->byRef(i)) {
      ad_params.appendWithRef(v);
    } else {
      ad_params.append(v);
    }
  }
  HPHP::TypedValue retval;
  g_vmContext->invokeFunc(
    &retval, f, ad_params.toArray(), obj, cls,
    nullptr, invName, HPHP::ExecutionContext::InvokeCuf
  );
  if (retval.m_type == HPHP::KindOfUninit) {
    return FAILURE;
  }

  MAKE_STD_ZVAL(*retval_ptr_ptr);
  HPHP::tvDup(retval, *(*retval_ptr_ptr)->tv());
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
  auto* name = clazz->hphp_class->name();
  *class_name_len = name->size();
  *class_name = estrndup(name->data(), *class_name_len);
  return 0;
}

ZEND_API void zend_update_property_null(zend_class_entry *scope, zval *object, const char *name, int name_length TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_NULL(tmp);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API void zend_update_property_bool(zend_class_entry *scope, zval *object, const char *name, int name_length, long value TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_BOOL(tmp, value);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API void zend_update_property_long(zend_class_entry *scope, zval *object, const char *name, int name_length, long value TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_LONG(tmp, value);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API void zend_update_property_double(zend_class_entry *scope, zval *object, const char *name, int name_length, double value TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_DOUBLE(tmp, value);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API void zend_update_property_string(zend_class_entry *scope, zval *object, const char *name, int name_length, const char *value TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_STRING(tmp, value, 1);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API void zend_update_property_stringl(zend_class_entry *scope, zval *object, const char *name, int name_length, const char *value, int value_len TSRMLS_DC) /* {{{ */
{
  zval *tmp;

  ALLOC_ZVAL(tmp);
  Z_UNSET_ISREF_P(tmp);
  Z_SET_REFCOUNT_P(tmp, 0);
  ZVAL_STRINGL(tmp, value, value_len, 1);
  zend_update_property(scope, object, name, name_length, tmp TSRMLS_CC);
}
/* }}} */

ZEND_API int _object_init_ex(zval *arg, zend_class_entry *class_type ZEND_FILE_LINE_DC TSRMLS_DC) /* {{{ */
{
  return _object_and_properties_init(arg, class_type, 0 ZEND_FILE_LINE_RELAY_CC TSRMLS_CC);
}
/* }}} */

ZEND_API int _object_init(zval *arg ZEND_FILE_LINE_DC TSRMLS_DC) /* {{{ */
{
  return _object_init_ex(arg, zend_standard_class_def ZEND_FILE_LINE_RELAY_CC TSRMLS_CC);
}
/* }}} */

ZEND_API int zend_declare_property(zend_class_entry *ce, const char *name, int name_length, zval *property, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_null(zend_class_entry *ce, const char *name, int name_length, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_bool(zend_class_entry *ce, const char *name, int name_length, long value, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_long(zend_class_entry *ce, const char *name, int name_length, long value, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_double(zend_class_entry *ce, const char *name, int name_length, double value, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_string(zend_class_entry *ce, const char *name, int name_length, const char *value, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API int zend_declare_property_stringl(zend_class_entry *ce, const char *name, int name_length, const char *value, int value_len, int access_type TSRMLS_DC) /* {{{ */
{
  // Done by our .idl
  return SUCCESS;
}
/* }}} */

ZEND_API void zend_update_property(zend_class_entry *scope, zval *object, const char *name, int name_length, zval *value TSRMLS_DC) {
  HPHP::String key(name, name_length, HPHP::CopyString);
  HPHP::String context(scope->name);
  Z_OBJVAL_P(object)->o_set(key, tvAsVariant(value->tv()), context);
}

ZEND_API zend_class_entry *zend_register_internal_class(zend_class_entry *orig_class_entry TSRMLS_DC) {
  auto cls = orig_class_entry->hphp_class;
  assert(cls);
  auto ce = zend_hphp_class_to_class_entry(cls);
  ce->create_object = orig_class_entry->create_object;
  return ce;
}

ZEND_API void object_properties_init(zend_object *object, zend_class_entry *class_type) {
}

/* This function requires 'properties' to contain all props declared in the
 * class and all props being public. If only a subset is given or the class
 * has protected members then you need to merge the properties separately by
 * calling zend_merge_properties(). */
ZEND_API int _object_and_properties_init(zval *arg, zend_class_entry *class_type, HashTable *properties ZEND_FILE_LINE_DC TSRMLS_DC) {
  assert(properties == 0);
  // Why is there no ZVAL_OBJVAL?
  Z_OBJVAL_P(arg) = new_ZendObjectData_Instance(class_type->hphp_class);
  Z_TYPE_P(arg) = IS_OBJECT;
  return SUCCESS;
}

ZEND_API zval *zend_read_property(zend_class_entry *scope, zval *object, const char *name, int name_length, zend_bool silent TSRMLS_DC) {
  not_implemented();
  return nullptr;
}

ZEND_API zend_class_entry *zend_register_internal_class_ex(zend_class_entry *class_entry, zend_class_entry *parent_ce, char *parent_name TSRMLS_DC) {
  auto ret = zend_register_internal_class(class_entry);
  if (parent_ce) {
    ret->create_object = parent_ce->create_object;
    assert(ret->hphp_class->parent() == parent_ce->hphp_class);
  }
  return ret;
}
