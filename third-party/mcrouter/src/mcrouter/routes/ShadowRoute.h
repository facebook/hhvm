/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <folly/Function.h>
#include <folly/Optional.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/routes/DefaultShadowPolicy.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/ShadowRouteIf.h"
#include "mcrouter/routes/ShadowSettings.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * Shadowing using dynamic settings.
 *
 * Always sends the request to normalRoute.
 * In addition, asynchronously sends the same request to shadowRoutes if key
 * hash is within settings range
 * Key range might be updated at runtime.
 * We can shadow to multiple shadow destinations for a given normal route.
 */
template <class RouterInfo, class ShadowPolicy>
class ShadowRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  static std::string routeName() {
    return "shadow";
  }

  ShadowRoute(
      std::shared_ptr<RouteHandleIf> normalRoute,
      ShadowData<RouterInfo> shadowData,
      ShadowPolicy shadowPolicy)
      : normal_(std::move(normalRoute)),
        shadowData_(std::move(shadowData)),
        shadowPolicy_(std::move(shadowPolicy)) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (t(*normal_, req)) {
      return true;
    }
    return fiber_local<RouterInfo>::runWithLocals([this, &req, &t]() mutable {
      fiber_local<RouterInfo>::addRequestClass(RequestClass::kShadow);
      for (auto& shadowData : shadowData_) {
        if (t(*shadowData.first, req)) {
          return true;
        }
      }
      return false;
    });
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    std::shared_ptr<const Request> adjustedNormalReq;
    folly::Optional<ReplyT<Request>> normalReply;
    for (const auto& iter : shadowData_) {
      if (shouldShadow(req, iter.second.get())) {
        auto shadow = iter.first;
        if (!shadow) {
          if (auto& reqCtx = fiber_local<RouterInfo>::getSharedCtx()) {
            MC_LOG_FAILURE(
                reqCtx->proxy().router().opts(),
                failure::Category::kInvalidConfig,
                "ShadowRoute: ShadowData has unexpected nullptr route handle");
          }
          continue;
        }

        if constexpr (ShadowPolicy::template supports<Request>()) {
          if (!adjustedNormalReq) {
            adjustedNormalReq = shadowPolicy_.makeAdjustedNormalRequest(req);
            assert(adjustedNormalReq);
          }

          if (!normalReply &&
              shadowPolicy_.template shouldDelayShadow<Request>()) {
            normalReply = normal_->route(*adjustedNormalReq);
          }

          dispatchShadowRequest(
              std::move(shadow),
              shadowPolicy_.makeShadowRequest(adjustedNormalReq),
              normalReply ? shadowPolicy_.makePostShadowReplyFn(*normalReply)
                          : nullptr);
        }
      }
    }

    return normalReply
        ? std::move(*normalReply)
        : normal_->route(adjustedNormalReq ? *adjustedNormalReq : req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> normal_;
  const ShadowData<RouterInfo> shadowData_;
  ShadowPolicy shadowPolicy_;

  template <class Request>
  bool shouldShadow(const Request& req, ShadowSettings* settings) const {
    auto& ctx = fiber_local<RouterInfo>::getSharedCtx();

    if (!settings) {
      if (ctx) {
        MC_LOG_FAILURE(
            ctx->proxy().router().opts(),
            failure::Category::kInvalidConfig,
            "ShadowRoute: ShadowSettings is nullptr");
      }
      return false;
    }

    auto bucketId = fiber_local<RouterInfo>::getBucketId();
    if (!ctx) {
      LOG_FAILURE(
          "mcrouter",
          failure::Category::kInvalidConfig,
          "ShadowRoute: ProxyRequestContext is nullptr. Ignoring randomness.");
      return settings->shouldShadowKey(req, bucketId);
    }

    return settings->shouldShadow(
        req, bucketId, ctx->proxy().randomGenerator());
  }

  template <class Request>
  void dispatchShadowRequest(
      std::shared_ptr<RouteHandleIf> shadow,
      std::shared_ptr<Request> adjustedReq,
      folly::Function<void(const ReplyT<Request>&)> postShadowReplyFn) const;
};

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeShadowRouteDefault(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> normalRoute,
    ShadowData<RouterInfo> shadowData,
    DefaultShadowPolicy shadowPolicy);

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
makeShadowRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> children,
    ProxyBase& proxy,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider);

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
makeShadowRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    ProxyBase& proxy,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "ShadowRoute-inl.h"
