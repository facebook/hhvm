/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/liboqs/OQSKeyExchange.h>

#if FIZZ_HAVE_OQS
#include <fizz/util/Status.h>
#include <folly/portability/GTest.h>

using namespace fizz;
using namespace fizz::liboqs;

namespace fizz::test {
TEST(OQSKeyExchangeTest, InvalidKeyGenerationTest) {
  EXPECT_THROW(OQSServerKeyExchange("invalid"), std::runtime_error);
  EXPECT_THROW(OQSClientKeyExchange("invalid"), std::runtime_error);
}

TEST(OQSKeyExchangeTest, SuccessKeyExchangeTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_768);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_768);

  Error err;
  EXPECT_EQ(clientKex.generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex.generateKeyPair(err), Status::Success);

  auto pubKey = clientKex.getKeyShare();
  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex.generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  auto cipherText = serverKex.getKeyShare();
  std::unique_ptr<folly::IOBuf> clientSharedSecret;
  EXPECT_EQ(
      clientKex.generateSharedSecret(
          clientSharedSecret, err, cipherText->coalesce()),
      Status::Success);

  EXPECT_TRUE(folly::IOBufEqualTo()(serverSharedSecret, clientSharedSecret));
}

TEST(OQSKeyExchangeTest, GetKeyShareBeforeGenerationTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_768);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_768);
  EXPECT_THROW(clientKex.getKeyShare(), std::runtime_error);
  EXPECT_THROW(serverKex.getKeyShare(), std::runtime_error);
}

TEST(OQSKeyExchangeTest, InvalidExternalInputTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_512);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_512);
  auto wrongClientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_768);
  auto wrongServerKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_768);

  Error err;
  EXPECT_EQ(clientKex.generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex.generateKeyPair(err), Status::Success);
  EXPECT_EQ(wrongClientKex.generateKeyPair(err), Status::Success);
  EXPECT_EQ(wrongServerKex.generateKeyPair(err), Status::Success);

  auto pubKey = clientKex.getKeyShare();
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sharedSecret;
        FIZZ_THROW_ON_ERROR(
            wrongServerKex.generateSharedSecret(
                sharedSecret, err, pubKey->coalesce()),
            err);
      },
      std::runtime_error);

  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex.generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  auto cipherText = serverKex.getKeyShare();
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sharedSecret;
        FIZZ_THROW_ON_ERROR(
            wrongClientKex.generateSharedSecret(
                sharedSecret, err, cipherText->coalesce()),
            err);
      },
      std::runtime_error);
}

TEST(OQSKeyExchangeTest, CloneTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_512);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_512);

  Error err;
  EXPECT_EQ(clientKex.generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex.generateKeyPair(err), Status::Success);

  auto pubKey = clientKex.getKeyShare();
  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex.generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  auto cipherText = serverKex.getKeyShare();
  std::unique_ptr<folly::IOBuf> clientSharedSecret;
  EXPECT_EQ(
      clientKex.generateSharedSecret(
          clientSharedSecret, err, cipherText->coalesce()),
      Status::Success);

  std::unique_ptr<KeyExchange> clientKexCopy;
  EXPECT_EQ(clientKex.clone(clientKexCopy, err), Status::Success);
  std::unique_ptr<KeyExchange> serverKexCopy;
  EXPECT_EQ(serverKex.clone(serverKexCopy, err), Status::Success);

  auto clientKeyShare = clientKex.getKeyShare();
  auto serverKeyShare = serverKex.getKeyShare();

  EXPECT_TRUE(
      folly::IOBufEqualTo()(clientKeyShare, clientKexCopy->getKeyShare()));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(serverKeyShare, serverKexCopy->getKeyShare()));
  std::unique_ptr<folly::IOBuf> originalSharedSecret;
  std::unique_ptr<folly::IOBuf> copySharedSecret;
  EXPECT_EQ(
      clientKex.generateSharedSecret(
          originalSharedSecret, err, serverKeyShare->coalesce()),
      Status::Success);
  EXPECT_EQ(
      clientKexCopy->generateSharedSecret(
          copySharedSecret, err, serverKeyShare->coalesce()),
      Status::Success);
  EXPECT_TRUE(folly::IOBufEqualTo()(originalSharedSecret, copySharedSecret));
}

TEST(OQSKeyExchangeTest, FailedCloneTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_768);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_768);
  std::unique_ptr<KeyExchange> kexClone;
  Error err;
  EXPECT_THROW(
      { FIZZ_THROW_ON_ERROR(clientKex.clone(kexClone, err), err); },
      std::runtime_error);
  EXPECT_THROW(
      { FIZZ_THROW_ON_ERROR(serverKex.clone(kexClone, err), err); },
      std::runtime_error);
}
} // namespace fizz::test
#endif
