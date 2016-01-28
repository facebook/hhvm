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

#ifndef incl_HPHP_ZEND_MATH_H_
#define incl_HPHP_ZEND_MATH_H_

#include <cmath>
#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PHP_ROUND_HALF_UP 1
#define PHP_ROUND_HALF_DOWN 2
#define PHP_ROUND_HALF_EVEN 3
#define PHP_ROUND_HALF_ODD 4

double php_math_round(double value, int places, int mode = PHP_ROUND_HALF_UP);

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
