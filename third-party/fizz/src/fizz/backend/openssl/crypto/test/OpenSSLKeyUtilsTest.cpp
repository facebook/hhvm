/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/backend/openssl/crypto/OpenSSLKeyUtils.h>
#include <fizz/crypto/test/TestUtil.h>

namespace fizz {
using namespace test;

namespace openssl {
namespace test {

TEST(ValidateECKey, GoodPrivateKey) {
  auto key = getPrivateKey(kP256Key);
  Error err;
  EXPECT_EQ(
      detail::validateECKey(err, key, NID_X9_62_prime256v1), Status::Success);
}

TEST(ValidateECKey, GoodPublicKey) {
  auto key = getPublicKey(kP256PublicKey);
  Error err;
  EXPECT_EQ(
      detail::validateECKey(err, key, NID_X9_62_prime256v1), Status::Success);
}

TEST(ValidateECKey, WrongKeyType) {
  auto key = getPrivateKey(kRSAKey);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          detail::validateECKey(err, key, NID_X9_62_prime256v1), err),
      std::runtime_error);
}

TEST(ValidateECKey, WrongCurve) {
  auto key = getPrivateKey(kP256Key);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          detail::validateECKey(err, key, NID_X9_62_prime239v3), err),
      std::runtime_error);
}

TEST(ValidateEdKey, GoodPrivateKey) {
  auto key = getPrivateKey(kEd25519Key);
  Error err;
  EXPECT_EQ(detail::validateEdKey(err, key, NID_ED25519), Status::Success);
}

TEST(ValidateEdKey, GoodPublicKey) {
  auto key = getPublicKey(kEd25519PublicKey);
  Error err;
  EXPECT_EQ(detail::validateEdKey(err, key, NID_ED25519), Status::Success);
}

TEST(ValidateEdKey, WrongKeyType) {
  auto key = getPrivateKey(kP256Key);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(detail::validateEdKey(err, key, NID_ED25519), err),
      std::runtime_error);
}

TEST(ValidateEdKey, WrongCurve) {
  auto key = getPrivateKey(kEd448Key);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(detail::validateEdKey(err, key, NID_ED25519), err),
      std::runtime_error);
}

} // namespace test
} // namespace openssl
} // namespace fizz
