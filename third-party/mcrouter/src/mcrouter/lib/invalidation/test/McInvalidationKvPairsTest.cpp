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

TEST(McInvalidationKvPairsTest, createDeleteAxonKvPairsTest) {
  std::string key = "key1";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(serialized);

  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(result.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(
      static_cast<DistributionType>(std::stoi(result.find(kType)->second)),
      DistributionType::Async);
}

TEST(McInvalidationKvPairsTest, createSetAxonKvPairsTest) {
  std::string key = "key1";
  memcache::McSetRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(
      serialized,
      std::nullopt,
      std::nullopt,
      std::nullopt,
      DistributionType::Async,
      DistributionOperation::Write);

  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(result.find(kOperation)->second)),
      DistributionOperation::Write);
  EXPECT_EQ(
      static_cast<DistributionType>(std::stoi(result.find(kType)->second)),
      DistributionType::Async);
}

TEST(McInvalidationKvPairsTest, createAxonKvPairsWithPoolAndRegionTest) {
  std::string key = "key1";
  std::string pool = "twmemcache.my_poool";
  std::string region = "altoonia";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result =
      McInvalidationKvPairs::createAxonKvPairs(serialized, region, pool);

  EXPECT_EQ(result.size(), 6);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(result.find(kPool)->second, pool);
  EXPECT_EQ(result.find(kRegion)->second, region);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(result.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(
      static_cast<DistributionType>(std::stoi(result.find(kType)->second)),
      DistributionType::Async);
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
      serialized, region, pool, message);

  EXPECT_EQ(result.size(), 7);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(result.find(kPool)->second, pool);
  EXPECT_EQ(result.find(kRegion)->second, region);
  EXPECT_EQ(result.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(result.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(
      static_cast<DistributionType>(std::stoi(result.find(kType)->second)),
      DistributionType::Async);
}

TEST(
    McInvalidationKvPairsTest,
    createAxonKvPairsWithPoolAndRegionAndMessageAndSrcRegionTest) {
  std::string key = "key1";
  std::string pool = "twmemcache.my_pool";
  std::string region = "altoona";
  std::string message = "DistributionRoute";
  std::string srcRegion = "texas";
  memcache::McDeleteRequest req(key);
  auto serialized =
      apache::thrift::CompactSerializer::serialize<std::string>(req);

  auto result = McInvalidationKvPairs::createAxonKvPairs(
      serialized,
      region,
      pool,
      message,
      DistributionType::Distribution,
      DistributionOperation::Delete,
      srcRegion);

  EXPECT_EQ(result.size(), 8);
  EXPECT_EQ(result.find(kSerialized)->second, serialized);
  EXPECT_EQ(
      result.find(kVersion)->second,
      folly::to<std::string>(
          facebook::memcache::invalidation::getMcInvalidationVersion()));
  EXPECT_EQ(result.find(kPool)->second, pool);
  EXPECT_EQ(result.find(kRegion)->second, region);
  EXPECT_EQ(result.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(result.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(
      static_cast<DistributionType>(std::stoi(result.find(kType)->second)),
      DistributionType::Distribution);
  EXPECT_EQ(result.find(kSourceRegion)->second, srcRegion);
}

} // namespace facebook::memcache::invalidation::test
