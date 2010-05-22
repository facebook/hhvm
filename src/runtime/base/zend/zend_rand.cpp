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

#include <runtime/base/zend/zend_math.h>
#include <util/thread_local.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/* MT RAND FUNCTIONS */

/*
  The following php_mt_...() functions are based on a C++ class MTRand by
  Richard J. Wagner. For more information see the web page at
  http://www-personal.engin.umich.edu/~wagnerr/MersenneTwister.html

  Mersenne Twister random number generator -- a C++ class MTRand
  Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
  Richard J. Wagner  v1.0  15 May 2003  rjwagner@writeme.com

  The Mersenne Twister is an algorithm for generating random numbers.  It
  was designed with consideration of the flaws in various other generators.
  The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
  are far greater.  The generator is also fast; it avoids multiplication and
  division, and it benefits from caches and pipelines.  For more information
  see the inventors' web page at http://www.math.keio.ac.jp/~matumoto/emt.html

  Reference
  M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
  Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
  Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

  Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
  Copyright (C) 2000 - 2003, Richard J. Wagner
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  3. The names of its contributors may not be used to endorse or promote
     products derived from this software without specific prior written
     permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The original code included the following notice:

  When you use this, send an email to: matumoto@math.keio.ac.jp
    with an appropriate reference to your work.

  It would be nice to CC: rjwagner@writeme.com and Cokus@math.washington.edu
  when you write.
*/

#define MT_N          (624)
#define N             MT_N                 // length of state vector
#define M             (397)                // a period parameter
#define hiBit(u)      ((u) & 0x80000000U)  // mask all but highest   bit of u
#define loBit(u)      ((u) & 0x00000001U)  // mask all but lowest    bit of u
#define loBits(u)     ((u) & 0x7FFFFFFFU)  // mask     the highest   bit of u
#define mixBits(u, v) (hiBit(u)|loBits(v)) // move hi bit of u to hi bit of v

#define twist(m,u,v) \
  (m ^ (mixBits(u,v)>>1) ^ ((uint32)(-(int32)(loBit(u))) & 0x9908b0dfU))

class RandData {
public:
  RandData()
    : seeded(false), left(0), next(NULL),
      lcg_seeded(false), lcg_s1(0), lcg_s2(0) {
    memset(state, 0, sizeof(state));
  }

  bool seeded;
  int left;
  uint32 state[N];
  uint32 *next;

  bool lcg_seeded;
  int32 lcg_s1;
  int32 lcg_s2;
};
static IMPLEMENT_THREAD_LOCAL(RandData, s_rand_data);

static inline void php_mt_initialize(uint32 seed, uint32 *state) {
  /* Initialize generator state with seed
     See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
     In previous versions, most significant bits (MSBs) of the seed affect
     only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto. */

  register uint32 *s = state;
  register uint32 *r = state;
  register int i = 1;

  *s++ = seed & 0xffffffffU;
  for( ; i < N; ++i ) {
    *s++ = ( 1812433253U * ( *r ^ (*r >> 30) ) + i ) & 0xffffffffU;
    r++;
  }
}

static inline void php_mt_reload() {
  /* Generate N new values in state
     Made clearer and faster by Matthew Bellew (matthew.bellew@home.com) */

  RandData *data = s_rand_data.get();
  register uint32 *state = data->state;
  register uint32 *p = state;
  register int i;

  for (i = N - M; i--; ++p)
    *p = twist(p[M], p[0], p[1]);
  for (i = M; --i; ++p)
    *p = twist(p[M-N], p[0], p[1]);
  *p = twist(p[M-N], p[0], state[0]);
  data->left = N;
  data->next = state;
}

void math_mt_srand(uint32 seed) {
  RandData *data = s_rand_data.get();

  /* Seed the generator with a simple uint32 */
  php_mt_initialize(seed, data->state);
  php_mt_reload();

  /* Seed only once */
  data->seeded = true;
}

static inline uint32 php_mt_rand() {
  // Pull a 32-bit integer from the generator state
  // Every other access function simply transforms the numbers extracted here

  register uint32 s1;

  RandData *data = s_rand_data.get();
  if (data->left == 0) {
    php_mt_reload();
  }
  --data->left;

  s1 = *data->next++;
  s1 ^= (s1 >> 11);
  s1 ^= (s1 <<  7) & 0x9d2c5680U;
  s1 ^= (s1 << 15) & 0xefc60000U;
  return ( s1 ^ (s1 >> 18) );
}

// Returns a random number from Mersenne Twister
long math_mt_rand(long min /* = 0 */, long max /* = RAND_MAX */) {
  if (!s_rand_data->seeded) {
    math_mt_srand(GENERATE_SEED());
  }

  /*
   * Melo: hmms.. randomMT() returns 32 random bits...
   * Yet, the previous php_rand only returns 31 at most.
   * So I put a right shift to loose the lsb. It *seems*
   * better than clearing the msb.
   * Update:
   * I talked with Cokus via email and it won't ruin the algorithm
   */
  long number = (php_mt_rand() >> 1);
  if (min != 0 || max != RAND_MAX) {
    RAND_RANGE(number, min, max, MT_RAND_MAX);
  }
  return number;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * combinedLCG() returns a pseudo random number in the range of (0, 1).
 * The function combines two CGs with periods of
 * 2^31 - 85 and 2^31 - 249. The period of this function
 * is equal to the product of both primes.
 */

#define MODMULT(a, b, c, m, s) q = s/a;s=b*(s-a*q)-c*q;if(s<0)s+=m

static void lcg_seed() {
  RandData *data = s_rand_data.get();
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0) {
    data->lcg_s1 = tv.tv_sec ^ (~tv.tv_usec);
  } else {
    data->lcg_s1 = 1;
  }
  data->lcg_s2 = (long)getpid();
  data->lcg_seeded = true;
}

double math_combined_lcg() {
  int32 q;
  int32 z;

  RandData *data = s_rand_data.get();
  if (!data->lcg_seeded) {
    lcg_seed();
  }

  MODMULT(53668, 40014, 12211, 2147483563L, data->lcg_s1);
  MODMULT(52774, 40692, 3791, 2147483399L, data->lcg_s2);

  z = data->lcg_s1 - data->lcg_s2;
  if (z < 1) {
    z += 2147483562;
  }

  return z * 4.656613e-10;
}

///////////////////////////////////////////////////////////////////////////////
}
