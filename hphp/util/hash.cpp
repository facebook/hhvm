/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

bool IsSSEHashSupported() {
#ifdef USE_SSECRC
  static folly::CpuId cpuid;
  return cpuid.sse42();
#else
  return false;
#endif
}

NEVER_INLINE
strhash_t hash_string_cs_fallback(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_SSECRC
  if (IsSSEHashSupported()) {
    return hash_string_cs_unaligned_crc(arKey, nKeyLength);
  }
#endif
  if (MurmurHash3::useHash128) {
    uint64_t h[2];
    MurmurHash3::hash128<true>(arKey, nKeyLength, 0, h);
    return strhash_t(h[0] & STRHASH_MASK);
  } else {
    uint32_t h = MurmurHash3::hash32<true>(arKey, nKeyLength, 0);
    return strhash_t(h & STRHASH_MASK);
  }
}

NEVER_INLINE
strhash_t hash_string_i_fallback(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_SSECRC
  if (IsSSEHashSupported()) {
    return hash_string_i_unaligned_crc(arKey, nKeyLength);
  }
#endif
  if (MurmurHash3::useHash128) {
    uint64_t h[2];
    MurmurHash3::hash128<false>(arKey, nKeyLength, 0, h);
    return strhash_t(h[0] & STRHASH_MASK);
  } else {
    uint32_t h = MurmurHash3::hash32<false>(arKey, nKeyLength, 0);
    return strhash_t(h & STRHASH_MASK);
  }
}

#if FACEBOOK && defined USE_SSECRC

NEVER_INLINE
strhash_t hash_string_cs(const char *arKey, uint32_t nKeyLength) {
  return hash_string_cs_fallback(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_fallback(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_cs_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_cs(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i(arKey, nKeyLength);
}

#endif

}
