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

#include "hphp/runtime/ext/hash/hash_blake3.h"
#include <blake3.h>

namespace HPHP {

struct PhpBlake3Ctx {
  blake3_hasher hasher;
};

hash_blake3::hash_blake3() : HashEngine(BLAKE3_KEY_LEN, BLAKE3_BLOCK_LEN, sizeof(PhpBlake3Ctx)) {}

void hash_blake3::hash_init(void* context_) {
  PhpBlake3Ctx* context = (PhpBlake3Ctx*)context_;
  blake3_hasher_init(&context->hasher);
}

void hash_blake3::hash_update(void *context_, const unsigned char *input,
                             unsigned int len) {
  PhpBlake3Ctx* context = (PhpBlake3Ctx*)context_;
  blake3_hasher_update(&context->hasher, input, len);
}

void hash_blake3::hash_final(unsigned char* output, void* context_) {
  PhpBlake3Ctx* context = (PhpBlake3Ctx*)context_;
  blake3_hasher_finalize(&context->hasher, output, BLAKE3_OUT_LEN);
}


} // namespace HPHP
