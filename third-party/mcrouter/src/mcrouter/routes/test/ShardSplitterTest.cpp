/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>

#include <gtest/gtest.h>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/routes/ShardSplitter.h"

using facebook::memcache::globals::HostidMock;
using facebook::memcache::mcrouter::ShardSplitter;

constexpr size_t kNumHostIds = 32768;

uint32_t nowInSec() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

TEST(ShardSplitter, basic) {
  ShardSplitter splitter(folly::dynamic::object("123", 10));
  folly::StringPiece shard;
  auto split = splitter.getShardSplit("abc:123:", shard);
  ASSERT_NE(nullptr, split);
  EXPECT_EQ("123", shard);
  shard = "";
  EXPECT_EQ(10, split->getOldSplitSize());
  EXPECT_EQ(10, split->getNewSplitSize());
  EXPECT_EQ(10, split->getSplitSizeForCurrentHost());
  EXPECT_TRUE(split->fanoutDeletesEnabled());
  EXPECT_EQ(nullptr, splitter.getShardSplit("abc:123aa:", shard));
  // Should return the default ShardSplitInfo
  split = splitter.getShardSplit("abc:12:", shard);
  ASSERT_NE(nullptr, split);
  EXPECT_EQ(1, split->getSplitSizeForCurrentHost());
}

folly::dynamic getConfigTemplate(uint32_t startTime) {
  return folly::dynamic::object(
      "123",
      folly::dynamic::object("old_split_size", 1)("new_split_size", 10)(
          "migration_period", 7200)("split_start", startTime));
}

TEST(ShardSplitter, beforeMigration) {
  auto config = getConfigTemplate(nowInSec() + 100);
  for (size_t i = 0; i < kNumHostIds; ++i) {
    HostidMock hostidMock(i);
    ShardSplitter splitter(config);
    folly::StringPiece shard;
    auto split = splitter.getShardSplit("abc:123:", shard);
    ASSERT_NE(nullptr, split);
    EXPECT_EQ(1, split->getSplitSizeForCurrentHost());
  }
}

TEST(ShardSplitter, afterMigration) {
  auto config = getConfigTemplate(nowInSec() - 8000);
  for (size_t i = 0; i < kNumHostIds; ++i) {
    HostidMock hostidMock(i);
    ShardSplitter splitter(config);
    folly::StringPiece shard;
    auto split = splitter.getShardSplit("abc:123:", shard);
    ASSERT_NE(nullptr, split);
    EXPECT_EQ(10, split->getSplitSizeForCurrentHost());
  }
}

TEST(ShardSplitter, defaultSplit) {
  ShardSplitter splitter(folly::dynamic::object("123", 10), 2);
  folly::StringPiece shard;
  auto split = splitter.getShardSplit("abc:123:", shard);
  ASSERT_NE(nullptr, split);
  EXPECT_EQ(10, split->getSplitSizeForCurrentHost());
  split = splitter.getShardSplit("bcd:234:", shard);
  ASSERT_NE(nullptr, split);
  EXPECT_EQ(2, split->getSplitSizeForCurrentHost());
}

void migrationTest(folly::dynamic config, double migrationPoint) {
  size_t numNew = 0;
  for (size_t i = 0; i < kNumHostIds; ++i) {
    HostidMock hostidMock(i);
    ShardSplitter splitter(config);
    folly::StringPiece shard;
    auto split = splitter.getShardSplit("abc:123:", shard);
    ASSERT_NE(nullptr, split);
    if (split->getSplitSizeForCurrentHost() == 10) {
      ++numNew;
    }
  }
  double migrated = numNew / (double)kNumHostIds;
  EXPECT_GT(migrationPoint + 0.02, migrated);
  EXPECT_LT(migrationPoint - 0.01, migrated);
}

TEST(ShardSplitter, migration10Pct) {
  auto config = getConfigTemplate(nowInSec() - 720);
  migrationTest(config, 0.1);
}

TEST(ShardSplitter, migration20Pct) {
  auto config = getConfigTemplate(nowInSec() - 1440);
  migrationTest(config, 0.2);
}

TEST(ShardSplitter, migration50Pct) {
  auto config = getConfigTemplate(nowInSec() - 3600);
  migrationTest(config, 0.5);
}

TEST(ShardSplitter, migration90Pct) {
  auto config = getConfigTemplate(nowInSec() - 6480);
  migrationTest(config, 0.9);
}
