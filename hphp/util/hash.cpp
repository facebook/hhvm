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

#include "hphp/util/assertions.h"

#include <folly/CpuId.h>

#include <random>

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
  return hash_string_cs_software(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i_fallback(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_HWCRC
  if (IsHWHashSupported()) {
    return hash_string_i_unaligned_crc(arKey, nKeyLength);
  }
#endif
  return hash_string_i_software(arKey, nKeyLength);
}

#endif

strhash_t hash_string_cs_software(const char *arKey, uint32_t nKeyLength) {
  uint64_t h[2];
  MurmurHash3::hash128<true>(arKey, nKeyLength, 0, h);
  return strhash_t(h[0] & STRHASH_MASK);
}

strhash_t hash_string_i_software(const char *arKey, uint32_t nKeyLength) {
  uint64_t h[2];
  MurmurHash3::hash128<false>(arKey, nKeyLength, 0, h);
  return strhash_t(h[0] & STRHASH_MASK);
}

///////////////////////////////////////////////////////////////////////////////

// This hash is based on "A Fast, Minimal Memory, Consistent Hash
// Algorithm" by John Lamping and Eric Veach.
size_t consistent_hash(int64_t key, size_t buckets, int64_t salt) {
  assertx(buckets > 0);
  assertx(buckets < std::numeric_limits<size_t>::max());
  std::seed_seq seed{key, salt};
  std::minstd_rand gen{seed};
  size_t b;
  size_t j = 0;
  do {
    b = j;
    auto const r =
      std::generate_canonical<double, std::numeric_limits<double>::digits>(gen);
    j = std::floor((b + 1) / r);
  } while (j < buckets);
  return b;
}

///////////////////////////////////////////////////////////////////////////////

}
