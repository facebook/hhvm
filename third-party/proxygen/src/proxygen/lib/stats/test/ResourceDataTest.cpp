/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/stats/ResourceData.h"
#include <folly/portability/GTest.h>

using namespace ::testing;
using namespace proxygen;

class ResourceDataTest : public ::testing::Test {};

TEST_F(ResourceDataTest, Percentiles) {
  std::vector<double> values{0.1, 0.5, 0.6, 0.7, 0.8};
  EXPECT_EQ(ResourceData::computePercentile(values, 0), 0.1);
  EXPECT_EQ(ResourceData::computePercentile(values, 19), 0.1);
  EXPECT_EQ(ResourceData::computePercentile(values, 20), 0.5);
  EXPECT_EQ(ResourceData::computePercentile(values, 21), 0.5);
  EXPECT_EQ(ResourceData::computePercentile(values, 39), 0.5);
  EXPECT_EQ(ResourceData::computePercentile(values, 40), 0.6);
  EXPECT_EQ(ResourceData::computePercentile(values, 41), 0.6);
  EXPECT_EQ(ResourceData::computePercentile(values, 59), 0.6);
  EXPECT_EQ(ResourceData::computePercentile(values, 60), 0.7);
  EXPECT_EQ(ResourceData::computePercentile(values, 61), 0.7);
  EXPECT_EQ(ResourceData::computePercentile(values, 79), 0.7);
  EXPECT_EQ(ResourceData::computePercentile(values, 80), 0.8);
  EXPECT_EQ(ResourceData::computePercentile(values, 81), 0.8);
  EXPECT_EQ(ResourceData::computePercentile(values, 90), 0.8);
  EXPECT_EQ(ResourceData::computePercentile(values, 99), 0.8);
  EXPECT_EQ(ResourceData::computePercentile(values, 100), 0.8);
}
