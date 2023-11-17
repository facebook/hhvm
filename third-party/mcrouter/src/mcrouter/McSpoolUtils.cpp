/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/McSpoolUtils.h"
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

FOLLY_NOINLINE bool spoolAxonProxy(
    const memcache::McDeleteRequest& req,
    const std::shared_ptr<AxonContext>& axonCtx,
    uint64_t bucketId,
    std::optional<std::string> targetRegion,
    std::optional<std::string> message) {
  std::optional<std::string> region;
  // if targetRegion has value, it's either x-region broadcast or directed
  // delete. For broadcast it will be an empty string, for x-region directed -
  // name of the region:
  if (targetRegion.has_value()) {
    region = targetRegion.value().empty() ? std::nullopt
                                          : std::optional(targetRegion.value());
    // otherwise if defaultRegionFilter property is set - use it for failed
    // invalidations:
  } else if (!axonCtx->defaultRegionFilter.empty()) {
    region.emplace(axonCtx->defaultRegionFilter);
    // otherwise it's a misconfig:
  } else {
    // if no target region configured, don't spool to Axon
    return false;
  }
  std::optional<std::string> pool;
  if (!axonCtx->poolFilter.empty()) {
    pool.emplace(axonCtx->poolFilter);
  }
  // Run off fiber to save fiber stack for serialization
  auto kvPairs =
      folly::fibers::runInMainContext([&req, &region, &pool, &message]() {
        auto finalReq =
            req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource) ==
                req.attributes_ref()->cend()
            ? std::move(addDeleteRequestSource(
                  req, memcache::McDeleteRequestSource::FAILED_INVALIDATION))
            : req;
        finalReq.key_ref()->stripRoutingPrefix();
        auto serialized = invalidation::McInvalidationKvPairs::serialize<
                              memcache::McDeleteRequest>(finalReq)
                              .template to<std::string>();
        return invalidation::McInvalidationKvPairs::createAxonKvPairs(
            serialized, std::move(region), std::move(pool), std::move(message));
      });
  return axonCtx->writeProxyFn(bucketId, std::move(kvPairs));
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
  folly::fibers::Baton b;
  auto res = false;
  auto attr = *req.attributes_ref();
  const auto asyncWriteStartUs = nowUs();
  auto asyncWriter = proxy->router().asyncWriter();
  if (asyncWriter && host) {
    res = asyncWriter->run([&b, &attr, &host, proxy, key, asynclogName]() {
      if (proxy->asyncLog().writeDelete(*host, key, asynclogName, attr)) {
        proxy->stats().increment(asynclog_spool_success_rate_stat);
      }
      b.post();
    });
  }

  if (!host) {
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
  } else {
    // Don't reply to the user until we safely logged the request to disk
    b.wait();
    const auto asyncWriteDurationUs = nowUs() - asyncWriteStartUs;
    proxy->stats().asyncLogDurationUs().insertSample(asyncWriteDurationUs);
    proxy->stats().increment(asynclog_requests_rate_stat);
  }
  return true;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
