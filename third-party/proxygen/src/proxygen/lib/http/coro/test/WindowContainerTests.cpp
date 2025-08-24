/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/coro/util/WindowContainer.h>

using namespace proxygen::coro;
using namespace testing;

namespace proxygen::coro::test {

TEST(WindowContainerTest, BelowKMinThreshold) {
  /** Verifies that processed() returns greater than 0 when half of the window
   * has been consumed.
   */
  constexpr uint32_t kCapacity = 64 * 1024;
  WindowContainer container{kCapacity};
  EXPECT_TRUE(container.reserve(kCapacity, 0));
  EXPECT_EQ(container.processed((kCapacity / 2) - 1), 0);
  EXPECT_GT(container.processed(1), 0);
}

TEST(WindowContainerTest, AboveKMinThreshold) {
  /** Verifies that when we process more than kMinThreshold bytes, processed()
   * will return greater than 0, even though half the window has not been
   * consumed yet. This will result in more frequent WINDOW_UPDATES.
   */
  constexpr uint32_t kCapacity = 1024 * 1024;
  constexpr uint32_t kMinThreshold = 128 * 1024;
  WindowContainer container{kCapacity};
  EXPECT_TRUE(container.reserve(kCapacity, 0));
  EXPECT_EQ(container.processed(kMinThreshold - 1), 0);
  EXPECT_GT(container.processed(1), 0);
}

} // namespace proxygen::coro::test
