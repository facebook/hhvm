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
#include "mcrouter/lib/network/gen/MemcacheRouteHandleIf.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

struct McBucketRouteSettings {
  size_t totalBuckets;
  size_t bucketizeUntil;
  std::string salt;
  std::string bucketizationKeyspace;
};

/**
 * The route handle is intended to be used to enable bucket-based routing.
 * When the bucket-based routing is enabled, the router will consistently
 * hash the key to a specific bucket out of configured "totalBuckets" count,
 * construct the routing key based on the resulted bucket and route the request
 * based on this key.
 *
 * This particular route handle only calculates the bucket id and adds it
 * to the fiber context.
 * The downstream route's responsibility is to fetch the bucket id from the
 * fiber context and make the actual routing decision.
 *
 * Config:
 * - bucketize(bool) - enable the bucketization
 * - total_buckets(int) - total number of buckets
 * - bucketize_until(int) - enable the handle for buckets until (exclusive)
 *   this number. Must be less than total_buckets. Needed for gradual migration.
 */
class McBucketRoute {
 public:
  McBucketRoute(
      std::shared_ptr<MemcacheRouteHandleIf> rh,
      McBucketRouteSettings& settings)
      : rh_(std::move(rh)),
        totalBuckets_(settings.totalBuckets),
        bucketizeUntil_(settings.bucketizeUntil),
        salt_(settings.salt),
        ch3_(totalBuckets_),
        bucketizationKeyspace_(settings.bucketizationKeyspace) {}

  std::string routeName() const {
    return fmt::format(
        "bucketize|total_buckets={}|bucketize_until={}|salt={}|bucketization_keyspace={}",
        totalBuckets_,
        bucketizeUntil_,
        salt_,
        bucketizationKeyspace_);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<MemcacheRouteHandleIf>& t) const {
    auto bucketId = folly::fibers::runInMainContext([this, &req]() {
      return ch3_(getRoutingKey<Request>(req, this->salt_));
    });
    if (bucketId < bucketizeUntil_) {
      if (auto* ctx = fiber_local<MemcacheRouterInfo>::getTraverseCtx()) {
        ctx->recordBucketizationData(
            req.key_ref()->keyWithoutRoute().str(),
            bucketId,
            bucketizationKeyspace_);
      }
      return fiber_local<MemcacheRouterInfo>::runWithLocals(
          [this, &req, &t, bucketId]() {
            fiber_local<MemcacheRouterInfo>::setBucketId(bucketId);
            return t(*rh_, req);
          });
    }
    return t(*rh_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto bucketId = folly::fibers::runInMainContext([this, &req]() {
      return ch3_(getRoutingKey<Request>(req, this->salt_));
    });
    auto& ctx = fiber_local<MemcacheRouterInfo>::getSharedCtx();

    if (UNLIKELY(ctx->recordingBucketData())) {
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
    if (bucketId < bucketizeUntil_) {
      auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
      proxy->stats().increment(bucketized_routing_stat);
      return fiber_local<MemcacheRouterInfo>::runWithLocals(
          [this, &req, bucketId]() {
            fiber_local<MemcacheRouterInfo>::setBucketId(bucketId);
            return rh_->route(req);
          });
    }
    return rh_->route(req);
  }

 private:
  const std::shared_ptr<MemcacheRouteHandleIf> rh_;
  const size_t totalBuckets_{0};
  const size_t bucketizeUntil_{0};
  const std::string salt_;
  const Ch3HashFunc ch3_;
  const std::string bucketizationKeyspace_;
};

std::shared_ptr<MemcacheRouteHandleIf> makeMcBucketRoute(
    std::shared_ptr<MemcacheRouteHandleIf> rh,
    const folly::dynamic& json);

} // namespace facebook::memcache::mcrouter
