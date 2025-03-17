/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstddef>

#include "mcrouter/McDistributionUtils.h"
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
  std::string srcRegion;
  bool secureWrites{false};
};

constexpr std::string_view kAsynclogDistributionEndpoint = "0.0.0.0";

/**
 * The route handle is used to route cross-region requests via DL
 *
 * Config:
 * - distribution_delete_rpc_enabled(bool) - enable sending the request via rpc
 *   after it is distributed
 * - replay(bool) - enable replay mode (for mcreplay)
 * - src_region(string) - the region where the distribution request originated
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
        replay_{settings.replay},
        srcRegion_{settings.srcRegion},
        secureWrites_(settings.secureWrites) {}

  std::string routeName() const {
    return fmt::format(
        "distribution|distributed_delete_rpc_enabled={}|replay={}|distribution_source_region={}|secure_writes={}",
        distributedDeleteRpcEnabled_ ? "true" : "false",
        replay_ ? "true" : "false",
        srcRegion_,
        secureWrites_ ? "true" : "false");
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
   * @param req request to route
   * @return Reply from the route handle
   *
   *  SETs can be:
   *   1. In-region rpc
   *      1.1 With no routing prefix
   *      1.2 With current region in the prefix
   *   2. Cross-region directed rpc SET with a region prefix not equal to the
   *      current region.
   *   3. Broadcast SET with routing prefix = /(star)/(star)/
   *
   * In the first case, we skip distribution.
   * In the second case, we write to Axon synchronously and return the reply of
   * the write success/failure. In the third case, we write to Axon
   * asynchronously, also send an RPC request to the local region and return the
   * reply of the RPC.
   */
  McSetReply route(const McSetRequest& req) const {
    // the `distributionRegionOpt` optional in the fiber can be in 3 states:
    // - empty (no distribution)
    // - holds a value of "" (broadcast distribution)
    // - holds a value of a region name (directed cross-region distribution)
    auto distributionRegionOpt =
        fiber_local<RouterInfo>::getDistributionTargetRegion();
    if (FOLLY_LIKELY(!distributionRegionOpt.has_value())) {
      return rh_->route(req);
    }

    auto axonCtx = fiber_local<RouterInfo>::getAxonCtx();
    auto bucketId = fiber_local<RouterInfo>::getBucketId();
    assert(axonCtx && bucketId);
    auto finalReq = req;
    finalReq.bucketId_ref() = fmt::to_string(*bucketId);
    auto distributionRegion = distributionRegionOpt.value().empty()
        ? std::string(kBroadcast)
        : std::move(distributionRegionOpt.value());
    // for directed cross-region distribution write to Axon synchronously:
    if (FOLLY_UNLIKELY(distributionRegion != kBroadcast)) {
      return distributeWithLogging(
                 finalReq,
                 distributeWriteRequest,
                 axonCtx,
                 *bucketId,
                 distributionRegion,
                 srcRegion_,
                 std::nullopt,
                 secureWrites_)
          .first;
    }
    folly::fibers::addTask([this,
                            bucketId,
                            ctx = fiber_local<RouterInfo>::getSharedCtx(),
                            axonCtx,
                            finalReq = std::move(finalReq),
                            distributionRegion =
                                std::move(distributionRegion)]() {
      auto [_, axonLogRes] = distributeWithLogging(
          finalReq,
          distributeWriteRequest,
          axonCtx,
          *bucketId,
          distributionRegion,
          srcRegion_,
          std::nullopt,
          secureWrites_);
      if (axonLogRes) {
        ctx->proxy().stats().increment(
            distribution_set_axon_write_success_stat);
      } else {
        ctx->proxy().stats().increment(distribution_set_axon_write_fail_stat);
      }
    });
    // route to the local region:
    fiber_local<RouterInfo>::getSharedCtx()->proxy().stats().increment(
        distribution_set_local_region_write_stat);
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
   *  In the first case, we skip distribution.
   *  In the second case, we write to Axon synchronously and return the reply of
   *  the write success/failure. In the third case, we write to Axon
   *  asynchronously, also send an RPC request to the local region and return
   * the reply of the RPC.
   *
   *  If write to distribution layer fails, we spool to Async log with the
   * routing prefix.
   *
   *  If this logic is run in mcreplay, we attempt writing to distribution layer
   * synchronously until it succeeds.
   */
  McDeleteReply route(const McDeleteRequest& req) const {
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    // In mcreplay case we try to infer target region from request
    auto distributionRegionOpt = FOLLY_LIKELY(!replay_)
        ? fiber_local<RouterInfo>::getDistributionTargetRegion()
        : inferDistributionRegionForReplay(req, proxy);

    if (FOLLY_LIKELY(!distributionRegionOpt.has_value())) {
      return rh_->route(req);
    }

    auto& axonCtx = fiber_local<RouterInfo>::getAxonCtx();
    auto bucketId = fiber_local<RouterInfo>::getBucketId();
    assert(axonCtx && bucketId);

    auto source = distributionRegionOpt.value().empty()
        ? memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION
        : memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION;
    auto finalReq = addDeleteRequestSource(req, source);
    finalReq.bucketId_ref() = fmt::to_string(*bucketId);
    auto distributionRegion = distributionRegionOpt.value().empty()
        ? std::string(kBroadcast)
        : std::move(distributionRegionOpt.value());
    // If it is replay or directed cross-region, we write to distribution
    // synchronously:
    if (FOLLY_UNLIKELY(replay_ || distributionRegion != kBroadcast)) {
      return distributeWithLogging(
                 finalReq,
                 distributeDeleteRequest,
                 axonCtx,
                 bucketId.value(),
                 replay_ ? invalidation::DistributionType::Async
                         : invalidation::DistributionType::Distribution,
                 distributionRegion,
                 srcRegion_,
                 std::nullopt)
          .first;
    }

    folly::fibers::addTask([this,
                            bucketId,
                            ctx = fiber_local<RouterInfo>::getSharedCtx(),
                            axonCtx,
                            finalReq = std::move(finalReq),
                            distributionRegion =
                                std::move(distributionRegion)]() {
      auto [_, spoolSucceeded] = distributeWithLogging(
          finalReq,
          distributeDeleteRequest,
          axonCtx,
          bucketId.value(),
          invalidation::DistributionType::Distribution,
          distributionRegion,
          srcRegion_,
          std::nullopt);

      if (FOLLY_UNLIKELY(!spoolSucceeded)) {
        const auto host =
            std::make_shared<AccessPoint>(kAsynclogDistributionEndpoint);
        spoolSucceeded |= spoolAsynclog(
            &ctx->proxy(),
            finalReq,
            host,
            true,
            fiber_local<RouterInfo>::getAsynclogName());
        if (!spoolSucceeded) {
          ctx->proxy().stats().increment(distribution_async_spool_failed_stat);
        }
      }
    });
    // route to the local region:
    return rh_->route(req);
  }

 private:
  const RouteHandlePtr rh_;
  const bool distributedDeleteRpcEnabled_;
  const bool replay_;
  const std::string srcRegion_;
  const bool secureWrites_;

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

  template <typename Request>
  void onBeforeDistribution(
      const Request& req,
      ProxyRequestContextWithInfo<RouterInfo>& ctx,
      const std::string& bucketId,
      const DestinationRequestCtx& dctx) const {
    ctx.onBeforeRequestSent(
        /*poolName*/ kDistributionTargetMarkerForLog,
        /*ap*/ AccessPoint::defaultAp(),
        /*strippedRoutingPrefix*/ folly::StringPiece(),
        /*request*/ req,
        /*requestClass*/ fiber_local<RouterInfo>::getRequestClass(),
        /*startTimeUs*/ dctx.startTime,
        /*bucketId*/ bucketId);
  }

  template <typename Request>
  void onAfterDistribution(
      const Request& req,
      const typename Request::reply_type& reply,
      ProxyRequestContextWithInfo<RouterInfo>& ctx,
      const std::string& bucketId,
      const DestinationRequestCtx& dctx) const {
    RpcStatsContext rpcContext;
    ctx.onReplyReceived(
        /*poolName*/ kDistributionTargetMarkerForLog,
        /*poolIndex*/ std::nullopt,
        /*ap*/ AccessPoint::defaultAp(),
        /*strippedRoutingPrefix*/ folly::StringPiece(),
        /*request*/ req,
        /*reply*/ reply,
        /*requestClass*/ fiber_local<RouterInfo>::getRequestClass(),
        /*startTimeUs*/ dctx.startTime,
        /*endTimeUs*/ dctx.endTime,
        /*poolStatIndex*/ -1,
        /*rpcStatsContext*/ rpcContext,
        /*networkTransportTimeUs*/
        fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
        /*extraDataCallback*/ fiber_local<RouterInfo>::getExtraDataCallbacks(),
        /*bucketId*/ bucketId);
  }

  template <typename Request, typename DistrFn, typename... Args>
  FOLLY_ALWAYS_INLINE std::pair<ReplyT<Request>, bool> distributeWithLogging(
      const Request& req,
      DistrFn&& distributionFn,
      Args... args) const {
    auto& ctx = *fiber_local<RouterInfo>::getSharedCtx();
    DestinationRequestCtx dctx(nowUs());
    onBeforeDistribution(req, ctx, *req.bucketId_ref(), dctx);

    auto axonLogRes = distributionFn(req, std::forward<Args>(args)...);
    auto reply = axonLogRes ? createReply(DefaultReply, req)
                            : ReplyT<Request>(carbon::Result::LOCAL_ERROR);
    dctx.endTime = nowUs();
    onAfterDistribution(req, reply, ctx, *req.bucketId_ref(), dctx);

    if (axonLogRes) {
      ctx.proxy().stats().increment(distribution_axon_write_success_stat);
    } else {
      ctx.proxy().stats().increment(distribution_axon_write_failed_stat);
    }
    return {reply, axonLogRes};
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
