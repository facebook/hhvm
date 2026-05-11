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
  Error err;
  EXPECT_THROW(
      {
        std::unique_ptr<OQSServerKeyExchange> kex;
        FIZZ_THROW_ON_ERROR(
            OQSServerKeyExchange::create(kex, err, "invalid"), err);
      },
      std::runtime_error);
  EXPECT_THROW(
      {
        std::unique_ptr<OQSClientKeyExchange> kex;
        FIZZ_THROW_ON_ERROR(
            OQSClientKeyExchange::create(kex, err, "invalid"), err);
      },
      std::runtime_error);
}

TEST(OQSKeyExchangeTest, SuccessKeyExchangeTest) {
  std::unique_ptr<OQSClientKeyExchange> clientKex;
  std::unique_ptr<OQSServerKeyExchange> serverKex;
  Error err;
  EXPECT_EQ(
      OQSClientKeyExchange::create(clientKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(serverKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);

  EXPECT_EQ(clientKex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex->generateKeyPair(err), Status::Success);

  std::unique_ptr<folly::IOBuf> pubKey;
  EXPECT_EQ(clientKex->getKeyShare(pubKey, err), Status::Success);
  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex->generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  std::unique_ptr<folly::IOBuf> cipherText;
  EXPECT_EQ(serverKex->getKeyShare(cipherText, err), Status::Success);
  std::unique_ptr<folly::IOBuf> clientSharedSecret;
  EXPECT_EQ(
      clientKex->generateSharedSecret(
          clientSharedSecret, err, cipherText->coalesce()),
      Status::Success);

  EXPECT_TRUE(folly::IOBufEqualTo()(serverSharedSecret, clientSharedSecret));
}

TEST(OQSKeyExchangeTest, GetKeyShareBeforeGenerationTest) {
  std::unique_ptr<OQSClientKeyExchange> clientKex;
  std::unique_ptr<OQSServerKeyExchange> serverKex;
  Error err;
  EXPECT_EQ(
      OQSClientKeyExchange::create(clientKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(serverKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> ret;
        FIZZ_THROW_ON_ERROR(clientKex->getKeyShare(ret, err), err);
      },
      std::runtime_error);
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> ret;
        FIZZ_THROW_ON_ERROR(serverKex->getKeyShare(ret, err), err);
      },
      std::runtime_error);
}

TEST(OQSKeyExchangeTest, InvalidExternalInputTest) {
  std::unique_ptr<OQSClientKeyExchange> clientKex;
  std::unique_ptr<OQSServerKeyExchange> serverKex;
  std::unique_ptr<OQSClientKeyExchange> wrongClientKex;
  std::unique_ptr<OQSServerKeyExchange> wrongServerKex;
  Error err;
  EXPECT_EQ(
      OQSClientKeyExchange::create(clientKex, err, OQS_KEM_alg_ml_kem_512),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(serverKex, err, OQS_KEM_alg_ml_kem_512),
      Status::Success);
  EXPECT_EQ(
      OQSClientKeyExchange::create(wrongClientKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(wrongServerKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);

  EXPECT_EQ(clientKex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(wrongClientKex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(wrongServerKex->generateKeyPair(err), Status::Success);

  std::unique_ptr<folly::IOBuf> pubKey;
  EXPECT_EQ(clientKex->getKeyShare(pubKey, err), Status::Success);
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sharedSecret;
        FIZZ_THROW_ON_ERROR(
            wrongServerKex->generateSharedSecret(
                sharedSecret, err, pubKey->coalesce()),
            err);
      },
      std::runtime_error);

  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex->generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  std::unique_ptr<folly::IOBuf> cipherText;
  EXPECT_EQ(serverKex->getKeyShare(cipherText, err), Status::Success);
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sharedSecret;
        FIZZ_THROW_ON_ERROR(
            wrongClientKex->generateSharedSecret(
                sharedSecret, err, cipherText->coalesce()),
            err);
      },
      std::runtime_error);
}

TEST(OQSKeyExchangeTest, CloneTest) {
  std::unique_ptr<OQSClientKeyExchange> clientKex;
  std::unique_ptr<OQSServerKeyExchange> serverKex;
  Error err;
  EXPECT_EQ(
      OQSClientKeyExchange::create(clientKex, err, OQS_KEM_alg_ml_kem_512),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(serverKex, err, OQS_KEM_alg_ml_kem_512),
      Status::Success);

  EXPECT_EQ(clientKex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(serverKex->generateKeyPair(err), Status::Success);

  std::unique_ptr<folly::IOBuf> pubKey;
  EXPECT_EQ(clientKex->getKeyShare(pubKey, err), Status::Success);
  std::unique_ptr<folly::IOBuf> serverSharedSecret;
  EXPECT_EQ(
      serverKex->generateSharedSecret(
          serverSharedSecret, err, pubKey->coalesce()),
      Status::Success);
  std::unique_ptr<folly::IOBuf> cipherText;
  EXPECT_EQ(serverKex->getKeyShare(cipherText, err), Status::Success);
  std::unique_ptr<folly::IOBuf> clientSharedSecret;
  EXPECT_EQ(
      clientKex->generateSharedSecret(
          clientSharedSecret, err, cipherText->coalesce()),
      Status::Success);

  std::unique_ptr<KeyExchange> clientKexCopy;
  EXPECT_EQ(clientKex->clone(clientKexCopy, err), Status::Success);
  std::unique_ptr<KeyExchange> serverKexCopy;
  EXPECT_EQ(serverKex->clone(serverKexCopy, err), Status::Success);

  std::unique_ptr<folly::IOBuf> clientKeyShare;
  EXPECT_EQ(clientKex->getKeyShare(clientKeyShare, err), Status::Success);
  std::unique_ptr<folly::IOBuf> serverKeyShare;
  EXPECT_EQ(serverKex->getKeyShare(serverKeyShare, err), Status::Success);

  std::unique_ptr<folly::IOBuf> clientCopyKeyShare;
  EXPECT_EQ(
      clientKexCopy->getKeyShare(clientCopyKeyShare, err), Status::Success);
  std::unique_ptr<folly::IOBuf> serverCopyKeyShare;
  EXPECT_EQ(
      serverKexCopy->getKeyShare(serverCopyKeyShare, err), Status::Success);
  EXPECT_TRUE(folly::IOBufEqualTo()(clientKeyShare, clientCopyKeyShare));
  EXPECT_TRUE(folly::IOBufEqualTo()(serverKeyShare, serverCopyKeyShare));
  std::unique_ptr<folly::IOBuf> originalSharedSecret;
  std::unique_ptr<folly::IOBuf> copySharedSecret;
  EXPECT_EQ(
      clientKex->generateSharedSecret(
          originalSharedSecret, err, serverKeyShare->coalesce()),
      Status::Success);
  EXPECT_EQ(
      clientKexCopy->generateSharedSecret(
          copySharedSecret, err, serverKeyShare->coalesce()),
      Status::Success);
  EXPECT_TRUE(folly::IOBufEqualTo()(originalSharedSecret, copySharedSecret));
}

TEST(OQSKeyExchangeTest, FailedCloneTest) {
  std::unique_ptr<OQSClientKeyExchange> clientKex;
  std::unique_ptr<OQSServerKeyExchange> serverKex;
  Error err;
  EXPECT_EQ(
      OQSClientKeyExchange::create(clientKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  EXPECT_EQ(
      OQSServerKeyExchange::create(serverKex, err, OQS_KEM_alg_ml_kem_768),
      Status::Success);
  std::unique_ptr<KeyExchange> kexClone;
  EXPECT_THROW(
      { FIZZ_THROW_ON_ERROR(clientKex->clone(kexClone, err), err); },
      std::runtime_error);
  EXPECT_THROW(
      { FIZZ_THROW_ON_ERROR(serverKex->clone(kexClone, err), err); },
      std::runtime_error);
}
} // namespace fizz::test
#endif
