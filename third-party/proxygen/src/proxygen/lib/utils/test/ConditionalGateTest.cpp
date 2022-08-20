/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <proxygen/lib/utils/ConditionalGate.h>

using namespace proxygen;

class ConditionalGateTest : public testing::Test {
 protected:
  enum class Things : uint8_t { Thing1 = 0, Thing2 };
  ConditionalGate<Things, 2> gate;
  bool done{false};
};

TEST_F(ConditionalGateTest, Delay) {
  gate.then([this] { done = true; });
  EXPECT_FALSE(done);
  gate.set(Things::Thing1);
  EXPECT_FALSE(done);
  gate.set(Things::Thing2);
  EXPECT_TRUE(done);
}

TEST_F(ConditionalGateTest, DelayMulti) {
  bool otherDone = false;
  gate.then([this] { done = true; });
  gate.then([&otherDone] { otherDone = true; });
  EXPECT_FALSE(done);
  EXPECT_FALSE(otherDone);
  gate.set(Things::Thing1);
  EXPECT_FALSE(done);
  EXPECT_FALSE(otherDone);
  gate.set(Things::Thing2);
  EXPECT_TRUE(done);
  EXPECT_TRUE(otherDone);
}

TEST_F(ConditionalGateTest, Immediate) {
  gate.set(Things::Thing1);
  EXPECT_FALSE(done);
  EXPECT_TRUE(gate.get(Things::Thing1));
  EXPECT_FALSE(gate.get(Things::Thing2));
  gate.set(Things::Thing2);
  EXPECT_FALSE(done);
  gate.then([this] { done = true; });
  EXPECT_TRUE(done);
  EXPECT_TRUE(gate.allConditionsMet());
}

TEST(ReadyGateTest, Delay) {
  ReadyGate ready;
  bool done = false;
  ready.then([&done] { done = true; });
  EXPECT_FALSE(done);
  ready.set();
  EXPECT_TRUE(done);
}

TEST(ReadyGateTest, Immediate) {
  ReadyGate ready;
  bool done = false;
  ready.set();
  EXPECT_FALSE(done);
  ready.then([&done] { done = true; });
  EXPECT_TRUE(done);
}
