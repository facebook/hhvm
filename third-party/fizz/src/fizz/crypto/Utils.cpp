/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Utils.h>
#include <fizz/fizz-config.h>

#if FIZZ_HAVE_SODIUM
#include <sodium.h>

#define fizz_crypto_init sodium_init
#define fizz_secure_memcmp sodium_memcmp
#define fizz_secure_memzero sodium_memzero

#else /* FIZZ_HAVE_SODIUM */

static int fizz_crypto_init() {
  return 0;
}

/*
 * The following functions were adapted from lib sodium:
 *  - fizz_secure_memcmp (from sodium's sodium_memzero)
 *  - fizz_secure_memzero (from sodium's sodium_memzero)
 *
 * Copyright (c) 2013-2025
 * Frank Denis <j at pureftpd dot org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

static int
fizz_secure_memcmp(const void* const b1_, const void* const b2_, size_t len) {
  const volatile unsigned char* volatile b1 =
      (const volatile unsigned char* volatile)b1_;
  const volatile unsigned char* volatile b2 =
      (const volatile unsigned char* volatile)b2_;

  size_t i;
  volatile unsigned char d = 0U;

  for (i = 0U; i < len; i++) {
    d |= b1[i] ^ b2[i];
  }
  return (1 & ((d - 1) >> 8)) - 1;
}

static void fizz_secure_memzero(void* const pnt, const size_t len) {
  volatile unsigned char* volatile pnt_ = (volatile unsigned char* volatile)pnt;
  size_t i = (size_t)0U;

  while (i < len) {
    pnt_[i++] = 0U;
  }
}
#endif /* FIZZ_HAVE_SODIUM */

namespace {

class InitFizz {
 public:
  InitFizz() {
    if (fizz_crypto_init() == -1) {
      throw std::runtime_error("Couldn't init libsodium");
    }
  }
};
} // namespace

namespace fizz {

bool CryptoUtils::equal(folly::ByteRange a, folly::ByteRange b) {
  if (a.size() != b.size()) {
    return false;
  }
  return fizz_secure_memcmp(a.data(), b.data(), a.size()) == 0;
}

void CryptoUtils::clean(folly::MutableByteRange range) {
  fizz_secure_memzero(range.data(), range.size());
}

void CryptoUtils::init() {
  static InitFizz initFizz;
}

} // namespace fizz
