/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mcrouter/McDistributionUtils.h"

namespace facebook::memcache::invalidation::test {
using namespace ::testing;

TEST(McDistributionUtilsTest, distributeSetTest) {
  std::string targetRegion = "texas";
  std::string sourceRegion = "altoona";
  std::string message = "test";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  uint64_t resultBucketId = 0;
  bool secureWrites = false;
  folly::F14FastMap<std::string, std::string> resultKvPairs;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto bucketId, auto&& kvPairs, bool isSecureWrites) {
        resultBucketId = bucketId;
        resultKvPairs = kvPairs;
        secureWrites = isSecureWrites;
        return true;
      };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};

  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);
  auto req = McSetRequest("key1");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto result = distributeWriteRequest(
      req, axonCtxPtr, bucketId, targetRegion, sourceRegion, message);
  EXPECT_TRUE(result);
  EXPECT_FALSE(secureWrites);
  EXPECT_EQ(resultBucketId, bucketId);
  EXPECT_EQ(resultKvPairs.size(), 8);

  auto expectedSerialized =
      invalidation::McInvalidationKvPairs::serialize<memcache::McSetRequest>(
          req)
          .to<std::string>();
  EXPECT_TRUE(resultKvPairs.find(kSerialized) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kRegion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kVersion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kPool) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kMessage) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kType) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kOperation) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kSourceRegion) != resultKvPairs.end());

  EXPECT_EQ(resultKvPairs.find(kSerialized)->second, expectedSerialized);

  EXPECT_EQ(resultKvPairs.find(kRegion)->second, targetRegion);
  EXPECT_EQ(resultKvPairs.find(kPool)->second, pool);
  EXPECT_EQ(
      resultKvPairs.find(kVersion)->second,
      folly::to<std::string>(getMcInvalidationVersion()));
  EXPECT_EQ(resultKvPairs.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionType>(
          std::stoi(resultKvPairs.find(kType)->second)),
      DistributionType::Distribution);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(resultKvPairs.find(kOperation)->second)),
      DistributionOperation::Write);
  EXPECT_EQ(resultKvPairs.find(kSourceRegion)->second, sourceRegion);
}

TEST(McDistributionUtilsTest, distributeSetWithSecurityTest) {
  std::string targetRegion = "texas";
  std::string sourceRegion = "altoona";
  std::string message = "test";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  uint64_t resultBucketId = 0;
  bool secureWrites = false;
  folly::F14FastMap<std::string, std::string> resultKvPairs;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto bucketId, auto&& kvPairs, bool isSecureWrites) {
        resultBucketId = bucketId;
        resultKvPairs = kvPairs;
        secureWrites = isSecureWrites;
        return true;
      };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};

  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);
  auto req = McSetRequest("key1");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto result = distributeWriteRequest(
      req,
      axonCtxPtr,
      bucketId,
      targetRegion,
      sourceRegion,
      message,
      /*secureWrites=*/true);
  EXPECT_TRUE(result);
  EXPECT_TRUE(secureWrites);
  EXPECT_EQ(resultBucketId, bucketId);
  EXPECT_EQ(resultKvPairs.size(), 8);

  auto expectedSerialized =
      invalidation::McInvalidationKvPairs::serialize<memcache::McSetRequest>(
          req)
          .to<std::string>();
  EXPECT_TRUE(resultKvPairs.find(kSerialized) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kRegion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kVersion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kPool) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kMessage) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kType) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kOperation) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kSourceRegion) != resultKvPairs.end());

  EXPECT_EQ(resultKvPairs.find(kSerialized)->second, expectedSerialized);

  EXPECT_EQ(resultKvPairs.find(kRegion)->second, targetRegion);
  EXPECT_EQ(resultKvPairs.find(kPool)->second, pool);
  EXPECT_EQ(
      resultKvPairs.find(kVersion)->second,
      folly::to<std::string>(getMcInvalidationVersion()));
  EXPECT_EQ(resultKvPairs.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionType>(
          std::stoi(resultKvPairs.find(kType)->second)),
      DistributionType::Distribution);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(resultKvPairs.find(kOperation)->second)),
      DistributionOperation::Write);
  EXPECT_EQ(resultKvPairs.find(kSourceRegion)->second, sourceRegion);
}

TEST(McDistributionUtilsTest, distributeDeleteDirectedTest) {
  std::string targetRegion = "texas";
  std::string sourceRegion = "altoona";
  std::string message = "test";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  uint64_t resultBucketId = 0;
  folly::F14FastMap<std::string, std::string> resultKvPairs;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto bucketId, auto&& kvPairs, bool) {
        resultBucketId = bucketId;
        resultKvPairs = kvPairs;
        return true;
      };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};

  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);
  auto req = McDeleteRequest("key1");
  auto result = distributeDeleteRequest(
      req,
      axonCtxPtr,
      bucketId,
      DistributionType::Distribution,
      targetRegion,
      sourceRegion,
      message);
  EXPECT_TRUE(result);
  EXPECT_EQ(resultBucketId, bucketId);
  EXPECT_EQ(resultKvPairs.size(), 8);

  req = mcrouter::addDeleteRequestSource(
      req, memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION);

  auto expectedSerialized =
      invalidation::McInvalidationKvPairs::serialize<memcache::McDeleteRequest>(
          req)
          .to<std::string>();
  EXPECT_TRUE(resultKvPairs.find(kSerialized) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kRegion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kVersion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kPool) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kMessage) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kType) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kOperation) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kSourceRegion) != resultKvPairs.end());

  EXPECT_EQ(resultKvPairs.find(kSerialized)->second, expectedSerialized);

  EXPECT_EQ(resultKvPairs.find(kRegion)->second, targetRegion);
  EXPECT_EQ(resultKvPairs.find(kPool)->second, pool);
  EXPECT_EQ(
      resultKvPairs.find(kVersion)->second,
      folly::to<std::string>(getMcInvalidationVersion()));
  EXPECT_EQ(resultKvPairs.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionType>(
          std::stoi(resultKvPairs.find(kType)->second)),
      DistributionType::Distribution);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(resultKvPairs.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(resultKvPairs.find(kSourceRegion)->second, sourceRegion);
}

