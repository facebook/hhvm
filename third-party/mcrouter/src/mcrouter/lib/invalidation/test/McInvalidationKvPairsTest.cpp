/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/invalidation/McInvalidationKvPairs.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <optional>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook::memcache::invalidation::test {
using namespace ::testing;

TEST(McInvalidationKvPairsTest, createAxonKvPairsTest) {
  std::string key = "key1";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(serialized);

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
}

TEST(McInvalidationKvPairsTest, createAxonKvPairsWithPoolAndRegionTest) {
  std::string key = "key1";
  std::string pool = "twmemcache.my_poool";
  std::string region = "altoonia";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(
      serialized, std::make_optional(region), std::make_optional(pool));

  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(result.find(kPool)->second, pool);
  EXPECT_EQ(result.find(kRegion)->second, region);
}

TEST(
    McInvalidationKvPairsTest,
    createAxonKvPairsWithPoolAndRegionAndMessageTest) {
  std::string key = "key1";
  std::string pool = "twmemcache.my_pool";
  std::string region = "altoona";
  std::string message = "DistributionRoute";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(
      serialized,
      std::make_optional(region),
      std::make_optional(pool),
      std::make_optional(message));

  EXPECT_EQ(result.size(), 5);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(result.find(kPool)->second, pool);
  EXPECT_EQ(result.find(kRegion)->second, region);
  EXPECT_EQ(result.find(kMessage)->second, message);
}

} // namespace facebook::memcache::invalidation::test
