/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Format.h>
#include <folly/Optional.h>
#include <folly/ScopeGuard.h>
#include <folly/fibers/FiberManager.h>

#include "mcrouter/AsyncLog.h"
#include "mcrouter/AsyncWriter.h"
#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McReqUtil.h"
#include "mcrouter/McSpoolUtils.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyDestination.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/FailoverUtil.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * Routes a request to a single ProxyDestination.
 * This is the lowest level in Mcrouter's RouteHandle tree.
 */
template <class RouterInfo, class Transport>
class DestinationRoute {
 public:
  std::string routeName() const {
    return folly::sformat(
        "host|pool={}|id={}|ap={}|timeout={}ms|keep_routing_prefix={}|msb={}",
        poolName_,
        indexInPool_,
        destination_->accessPoint()->toString(),
        timeout_.count(),
        keepRoutingPrefix_,
        destination_->accessPoint()->getFailureDomain());
  }

  /**
   * @param destination The destination where the request is to be sent
   */
  DestinationRoute(
      std::shared_ptr<ProxyDestination<Transport>> destination,
      folly::StringPiece poolName,
      size_t indexInPool,
      int32_t poolStatIdx,
      std::chrono::milliseconds timeout,
      bool disableRequestDeadlineCheck,
      bool keepRoutingPrefix)
      : destination_(std::move(destination)),
        poolName_(poolName),
        indexInPool_(indexInPool),
        poolStatIndex_(poolStatIdx),
        timeout_(timeout),
        disableRequestDeadlineCheck_(disableRequestDeadlineCheck),
        keepRoutingPrefix_(keepRoutingPrefix) {
    destination_->setPoolStatsIndex(poolStatIdx);
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<typename RouterInfo::RouteHandleIf>& t) const {
    PoolContext poolContext{
        poolName_,
        indexInPool_,
        fiber_local<RouterInfo>::getRequestClass().is(RequestClass::kShadow)};
    const auto& accessPoint = *destination_->accessPoint();
    if (auto* ctx = fiber_local<RouterInfo>::getTraverseCtx()) {
      ctx->recordDestination(poolContext, accessPoint);
    }
    return t(accessPoint, poolContext, req) &&
        !destination_->tracker()->isTko();
  }

  memcache::McDeleteReply route(const memcache::McDeleteRequest& req) const {
    auto& axonCtx = fiber_local<RouterInfo>::getAxonCtx();
    auto bucketId = fiber_local<RouterInfo>::getBucketId();
    // Axon invalidation only enabled when request is bucketized
    if (FOLLY_UNLIKELY(bucketId && axonCtx && axonCtx->allDelete)) {
      auto finalReq = addDeleteRequestSource(
          req, memcache::McDeleteRequestSource::FAILED_INVALIDATION);
      // Make sure bucket id is set in request
      finalReq.bucketId_ref() = fmt::to_string(*bucketId);
      spool(finalReq, axonCtx, bucketId);
      auto reply = createReply(DefaultReply, finalReq);
      reply.setDestination(destination_->accessPoint());
      return reply;
    }
    auto reply = routeWithDestination(req);
    if (isFailoverErrorResult(*reply.result_ref()) &&
        spool(req, axonCtx, bucketId)) {
      reply = createReply(DefaultReply, req);
      reply.setDestination(destination_->accessPoint());
    }
    return reply;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    return routeWithDestination(req);
  }

 private:
  const std::shared_ptr<ProxyDestination<Transport>> destination_;
  const folly::StringPiece poolName_;
  const size_t indexInPool_;
  const int32_t poolStatIndex_{-1};
  const std::chrono::milliseconds timeout_;
  size_t pendingShadowReqs_{0};
  const bool disableRequestDeadlineCheck_;
  const bool keepRoutingPrefix_;

  template <class Request>
  ReplyT<Request> routeWithDestination(const Request& req) const {
    auto reply = checkAndRoute(req);
    reply.setDestination(destination_->accessPoint());
    return reply;
  }

