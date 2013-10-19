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

#if ZEND_USE_TOLOWER_L
#include <locale.h>
static _locale_t current_locale = NULL;
/* this is true global! may lead to strange effects on ZTS, but so may setlocale() */
#define zend_tolower(c) _tolower_l(c, current_locale)
#else
#define zend_tolower(c) tolower(c)
#endif


/**
 * Checks whether the string "str" with length "length" is numeric. The value
 * of allow_errors determines whether it's required to be entirely numeric, or
 * just its prefix. Leading whitespace is allowed.
 *
 * The function returns 0 if the string did not contain a valid number; IS_LONG
 * if it contained a number that fits within the range of a long; or IS_DOUBLE
 * if the number was out of long range or contained a decimal point/exponent.
 * The number's value is returned into the respective pointer, *lval or *dval,
 * if that pointer is not NULL.
 *
 * This variant also gives information if a string that represents an integer
 * could not be represented as such due to overflow. It writes 1 to oflow_info
 * if the integer is larger than LONG_MAX and -1 if it's smaller than LONG_MIN.
 */
zend_uchar is_numeric_string_ex(const char *str, int length, long *lval, double *dval, int allow_errors, int *oflow_info) {
  const char *ptr;
  int base = 10, digits = 0, dp_or_e = 0;
  double local_dval;
  zend_uchar type;

  if (!length) {
    return 0;
  }

  if (oflow_info != NULL) {
    *oflow_info = 0;
  }

  /* Skip any whitespace
   * This is much faster than the isspace() function */
  while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\v' || *str == '\f') {
    str++;
    length--;
  }
  ptr = str;

  if (*ptr == '-' || *ptr == '+') {
    ptr++;
  }

  if (ZEND_IS_DIGIT(*ptr)) {
    /* Handle hex numbers
     * str is used instead of ptr to disallow signs and keep old behavior */
    if (length > 2 && *str == '0' && (str[1] == 'x' || str[1] == 'X')) {
      base = 16;
      ptr += 2;
    }

    /* Skip any leading 0s */
    while (*ptr == '0') {
      ptr++;
    }

    /* Count the number of digits. If a decimal point/exponent is found,
     * it's a double. Otherwise, if there's a dval or no need to check for
     * a full match, stop when there are too many digits for a long */
    for (type = IS_LONG; !(digits >= MAX_LENGTH_OF_LONG && (dval || allow_errors == 1)); digits++, ptr++) {
check_digits:
      if (ZEND_IS_DIGIT(*ptr) || (base == 16 && ZEND_IS_XDIGIT(*ptr))) {
        continue;
      } else if (base == 10) {
        if (*ptr == '.' && dp_or_e < 1) {
          goto process_double;
        } else if ((*ptr == 'e' || *ptr == 'E') && dp_or_e < 2) {
          const char *e = ptr + 1;

          if (*e == '-' || *e == '+') {
            ptr = e++;
          }
          if (ZEND_IS_DIGIT(*e)) {
            goto process_double;
          }
        }
      }

      break;
    }

    if (base == 10) {
      if (digits >= MAX_LENGTH_OF_LONG) {
        if (oflow_info != NULL) {
          *oflow_info = *str == '-' ? -1 : 1;
        }
        dp_or_e = -1;
        goto process_double;
      }
    } else if (!(digits < SIZEOF_LONG * 2 || (digits == SIZEOF_LONG * 2 && ptr[-digits] <= '7'))) {
      if (dval) {
        local_dval = HPHP::zend_hex_strtod(str, &ptr);
      }
      if (oflow_info != NULL) {
        *oflow_info = 1;
      }
      type = IS_DOUBLE;
    }
  } else if (*ptr == '.' && ZEND_IS_DIGIT(ptr[1])) {
process_double:
    type = IS_DOUBLE;

    /* If there's a dval, do the conversion; else continue checking
     * the digits if we need to check for a full match */
    if (dval) {
      local_dval = HPHP::zend_strtod(str, &ptr);
    } else if (allow_errors != 1 && dp_or_e != -1) {
      dp_or_e = (*ptr++ == '.') ? 1 : 2;
      goto check_digits;
    }
  } else {
    return 0;
  }

  if (ptr != str + length) {
    if (!allow_errors) {
      return 0;
    }
    if (allow_errors == -1) {
      zend_error(E_NOTICE, "A non well formed numeric value encountered");
    }
  }

  if (type == IS_LONG) {
    if (digits == MAX_LENGTH_OF_LONG - 1) {
      int cmp = strcmp(&ptr[-digits], long_min_digits);

      if (!(cmp < 0 || (cmp == 0 && *str == '-'))) {
        if (dval) {
          *dval = HPHP::zend_strtod(str, NULL);
        }
        if (oflow_info != NULL) {
          *oflow_info = *str == '-' ? -1 : 1;
        }

        return IS_DOUBLE;
      }
    }

    if (lval) {
      *lval = strtol(str, NULL, base);
    }

    return IS_LONG;
  } else {
    if (dval) {
      *dval = local_dval;
    }

    return IS_DOUBLE;
  }
}

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
