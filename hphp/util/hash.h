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

#ifndef incl_HPHP_HASH_H_
#define incl_HPHP_HASH_H_

#include <stdint.h>
#include <cstring>

#include "hphp/util/portability.h"

#if defined __x86_64__
#  if (!defined USE_SSECRC) && (defined FACEBOOK)
#    define USE_SSECRC
#  endif
#else
#  undef USE_SSECRC
#endif

// Killswitch
#if NO_SSECRC
#  undef USE_SSECRC
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef int32_t strhash_t;
const strhash_t STRHASH_MASK = 0x7fffffff;
const strhash_t STRHASH_MSB  = 0x80000000;

inline long long hash_int64(long long key) {
#ifdef USE_SSECRC
  long long res = 0;
  __asm__("crc32q %1, %0\n" : "+r" (res) : "rm" (key));
  // Upper 32 bits are zeros here
  return res;
#else
  // "64 bit Mix Functions", from Thomas Wang's "Integer Hash Function."
  // http://www.concentric.net/~ttwang/tech/inthash.htm
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ ((unsigned long long)key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ ((unsigned long long)key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ ((unsigned long long)key >> 28);
  key = key + (key << 31);
  return key < 0 ? -key : key;
#endif
}

inline long long hash_int64_pair(long long k1, long long k2) {
  // Shift the first key, so (a,b) hashes somewhere other than (b,a)
  return (hash_int64(k1) << 1) ^ hash_int64(k2);
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

// It is tempting to make useHash128 depend on whether the architecture is 32-
// or 64-bit, but changing which hash function is used also requires
// regenerating system files, so this setting is hardcoded here.
const bool useHash128 = true;

#define ROTL32(x,y) rotl32(x,y)
#define ROTL64(x,y) rotl64(x,y)
#define BIG_CONSTANT(x) (x##LLU)

ALWAYS_INLINE uint64_t rotl64(uint64_t x, int8_t r) {
  return (x << r) | (x >> (64 - r));
}

ALWAYS_INLINE uint32_t rotl32(uint32_t x, int8_t r) {
  return (x << r) | (x >> (32 - r));
}

template <bool caseSensitive>
ALWAYS_INLINE uint64_t getblock64(const uint64_t *p, int i) {
  uint64_t block = p[i];
  if (!caseSensitive) {
    block &= 0xdfdfdfdfdfdfdfdfLLU; // a-z => A-Z
  }
  return block;
}

template <bool caseSensitive>
ALWAYS_INLINE uint32_t getblock32(const uint32_t *p, int i) {
  uint32_t block = p[i];
  if (!caseSensitive) {
    block &= 0xdfdfdfdfU; // a-z => A-Z
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

ALWAYS_INLINE uint32_t fmix32(uint32_t h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

template <bool caseSensitive>
ALWAYS_INLINE uint32_t hash32(const void *key, size_t len, uint32_t seed) {
  const uint8_t *data = (const uint8_t *)key;
  const size_t nblocks = len / 4;
  uint32_t h1 = seed;
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  //----------
  // body
  const uint32_t *blocks = (const uint32_t *)(data + nblocks*4);
  for(size_t i = -nblocks; i; i++) {
    uint32_t k1 = getblock32<caseSensitive>(blocks, i);
    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;
    h1 ^= k1;
    h1 = ROTL32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  //----------
  // tail
  const uint8_t *tail = (const uint8_t*)(data + nblocks*4);
  uint32_t k1 = 0;
  switch(len & 3) {
  case 3: k1 ^= getblock8<caseSensitive>(tail, 2) << 16;
  case 2: k1 ^= getblock8<caseSensitive>(tail, 1) << 8;
  case 1: k1 ^= getblock8<caseSensitive>(tail, 0);
          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization
  h1 ^= len;
  h1 = fmix32(h1);

  return h1;
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
  const uint64_t *blocks = (const uint64_t *)(data);
  for(size_t i = 0; i < nblocks; i++)
  {
    uint64_t k1 = getblock64<caseSensitive>(blocks,i*2+0);
    uint64_t k2 = getblock64<caseSensitive>(blocks,i*2+1);
    k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
    k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
    h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
  }

  //----------
  // tail
  const uint8_t *tail = (const uint8_t*)(data + nblocks*16);
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
  };

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

#undef ROTL32
#undef ROTL64
#undef BIG_CONSTANT
///////////////////////////////////////////////////////////////////////////////
} // namespace MurmurHash3

#ifdef USE_SSECRC

// Returns a 31-bit case-sensitive hash value for a string. str[count] does not
// affect the result. The function returns nonzero when count is 0. When not
// inlined, the generated code fits in 64 bytes. When inlined, it offers plenty
// of flexibility on register allocation, except that %rcx will have to be used.
// Warning: this function could read beyond [0, count). Thus it could crash if
// the address goes to an invalid page. The following needs to hold before you
// call this, (str + count + 7) / PAGESIZE == (str + count) / PAGESIZE.
// Sufficient conditions include (str & 7 == 0) || (count & 7 == 0).
inline uint32_t FOLLY_DISABLE_ADDRESS_SANITIZER
crc8_cs_unsafe(const char* str, uint32_t count) {
  uint64_t result = 0xffffffffu;
  uint64_t data;
  __asm__(
    "test %2, %2\n\t"                   // if (count == 0)
    "jz _hcsend%=\n"                    //   goto hashend;
    "_hcsl8%=:\n\t"                     // while (true) {
    "mov (%1), %3\n\t"                  //   data = *(uint64_t*)str;
    "sub $8, %2\n\t"                    //   count -= 8;
    "jle _hcstail%=\n\t"                //   if (count <= 0) break;
    "addq $8, %1\n\t"                   //   str += 8;
    "crc32q %3, %0\n"                   //   result = crc32(result, data);
    "jmp _hcsl8%=\n"                    // }
    "_hcstail%=:\n\t"                   // assert(-7 <= count <= 0)
    "shl $3, %2\n\t"                    // count *= 8; // [-56, 0]
    "neg %2\n\t"                        // count = -count;
    "shl %%cl, %3\n\t"                  // data <<= count;
    "crc32q %3, %0\n"                   // result = crc32(result, data);
    "_hcsend%=:\n\t"                    // hashend:
    : "+r" (result), "+r" (str), "+c" (count), "=r" (data)
  );
  return uint32_t(result) >> 1;
}

// Returns a 31-bit case-insensitive hash value for a string. str[count] does
// not affect the result. The function returns nonzero when count is 0. When not
// inlined, the generated code fits in 64 bytes. When inlined, it offers plenty
// of flexibility on register allocation, except that %rcx will have to be used.
// Warning: this function could read beyond [0, count). Thus it could crash if
// the address goes to an invalid page. Sufficient conditions for safety
// guarantee include (str & 7 == 0) || (count & 7 == 0).
inline uint32_t FOLLY_DISABLE_ADDRESS_SANITIZER
crc8_i_unsafe(const char* str, uint32_t count) {
  uint64_t result;
  uint64_t data;
  // The code is carefully optimized such that HPHP::StringData::hashHelper()
  // fits in a cache line when this function is inlined there. Currently it is
  // exactly 64 bytes, so any increase in code size is not quite tolerable.
  __asm__(
    "xor %%eax, %%eax\n\t"              // result = 0;
    "not %%eax\n\t"                     // result = 0xffffffff;
    "test %2, %2\n\t"                   // if (count == 0)
    "jz _hashend%=\n"                   //   goto hashend;
    "_hload8%=:\n\t"                    // while (true) {
    "movq $0xdfdfdfdfdfdfdfdf, %3\n\t"  //   data = MASK
    "andq (%1), %3\n\t"                 //   data &= *(uint64_t*)str;
    "sub $8, %2\n\t"                    //   count -= 8;
    "jle _htail%=\n\t"                  //   if (count <= 0) break;
    "addq $8, %1\n\t"                   //   str += 8;
    "crc32q %3, %0\n"                   //   result = crc32(result, data);
    "jmp _hload8%=\n"                   // }
    "_htail%=:\n\t"                     // assert(-7 <= count <= 0)
    "shl $3, %2\n\t"                    // count *= 8; // [-56, 0]
    "neg %2\n\t"                        // count = -count;
    "shl %%cl, %3\n\t"                  // data <<= count;
    "crc32q %3, %0\n"                   // result = crc32(result, data);
    "_hashend%=:\n\t"                   // hashend:
    : "=a" (result), "+r" (str), "+c" (count), "=r" (data)
  );
  return uint32_t(result) >> 1;
}

// This is the safe version of crc8_cs_unsafe, it won't read str[count]. It is
// slower than the unsafe version, but still fits in a 64-byte cache line when
// not inlined. Use it when you are not sure.
inline uint32_t crc8_cs_safe(const char* str, uint32_t count) {
  uint64_t result = 0xffffffff;
  uint64_t temp;
  __asm__(
    "_hcil8%=:\n\t"                     // while (true) {
    "subl $8, %2\n\t"                   //   count -= 8;
    "js _hcitail%=\n\t"                 //   if (count < 0) break;
    "movq (%1), %3\n\t"                 //   data &= *(uint64_t*)str;
    "addq $8, %1\n\t"                   //   str += 8;
    "crc32q %3, %0\n"                   //   result = crc32(result, data);
    "jmp _hcil8%=\n"                    // }
    "_hcitail%=:\n\t"                   // assert(-8 <= count < 0);
    "add $8, %2\n\t"                    // count += 8;
    "jz _hciend%=\n\t"                  // if (count == 0) goto hcsend;
    "xor %%edx, %%edx\n"                // data = 0;
    "_hcil1%=:\n\t"                     // hcil1:
    "movb (%1), %%dl\n\t"               // data = data | *str
    "inc %1\n\t"                        // ++str;
    "rorq $8, %%rdx\n\t"                // newly read byte -> MSB
    "loop _hcil1%=\n\t"                 // if (--count != 0) goto hcil1;
    "crc32q %3, %0\n"                   // result = crc32(result, data);
    "_hciend%=:\n\t"                    // hciend
    : "+r" (result), "+r" (str), "+c" (count), "=d" (temp)
  );
  return uint32_t(result) >> 1;
}

// This is the safe version of crc8_i_unsafe, it won't read str[count]. It is
// slower than the unsafe version, and needs the caller to fill %rdx with the
// case mask to get case insensitivity (you could pass uint64_t{-1LL} to get a
// case sensitive version if you want the two versions to share code). When not
// inlined, the code fits exactly in a cache line.
inline uint32_t crc8_i_safe(const char* str, uint32_t count, uint64_t mask) {
  // %rdx: data/initial mask passed in
  // %r8: copy of mask
  // %rsi: count
  uint64_t result = 0xffffffff;
  __asm__(
    "movq %3, %%r8\n"                   // mask = data;
    "_hcil8%=:\n\t"                     // while (true) {
    "subl $8, %2\n\t"                   //   count -= 8;
    "js _hcitail%=\n\t"                 //   if (count < 0) break;
    "andq (%1), %3\n\t"                 //   data &= *(uint64_t*)str;
    "addq $8, %1\n\t"                   //   str += 8;
    "crc32q %3, %0\n"                   //   result = crc32(result, data);
    "movq %%r8, %3\n\t"                 //   data = mask;
    "jmp _hcil8%=\n"                    // }
    "_hcitail%=:\n\t"                   // assert(-8 <= count < 0);
    "add $8, %%esi\n\t"                 // count += 8;
    "jz _hciend%=\n\t"                  // if (count == 0) goto hcsend;
    "mov %%esi, %%ecx\n"                // ecx = count;
    "xor %%edx, %%edx\n"                // data = 0;
    "_hcil1%=:\n\t"                     // hcil1:
    "movb (%1), %%dl\n\t"               // data = data | *str
    "inc %1\n\t"                        // ++str;
    "rorq $8, %%rdx\n\t"                // newly read byte -> MSB
    "loop _hcil1%=\n\t"                 // if (--count != 0) goto hcil1;
    "andq %%r8, %%rdx\n\t"              // data &= mask;
    "crc32q %%rdx, %0\n"                // result = crc32(result, data);
    "_hciend%=:\n\t"                    // hciend
    : "+r" (result), "+r" (str), "+S" (count), "+d" (mask)
    :
    : "%rcx", "%r8"
  );
  return uint32_t(result) >> 1;
}

#endif

inline strhash_t hash_string_cs(const char *arKey, int nKeyLength) {
#ifdef USE_SSECRC
  uint32_t r = crc8_cs_safe(arKey, nKeyLength);
  return strhash_t(r);
#else
   if (MurmurHash3::useHash128) {
    uint64_t h[2];
    MurmurHash3::hash128<true>(arKey, nKeyLength, 0, h);
    return strhash_t(h[0] & STRHASH_MASK);
  } else {
    uint32_t h = MurmurHash3::hash32<true>(arKey, nKeyLength, 0);
    return strhash_t(h & STRHASH_MASK);
  }
#endif
}

// Eight functions, in the form of hash_string(_i)?(_inline)?(_unsafe)?.
// _i: whether case sensitive or not, actually in current implementation,
// hash_string* always calls hash_string_i*.
// _inline: whether the function should be inlined, sometimes it is not quite
// strictly followed, in fact only one or two functions are not inlined among
// the eight.
// _unsafe: whether reading beyond the end is impossible or tolerable.
// Safe version could call unsafe version after checking when using CRC.
// Unsafe version is an alias of the safe version when not using CRC.

#ifdef USE_SSECRC
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength,
                        uint64_t mask = 0xdfdfdfdfdfdfdfdfull);

inline strhash_t hash_string(const char *arKey, uint32_t nKeyLength,
                             uint64_t mask = 0xdfdfdfdfdfdfdfdfull){
  return hash_string_i(arKey, nKeyLength, mask);
}
#else
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength);

inline strhash_t hash_string(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i(arKey, nKeyLength);
}
#endif

#ifdef USE_SSECRC
// The function is not inlined when using CRC.
strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength);
#else
inline strhash_t hash_string_i_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i(arKey, nKeyLength);
}
#endif

inline strhash_t hash_string_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_unsafe(arKey, nKeyLength);
}

inline strhash_t
hash_string_i_inline_unsafe(const char *arKey, uint32_t nKeyLength) {
#ifdef USE_SSECRC
  return crc8_i_unsafe(arKey, nKeyLength);
#else
  if (MurmurHash3::useHash128) {
    uint64_t h[2];
    MurmurHash3::hash128<false>(arKey, nKeyLength, 0, h);
    return strhash_t(h[0] & STRHASH_MASK);
  } else {
    uint32_t h = MurmurHash3::hash32<false>(arKey, nKeyLength, 0);
    return strhash_t(h & STRHASH_MASK);
  }
#endif
}

inline strhash_t
hash_string_inline_unsafe(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_inline_unsafe(arKey, nKeyLength);
}

#ifdef USE_SSECRC
inline strhash_t hash_string_i_inline(const char *arKey, uint32_t nKeyLength,
                                      uint64_t mask = 0xdfdfdfdfdfdfdfdfull) {
  return crc8_i_safe(arKey, nKeyLength, mask);
}

inline strhash_t hash_string_inline(const char *arKey, uint32_t nKeyLength,
                                    uint64_t mask = 0xdfdfdfdfdfdfdfdfull) {
  return hash_string_i_inline(arKey, nKeyLength);
}

#else
inline strhash_t hash_string_i_inline(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_inline_unsafe(arKey, nKeyLength);
}

inline strhash_t hash_string_inline(const char *arKey, uint32_t nKeyLength) {
  return hash_string_i_inline(arKey, nKeyLength);
}

#endif



/**
 * We probably should get rid of this, so to detect code generation errors,
 * where a binary string is treated as a NULL-terminated literal. Do we ever
 * allow binary strings as array keys or symbol names?
 */
inline strhash_t hash_string(const char *arKey) {
  return hash_string(arKey, strlen(arKey));
}

inline strhash_t hash_string_i(const char *arKey) {
  return hash_string_i(arKey, strlen(arKey));
}

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

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HASH_H_
