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
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <ctype.h>

#include "zend.h"
#include "zend_operators.h"
#include "zend_variables.h"
#include "zend_globals.h"
#include "zend_list.h"
#include "zend_API.h"
#include "zend_exceptions.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_operators.h"
#include "hphp/runtime/base/tv-helpers.h"

#if ZEND_USE_TOLOWER_L
#include <locale.h>
static _locale_t current_locale = NULL;
/* this is true global! may lead to strange effects on ZTS, but so may setlocale() */
#define zend_tolower(c) _tolower_l(c, current_locale)
#else
#define zend_tolower(c) tolower(c)
#endif

ZEND_API int zend_atoi(const char *str, int str_len) /* {{{ */
{
	int retval;

	if (!str_len) {
		str_len = strlen(str);
	}
	retval = strtol(str, NULL, 0);
	if (str_len>0) {
		switch (str[str_len-1]) {
			case 'g':
			case 'G':
				retval *= 1024;
				/* break intentionally missing */
			case 'm':
			case 'M':
				retval *= 1024;
				/* break intentionally missing */
			case 'k':
			case 'K':
				retval *= 1024;
				break;
		}
	}
	return retval;
}
/* }}} */

ZEND_API long zend_atol(const char *str, int str_len) /* {{{ */
{
	long retval;

	if (!str_len) {
		str_len = strlen(str);
	}
	retval = strtol(str, NULL, 0);
	if (str_len>0) {
		switch (str[str_len-1]) {
			case 'g':
			case 'G':
				retval *= 1024;
				/* break intentionally missing */
			case 'm':
			case 'M':
				retval *= 1024;
				/* break intentionally missing */
			case 'k':
			case 'K':
				retval *= 1024;
				break;
		}
	}
	return retval;
}
/* }}} */

HPHP::DataType& Z_TYPE(const zval& z) {
  HPHP::DataType* dt = &const_cast<zval*>(&z)->tv()->m_type;
  assert(*dt != HPHP::KindOfRef);
  if (*dt == HPHP::KindOfStaticString) {
    *dt = IS_STRING;
  }
  return *dt;
}

ZEND_API int string_compare_function_ex(zval *result, zval *op1, zval *op2, zend_bool case_insensitive TSRMLS_DC) /* {{{ */
{
  zval op1_copy, op2_copy;
  int use_copy1 = 0, use_copy2 = 0;

  if (Z_TYPE_P(op1) != IS_STRING) {
    zend_make_printable_zval(op1, &op1_copy, &use_copy1);
  }
  if (Z_TYPE_P(op2) != IS_STRING) {
    zend_make_printable_zval(op2, &op2_copy, &use_copy2);
  }

  if (use_copy1) {
    op1 = &op1_copy;
  }
  if (use_copy2) {
    op2 = &op2_copy;
  }

  if (case_insensitive) {
    ZVAL_LONG(result, zend_binary_zval_strcasecmp(op1, op2));
  } else {
    ZVAL_LONG(result, zend_binary_zval_strcmp(op1, op2));
  }

  if (use_copy1) {
    zval_dtor(op1);
  }
  if (use_copy2) {
    zval_dtor(op2);
  }
  return SUCCESS;
}
/* }}} */

ZEND_API int string_compare_function(zval *result, zval *op1, zval *op2 TSRMLS_DC) /* {{{ */
{
  return string_compare_function_ex(result, op1, op2, 0 TSRMLS_CC);
}

ZEND_API int zend_binary_strcasecmp_l(const char *s1, uint len1, const char *s2, uint len2) /* {{{ */
{
  int len;
  int c1, c2;

  if (s1 == s2) {
    return 0;
  }

  len = MIN(len1, len2);
  while (len--) {
    c1 = zend_tolower((int)*(unsigned char *)s1++);
    c2 = zend_tolower((int)*(unsigned char *)s2++);
    if (c1 != c2) {
      return c1 - c2;
    }
  }

  return len1 - len2;
}

ZEND_API int zend_binary_zval_strcasecmp(zval *s1, zval *s2) /* {{{ */
{
  return zend_binary_strcasecmp_l(Z_STRVAL_P(s1), Z_STRLEN_P(s1), Z_STRVAL_P(s2), Z_STRLEN_P(s2));
}

ZEND_API int zend_binary_strcmp(const char *s1, uint len1, const char *s2, uint len2) /* {{{ */
{
  int retval;

  if (s1 == s2) {
    return 0;
  }
  retval = memcmp(s1, s2, MIN(len1, len2));
  if (!retval) {
    return (len1 - len2);
  } else {
    return retval;
  }
}

ZEND_API int zend_binary_zval_strcmp(zval *s1, zval *s2) /* {{{ */
{
  return zend_binary_strcmp(Z_STRVAL_P(s1), Z_STRLEN_P(s1), Z_STRVAL_P(s2), Z_STRLEN_P(s2));
}

ZEND_API zend_bool instanceof_function_ex(const zend_class_entry *instance_ce, const zend_class_entry *ce, zend_bool interfaces_only TSRMLS_DC) {
  return instance_ce->hphp_class->classof(ce->hphp_class);
}
ZEND_API zend_bool instanceof_function(const zend_class_entry *instance_ce, const zend_class_entry *ce TSRMLS_DC) {
  return instanceof_function_ex(instance_ce, ce, 0 TSRMLS_CC);
}

ZEND_API void _convert_to_string(zval *op ZEND_FILE_LINE_DC) {
  tvCastToStringInPlace(op->tv());
}
ZEND_API void convert_to_long(zval *op) {
  tvCastToInt64InPlace(op->tv());
}
ZEND_API void convert_to_double(zval *op) {
  tvCastToDoubleInPlace(op->tv());
}
ZEND_API void convert_to_boolean(zval *op) {
  tvCastToBooleanInPlace(op->tv());
}
ZEND_API void convert_to_array(zval *op) {
  tvCastToArrayInPlace(op->tv());
}