  template <class Request>
  ReplyT<Request> checkAndRoute(const Request& req) const {
    auto& ctx = fiber_local<RouterInfo>::getSharedCtx();
    auto requestClass = fiber_local<RouterInfo>::getRequestClass();
    bool isShadow = requestClass.is(RequestClass::kShadow);
    auto proxy = &ctx->proxy();
    // If not a shadow destination, check if the request deadline has exceeded
    // If yes, return DeadlineExceeded reply
    if (!isShadow && !disableRequestDeadlineCheck_ &&
        isRequestDeadlineExceeded(req)) {
      // Return remote error until all clients are updated to latest version
      // And un-comment the following line for returning the correct response
      // return constructAndLog(req, *ctx, DeadlineExceededReply);
      return constructAndLog(
          req,
          *ctx,
          RemoteErrorReply,
          std::string("Failed to send request - deadline exceeded"));
    }

    carbon::Result tkoReason;
    if (!destination_->maySend(tkoReason)) {
      return constructAndLog(
          req,
          *ctx,
          TkoReply,
          folly::to<std::string>(
              "Server unavailable. Reason: ",
              carbon::resultToString(tkoReason)));
    }

    if (poolStatIndex_ >= 0) {
      ctx->setPoolStatsIndex(poolStatIndex_);
    }
    if (ctx->recording()) {
      ctx->recordDestination(
          PoolContext{poolName_, indexInPool_, isShadow},
          *destination_->accessPoint());
      return constructAndLog(req, *ctx, DefaultReply, req);
    }

    if (isShadow) {
      if ((proxy->router().opts().target_max_shadow_requests > 0 &&
           pendingShadowReqs_ >=
               proxy->router().opts().target_max_shadow_requests) ||
          (proxy->router().opts().proxy_max_inflight_shadow_requests > 0 &&
           proxy->stats().getValue(destination_inflight_shadow_reqs_stat) >=
               proxy->router().opts().proxy_max_inflight_shadow_requests)) {
        return constructAndLog(req, *ctx, ErrorReply);
      }
      auto& mutableCounter = const_cast<size_t&>(pendingShadowReqs_);
      ++mutableCounter;
      proxy->stats().increment(destination_inflight_shadow_reqs_stat, 1);
      proxy->stats().setValue(
          destination_max_inflight_shadow_reqs_stat,
          std::max(
              proxy->stats().getValue(
                  destination_max_inflight_shadow_reqs_stat),
              proxy->stats().getValue(destination_inflight_reqs_stat)));
    }

    SCOPE_EXIT {
      if (isShadow) {
        auto& mutableCounter = const_cast<size_t&>(pendingShadowReqs_);
        --mutableCounter;
        proxy->stats().decrement(destination_inflight_shadow_reqs_stat, 1);
      }
    };

    return doRoute(req, *ctx);
  }

  template <class Request, class... Args>
  ReplyT<Request> constructAndLog(
      const Request& req,
      ProxyRequestContextWithInfo<RouterInfo>& ctx,
      Args&&... args) const {
    auto now = nowUs();
    auto reply = createReply<Request>(std::forward<Args>(args)...);
    auto bucketIdOptional = bucketIdOpt();
    std::string_view bucketId;
    if (bucketIdOptional.has_value()) {
      bucketId = *bucketIdOptional;
    }
    std::optional<Request> newReq;
    folly::StringPiece strippedRoutingPrefix;
    if (!keepRoutingPrefix_ && !req.key_ref()->routingPrefix().empty()) {
      newReq.emplace(req);
      newReq->key_ref()->stripRoutingPrefix();
      strippedRoutingPrefix = req.key_ref()->routingPrefix();
    }
    const auto& reqToSend = newReq ? *newReq : req;
    RpcStatsContext rpcContext;
    ctx.onBeforeRequestSent(
        poolName_,
        *destination_->accessPoint(),
        strippedRoutingPrefix,
        reqToSend,
        fiber_local<RouterInfo>::getRequestClass(),
        now,
        bucketId);
    ctx.onReplyReceived(
        poolName_,
        std::optional<size_t>(indexInPool_),
        *destination_->accessPoint(),
        strippedRoutingPrefix,
        reqToSend,
        reply,
        fiber_local<RouterInfo>::getRequestClass(),
        now,
        now,
        poolStatIndex_,
        rpcContext,
        fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
        fiber_local<RouterInfo>::getExtraDataCallbacks(),
        bucketId);
    return reply;
  }

