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
#include "hphp/util/word-mem.h"
#include "hphp/util/assertions.h"
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

namespace HPHP {

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


#ifdef USE_SSECRC

NEVER_INLINE
strhash_t hash_string_cs_crc(const char *arKey, uint32_t nKeyLength) {
  return crc8_cs_unsafe(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_cs_unaligned_crc(const char *arKey, uint32_t nKeyLength) {
  return crc8_cs_safe(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i_crc(const char *arKey, uint32_t nKeyLength) {
  return crc8_i_unsafe(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i_unaligned_crc(const char *arKey, uint32_t nKeyLength) {
  return crc8_i_safe(arKey, nKeyLength);
}

// Filler code as place holders. These are not really nop, as GAS doesn't
// support multi-byte NOP very well.
#define NOP10BYTES __asm__ __volatile__("movq $0xf0ffffffffffffff, %rax\n");
#define NOP5BYTES __asm__ __volatile__("movl $0xf0ffffff, %eax\n");
#define NOP55BYTES NOP10BYTES; NOP10BYTES; NOP10BYTES; NOP10BYTES; NOP10BYTES; \
  NOP5BYTES;

NEVER_INLINE
strhash_t hash_string_cs(const char *arKey, uint32_t nKeyLength) {
  assert(((uint64_t)arKey & 0x7) == 0);
  NOP55BYTES;
  return hash_string_cs_fallback(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i(const char *arKey, uint32_t nKeyLength) {
  assert(((uint64_t)arKey & 0x7) == 0);
  NOP55BYTES;
  return hash_string_i_fallback(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_cs_unaligned(const char *arKey, uint32_t nKeyLength) {
  NOP55BYTES;
  return hash_string_cs_fallback(arKey, nKeyLength);
}

NEVER_INLINE
strhash_t hash_string_i_unaligned(const char *arKey, uint32_t nKeyLength) {
  // This function will occupy 80 bytes when optimization is on.
  NOP10BYTES;
  NOP10BYTES;
  NOP55BYTES;
  return hash_string_i_fallback(arKey, nKeyLength);
}

typedef strhash_t (*hash_func)(const char*, uint32_t);

#ifdef __OPTIMIZE__
NEVER_INLINE
static void copyHashFunc(hash_func dst, hash_func src, uint32_t sz = 64) {
  wordcpy((char*)dst, (char*)src, sz);
}
#endif

void copyHashFuncs() {
#ifdef __OPTIMIZE__
  // When optimization is off, the calling convention can be different and code
  // size is also larger.
  if (IsSSEHashSupported()) {
    copyHashFunc(hash_string_cs, hash_string_cs_crc, 64);
    copyHashFunc(hash_string_cs_unaligned, hash_string_cs_unaligned_crc, 64);
    copyHashFunc(hash_string_i, hash_string_i_crc, 64);
    copyHashFunc(hash_string_i_unaligned, hash_string_i_unaligned_crc, 80);
  }
#endif
}

#else

void copyHashFuncs() {}

#endif // USE_SSECRC
}
