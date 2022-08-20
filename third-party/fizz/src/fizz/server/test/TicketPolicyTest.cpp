/*
 *  Copyright (c) 2020-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/test/Mocks.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/clock/test/Mocks.h>
#include <fizz/server/TicketPolicy.h>

using namespace fizz::test;
using namespace testing;

namespace fizz {
namespace server {
namespace test {

class TicketPolicyTest : public Test {
 public:
  ~TicketPolicyTest() override = default;
  void SetUp() override {
    clock_ = std::make_shared<MockClock>();
    policy_.setClock(clock_);
  }

 protected:
  TicketPolicy policy_;
  std::shared_ptr<MockClock> clock_;
};

TEST_F(TicketPolicyTest, TestFreshHandshake) {
  auto now = std::chrono::system_clock::now();
  EXPECT_CALL(*clock_, getCurrentTime()).WillRepeatedly(Return(now));
  policy_.setTicketValidity(std::chrono::seconds(5));
  policy_.setHandshakeValidity(std::chrono::seconds(10));
  ResumptionState resState;

  // 9 sec valid handshake, 5 sec valid ticket -> 5 sec validity
  resState.handshakeTime = now - std::chrono::seconds(1);
  EXPECT_EQ(policy_.remainingValidity(resState), std::chrono::seconds(5));
  EXPECT_TRUE(policy_.shouldAccept(resState));

  // 4 sec valid handshake, 5 sec valid ticket -> 4 sec validity
  resState.handshakeTime = now - std::chrono::seconds(6);
  EXPECT_EQ(policy_.remainingValidity(resState), std::chrono::seconds(4));
  EXPECT_TRUE(policy_.shouldAccept(resState));

  // 12 sec valid handshake (clock skew), 15 second valid ticket -> 10 sec
  // validity (bounded by handshakeValidity)
  policy_.setTicketValidity(std::chrono::seconds(15));
  resState.handshakeTime = now + std::chrono::seconds(2);
  EXPECT_EQ(policy_.remainingValidity(resState), std::chrono::seconds(10));
  EXPECT_TRUE(policy_.shouldAccept(resState));
}

TEST_F(TicketPolicyTest, TestStaleHandshake) {
  auto now = std::chrono::system_clock::now();
  EXPECT_CALL(*clock_, getCurrentTime()).WillRepeatedly(Return(now));
  policy_.setTicketValidity(std::chrono::seconds(5));
  policy_.setHandshakeValidity(std::chrono::seconds(10));

  ResumptionState resState;
  resState.handshakeTime = now - std::chrono::seconds(12);
  EXPECT_EQ(policy_.remainingValidity(resState), std::chrono::seconds(-2));
  EXPECT_FALSE(policy_.shouldAccept(resState));
}

} // namespace test
} // namespace server
} // namespace fizz
