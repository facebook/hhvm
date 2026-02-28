/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/ExponentialSmoothData.h"

using facebook::memcache::mcrouter::ExponentialSmoothData;

TEST(ExponentialSmoothData, sanity) {
  ExponentialSmoothData<16> data;
  EXPECT_FALSE(data.hasValue());
  EXPECT_DOUBLE_EQ(0.0, data.value());

  data.insertSample(10);
  EXPECT_TRUE(data.hasValue());
  EXPECT_DOUBLE_EQ(10.0, data.value());

  data.insertSample(20);
  EXPECT_TRUE(data.hasValue());
  EXPECT_LT(10.0, data.value());
  EXPECT_GT(20.0, data.value());
}

TEST(ExponentialSmoothData, one_bucket) {
  ExponentialSmoothData<1> data;
  EXPECT_FALSE(data.hasValue());
  EXPECT_DOUBLE_EQ(0.0, data.value());

  data.insertSample(5);
  EXPECT_TRUE(data.hasValue());
  EXPECT_DOUBLE_EQ(5.0, data.value());

  data.insertSample(4);
  EXPECT_TRUE(data.hasValue());
  EXPECT_DOUBLE_EQ(4.0, data.value());
}
