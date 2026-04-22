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

  {
    Error err;
    FIZZ_THROW_ON_ERROR(clientKex.generateKeyPair(err), err);
  }
  {
    Error err;
    FIZZ_THROW_ON_ERROR(serverKex.generateKeyPair(err), err);
  }

  auto pubKey = clientKex.getKeyShare();
  auto serverSharedSecret = serverKex.generateSharedSecret(pubKey->coalesce());
  auto cipherText = serverKex.getKeyShare();
  auto clientSharedSecret =
      clientKex.generateSharedSecret(cipherText->coalesce());

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

  {
    Error err;
    FIZZ_THROW_ON_ERROR(clientKex.generateKeyPair(err), err);
  }
  {
    Error err;
    FIZZ_THROW_ON_ERROR(serverKex.generateKeyPair(err), err);
  }
  {
    Error err;
    FIZZ_THROW_ON_ERROR(wrongClientKex.generateKeyPair(err), err);
  }
  {
    Error err;
    FIZZ_THROW_ON_ERROR(wrongServerKex.generateKeyPair(err), err);
  }

  auto pubKey = clientKex.getKeyShare();
  EXPECT_THROW(
      wrongServerKex.generateSharedSecret(pubKey->coalesce()),
      std::runtime_error);

  serverKex.generateSharedSecret(pubKey->coalesce());
  auto cipherText = serverKex.getKeyShare();
  EXPECT_THROW(
      wrongClientKex.generateSharedSecret(cipherText->coalesce()),
      std::runtime_error);
}

TEST(OQSKeyExchangeTest, CloneTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_512);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_512);

  {
    Error err;
    FIZZ_THROW_ON_ERROR(clientKex.generateKeyPair(err), err);
  }
  {
    Error err;
    FIZZ_THROW_ON_ERROR(serverKex.generateKeyPair(err), err);
  }

  auto pubKey = clientKex.getKeyShare();
  auto serverSharedSecret = serverKex.generateSharedSecret(pubKey->coalesce());
  auto cipherText = serverKex.getKeyShare();
  auto clientSharedSecret =
      clientKex.generateSharedSecret(cipherText->coalesce());

  auto clientKexCopy = clientKex.clone();
  auto serverKexCopy = serverKex.clone();

  auto clientKeyShare = clientKex.getKeyShare();
  auto serverKeyShare = serverKex.getKeyShare();

  EXPECT_TRUE(
      folly::IOBufEqualTo()(clientKeyShare, clientKexCopy->getKeyShare()));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(serverKeyShare, serverKexCopy->getKeyShare()));
  EXPECT_TRUE(
      folly::IOBufEqualTo()(
          clientKex.generateSharedSecret(serverKeyShare->coalesce()),
          clientKexCopy->generateSharedSecret(serverKeyShare->coalesce())));
}

TEST(OQSKeyExchangeTest, FailedCloneTest) {
  auto clientKex = OQSClientKeyExchange(OQS_KEM_alg_ml_kem_768);
  auto serverKex = OQSServerKeyExchange(OQS_KEM_alg_ml_kem_768);
  EXPECT_THROW(clientKex.clone(), std::runtime_error);
  EXPECT_THROW(serverKex.clone(), std::runtime_error);
}
} // namespace fizz::test
#endif
