/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/signature/Signature.h>

using namespace folly;
using namespace folly::ssl;
using namespace testing;

namespace fizz {
namespace test {

class RSAPSSTest : public Test {
  void SetUp() override {
    OpenSSL_add_all_algorithms();
  }
};

static EvpPkeyUniquePtr generateKey() {
  std::unique_ptr<
      EVP_PKEY_CTX,
      folly::static_function_deleter<EVP_PKEY_CTX, &EVP_PKEY_CTX_free>>
      ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
  EVP_PKEY_keygen_init(ctx.get());
  EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048);
  EVP_PKEY* keyPtr{nullptr};
  EVP_PKEY_keygen(ctx.get(), &keyPtr);
  return EvpPkeyUniquePtr(keyPtr);
}

TEST_F(RSAPSSTest, TestSignVerify) {
  OpenSSLSignature<KeyType::RSA> rsa;
  rsa.setKey(generateKey());
  static constexpr StringPiece msg{"message"};
  auto sig = rsa.sign<SignatureScheme::rsa_pss_sha256>(msg);
  rsa.verify<SignatureScheme::rsa_pss_sha256>(msg, sig->coalesce());
}

TEST_F(RSAPSSTest, TestVerifyDifferent) {
  OpenSSLSignature<KeyType::RSA> rsa;
  rsa.setKey(generateKey());
  static constexpr StringPiece msg1{"message"};
  static constexpr StringPiece msg2{"somethingelse"};
  auto sig = rsa.sign<SignatureScheme::rsa_pss_sha256>(msg1);
  EXPECT_THROW(
      rsa.verify<SignatureScheme::rsa_pss_sha256>(msg2, sig->coalesce()),
      std::runtime_error);
}

TEST_F(RSAPSSTest, TestVerifyFailure) {
  OpenSSLSignature<KeyType::RSA> rsa;
  rsa.setKey(generateKey());
  static constexpr StringPiece msg{"message"};
  auto sig = rsa.sign<SignatureScheme::rsa_pss_sha256>(msg);
  sig->writableData()[1] ^= 0x2;
  EXPECT_THROW(
      rsa.verify<SignatureScheme::rsa_pss_sha256>(msg, sig->coalesce()),
      std::runtime_error);
}
} // namespace test
} // namespace fizz
