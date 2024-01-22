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

#pragma once

#include <stdint.h>
#include <cstring>
#include <string_view>

#include "hphp/util/portability.h"

#ifndef HHVM_FACEBOOK
#  include "hphp/util/hphp-config.h"
#endif

#if defined(__x86_64__)
#  include <nmmintrin.h>
#  if (!defined USE_HWCRC)
#    define USE_HWCRC
#  endif
#elif defined __aarch64__ && defined ENABLE_AARCH64_CRC
#  if (!defined USE_HWCRC)
#    define USE_HWCRC
#  endif
#else
#  undef USE_HWCRC
#endif

// Killswitch
#if NO_HWCRC
#  undef USE_HWCRC
#endif

namespace HPHP {

bool IsHWHashSupported();

///////////////////////////////////////////////////////////////////////////////

using strhash_t = int32_t;
using inthash_t = int32_t;
constexpr strhash_t STRHASH_MASK = 0x7fffffff;
constexpr strhash_t STRHASH_MSB  = 0x80000000;

inline size_t hash_int64_fallback(uint64_t key) {
  // "64 bit Mix Functions", from Thomas Wang's "Integer Hash Function."
  // http://www.concentric.net/~ttwang/tech/inthash.htm
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  return static_cast<size_t>(static_cast<uint32_t>(key));
}

ALWAYS_INLINE size_t hash_int64(uint64_t k) {
#if defined(USE_HWCRC) && defined(__SSE4_2__)
  return _mm_crc32_u64(0, k);
#elif defined(USE_HWCRC) && defined(ENABLE_AARCH64_CRC)
  size_t res;
  __asm("crc32cx %w0, wzr, %x1\n" : "=r"(res) : "r"(k));
  return res;
#else
  return hash_int64_fallback(k);
#endif
}


inline size_t hash_int64_pair(uint64_t k1, uint64_t k2) {
#if defined(USE_HWCRC) && defined(__SSE4_2__)
  // crc32 is commutative, so we need to perturb k1 so that (k1, k2) hashes
  // differently from (k2, k1).
  k1 += k1;
  return _mm_crc32_u64(k1, k2);
#elif defined(USE_HWCRC) && defined(ENABLE_AARCH64_CRC)
  size_t res;
  k1 += k1;
  __asm("crc32cx %w0, %w1, %x2\n" : "=r"(res) : "r"(k2), "r"(k1));
  return res;
#else
  return (hash_int64(k1) << 1) ^ hash_int64(k2);
#endif

}

namespace MurmurHash3 {
///////////////////////////////////////////////////////////////////////////////
// The following code is based on MurmurHash3:
//   http://code.google.com/p/smhasher/wiki/MurmurHash3
//
// The case-insensitive version converts lowercase characters to uppercase
// under the assumption that character data are 7-bit ASCII. This should work
// as identifiers usually only contain alphanumeric characters and the
// underscore. Although PHP allows higher ASCII characters (> 127) in an
// identifier, they should be very rare, and do not change the correctness.

#define ROTL64(x,y) rotl64(x,y)
#define BIG_CONSTANT(x) (x##LLU)

ALWAYS_INLINE uint64_t rotl64(uint64_t x, int8_t r) {
  return (x << r) | (x >> (64 - r));
}

template <bool caseSensitive>
ALWAYS_INLINE uint64_t getblock64(const uint8_t *p) {
  uint64_t block;
  // Read 64 bits without violating alias rules. Compiler will optimize the
  // `memcpy` away.
  std::memcpy(&block, p, sizeof(block));
  if (!caseSensitive) {
    block &= 0xdfdfdfdfdfdfdfdfLLU; // a-z => A-Z
  }
  return block;
}

template <bool caseSensitive>
ALWAYS_INLINE uint8_t getblock8(const uint8_t *p, int i) {
  uint8_t block = p[i];
  if (!caseSensitive) {
    block &= 0xdfU; // a-z => A-Z
  }
  return block;
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
ALWAYS_INLINE uint64_t fmix64(uint64_t k) {
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;
  return k;
}

// Optimized for 64-bit architectures.  MurmurHash3 also implements a 128-bit
// hash that is optimized for 32-bit architectures (omitted here).
template <bool caseSensitive>
ALWAYS_INLINE void hash128(const void *key, size_t len, uint64_t seed,
                           uint64_t out[2]) {
  const uint8_t *data = (const uint8_t *)key;
  const size_t nblocks = len / 16;
  uint64_t h1 = seed;
  uint64_t h2 = seed;
  const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
  const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

  //----------
  // body
  const uint8_t * head = data;
  const uint8_t * tail = data + nblocks*16;
  for(const uint8_t *b = head; b < tail; b += 16)
  {
    uint64_t k1 = getblock64<caseSensitive>(b);
    uint64_t k2 = getblock64<caseSensitive>(b + 8);
    k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
    k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
    h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
  }

  //----------
  // tail
  uint64_t k1 = 0;
  uint64_t k2 = 0;
  switch(len & 15)
  {
  case 15: k2 ^= uint64_t(getblock8<caseSensitive>(tail, 14)) << 48;
  case 14: k2 ^= uint64_t(getblock8<caseSensitive>(tail, 13)) << 40;
  case 13: k2 ^= uint64_t(getblock8<caseSensitive>(tail, 12)) << 32;
  case 12: k2 ^= uint64_t(getblock8<caseSensitive>(tail, 11)) << 24;
  case 11: k2 ^= uint64_t(getblock8<caseSensitive>(tail, 10)) << 16;
  case 10: k2 ^= uint64_t(getblock8<caseSensitive>(tail,  9)) << 8;
  case  9: k2 ^= uint64_t(getblock8<caseSensitive>(tail,  8)) << 0;
           k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

  case  8: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  7)) << 56;
  case  7: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  6)) << 48;
  case  6: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  5)) << 40;
  case  5: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  4)) << 32;
  case  4: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  3)) << 24;
  case  3: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  2)) << 16;
  case  2: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  1)) << 8;
  case  1: k1 ^= uint64_t(getblock8<caseSensitive>(tail,  0)) << 0;
           k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
  }

  //----------
  // finalization
  h1 ^= len; h2 ^= len;
  h1 += h2;
  h2 += h1;
  h1 = fmix64(h1);
  h2 = fmix64(h2);
  h1 += h2;
  h2 += h1;

  ((uint64_t*)out)[0] = h1;
  ((uint64_t*)out)[1] = h2;
}

