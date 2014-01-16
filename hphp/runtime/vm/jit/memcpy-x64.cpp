/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "folly/Portability.h"

// ASan is less precise than valgrind and believes this function overruns reads
#if defined(__x86_64__) && !defined(FOLLY_SANITIZE_ADDRESS)

#include <emmintrin.h>
#include <stdint.h>

#include "hphp/util/util.h"
#include "hphp/util/assertions.h"

extern "C" {

void*
memcpy(void* vdest, const void* vsrc, size_t len) {
  auto src  = (const char*)vsrc;
  auto dest = (char*) vdest;

  // We do a lot of small memcpy's, so we don't want to neglect their
  // performance. For smaller-than-a-cacheline entities, just use
  // regular loads and stores.
  if (len & 0x3f) {
    unsigned nBytes = len & 7;
    switch (nBytes) {
    case 7: dest[6] = src[6];
    case 6: dest[5] = src[5];
    case 5: dest[4] = src[4];
    case 4: dest[3] = src[3];
    case 3: dest[2] = src[2];
    case 2: dest[1] = src[1];
    case 1: dest[0] = src[0];
    case 0: break;
    }
    len -= nBytes;
    dest += nBytes;
    src += nBytes;
    assert((len & 7) == 0);
    unsigned nQwordBytes = len & 0x3f;
    auto qdest = (uint64_t*)dest;
    auto qsrc = (const uint64_t*)src;
    unsigned nQwords = nQwordBytes >> 3;
    switch (nQwords) {
    case 7: qdest[6] = qsrc[6];
    case 6: qdest[5] = qsrc[5];
    case 5: qdest[4] = qsrc[4];
    case 4: qdest[3] = qsrc[3];
    case 3: qdest[2] = qsrc[2];
    case 2: qdest[1] = qsrc[1];
    case 1: qdest[0] = qsrc[0];
    case 0: break;
    }
    len -= nQwordBytes;
    dest += nQwordBytes;
    src += nQwordBytes;
  }

  // Do the bulk with fat loads/stores.
  assert((len & 0x3f) == 0);
  while (len) {
    auto dqdest = (__m128i*)dest;
    auto dqsrc = (__m128i*)src;
    __m128i xmm0 = _mm_loadu_si128(dqsrc + 0);
    __m128i xmm1 = _mm_loadu_si128(dqsrc + 1);
    __m128i xmm2 = _mm_loadu_si128(dqsrc + 2);
    __m128i xmm3 = _mm_loadu_si128(dqsrc + 3);
    len -= 64;
    dest += 64;
    src += 64;
    _mm_storeu_si128(dqdest + 0, xmm0);
    _mm_storeu_si128(dqdest + 1, xmm1);
    _mm_storeu_si128(dqdest + 2, xmm2);
    _mm_storeu_si128(dqdest + 3, xmm3);
  }
  return vdest;
}

}
#endif // __x86_64__

