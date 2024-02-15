/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/lib/Ch3HashFunc.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

namespace detail {
std::vector<std::pair<std::string, Ch3HashFunc>> buildPrefixMap(
    const std::unordered_map<std::string, uint64_t>& map);

FOLLY_ALWAYS_INLINE size_t getBucket(
    const folly::StringPiece key,
    const memcache::LowerBoundPrefixMap<Ch3HashFunc>& prefixMap,
    const Ch3HashFunc& defaultCh3) {
  if (!prefixMap.empty()) {
    auto it = prefixMap.findPrefix(key);
    if (it != prefixMap.end()) {
      return defaultCh3(folly::to<std::string>(it->key(), it->value()(key)));
    }
  }
  return defaultCh3(key);
}
} // namespace detail

struct McBucketRouteSettings {
  size_t totalBuckets;
  std::string salt;
  std::string bucketizationKeyspace;
  std::unordered_map<std::string, uint64_t> prefixToBuckets;
};

/**
 * The route handle is intended to be used to enable bucket-based routing.
 * When the bucket-based routing is enabled, the router will consistently
 * hash the key to a specific bucket out of configured "totalBuckets" count,
 * construct the routing key based on the resulted bucket and route the request
 * based on this key.
 *
 * Additional feature is co-location of keys with common prefixes into a small
 * set of buckets to improve performance - can be enabled by adding a
 * "total_buckets_by_prefix" config like this:
 *
 *   "bucketize": True,
 *   "total_buckets": 1000000,
 *   "total_buckets_by_prefix": {
 *     "asdf": 1000,
 *     "qwert": 2000,
 *     "zxcv": 3000
 *   },
 *   "bucketization_keyspace": "main",
 *
 * This particular route handle only calculates the bucket id and adds it
 * to the fiber context.
 * The downstream route's responsibility is to fetch the bucket id from the
 * fiber context and make the actual routing decision.
 *
 * Config:
 * - bucketize(bool) - enable the bucketization
 * - total_buckets(int) - total number of buckets
 * - total_buckets_by_prefix(object) - map of prefixes to total_buckets. Used to
 * co-locate keys under a prefix into a smaller set of buckets to increase
 * batching.
 */
template <class RouterInfo>
class McBucketRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  McBucketRoute(RouteHandlePtr rh, McBucketRouteSettings& settings)
      : rh_(std::move(rh)),
        totalBuckets_(settings.totalBuckets),
        ch3_(totalBuckets_),
        bucketizationKeyspace_(settings.bucketizationKeyspace),
        prefixMap_(detail::buildPrefixMap(settings.prefixToBuckets)) {}

  std::string routeName() const {
    return fmt::format(
        "bucketize|total_buckets={}|bucketization_keyspace={}|prefix_map_enabled={}",
        totalBuckets_,
        bucketizationKeyspace_,
        prefixMap_.empty() ? "false" : "true");
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    // when we execute traverse providing a pre-calculated bucket id,
    // no need to recalculate it:
    if (fiber_local<RouterInfo>::getBucketId().has_value()) {
      return t(*rh_, req);
    }
    auto bucketId = folly::fibers::runInMainContext([this, &req]() {
      return detail::getBucket(getRoutingKey<Request>(req), prefixMap_, ch3_);
    });
    if (auto* ctx = fiber_local<RouterInfo>::getTraverseCtx()) {
      ctx->recordBucketizationData(
          req.key_ref()->keyWithoutRoute().str(),
          bucketId,
          bucketizationKeyspace_);
    }
    return fiber_local<RouterInfo>::runWithLocals([this, &req, &t, bucketId]() {
      fiber_local<RouterInfo>::setBucketId(bucketId);
      return t(*rh_, req);
    });
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto bucketId = folly::fibers::runInMainContext([this, &req]() {
      return detail::getBucket(getRoutingKey<Request>(req), prefixMap_, ch3_);
    });
    auto& ctx = fiber_local<RouterInfo>::getSharedCtx();

    if (FOLLY_UNLIKELY(ctx->recordingBucketData())) {
      ctx->recordBucketizationData(
          req.key_ref()->keyWithoutRoute().str(),
          bucketId,
          bucketizationKeyspace_);
      return createReply<Request>(DefaultReply, req);
    }
    return routeImpl(req, bucketId);
  }

  template <class Request>
  ReplyT<Request> routeImpl(const Request& req, const size_t bucketId) const {
    auto proxy = &fiber_local<RouterInfo>::getSharedCtx()->proxy();
    proxy->stats().increment(bucketized_routing_stat);
    return fiber_local<RouterInfo>::runWithLocals([this, &req, bucketId]() {
      fiber_local<RouterInfo>::setBucketId(bucketId);
      return rh_->route(req);
    });
  }

 private:
  const RouteHandlePtr rh_;
  const size_t totalBuckets_{0};
  const Ch3HashFunc ch3_;
  const std::string bucketizationKeyspace_;
  const memcache::LowerBoundPrefixMap<Ch3HashFunc> prefixMap_;
};

McBucketRouteSettings parseMcBucketRouteSettings(const folly::dynamic& json);

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeMcBucketRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json);

} // namespace facebook::memcache::mcrouter

#include "mcrouter/routes/McBucketRoute-inl.h"
