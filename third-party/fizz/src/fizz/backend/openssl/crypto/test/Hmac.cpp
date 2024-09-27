/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/Hasher.h>
#include <fizz/crypto/test/HmacTest.h>
#include <folly/portability/GTest.h>

namespace fizz::openssl::test {

class OpenSSLHmacTest : public ::testing::Test {};

TEST(OpenSSLHmacTest, TestSha256) {
  fizz::test::runHmacTest(fizz::openssl::hasherFactory<fizz::Sha256>());
}

TEST(OpenSSLHmacTest, TestSha384) {
  fizz::test::runHmacTest(fizz::openssl::hasherFactory<fizz::Sha384>());
}

TEST(OpenSSLHmacTest, TestSha512) {
  fizz::test::runHmacTest(fizz::openssl::hasherFactory<fizz::Sha512>());
}

} // namespace fizz::openssl::test