#undef ROTL64
#undef BIG_CONSTANT
///////////////////////////////////////////////////////////////////////////////
} // namespace MurmurHash3

// Four functions for hashing: hash_string_(cs|i)(_unsafe)?.
//   cs: case-sensitive;
//   i: case-insensitive;
//   unsafe: safe for strings aligned at 8-byte boundary;

#if defined USE_HWCRC && (defined __SSE4_2__ || defined ENABLE_AARCH64_CRC)

// We will surely use CRC32, these are implemented directly in hash-crc-*.S
strhash_t hash_string_cs_unsafe(const char *arKey, uint32_t nKeyLength);
strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength);
strhash_t hash_string_cs(const char *arKey, uint32_t nKeyLength);
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength);
#else

strhash_t hash_string_cs_fallback(const char*, uint32_t);
strhash_t hash_string_i_fallback(const char*, uint32_t);

// We may need to do CPUID checks in the fallback versions, only when we are not
// sure CRC hash is used.
inline strhash_t hash_string_cs(const char *arKey, uint32_t nKeyLength) {
  return hash_string_cs_fallback(arKey, nKeyLength);
}

inline
strhash_t hash_string_cs_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_cs_fallback(arKey, nKeyLength);
}

inline strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_fallback(arKey, nKeyLength);
}

inline
strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_fallback(arKey, nKeyLength);
}

#endif

// Convenience wrapper for std::string_view
inline strhash_t hash_string_cs(std::string_view s) {
  return hash_string_cs(s.data(), s.size());
}

// These functions implement hashing in software. And will return the same thing
// between different hardware
strhash_t hash_string_cs_software(const char*, uint32_t);
strhash_t hash_string_i_software(const char*, uint32_t);

// This function returns true and sets the res parameter if arKey
// is a non-empty string that matches one of the following conditions:
//   1) The string is "0".
//   2) The string starts with a non-zero digit, followed by at most
//      18 more digits, and is less than or equal to 2^63 - 1.
//   3) The string starts with a negative sign, followed by a non-zero
//      digit, followed by at most 18 more digits, and is greater than
//      or equal to -2^63.
inline bool is_strictly_integer(const char* arKey, size_t nKeyLength,
                                int64_t& res) {
  if ((unsigned char)(arKey[0] - '-') > ('9' - '-'))
    return false;
  if (nKeyLength <= 19 ||
      (arKey[0] == '-' && nKeyLength == 20)) {
    unsigned long long num = 0;
    bool neg = false;
    unsigned i = 0;
    if (arKey[0] == '-') {
      neg = true;
      i = 1;
      // The string "-" is NOT strictly an integer
      if (nKeyLength == 1)
        return false;
      // A string that starts with "-0" is NOT strictly an integer
      if (arKey[1] == '0')
        return false;
    } else if (arKey[0] == '0') {
      // The string "0" is strictly an integer
      if (nKeyLength == 1) {
        res = 0;
        return true;
      }
      // A string that starts with "0" followed by at least one digit
      // is NOT strictly an integer
      return false;
    }
    bool good = true;
    for (; i < nKeyLength; ++i) {
      if (arKey[i] >= '0' && arKey[i] <= '9') {
        num = 10*num + (arKey[i] - '0');
      }
      else {
        good = false;
        break;
      }
    }
    if (good) {
      if (num <= 0x7FFFFFFFFFFFFFFFULL ||
          (neg && num == 0x8000000000000000ULL)) {
        res = neg ? 0 - num : (long long)num;
        return true;
      }
    }
  }
  return false;
}

struct StringData;

///////////////////////////////////////////////////////////////////////////////

// Map an integer into one of N buckets. The mapping is calculated
// such that increasing or decreasing the number of buckets causes
// minimal disturbance to the mapping. (Most keys will continue to map
// to the same bucket they did previously). Salt can be used to
// calculate different mappings for the same set of keys.
size_t consistent_hash(int64_t key, size_t buckets, int64_t salt = 0);

///////////////////////////////////////////////////////////////////////////////

}

#if defined(USE_HWCRC) && !defined(__SSE4_2__)
// The following functions are implemented in ASM directly for x86_64 and ARM
extern "C" {
  HPHP::strhash_t hash_string_cs_crc(const char*, uint32_t);
  HPHP::strhash_t hash_string_i_crc(const char*, uint32_t);
  HPHP::strhash_t hash_string_cs_unaligned_crc(const char*, uint32_t);
  HPHP::strhash_t hash_string_i_unaligned_crc(const char*, uint32_t);
}
#endif
