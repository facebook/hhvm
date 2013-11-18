/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_MATH_H_
#define incl_HPHP_ZEND_MATH_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PHP_ROUND_HALF_UP 1
#define PHP_ROUND_HALF_DOWN 2
#define PHP_ROUND_HALF_EVEN 3
#define PHP_ROUND_HALF_ODD 4

inline double php_round_helper(double value, int mode) {
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

inline double php_math_round(double value, int places,
                                    int mode = PHP_ROUND_HALF_UP) {
  double tmp_value;

  if (std::isinf(value)) {
      return value;
  }

  int precision_places = 14 - floor(log10(fabs(value)));
  float f1 = pow(10.0, (double)abs(places));

  /* If the decimal precision guaranteed by FP arithmetic is higher than
   * the requested places BUT is small enough to make sure a non-zero value
   * is returned, pre-round the result to the precision */
  if (precision_places > places && precision_places - places < 15) {
    double f2 = pow(10.0, (double)abs(precision_places));
    if (precision_places >= 0) {
      tmp_value = value * f2;
    } else {
      tmp_value = value / f2;
    }
    /* preround the result (tmp_value will always be something * 1e14,
     * thus never larger than 1e15 here) */
    tmp_value = php_round_helper(tmp_value, mode);
    /* now correctly move the decimal point */
    f2 = pow(10.0, (double)abs(places - precision_places));
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

/* System Rand functions */
#ifndef RAND_MAX
#define RAND_MAX (1<<15)
#endif

#define MT_RAND_MAX 0x7FFFFFFFLL

/*
 * A bit of tricky math here.  We want to avoid using a modulus because
 * that simply tosses the high-order bits and might skew the distribution
 * of random values over the range.  Instead we map the range directly.
 *
 * We need to map the range from 0...M evenly to the range a...b
 * Let n = the random number and n' = the mapped random number
 *
 * Then we have: n' = a + n(b-a)/M
 *
 * We have a problem here in that only n==M will get mapped to b which
 # means the chances of getting b is much much less than getting any of
 # the other values in the range.  We can fix this by increasing our range
 # artifically and using:
 #
 #               n' = a + n(b-a+1)/M
 *
 # Now we only have a problem if n==M which would cause us to produce a
 # number of b+1 which would be bad.  So we bump M up by one to make sure
 # this will never happen, and the final algorithm looks like this:
 #
 #               n' = a + n(b-a+1)/(M+1)
 *
 * -RL
 */
#define RAND_RANGE(__n, __min, __max, __tmax) \
  (__n) = (__min) + (int64_t) ((double) ((double)(__max) - (__min) + 1.0) * \
                            ((__n) / ((__tmax) + 1.0)))

#define GENERATE_SEED() \
  (((long) (time(0) * getpid())) ^ ((long) (1000000.0 * math_combined_lcg())))


///////////////////////////////////////////////////////////////////////////////

void math_mt_srand(uint32_t seed);
int64_t math_mt_rand(int64_t min = 0, int64_t max = RAND_MAX);
double math_combined_lcg();
int64_t math_generate_seed();
void zend_get_rand_data();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_MATH_H_
