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

#ifndef __HPHP_ZEND_MATH_H__
#define __HPHP_ZEND_MATH_H__

#include <util/base.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifndef PHP_ROUND_FUZZ
# ifndef PHP_WIN32
#  define PHP_ROUND_FUZZ 0.50000000001
# else
#  define PHP_ROUND_FUZZ 0.5
# endif
#endif

#define PHP_ROUND_WITH_FUZZ(val, places) {			\
    double tmp_val=val, f = pow(10.0, (double) places);         \
    tmp_val *= f;                                               \
    if (tmp_val >= 0.0) {                                       \
      tmp_val = floor(tmp_val + PHP_ROUND_FUZZ);                \
    } else {                                                    \
      tmp_val = ceil(tmp_val - PHP_ROUND_FUZZ);                 \
    }                                                           \
    tmp_val /= f;                                               \
    val = !isnan(tmp_val) ? tmp_val : val;                      \
  }                                                             \

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
  (__n) = (__min) + (long) ((double) ((double)(__max) - (__min) + 1.0) * \
                            ((__n) / ((__tmax) + 1.0)))

#define GENERATE_SEED() \
  (long) (time(0) * getpid() * 1000000 * math_combined_lcg())

///////////////////////////////////////////////////////////////////////////////

void math_mt_srand(uint32 seed);
long math_mt_rand(long min = 0, long max = RAND_MAX);
double math_combined_lcg();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_MATH_H__
