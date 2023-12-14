/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/client/PskSerializationUtils.h>
#include <fizz/client/test/Utilities.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;

namespace fizz {
namespace client {
namespace test {

class PskSerializationTest : public Test {
 public:
  void SetUp() override {
    ticketTime_ = std::chrono::system_clock::now();
    factory_ = std::make_unique<fizz::test::MockFactory>();
  }

 protected:
  CachedPsk getCachedPsk(std::string pskName) {
    return getTestPsk(pskName, ticketTime_);
  }

  std::chrono::system_clock::time_point ticketTime_;
  std::unique_ptr<fizz::Factory> factory_;
};

TEST_F(PskSerializationTest, TestSerialization) {
  auto psk = getCachedPsk("SomePsk");
  std::string serializedPsk = fizz::client::serializePsk(psk);
  EXPECT_FALSE(serializedPsk.empty());
  auto psk2 = fizz::client::deserializePsk(serializedPsk, *factory_);
  EXPECT_EQ(psk.psk, psk2.psk);
  EXPECT_EQ(psk.secret, psk2.secret);
  EXPECT_EQ(psk.type, psk2.type);
  EXPECT_EQ(psk.version, psk2.version);
  EXPECT_EQ(psk.cipher, psk2.cipher);
  EXPECT_EQ(psk.group, psk2.group);
  EXPECT_EQ(psk.maxEarlyDataSize, psk2.maxEarlyDataSize);
  EXPECT_EQ(psk.ticketAgeAdd, psk2.ticketAgeAdd);
  EXPECT_EQ(psk.alpn, psk2.alpn);
  // Time may have slight rounding errors, make sure its negligible
  auto ticketHandshakeTimeDiff =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketHandshakeTime - psk2.ticketHandshakeTime);
  auto ticketExpirationTimeDiff =
      std::chrono::duration_cast<std::chrono::seconds>(
          psk.ticketExpirationTime - psk2.ticketExpirationTime);
  auto ticketIssueTimeDiff =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketIssueTime - psk2.ticketIssueTime);
  EXPECT_EQ(ticketHandshakeTimeDiff.count(), 0);
  EXPECT_EQ(ticketExpirationTimeDiff.count(), 0);
  EXPECT_EQ(ticketIssueTimeDiff.count(), 0);
}

TEST_F(PskSerializationTest, TestInvalidSerializationThrows) {
  auto psk = getCachedPsk("SomePsk");
  std::string serializedPsk = fizz::client::serializePsk(psk);
  EXPECT_FALSE(serializedPsk.empty());
  serializedPsk.insert(10, "HI!");
  ASSERT_ANY_THROW({ fizz::client::deserializePsk(serializedPsk, *factory_); });
}

} // namespace test
} // namespace client
} // namespace fizz
