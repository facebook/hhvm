/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/FlowController.h>

#include <gtest/gtest.h>

class FlowControllerTest : public ::testing::Test {
 protected:
  FlowController flowController;

  void SetUp() override {
    flowController = FlowController(100);
  }
};

TEST_F(FlowControllerTest, InitialState) {
  EXPECT_EQ(flowController.getCurrentOffset(), 0);
  EXPECT_EQ(flowController.getMaxOffset(), 100);
}

TEST_F(FlowControllerTest, ReserveWithinLimit) {
  EXPECT_TRUE(flowController.reserve(50));
  EXPECT_EQ(flowController.getCurrentOffset(), 50);
  EXPECT_FALSE(flowController.isBlocked());
}

TEST_F(FlowControllerTest, ReserveExceedsLimit) {
  EXPECT_FALSE(flowController.reserve(150));
  EXPECT_EQ(flowController.getCurrentOffset(), 0);
  EXPECT_FALSE(flowController.isBlocked());
}

TEST_F(FlowControllerTest, GrantIncreasesMaxOffset) {
  EXPECT_TRUE(flowController.reserve(100));
  EXPECT_TRUE(flowController.isBlocked());
  EXPECT_TRUE(flowController.grant(150));
  EXPECT_EQ(flowController.getMaxOffset(), 150);
}

TEST_F(FlowControllerTest, GrantDoesNotDecreaseMaxOffset) {
  EXPECT_FALSE(flowController.grant(50));
  EXPECT_EQ(flowController.getMaxOffset(), 100);
}

TEST_F(FlowControllerTest, GetAvailable) {
  EXPECT_EQ(flowController.getAvailable(), 100);

  flowController.reserve(50);
  EXPECT_EQ(flowController.getAvailable(), 50);

  flowController.grant(150);
  EXPECT_EQ(flowController.getAvailable(), 100);
}
