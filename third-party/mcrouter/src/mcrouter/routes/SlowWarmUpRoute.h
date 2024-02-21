/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/json/dynamic.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/SlowWarmUpRouteSettings.h"

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * This route handle allows slow warm up of cold memcached boxes. All it does is
 * route to "failoverRoute" straight away if the box is cold (instead of wasting
 * a network roundtrip), so it's purpose is to be used together with a failover
 * route handle.
 *
 * One SlowWarmUpRoute is created for each ProxyDestination, which allows us to
 * keep state related to the destination.
 *
 * This route handle is flexible and can be configured via the "settings"
 * property. Bellow is a list of parameters that can be tweaked to adjust
 * the behavior of this route handle:
 *
 * "enable_threshold": Threshold (double between 0 and 1) that will be used to
 *                     put the server in the warmup state. Whenever the hit rate
 *                     of the server goes bellow that threshold, we enter warmup
 *                     state.
 * "disable_threshold": Threshold (double between 0 and 1) that will be used to
 *                      remove the server from the warmup state. Whenver the hit
 *                      rate goes up that threshold, we exit warmup state (if
 *                      the box is being warmed up).
 * "start": Fraction (double between 0 and 1) of requests that we should send
 *          to the server being warmed up when its hit rate is 0.
 * "step": Step by which we increment the percentage of requests sent to the
 *         server.
 * "min_requests": Minimum number of requests necessary to start calculating
 *                 the hit rate. Before that number is reached, the destination
 *                 is considered "warm".
 *
 * To summarize, if a server is being warmed up, the percentage of requests to
 * send to server is calculated by the formula:
 *    start + (step * hitRate)
 */
template <class RouterInfo>
class SlowWarmUpRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  static std::string routeName() {
    return "slow-warmup";
  }

  SlowWarmUpRoute(
      std::shared_ptr<RouteHandleIf> target,
      std::shared_ptr<RouteHandleIf> failoverTarget,
      std::shared_ptr<SlowWarmUpRouteSettings> settings)
      : target_(std::move(target)),
        failoverTarget_(std::move(failoverTarget)),
        settings_(std::move(settings)) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (t(*target_, req)) {
      return true;
    }
    return t(*failoverTarget_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req, carbon::GetLikeT<Request> = 0)
      const {
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    if (warmingUp() && !shouldSendRequest(proxy.randomGenerator())) {
      return fiber_local<RouterInfo>::runWithLocals([this, &req]() {
        fiber_local<RouterInfo>::addRequestClass(RequestClass::kFailover);
        return failoverTarget_->route(req);
      });
    }

    return routeImpl(req);
  }

  template <class Request>
  ReplyT<Request> route(
      const Request& req,
      carbon::OtherThanT<Request, carbon::GetLike<>> = 0) const {
    return routeImpl(req);
  }

  template <class Request>
  ReplyT<Request> routeImpl(const Request& req) const {
    auto reply = target_->route(req);
    if (isHitResult(*reply.result_ref())) {
      ++stats_.hits;
    } else if (isMissResult(*reply.result_ref())) {
      ++stats_.misses;
    }
    return reply;
  }

 private:
  struct WarmUpStats {
    uint64_t hits{0};
    uint64_t misses{0};
    bool enabled{false};
  };

  const std::shared_ptr<RouteHandleIf> target_;
  const std::shared_ptr<RouteHandleIf> failoverTarget_;
  const std::shared_ptr<SlowWarmUpRouteSettings> settings_;
  mutable WarmUpStats stats_;

  bool warmingUp() const {
    if (stats_.enabled) {
      stats_.enabled = hitRate() < settings_->disableThreshold();
    } else {
      stats_.enabled = hitRate() < settings_->enableThreshold();
    }
    return stats_.enabled;
  }

  double hitRate() const {
    uint64_t total = stats_.hits + stats_.misses;
    if (total < settings_->minRequests()) {
      return 1.0;
    }
    return stats_.hits / static_cast<double>(total);
  }

  template <class RNG>
  bool shouldSendRequest(RNG& rng) const {
    double target = settings_->start() + (hitRate() * settings_->step());
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(
               rng) <= target;
  }
};

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeSlowWarmUpRoute(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> target,
    std::shared_ptr<typename RouterInfo::RouteHandleIf> failoverTarget,
    std::shared_ptr<SlowWarmUpRouteSettings> settings) {
  return makeRouteHandleWithInfo<RouterInfo, SlowWarmUpRoute>(
      std::move(target), std::move(failoverTarget), std::move(settings));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
