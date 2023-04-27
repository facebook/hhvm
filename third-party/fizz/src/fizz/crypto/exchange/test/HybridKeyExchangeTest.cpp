/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/exchange/HybridKeyExchange.h>
#include <fizz/crypto/exchange/test/Mocks.h>
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
    kex = std::make_unique<HybridKeyExchange>(
        std::move(firstKex), std::move(secondKex));
  }
};

// TODO: changed to typed test to test all combinations.
TEST_F(HybridKeyExchangeTest, KeyShareRetrievalBeforeGenerationTest) {
  EXPECT_THROW(kex->getKeyShare(), std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, KeyShareGenerationTest) {
  kex->generateKeyPair();
  mockKex.generateKeyPair();
  auto combinedKey = mockKex.getKeyShare();
  auto secondKexKey = mockKex.getKeyShare();
  combinedKey->appendToChain(std::move(secondKexKey));
  auto keyShare = kex->getKeyShare();
  EXPECT_TRUE(folly::IOBufEqualTo()(combinedKey, keyShare));
}

TEST_F(HybridKeyExchangeTest, SharedSecretGenerationTest) {
  kex->generateKeyPair();
  mockKex.generateKeyPair();
  // Get the combined public key
  auto combinedKey = mockKex.getKeyShare();
  auto secondKexKey = mockKex.getKeyShare();
  combinedKey->appendToChain(std::move(secondKexKey));
  // Calculate the shared secret using HybridKeyExchange::generateSharedSecret()
  auto sharedSecret = kex->generateSharedSecret(combinedKey->coalesce());
  // Calculate the shared secret individually
  auto combinedSharedSecret =
      mockKex.generateSharedSecret(mockKex.getKeyShare()->coalesce());
  combinedSharedSecret->appendToChain(
      mockKex.generateSharedSecret(mockKex.getKeyShare()->coalesce()));
  // They should be of the same value
  EXPECT_TRUE(folly::IOBufEqualTo()(combinedSharedSecret, sharedSecret));
}

TEST_F(HybridKeyExchangeTest, SharedSecretGenerationOnIllegalInputTest) {
  kex->generateKeyPair();
  mockKex.generateKeyPair();
  // Get the truncated public key
  auto publicKey = mockKex.getKeyShare();
  EXPECT_THROW(
      kex->generateSharedSecret(publicKey->coalesce()), std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, CloneTest) {
  kex->generateKeyPair();
  auto kexCopy = kex->clone();
  auto keyShare = kex->getKeyShare();
  auto keyShareCopy = kexCopy->getKeyShare();
  EXPECT_TRUE(folly::IOBufEqualTo()(keyShare, keyShareCopy));
}

TEST_F(HybridKeyExchangeTest, InstantiationUsingNullPointerTest) {
  EXPECT_THROW(HybridKeyExchange(nullptr, nullptr), std::runtime_error);
}

TEST_F(HybridKeyExchangeTest, ZeroKeyLengthTest) {
  auto firstKex = std::make_unique<MockKeyExchange>();
  firstKex->setForHybridKeyExchange();
  firstKex->setReturnZeroKeyLength();
  auto secondKex = std::make_unique<MockKeyExchange>();
  secondKex->setForHybridKeyExchange();
  secondKex->setReturnZeroKeyLength();
  auto kex1 = std::make_unique<HybridKeyExchange>(
      std::move(firstKex), std::move(secondKex));
  EXPECT_THROW(kex1->generateKeyPair(), std::runtime_error);
}

} // namespace test
} // namespace fizz
