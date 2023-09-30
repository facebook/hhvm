/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/RateLimiter.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

namespace mcrouter {

/**
 * Requests sent through this route will be rate limited according
 * to settings in the RateLimiter passed to the constructor.
 *
 * See comments in TokenBucket.h for algorithm details.
 */
template <class RouteHandleIf>
class RateLimitRoute {
 public:
  std::string routeName() const {
    auto rlStr = rl_.toDebugStr();
    if (rlStr.empty()) {
      return "rate-limit";
    }
    return "rate-limit|" + rlStr;
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    // Traverse intentionally doesn't implement rate-limiting or fallback, to
    // avoid changing state
    return t(*target_, req);
  }

  RateLimitRoute(
      std::shared_ptr<RouteHandleIf> target,
      RateLimiter rl,
      std::shared_ptr<RouteHandleIf> fallback)
      : target_(std::move(target)),
        fallback_(std::move(fallback)),
        rl_(std::move(rl)) {}

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    if (FOLLY_LIKELY(rl_.canPassThrough<Request>())) {
      return target_->route(req);
    }
    if (fallback_) {
      return fallback_->route(req);
    }
    return createReply(DefaultReply, req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> target_;
  const std::shared_ptr<RouteHandleIf> fallback_;
  RateLimiter rl_;
};

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> createRateLimitRoute(
    std::shared_ptr<RouteHandleIf> normalRoute,
    RateLimiter rateLimiter,
    std::shared_ptr<RouteHandleIf> fallbackRoute = nullptr) {
  return makeRouteHandle<RouteHandleIf, RateLimitRoute>(
      std::move(normalRoute), std::move(rateLimiter), std::move(fallbackRoute));
}

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> makeRateLimitRoute(
    RouteHandleFactory<RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "RateLimitRoute is not an object");
  auto jtarget = json.get_ptr("target");
  checkLogic(jtarget, "RateLimitRoute: target not found");
  auto target = factory.create(*jtarget);
  auto jrates = json.get_ptr("rates");
  checkLogic(jrates, "RateLimitRoute: rates not found");
  std::shared_ptr<RouteHandleIf> fallback;
  if (auto jfallback = json.get_ptr("fallback")) {
    checkLogic(
        jfallback != jtarget,
        "RateLimitRoute: target and fallback are the same");
    fallback = factory.create(*jfallback);
  }
  return createRateLimitRoute(
      std::move(target), RateLimiter(*jrates), std::move(fallback));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