  template <class Request>
  ReplyT<Request> doRoute(
      const Request& req,
      ProxyRequestContextWithInfo<RouterInfo>& ctx) const {
    DestinationRequestCtx dctx(nowUs());
    std::optional<Request> newReq;
    folly::StringPiece strippedRoutingPrefix;
    if (!keepRoutingPrefix_ && !req.key_ref()->routingPrefix().empty()) {
      newReq.emplace(req);
      newReq->key_ref()->stripRoutingPrefix();
      strippedRoutingPrefix = req.key_ref()->routingPrefix();
    }
    maybeAddBucketId(newReq, req);

    uint64_t remainingDeadlineTime = 0;
    uint64_t totalDestTimeout = 0;
    auto requestClass = fiber_local<RouterInfo>::getRequestClass();
    bool isShadow = requestClass.is(RequestClass::kShadow);

    if (!isShadow && !disableRequestDeadlineCheck_) {
      auto remainingTime = getRemainingTime(req);
      // If deadline request is being used, initialize total timeout
      // (sum of request timeout and connect timeout) and remaining time to
      // deadline
      if (remainingTime.first) {
        remainingDeadlineTime = remainingTime.second;
        totalDestTimeout =
            timeout_.count() + destination_->shortestConnectTimeout().count();
      }
    }

    // Copy the request if failover count is greater than zero or if the
    // total destination timeout is less the remaining time to deadline
    if (fiber_local<RouterInfo>::getFailoverCount() > 0 ||
        totalDestTimeout < remainingDeadlineTime) {
      if (!newReq) {
        newReq.emplace(req);
      }
      if (totalDestTimeout < remainingDeadlineTime) {
        auto proxy = &fiber_local<RouterInfo>::getSharedCtx()->proxy();
        proxy->stats().increment(request_deadline_num_copy_stat);
        setRequestDeadline(*newReq, totalDestTimeout);
      }
      if (fiber_local<RouterInfo>::getFailoverCount() > 0) {
        carbon::detail::setRequestFailover(*newReq);
        incFailoverHopCount(
            *newReq, fiber_local<RouterInfo>::getFailoverCount());
      }
    }

    const auto& reqToSend = newReq ? *newReq : req;
    auto bucketIdOptional = getBucketId(reqToSend);
    std::string_view bucketId;
    if (bucketIdOptional.has_value()) {
      bucketId = *bucketIdOptional;
    }

    ctx.onBeforeRequestSent(
        poolName_,
        *destination_->accessPoint(),
        strippedRoutingPrefix,
        reqToSend,
        fiber_local<RouterInfo>::getRequestClass(),
        dctx.startTime,
        bucketId);
    RpcStatsContext rpcContext;
    auto reply = destination_->send(reqToSend, dctx, timeout_, rpcContext);
    ctx.onReplyReceived(
        poolName_,
        std::optional<size_t>(indexInPool_),
        *destination_->accessPoint(),
        strippedRoutingPrefix,
        reqToSend,
        reply,
        fiber_local<RouterInfo>::getRequestClass(),
        dctx.startTime,
        dctx.endTime,
        poolStatIndex_,
        rpcContext,
        fiber_local<RouterInfo>::getNetworkTransportTimeUs(),
        fiber_local<RouterInfo>::getExtraDataCallbacks(),
        bucketId);

    fiber_local<RouterInfo>::incNetworkTransportTimeBy(
        dctx.endTime - dctx.startTime);
    fiber_local<RouterInfo>::setServerLoad(rpcContext.serverLoad);
    return reply;
  }

  template <class Request>
  typename std::
      enable_if_t<facebook::memcache::HasBucketIdTrait<Request>::value, void>
      maybeAddBucketId(
          std::optional<Request>& newReq,
          const Request& originalReq) const {
    auto bucketIdOptional = bucketIdOpt();
    if (FOLLY_LIKELY(!bucketIdOptional.has_value())) {
      return;
    }
    auto bucketId = *bucketIdOptional;
    if (FOLLY_UNLIKELY(!newReq.has_value())) {
      newReq.emplace(originalReq);
    }
    newReq->bucketId_ref() = bucketId;
  }

  template <class Request>
  typename std::
      enable_if_t<!facebook::memcache::HasBucketIdTrait<Request>::value, void>
      maybeAddBucketId(std::optional<Request>, const Request&) const {}

  FOLLY_ERASE std::optional<std::string> bucketIdOpt() const {
    auto bucketIdOptional = fiber_local<RouterInfo>::getBucketId();
    std::optional<std::string> bucketId = std::nullopt;
    if (bucketIdOptional.has_value()) {
      bucketId = folly::to<std::string>(*bucketIdOptional);
    }
    return bucketId;
  }

  FOLLY_NOINLINE bool spool(
      const McDeleteRequest& req,
      const std::shared_ptr<AxonContext>& axonCtx,
      const std::optional<uint64_t>& bucketId) const {
    // return true if axonlog is enabled and appending to axon client succeed.
    auto axonLogRes =
        bucketId && axonCtx && spoolAxonProxy(req, axonCtx, *bucketId);
    // Try spool asynclog for mcreplay when:
    // 1. Axon is not enabled
    // 2. Axon is enabled, but isn't configured with fallback to Asynclog.
    // 3. when fallback to asynclog is enabled, only attempt to spool asynclog
    // if axon write is failed
    bool asyncLogRes = false;
    if (!axonCtx || !axonCtx->fallbackAsynclog || !axonLogRes) {
      // return true if asyclog is enabled
      asyncLogRes = spoolAsynclog(
          &fiber_local<RouterInfo>::getSharedCtx()->proxy(),
          req,
          destination_->accessPoint(),
          keepRoutingPrefix_,
          fiber_local<RouterInfo>::getAsynclogName());
    }
    return asyncLogRes || axonLogRes;
  }
};

template <class RouterInfo, class Transport>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeDestinationRoute(
    std::shared_ptr<ProxyDestination<Transport>> destination,
    folly::StringPiece poolName,
    size_t indexInPool,
    int32_t poolStatsIndex,
    std::chrono::milliseconds timeout,
    bool disableRequestDeadlineCheck,
    bool keepRoutingPrefix) {
  return makeRouteHandleWithInfo<RouterInfo, DestinationRoute, Transport>(
      std::move(destination),
      poolName,
      indexInPool,
      poolStatsIndex,
      timeout,
      disableRequestDeadlineCheck,
      keepRoutingPrefix);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
