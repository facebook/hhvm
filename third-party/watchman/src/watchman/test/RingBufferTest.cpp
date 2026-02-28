/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include "watchman/RingBuffer.h"

using namespace watchman;

TEST(RingBufferTest, writes_can_be_read) {
  RingBuffer<int> rb{2};
  rb.write(10);
  rb.write(11);
  auto result = rb.readAll();
  EXPECT_EQ(2, result.size());
  EXPECT_EQ(10, result[0]);
  EXPECT_EQ(11, result[1]);

  rb.write(12);
  result = rb.readAll();
  EXPECT_EQ(11, result[0]);
  EXPECT_EQ(12, result[1]);
}

TEST(RingBufferTest, writes_can_be_cleared) {
  RingBuffer<int> rb{10};
  rb.write(3);
  rb.write(4);
  auto result = rb.readAll();
  EXPECT_EQ(2, result.size());
  EXPECT_EQ(3, result[0]);
  EXPECT_EQ(4, result[1]);
  rb.clear();
  rb.write(5);
  result = rb.readAll();
  EXPECT_EQ(1, result.size());
  EXPECT_EQ(5, result[0]);
}
