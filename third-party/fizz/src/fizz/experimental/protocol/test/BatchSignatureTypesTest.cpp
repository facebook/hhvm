/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/experimental/protocol/BatchSignatureTypes.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

TEST(BatchSignatureTypesTest, TestBatchSignatureSchemes) {
  BatchSignatureSchemes<Sha256> sha256Schemes;
  EXPECT_EQ(
      sha256Schemes.getFromBaseScheme(SignatureScheme::ed25519), folly::none);
  EXPECT_EQ(
      sha256Schemes.getFromBaseScheme(
          SignatureScheme::ecdsa_secp256r1_sha256_batch),
      folly::none);
  EXPECT_EQ(
      sha256Schemes.getFromBaseScheme(
          SignatureScheme::ecdsa_secp521r1_sha512_batch),
      folly::none);
  EXPECT_EQ(
      sha256Schemes.getFromBaseScheme(SignatureScheme::ecdsa_secp256r1_sha256),
      SignatureScheme::ecdsa_secp256r1_sha256_batch);
  EXPECT_EQ(
      sha256Schemes.getFromBaseScheme(SignatureScheme::rsa_pss_sha256),
      SignatureScheme::rsa_pss_sha256_batch);
}

TEST(BatchSignatureTypesTest, TestGetBatchSchemeInfo) {
  auto info = getBatchSchemeInfo(SignatureScheme::ecdsa_secp256r1_sha256);
  EXPECT_FALSE(info.hasValue());
  info = getBatchSchemeInfo(SignatureScheme::ecdsa_secp256r1_sha256_batch);
  EXPECT_TRUE(info.hasValue());
  EXPECT_EQ(info.value().baseScheme, SignatureScheme::ecdsa_secp256r1_sha256);
  info = getBatchSchemeInfo(
      SignatureScheme::ecdsa_secp256r1_sha256_batch,
      {SignatureScheme::ecdsa_secp384r1_sha384});
  EXPECT_FALSE(info.hasValue());
}

} // namespace test
} // namespace fizz
