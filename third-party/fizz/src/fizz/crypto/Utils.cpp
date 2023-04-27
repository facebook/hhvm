/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Utils.h>

#include <sodium.h>

#include <folly/ssl/Init.h>

namespace {

class InitFizz {
 public:
  InitFizz() {
    if (sodium_init() == -1) {
      throw std::runtime_error("Couldn't init libsodium");
    }

    folly::ssl::init();
  }
};
} // namespace

namespace fizz {

bool CryptoUtils::equal(folly::ByteRange a, folly::ByteRange b) {
  if (a.size() != b.size()) {
    return false;
  }
  return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
}

void CryptoUtils::clean(folly::MutableByteRange range) {
  sodium_memzero(range.data(), range.size());
}

void CryptoUtils::init() {
  static InitFizz initFizz;
}
} // namespace fizz
