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

#include "hphp/runtime/ext/hash/hash_keyed_blake3.h"
#include <blake3.h>

#if defined(HPHP_OSS)
namespace facebook { namespace blake3 {
struct blake3_constants {
  static constexpr char const * const BLAKE3_HASH_KEY_ = "12345-67890";
  static constexpr char const * BLAKE3_HASH_KEY() {
    return BLAKE3_HASH_KEY_;
  }
};
} // namespace blake3
} // namespace facebook
#else
#include "blake3/gen-cpp2/blake3_constants.h"
#endif

namespace HPHP {

typedef struct {
  blake3_hasher hasher;
} PHP_KEYED_BLAKE3_CTX;

hash_keyed_blake3::hash_keyed_blake3() : HashEngine(BLAKE3_KEY_LEN, BLAKE3_BLOCK_LEN, sizeof(PHP_KEYED_BLAKE3_CTX)) {}

void hash_keyed_blake3::hash_init(void* context_) {
  PHP_KEYED_BLAKE3_CTX* context = (PHP_KEYED_BLAKE3_CTX*)context_;
  constexpr auto* const key = facebook::blake3::blake3_constants::BLAKE3_HASH_KEY();
  blake3_hasher hasher;
  const uint8_t(&keyArray)[BLAKE3_KEY_LEN] =
      *reinterpret_cast<const uint8_t(*)[BLAKE3_KEY_LEN]>(key);
  blake3_hasher_init_keyed(&hasher, keyArray);
  context->hasher = hasher;
}

void hash_keyed_blake3::hash_update(void *context_, const unsigned char *input,
                             unsigned int len) {
  PHP_KEYED_BLAKE3_CTX* context = (PHP_KEYED_BLAKE3_CTX*)context_;
  blake3_hasher_update(&context->hasher, input, len);
}

void hash_keyed_blake3::hash_final(unsigned char* output, void* context_) {
  PHP_KEYED_BLAKE3_CTX* context = (PHP_KEYED_BLAKE3_CTX*)context_;
  blake3_hasher_finalize(&context->hasher, output, BLAKE3_OUT_LEN);
}

} // namespace HPHP