TEST(McDistributionUtilsTest, distributeDeleteBroadcastTest) {
  std::string targetRegion = "DistributionRoute";
  std::string sourceRegion = "altoona";
  std::string message = "test";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  uint64_t resultBucketId = 0;
  folly::F14FastMap<std::string, std::string> resultKvPairs;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto bucketId, auto&& kvPairs, bool) {
        resultBucketId = bucketId;
        resultKvPairs = kvPairs;
        return true;
      };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};

  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);
  auto req = McDeleteRequest("key1");
  auto result = distributeDeleteRequest(
      req,
      axonCtxPtr,
      bucketId,
      DistributionType::Distribution,
      targetRegion,
      sourceRegion,
      message);
  EXPECT_TRUE(result);
  EXPECT_EQ(resultBucketId, bucketId);
  EXPECT_EQ(resultKvPairs.size(), 8);

  req = mcrouter::addDeleteRequestSource(
      req,
      memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);

  auto expectedSerialized =
      invalidation::McInvalidationKvPairs::serialize<memcache::McDeleteRequest>(
          req)
          .to<std::string>();
  EXPECT_TRUE(resultKvPairs.find(kSerialized) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kVersion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kPool) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kMessage) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kType) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kOperation) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kSourceRegion) != resultKvPairs.end());

  EXPECT_EQ(resultKvPairs.find(kSerialized)->second, expectedSerialized);

  EXPECT_EQ(resultKvPairs.find(kPool)->second, pool);
  EXPECT_EQ(
      resultKvPairs.find(kVersion)->second,
      folly::to<std::string>(getMcInvalidationVersion()));
  EXPECT_EQ(resultKvPairs.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionType>(
          std::stoi(resultKvPairs.find(kType)->second)),
      DistributionType::Distribution);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(resultKvPairs.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(resultKvPairs.find(kSourceRegion)->second, sourceRegion);
}

TEST(McDistributionUtilsTest, spoolDeleteTest) {
  std::string targetRegion = "texas";
  std::string sourceRegion = "altoona";
  std::string message = "test";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  uint64_t resultBucketId = 0;
  folly::F14FastMap<std::string, std::string> resultKvPairs;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto bucketId, auto&& kvPairs, bool) {
        resultBucketId = bucketId;
        resultKvPairs = kvPairs;
        return true;
      };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};

  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);
  auto req = McDeleteRequest("key1");
  auto result = distributeDeleteRequest(
      req,
      axonCtxPtr,
      bucketId,
      DistributionType::Async,
      targetRegion,
      sourceRegion,
      message);
  EXPECT_TRUE(result);
  EXPECT_EQ(resultBucketId, bucketId);
  EXPECT_EQ(resultKvPairs.size(), 8);

  req = mcrouter::addDeleteRequestSource(
      req, memcache::McDeleteRequestSource::FAILED_INVALIDATION);

  auto expectedSerialized =
      invalidation::McInvalidationKvPairs::serialize<memcache::McDeleteRequest>(
          req)
          .to<std::string>();
  EXPECT_TRUE(resultKvPairs.find(kSerialized) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kRegion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kVersion) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kPool) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kMessage) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kType) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kOperation) != resultKvPairs.end());
  EXPECT_TRUE(resultKvPairs.find(kSourceRegion) != resultKvPairs.end());

  EXPECT_EQ(resultKvPairs.find(kSerialized)->second, expectedSerialized);

  EXPECT_EQ(resultKvPairs.find(kRegion)->second, targetRegion);
  EXPECT_EQ(resultKvPairs.find(kPool)->second, pool);
  EXPECT_EQ(
      resultKvPairs.find(kVersion)->second,
      folly::to<std::string>(getMcInvalidationVersion()));
  EXPECT_EQ(resultKvPairs.find(kMessage)->second, message);
  EXPECT_EQ(
      static_cast<DistributionType>(
          std::stoi(resultKvPairs.find(kType)->second)),
      DistributionType::Async);
  EXPECT_EQ(
      static_cast<DistributionOperation>(
          std::stoi(resultKvPairs.find(kOperation)->second)),
      DistributionOperation::Delete);
  EXPECT_EQ(resultKvPairs.find(kSourceRegion)->second, sourceRegion);
}
} // namespace facebook::memcache::invalidation::test
