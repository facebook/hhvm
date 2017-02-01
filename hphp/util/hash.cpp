/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/util/hash.h"
#include <folly/CpuId.h>

namespace HPHP {

bool IsHWHashSupported() {
#ifdef USE_HWCRC
#if defined __SSE4_2__
  return true;
#else
  static folly::CpuId cpuid;
  return cpuid.sse42();
#endif
#else
  return false;
#endif
}

#if !defined(USE_HWCRC) || !(defined(__SSE4_2__) || \
                             defined(ENABLE_AARCH64_CRC))
NEVER_INLINE
strhash_t hash_string_cs_fallback(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_HWCRC
  if (IsHWHashSupported()) {
    return hash_string_cs_unaligned_crc(arKey, nKeyLength);
  }
#endif
  uint64_t h[2];
  MurmurHash3::hash128<true>(arKey, nKeyLength, 0, h);
  return strhash_t(h[0] & STRHASH_MASK);
}

NEVER_INLINE
strhash_t hash_string_i_fallback(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_HWCRC
  if (IsHWHashSupported()) {
    return hash_string_i_unaligned_crc(arKey, nKeyLength);
  }
#endif
  uint64_t h[2];
  MurmurHash3::hash128<false>(arKey, nKeyLength, 0, h);
  return strhash_t(h[0] & STRHASH_MASK);
}

#endif

}
