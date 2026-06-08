/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/test/BigValueRouteTestBase.h"

TEST(BigValueRouteTest, smallvalue) {
  facebook::memcache::mcrouter::testSmallvalue<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalueWithFlag) {
  facebook::memcache::mcrouter::testBigvalueWithFlag<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalueWithoutFlag) {
  facebook::memcache::mcrouter::testBigvalueWithoutFlag<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, bigvalue) {
  facebook::memcache::mcrouter::testBigvalue<
      facebook::memcache::MemcacheRouterInfo>();
}

TEST(BigValueRouteTest, chunksInfoParsing) {
  namespace mc = facebook::memcache::mcrouter;
  uint32_t version = 0, numChunks = 0;
  uint64_t suffix = 0;

  EXPECT_TRUE(
      mc::detail::parseChunksInfo("1-2-7419256", version, numChunks, suffix));
  EXPECT_EQ(1, version);
  EXPECT_EQ(2, numChunks);
  EXPECT_EQ(7419256, suffix);

  EXPECT_FALSE(mc::detail::parseChunksInfo("1-2", version, numChunks, suffix));
  EXPECT_FALSE(
      mc::detail::parseChunksInfo("1-2-3-4", version, numChunks, suffix));
  EXPECT_FALSE(
      mc::detail::parseChunksInfo("1-2-x", version, numChunks, suffix));
  EXPECT_FALSE(mc::detail::parseChunksInfo("", version, numChunks, suffix));

  // Regression: must stay within the StringPiece bounds and never read trailing
  // bytes. The crash was sscanf scanning a non-NUL-terminated buffer past its
  // end.
  folly::StringPiece backing("1-2-3456789");
  EXPECT_TRUE(
      mc::detail::parseChunksInfo(
          backing.subpiece(0, 7), version, numChunks, suffix));
  EXPECT_EQ(345, suffix);
}
