/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/zend-math.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static inline int php_intlog10abs(double value) {
  int result;
  value = fabs(value);

  if (value < 1e-8 || value > 1e22) {
    result = (int)floor(log10(value));
  } else {
    static const double values[] = {
      1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
      1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
      1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
    /* Do a binary search with 5 steps */
    result = 15;
    if (value < values[result]) {
      result -= 8;
    } else {
      result += 8;
    }
    if (value < values[result]) {
      result -= 4;
    } else {
      result += 4;
    }
    if (value < values[result]) {
      result -= 2;
    } else {
      result += 2;
    }
    if (value < values[result]) {
      result -= 1;
    } else {
      result += 1;
    }
    if (value < values[result]) {
      result -= 1;
    }
    result -= 8;
  }
  return result;
}
/* }}} */

static inline double php_intpow10(int power) {
  static const double powers[] = {
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
    1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
    1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

  /* Not in lookup table */
  if (power < 0 || power > 22) {
    return pow(10.0, (double)power);
  }
  return powers[power];
}

static inline double php_round_helper(double value, int mode) {
  double tmp_value;

  if (value >= 0.0) {
    tmp_value = floor(value + 0.5);
    if ((mode == PHP_ROUND_HALF_DOWN && value == (-0.5 + tmp_value)) ||
        (mode == PHP_ROUND_HALF_EVEN && value == (0.5 + 2 * floor(tmp_value/2.0))) ||
        (mode == PHP_ROUND_HALF_ODD  && value == (0.5 + 2 * floor(tmp_value/2.0) - 1.0)))
    {
      tmp_value = tmp_value - 1.0;
    }
  } else {
    tmp_value = ceil(value - 0.5);
    if ((mode == PHP_ROUND_HALF_DOWN && value == (0.5 + tmp_value)) ||
        (mode == PHP_ROUND_HALF_EVEN && value == (-0.5 + 2 * ceil(tmp_value/2.0))) ||
        (mode == PHP_ROUND_HALF_ODD  && value == (-0.5 + 2 * ceil(tmp_value/2.0) + 1.0)))
    {
      tmp_value = tmp_value + 1.0;
    }
  }

  return tmp_value;
}

double php_math_round(double value, int places,
                      int mode /* = PHP_ROUND_HALF_UP */) {
  double tmp_value;

  if (std::isinf(value)) {
      return value;
  }

  int precision_places = 14 - php_intlog10abs(value);
  double f1 = php_intpow10(abs(places));

  /* If the decimal precision guaranteed by FP arithmetic is higher than
   * the requested places BUT is small enough to make sure a non-zero value
   * is returned, pre-round the result to the precision */
  if (precision_places > places && precision_places - places < 15) {
    double f2 = php_intpow10(abs(precision_places));
    if (precision_places >= 0) {
      tmp_value = value * f2;
    } else {
      tmp_value = value / f2;
    }
    /* preround the result (tmp_value will always be something * 1e14,
     * thus never larger than 1e15 here) */
    tmp_value = php_round_helper(tmp_value, mode);
    /* now correctly move the decimal point */
    f2 = php_intpow10(abs(places - precision_places));
    /* because places < precision_places */
    tmp_value = tmp_value / f2;
  } else {
    /* adjust the value */
    if (places >= 0) {
      tmp_value = value * f1;
    } else {
      tmp_value = value / f1;
    }
    /* This value is beyond our precision, so rounding it is pointless */
    if (fabs(tmp_value) >= 1e15) {
      return value;
    }
  }

  /* round the temp value */
  tmp_value = php_round_helper(tmp_value, mode);

  /* see if it makes sense to use simple division to round the value */
  if (abs(places) < 23) {
    if (places > 0) {
      tmp_value /= f1;
    } else {
      tmp_value *= f1;
    }
  } else {
    /* Simple division can't be used since that will cause wrong results.
     * Instead, the number is converted to a string and back again using
     * strtod(). strtod() will return the nearest possible FP value for
     * that string. */

    /* 40 Bytes should be more than enough for this format string. The
     * float won't be larger than 1e15 anyway. But just in case, use
     * snprintf() and make sure the buffer is zero-terminated */
    char buf[40];
    snprintf(buf, 39, "%15fe%d", tmp_value, -places);
    buf[39] = '\0';
    tmp_value = strtod(buf, nullptr);

    /* couldn't convert to string and back */
    if (std::isinf(tmp_value)) {
      tmp_value = value;
    }
  }

  return tmp_value;
}


///////////////////////////////////////////////////////////////////////////////
}
