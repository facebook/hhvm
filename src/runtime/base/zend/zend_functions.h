/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_ZEND_FUNCTIONS_H__
#define __HPHP_ZEND_FUNCTIONS_H__

#include <runtime/base/types.h>
#include <util/hash.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// zend logic: These are not string utilities, but zend's special language
// semantics.

/**
 * Testing whether a string is numeric or not.
 */
DataType is_numeric_string(const char *str, int length, int64 *lval,
                           double *dval, int allow_errors = 0);

/**
 * Zend's way of incrementing a string. Definitely something we want to get rid
 * of in the future.
 */
char *increment_string(char *s, int len);

/**
 * Whether or not a string is a valid variable name.
 */
bool is_valid_var_name(const char *var_name, int len);

///////////////////////////////////////////////////////////////////////////////

/**
 * Adapted from ap_php_conv_10 for fast signed integer to string conversion.
 */
inline char *
conv_10(register long num, register int *is_negative, char *buf_end,
        register int *len)
{
  register char *p = buf_end;
  register unsigned long magnitude;

  *is_negative = (num < 0);

  /*
   * On a 2's complement machine, negating the most negative integer
   * results in a number that cannot be represented as a signed integer.
   * Here is what we do to obtain the number's magnitude:
   *      a. add 1 to the number
   *      b. negate it (becomes positive)
   *      c. convert it to unsigned
   *      d. add 1
   */
  if (*is_negative) {
    long t = num + 1;
    magnitude = ((unsigned long) - t) + 1;
  } else {
    magnitude = (unsigned long) num;
  }

  /*
   * We use a do-while loop so that we write at least 1 digit
   */
  do {
    unsigned long new_magnitude = magnitude / 10;

    *--p = (char)(magnitude - new_magnitude * 10 + '0');
    magnitude = new_magnitude;
  }
  while (magnitude);

  if (*is_negative) {
    *--p = '-';
  }

  *len = buf_end - p;
  return (p);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_FUNCTIONS_H__
