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
      [&](auto bucketId,
          auto&& kvPairs,
          bool isSecureWrites,
          const std::optional<std::string>&) -> folly::exception_wrapper {
    resultBucketId = bucketId;
    resultKvPairs = kvPairs;
    secureWrites = isSecureWrites;
    return folly::exception_wrapper();
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
  req.value() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto result = distributeWriteRequest(
      req, axonCtxPtr, bucketId, targetRegion, sourceRegion, message);
  EXPECT_FALSE(result.has_exception_ptr());
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
      [&](auto bucketId,
          auto&& kvPairs,
          bool isSecureWrites,
          const std::optional<std::string>&) -> folly::exception_wrapper {
    resultBucketId = bucketId;
    resultKvPairs = kvPairs;
    secureWrites = isSecureWrites;
    return folly::exception_wrapper();
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
  req.value() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto result = distributeWriteRequest(
      req,
      axonCtxPtr,
      bucketId,
      targetRegion,
      sourceRegion,
      message,
      /*secureWrites=*/true);
  EXPECT_FALSE(result.has_exception_ptr());
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
      [&](auto bucketId,
          auto&& kvPairs,
          bool,
          const std::optional<std::string>&) -> folly::exception_wrapper {
    resultBucketId = bucketId;
    resultKvPairs = kvPairs;
    return folly::exception_wrapper();
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
  EXPECT_FALSE(result.has_exception_ptr());
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
      [&](auto bucketId,
          auto&& kvPairs,
          bool,
          const std::optional<std::string>&) -> folly::exception_wrapper {
    resultBucketId = bucketId;
    resultKvPairs = kvPairs;
    return folly::exception_wrapper();
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
  EXPECT_FALSE(result.has_exception_ptr());
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
      [&](auto bucketId,
          auto&& kvPairs,
          bool,
          const std::optional<std::string>&) -> folly::exception_wrapper {
    resultBucketId = bucketId;
    resultKvPairs = kvPairs;
    return folly::exception_wrapper();
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
  EXPECT_FALSE(result.has_exception_ptr());
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

// The dedup key passed to the write proxy is the request key, so all writes to
// the same key collapse within the proxy's dedup window (hot-key suppression)
// regardless of value, while writes to different keys never collapse.
TEST(McDistributionUtilsTest, distributeSetKeyBasedDedupeKeyTest) {
  std::string targetRegion = "texas";
  std::string sourceRegion = "altoona";
  std::string pool = "main";
  uint64_t bucketId = 234532;
  std::optional<std::string> resultDedupeKey;

  facebook::memcache::mcrouter::AxonProxyWriteFn writeProxyFn =
      [&](auto,
          auto&&,
          bool,
          std::optional<std::string> dedupeKey) -> folly::exception_wrapper {
    resultDedupeKey = std::move(dedupeKey);
    return folly::exception_wrapper();
  };

  auto axonCtx = mcrouter::AxonContext{
      .fallbackAsynclog = false,
      .allDelete = false,
      .writeProxyFn = writeProxyFn,
      .defaultRegionFilter = sourceRegion,
      .poolFilter = pool};
  auto axonCtxPtr =
      std::make_shared<facebook::memcache::mcrouter::AxonContext>(axonCtx);

  auto makeReq = [](folly::StringPiece key, folly::StringPiece value) {
    auto req = McSetRequest(key);
    req.value() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, value);
    return req;
  };

  // Dedup key is the request key itself.
  auto req = makeReq("key1", "value");
  EXPECT_FALSE(distributeWriteRequest(
                   req, axonCtxPtr, bucketId, targetRegion, sourceRegion)
                   .has_exception_ptr());
  ASSERT_TRUE(resultDedupeKey.has_value());
  EXPECT_EQ(*resultDedupeKey, "key1");
  const auto firstKey = *resultDedupeKey;

  // Same key, different value -> same dedup key (collapses the hot key).
  resultDedupeKey.reset();
  auto sameKeyReq = makeReq("key1", "value2");
  EXPECT_FALSE(distributeWriteRequest(
                   sameKeyReq, axonCtxPtr, bucketId, targetRegion, sourceRegion)
                   .has_exception_ptr());
  ASSERT_TRUE(resultDedupeKey.has_value());
  EXPECT_EQ(*resultDedupeKey, firstKey);

  // Different key -> different dedup key (must NOT collapse).
  resultDedupeKey.reset();
  auto otherKeyReq = makeReq("key2", "value");
  EXPECT_FALSE(
      distributeWriteRequest(
          otherKeyReq, axonCtxPtr, bucketId, targetRegion, sourceRegion)
          .has_exception_ptr());
  ASSERT_TRUE(resultDedupeKey.has_value());
  EXPECT_NE(*resultDedupeKey, firstKey);
}
} // namespace facebook::memcache::invalidation::test
