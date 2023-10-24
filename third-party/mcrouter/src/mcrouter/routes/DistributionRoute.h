/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>

#include "mcrouter/McSpoolUtils.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestContextTyped.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/invalidation/McInvalidationDefs.h"
#include "mcrouter/lib/network/AccessPoint.h"
#include "mcrouter/routes/RoutingUtils.h"

namespace facebook::memcache::mcrouter {

struct DistributionRouteSettings {
  bool distributedDeleteRpcEnabled{true};
  bool replay{false};
};

constexpr std::string_view kAsynclogDistributionEndpoint = "0.0.0.0";

/**
 * The route handle is used to route cross-region requests via DL
 *
 * Config:
 * - distribution_delete_rpc_enabled(bool) - enable sending the request via rpc
 *   after it is distributed
 * - replay(bool) - enable replay mode (for mcreplay)
 */
template <class RouterInfo>
class DistributionRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  DistributionRoute(RouteHandlePtr rh, DistributionRouteSettings& settings)
      : rh_(std::move(rh)),
        distributedDeleteRpcEnabled_(settings.distributedDeleteRpcEnabled),
        replay_{settings.replay} {}

  std::string routeName() const {
    return fmt::format(
        "distribution|distributed_delete_rpc_enabled={}|replay={}",
        distributedDeleteRpcEnabled_ ? "true" : "false",
        replay_ ? "true" : "false");
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*rh_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    return rh_->route(req);
  }

  /**
   *  Delete can be:
   *   1. In-region rpc delete
   *      1.1 With no routing prefix
   *      1.2 With current region in the prefix
   *   2. Cross-region rpc delete having routing prefix with another region
   *   3. Broadcast delete with routing prefix = /(star)/(star)/
   *
   *  If distribution is enabled, we write 2 and 3 to Axon.
   *  If write to Axon fails, we spool to Async log with the routing prefix.
   */
  McDeleteReply route(const McDeleteRequest& req) const {
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    // In mcreplay case we try to infer target region from request
    auto distributionRegion = FOLLY_LIKELY(!replay_)
        ? fiber_local<RouterInfo>::getDistributionTargetRegion()
        : inferDistributionRegionForReplay(req, proxy);

    if (FOLLY_LIKELY(!distributionRegion.has_value())) {
      return rh_->route(req);
    }

    auto& axonCtx = fiber_local<RouterInfo>::getAxonCtx();
    auto bucketId = fiber_local<RouterInfo>::getBucketId();
    assert(axonCtx && bucketId);

    bool spoolSucceeded = false;
    auto source = distributionRegion.value().empty()
        ? memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION
        : memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION;
    auto finalReq = addDeleteRequestSource(req, source);
    finalReq.bucketId_ref() = fmt::to_string(*bucketId);
    auto axonLogRes = spoolAxonProxy(
        finalReq, axonCtx, *bucketId, std::move(*distributionRegion));
    if (axonLogRes) {
      proxy.stats().increment(distribution_axon_write_success_stat);
    }
    spoolSucceeded |= axonLogRes;
    if (FOLLY_UNLIKELY(!axonLogRes)) {
      proxy.stats().increment(distribution_axon_write_failed_stat);
      const auto host =
          std::make_shared<AccessPoint>(kAsynclogDistributionEndpoint);
      spoolSucceeded |= spoolAsynclog(
          &proxy,
          finalReq,
          host,
          true,
          fiber_local<RouterInfo>::getAsynclogName());
    }
    if (!spoolSucceeded) {
      proxy.stats().increment(distribution_async_spool_failed_stat);
    }
    // if spool to Axon or Asynclog succeeded and rpc is disabled, we return
    // default reply to the client:
    if (!distributedDeleteRpcEnabled_) {
      return spoolSucceeded ? createReply(DefaultReply, finalReq)
                            : McDeleteReply(carbon::Result::LOCAL_ERROR);
    }
    return rh_->route(req);
  }

 private:
  const RouteHandlePtr rh_;
  const bool distributedDeleteRpcEnabled_;
  const bool replay_;

  std::optional<std::string> inferDistributionRegionForReplay(
      const McDeleteRequest& req,
      ProxyBase& proxy) const {
    auto sourceIt =
        req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource);
    if (FOLLY_LIKELY(sourceIt == req.attributes_ref()->end())) {
      return std::nullopt;
    }
    switch (static_cast<McDeleteRequestSource>(sourceIt->second)) {
      case McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION:
        if (!req.key_ref()->routingPrefix().empty()) {
          auto routingPrefix = RoutingPrefix(req.key_ref()->routingPrefix());
          return routingPrefix.getRegion().str();
        }
        proxy.stats().increment(
            distribution_replay_xregion_directed_no_prefix_error_stat);
        throw std::logic_error(
            "Cross-region directed invalidation request must have routing prefix");
      case McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION:
        return "";
      default:
        return std::nullopt;
    }
  }
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeDistributionRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json);

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeDistributionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

} // namespace facebook::memcache::mcrouter

#include "mcrouter/routes/DistributionRoute-inl.h"
