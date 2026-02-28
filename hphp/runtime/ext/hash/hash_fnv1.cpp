/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/hash/hash_fnv1.h"

#define PHP_FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define PHP_FNV_32_PRIME ((uint32_t)0x01000193)

#define PHP_FNV1_64_INIT ((uint64_t)0xcbf29ce484222325ULL)
#define PHP_FNV_64_PRIME ((uint64_t)0x100000001b3ULL)


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PhpFnv132Ctx {
  uint32_t state;
};

struct PhpFnv164Ctx{
  uint64_t state;
};

hash_fnv132::hash_fnv132(bool a)
  : HashEngine(4, 4, sizeof(PhpFnv132Ctx)), m_a(a) {
}

hash_fnv164::hash_fnv164(bool a)
  : HashEngine(8, 4, sizeof(PhpFnv164Ctx)), m_a(a) {
}

void hash_fnv132::hash_init(void *context_) {
  PhpFnv132Ctx *context = (PhpFnv132Ctx*)context_;
  context->state = PHP_FNV1_32_INIT;
}

void hash_fnv164::hash_init(void *context_) {
  PhpFnv164Ctx *context = (PhpFnv164Ctx*)context_;
  context->state = PHP_FNV1_64_INIT;
}

void hash_fnv132::hash_update(void *context_, const unsigned char *input,
                             unsigned int len) {
  PhpFnv132Ctx *context = (PhpFnv132Ctx*)context_;
  const unsigned char *bp = (unsigned char *)input;   /* start of buffer */
  const unsigned char *be = bp + len;      /* beyond end of buffer */

  /*
   * FNV-1 hash each octet in the buffer
   */
  while (bp < be) {
    if (m_a == false) {
      /* multiply by the 32 bit FNV magic prime mod 2^32 */
      context->state *= PHP_FNV_32_PRIME;

      /* xor the bottom with the current octet */
      context->state ^= (uint32_t)*bp++;
    } else {
      /* xor the bottom with the current octet */
      context->state ^= (uint32_t)*bp++;

      /* multiply by the 32 bit FNV magic prime mod 2^32 */
      context->state *= PHP_FNV_32_PRIME;
    }
  }
}

void hash_fnv164::hash_update(void *context_, const unsigned char *input,
                             unsigned int len) {
  PhpFnv164Ctx *context = (PhpFnv164Ctx*)context_;
  unsigned char *bp = (unsigned char *)input;   /* start of buffer */
  unsigned char *be = bp + len;      /* beyond end of buffer */

  /*
   * FNV-1 hash each octet of the buffer
   */
  while (bp < be) {
    if (m_a == false) {
      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      context->state *= PHP_FNV_64_PRIME;

      /* xor the bottom with the current octet */
      context->state ^= (uint64_t)*bp++;
    } else {
      /* xor the bottom with the current octet */
      context->state ^= (uint64_t)*bp++;

      /* multiply by the 64 bit FNV magic prime mod 2^64 */
      context->state *= PHP_FNV_64_PRIME;
    }
  }
}

void hash_fnv132::hash_final(unsigned char *digest, void *context_) {
  PhpFnv132Ctx *context = (PhpFnv132Ctx*)context_;
#ifdef WORDS_BIGENDIAN
    memcpy(digest, &context->state, 4);
#else
    int i = 0;
    unsigned char *c = (unsigned char *) &context->state;

    for (i = 0; i < 4; i++) {
        digest[i] = c[3 - i];
    }
#endif
}

void hash_fnv164::hash_final(unsigned char *digest, void *context_) {
  PhpFnv164Ctx *context = (PhpFnv164Ctx*)context_;
#ifdef WORDS_BIGENDIAN
    memcpy(digest, &context->state, 8);
#else
    int i = 0;
    unsigned char *c = (unsigned char *) &context->state;

    for (i = 0; i < 8; i++) {
        digest[i] = c[7 - i];
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
