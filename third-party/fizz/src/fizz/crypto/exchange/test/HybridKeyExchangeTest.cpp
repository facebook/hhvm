/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/exchange/HybridKeyExchange.h>
#include <fizz/crypto/exchange/test/Mocks.h>
#include <fizz/util/Status.h>
#include <folly/portability/GTest.h>

using namespace fizz;

namespace fizz {
namespace test {
class HybridKeyExchangeTest : public testing::Test {
 public:
  std::unique_ptr<HybridKeyExchange> kex;
  MockKeyExchange mockKex;

  HybridKeyExchangeTest() {
    mockKex.setForHybridKeyExchange();
    auto firstKex = std::make_unique<MockKeyExchange>();
    firstKex->setForHybridKeyExchange();
    auto secondKex = std::make_unique<MockKeyExchange>();
    secondKex->setForHybridKeyExchange();
    Error err;
    FIZZ_THROW_ON_ERROR(
        HybridKeyExchange::create(
            kex, err, std::move(firstKex), std::move(secondKex)),
        err);
  }
};

// TODO: changed to typed test to test all combinations.
TEST_F(HybridKeyExchangeTest, KeyShareRetrievalBeforeGenerationTest) {
  EXPECT_THROW(kex->getKeyShare(), std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, KeyShareGenerationTest) {
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(mockKex.generateKeyPair(err), Status::Success);
  auto combinedKey = mockKex.getKeyShare();
  auto secondKexKey = mockKex.getKeyShare();
  combinedKey->appendToChain(std::move(secondKexKey));
  auto keyShare = kex->getKeyShare();
  EXPECT_TRUE(folly::IOBufEqualTo()(combinedKey, keyShare));
}

TEST_F(HybridKeyExchangeTest, SharedSecretGenerationTest) {
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(mockKex.generateKeyPair(err), Status::Success);
  // Get the combined public key
  auto combinedKey = mockKex.getKeyShare();
  auto secondKexKey = mockKex.getKeyShare();
  combinedKey->appendToChain(std::move(secondKexKey));
  // Calculate the shared secret using HybridKeyExchange::generateSharedSecret()
  std::unique_ptr<folly::IOBuf> sharedSecret;
  EXPECT_EQ(
      kex->generateSharedSecret(sharedSecret, err, combinedKey->coalesce()),
      Status::Success);
  // Calculate the shared secret individually
  std::unique_ptr<folly::IOBuf> combinedSharedSecret;
  EXPECT_EQ(
      mockKex.generateSharedSecret(
          combinedSharedSecret, err, mockKex.getKeyShare()->coalesce()),
      Status::Success);
  std::unique_ptr<folly::IOBuf> secondSharedSecret;
  EXPECT_EQ(
      mockKex.generateSharedSecret(
          secondSharedSecret, err, mockKex.getKeyShare()->coalesce()),
      Status::Success);
  combinedSharedSecret->appendToChain(std::move(secondSharedSecret));
  // They should be of the same value
  EXPECT_TRUE(folly::IOBufEqualTo()(combinedSharedSecret, sharedSecret));
}

TEST_F(HybridKeyExchangeTest, SharedSecretGenerationOnIllegalInputTest) {
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
  EXPECT_EQ(mockKex.generateKeyPair(err), Status::Success);
  // Get the truncated public key
  auto publicKey = mockKex.getKeyShare();
  EXPECT_THROW(
      {
        std::unique_ptr<folly::IOBuf> sharedSecret;
        FIZZ_THROW_ON_ERROR(
            kex->generateSharedSecret(sharedSecret, err, publicKey->coalesce()),
            err);
      },
      std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, CloneTest) {
  Error err;
  EXPECT_EQ(kex->generateKeyPair(err), Status::Success);
  auto kexCopy = kex->clone();
  auto keyShare = kex->getKeyShare();
  auto keyShareCopy = kexCopy->getKeyShare();
  EXPECT_TRUE(folly::IOBufEqualTo()(keyShare, keyShareCopy));
}

TEST_F(HybridKeyExchangeTest, InstantiationUsingNullPointerTest) {
  EXPECT_THROW(
      {
        std::unique_ptr<HybridKeyExchange> hybridKex;
        Error err;
        FIZZ_THROW_ON_ERROR(
            HybridKeyExchange::create(hybridKex, err, nullptr, nullptr), err);
      },
      std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, ZeroKeyLengthTest) {
  auto firstKex = std::make_unique<MockKeyExchange>();
  firstKex->setForHybridKeyExchange();
  firstKex->setReturnZeroKeyLength();
  auto secondKex = std::make_unique<MockKeyExchange>();
  secondKex->setForHybridKeyExchange();
  secondKex->setReturnZeroKeyLength();
  std::unique_ptr<HybridKeyExchange> kex1;
  Error err;
  EXPECT_EQ(
      HybridKeyExchange::create(
          kex1, err, std::move(firstKex), std::move(secondKex)),
      Status::Success);
  EXPECT_THROW(
      { FIZZ_THROW_ON_ERROR(kex1->generateKeyPair(err), err); },
      std::runtime_error);
}

} // namespace test
} // namespace fizz
