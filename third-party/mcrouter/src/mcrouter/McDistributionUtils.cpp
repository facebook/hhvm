/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/McDistributionUtils.h"

#include <optional>

namespace facebook {
namespace memcache {
namespace mcrouter {

memcache::McDeleteRequest addDeleteRequestSource(
    const memcache::McDeleteRequest& req,
    memcache::McDeleteRequestSource source) {
  memcache::McDeleteRequest ret = req;
  ret.attributes_ref()->emplace(
      memcache::kMcDeleteReqAttrSource, static_cast<uint8_t>(source));
  return ret;
}

FOLLY_NOINLINE bool distributeWriteRequest(
    const memcache::McSetRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId,
    const std::string& targetRegion,
    const std::string& sourceRegion,
    std::optional<std::string> message,
    bool secureWrites) {
  std::optional<std::string> pool;
  if (!axonCtx->poolFilter.empty()) {
    pool.emplace(axonCtx->poolFilter);
  }

  // Run serialization off fiber
  auto kvPairs = folly::fibers::runInMainContext([&req,
                                                  &targetRegion,
                                                  &pool,
                                                  &message,
                                                  &sourceRegion]() {
    auto finalReq = req;
    finalReq.key_ref()->stripRoutingPrefix();
    auto serialized =
        invalidation::McInvalidationKvPairs::serialize<memcache::McSetRequest>(
            finalReq)
            .template to<std::string>();
    return invalidation::McInvalidationKvPairs::createAxonKvPairs(
        std::move(serialized),
        targetRegion,
        std::move(pool),
        std::move(message),
        invalidation::DistributionType::Distribution,
        invalidation::DistributionOperation::Write,
        sourceRegion);
  });
  return axonCtx->writeProxyFn(bucketId, std::move(kvPairs), secureWrites);
}

FOLLY_NOINLINE bool distributeDeleteRequest(
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId,
    invalidation::DistributionType type,
    const std::string& targetRegion,
    const std::string& sourceRegion,
    std::optional<std::string> message) {
  std::optional<std::string> region;
  if (targetRegion.empty()) {
    // targetRegion must be set
    return false;
  }
  region = targetRegion;
  std::optional<std::string> pool;
  if (!axonCtx->poolFilter.empty()) {
    pool.emplace(axonCtx->poolFilter);
  }
  // Run off fiber to save fiber stack for serialization
  auto kvPairs = folly::fibers::runInMainContext(
      [&req, &region, &pool, &message, &sourceRegion, type]() {
        auto source = McDeleteRequestSource::FAILED_INVALIDATION;
        if (type == invalidation::DistributionType::Distribution) {
          if (region.value() == kBroadcast) {
            source = McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION;
          } else {
            source = McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION;
          }
        }
        memcache::McDeleteRequest finalReq =
            req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource) ==
                req.attributes_ref()->cend()
            ? std::move(addDeleteRequestSource(req, source))
            : req;
        finalReq.key_ref()->stripRoutingPrefix();
        auto serialized = invalidation::McInvalidationKvPairs::serialize<
                              memcache::McDeleteRequest>(finalReq)
                              .template to<std::string>();
        return invalidation::McInvalidationKvPairs::createAxonKvPairs(
            std::move(serialized),
            std::move(region),
            std::move(pool),
            std::move(message),
            type,
            invalidation::DistributionOperation::Delete,
            sourceRegion);
      });
  return axonCtx->writeProxyFn(
      bucketId, std::move(kvPairs), /*secureWrites*/ false);
}

FOLLY_NOINLINE bool spoolAsynclog(
    ProxyBase* proxy,
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<const AccessPoint>& host,
    bool keepRoutingPrefix,
    folly::StringPiece asynclogName) {
  if (asynclogName.empty()) {
    return false;
  }
  folly::StringPiece key = keepRoutingPrefix ? req.key_ref()->fullKey()
                                             : req.key_ref()->keyWithoutRoute();
  folly::fibers::Baton baton;
  auto res = false;
  auto attr = *req.attributes_ref();
  const auto asyncWriteStartUs = nowUs();
  auto asyncWriter = proxy->router().asyncWriter();
  if (asyncWriter && host) {
    res = asyncWriter->run([&baton, &attr, &host, proxy, key, asynclogName]() {
      if (proxy->asyncLog().writeDelete(*host, key, asynclogName, attr)) {
        proxy->stats().increment(asynclog_spool_success_rate_stat);
      }
      baton.post();
    });
  }

  if (host && res) {
    // Don't reply to the user until we safely logged the request to disk
    baton.wait();
    const auto asyncWriteDurationUs = nowUs() - asyncWriteStartUs;
    proxy->stats().asyncLogDurationUs().insertSample(asyncWriteDurationUs);
    proxy->stats().increment(asynclog_requests_rate_stat);
  } else if (!host) {
    MC_LOG_FAILURE(
        proxy->router().opts(),
        memcache::failure::Category::kBrokenLogic,
        "Failed to enqueue asynclog request (key {}, pool {}) due to missing host info",
        key,
        asynclogName);
  } else if (!res) {
    MC_LOG_FAILURE(
        proxy->router().opts(),
        memcache::failure::Category::kOutOfResources,
        "Could not enqueue asynclog request (key {}, pool {})",
        key,
        asynclogName);
  }
  return true;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
