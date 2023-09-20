/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <chrono>

#include <folly/Format.h>
#include <folly/fibers/Baton.h>

#include <folly/experimental/ReadMostlySharedPtr.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

// Latency helpers
template <typename Message, typename = std::enable_if_t<true>>
class HasBeforeLatencyUsTrait : public std::false_type {};
template <typename Message>
class HasBeforeLatencyUsTrait<
    Message,
    std::void_t<
        decltype(std::declval<std::decay_t<Message>&>().beforeLatencyUs_ref())>>
    : public std::true_type {};

template <typename Message, typename = std::enable_if_t<true>>
class HasAfterLatencyUsTrait : public std::false_type {};
template <typename Message>
class HasAfterLatencyUsTrait<
    Message,
    std::void_t<
        decltype(std::declval<std::decay_t<Message>&>().afterLatencyUs_ref())>>
    : public std::true_type {};

namespace detail {
folly::ReadMostlySharedPtr<folly::Timekeeper> getTimekeeperHighResSingleton();
}
/**
 * Injects latency before and/or after sending the request down to it's child.
 */
template <class RouterInfo>
class LatencyInjectionRoute {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

  /**
   * Constructs the latency injection route.
   *
   * @param rh              The child route handle
   * @param beforeLatency   Latency to inject before sending the request to "rh"
   * @param afterLatency    Latency to inject after sending the request to "rh"
   * @param totalLatency    The target  total latency in "rh", if set, pad
   * latency to this value if the total latency after injection is still under
   * this value
   * @param requestLatency  Boolean, inject latency before/after sending the
   * request using values from the request payload
   * @param maxRequestLatency  if not 0, the max amount of "requestLatency"
   * that is allowed to inject before or after sending the request
   */
  LatencyInjectionRoute(
      RouteHandlePtr rh,
      std::chrono::milliseconds beforeLatency,
      std::chrono::milliseconds afterLatency,
      std::chrono::milliseconds totalLatency,
      bool requestLatency,
      std::chrono::microseconds maxRequestLatency)
      : rh_(std::move(rh)),
        beforeLatency_(beforeLatency),
        afterLatency_(afterLatency),
        totalLatency_(totalLatency),
        requestLatency_(requestLatency),
        maxRequestLatency_(maxRequestLatency) {
    assert(rh_);
    assert(
        beforeLatency_.count() > 0 || afterLatency_.count() > 0 ||
        totalLatency_.count() > 0 || requestLatency_);
  }

  std::string routeName() const {
    return folly::sformat(
        "latency-injection|before:{}ms|after:{}ms|total:{}ms|"
        "request_payload:{}|max_request_latency_us:{}",
        beforeLatency_.count(),
        afterLatency_.count(),
        totalLatency_.count(),
        requestLatency_ ? "true" : "false",
        maxRequestLatency_.count());
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*rh_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto& proxy = fiber_local<RouterInfo>::getSharedCtx()->proxy();
    const auto before_ms = getCurrentTimeInMs();
    SCOPE_EXIT {
      if (totalLatency_.count() > 0) {
        auto elapsed =
            std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);

        // Pad latency out to total latency configured.
        if (totalLatency_ > elapsed) {
          proxy.stats().increment(mcrouter::total_latency_injected_stat);
          folly::fibers::Baton totalBaton;
          totalBaton.try_wait_for(totalLatency_ - elapsed);
        }
      }
    };

    // Fixed latency added before request is sent.
    if (beforeLatency_.count() > 0) {
      proxy.stats().increment(mcrouter::before_latency_injected_stat);
      folly::fibers::Baton beforeBaton;
      beforeBaton.try_wait_for(beforeLatency_);
    }

    std::chrono::microseconds beforeReqLatency{0};
    std::chrono::microseconds afterReqLatency{0};
    std::optional<Request> newReq;

    // Both beforeLatencyUs and afterLatencyUs fields have to be defined
    // for request specific latency injection to happen.
    if constexpr (
        HasBeforeLatencyUsTrait<Request>::value &&
        HasAfterLatencyUsTrait<Request>::value) {
      if (requestLatency_) {
        if (req.beforeLatencyUs_ref().has_value()) {
          beforeReqLatency =
              std::chrono::microseconds(*req.beforeLatencyUs_ref());
        }
        if (req.afterLatencyUs_ref().has_value()) {
          afterReqLatency =
              std::chrono::microseconds(*req.afterLatencyUs_ref());
        }
        // Reset request latencies to avoid downstream latency injection
        if (beforeReqLatency.count() > 0 || afterReqLatency.count() > 0) {
          newReq.emplace(req);
          newReq->afterLatencyUs_ref() = 0;
          newReq->beforeLatencyUs_ref() = 0;
        }
        // Inject per request latency if enabled
        if (beforeReqLatency.count() > 0 &&
            ((maxRequestLatency_.count() == 0) ||
             (beforeReqLatency.count() <= maxRequestLatency_.count()))) {
          proxy.stats().increment(
              mcrouter::before_request_latency_injected_stat);
          fiber_local<RouterInfo>::accumulateBeforeReqInjectedLatencyUs(
              beforeReqLatency.count());
          folly::futures::sleep(
              beforeReqLatency, detail::getTimekeeperHighResSingleton().get())
              .get();
        }
      }
    }

    auto& reqToSend = newReq ? *newReq : req;
    auto reply = rh_->route(reqToSend);
    // Optimize out the logic that is related to request level latency injection
    // for protocols that does not support it
    if constexpr (HasAfterLatencyUsTrait<Request>::value) {
      if (afterReqLatency.count() > 0 &&
          ((maxRequestLatency_.count() == 0) ||
           (afterReqLatency.count() <= maxRequestLatency_.count()))) {
        proxy.stats().increment(mcrouter::after_request_latency_injected_stat);
        fiber_local<RouterInfo>::accumulateAfterReqInjectedLatencyUs(
            afterReqLatency.count());
        folly::futures::sleep(
            afterReqLatency, detail::getTimekeeperHighResSingleton().get())
            .get();
      }
    }

    // Fixed latency added after reply is received.
    if (afterLatency_.count() > 0) {
      proxy.stats().increment(mcrouter::after_latency_injected_stat);
      folly::fibers::Baton afterBaton;
      afterBaton.try_wait_for(afterLatency_);
    }

    return reply;
  }

 private:
  const RouteHandlePtr rh_;
  const std::chrono::milliseconds beforeLatency_;
  const std::chrono::milliseconds afterLatency_;
  const std::chrono::milliseconds totalLatency_;
  bool requestLatency_;
  const std::chrono::microseconds maxRequestLatency_;
};

/**
 * Creates a LatencyInjectRoute from a json config.
 *
 * Sample json:
 * {
 *   "type": "LatencyInjectionRoute",
 *   "child": "PoolRoute|pool_name",
 *   "before_latency_ms": 10,
 *   "after_latency_ms": 20
 * }
 */
template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeLatencyInjectionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "LoadBalancerRoute: config is not an object.");

  auto jChild = json.get_ptr("child");
  checkLogic(
      jChild != nullptr, "LatencyInjectionRoute: 'child' property is missing.");
  auto child = factory.create(*jChild);

  auto jBeforeLatency = json.get_ptr("before_latency_ms");
  auto jAfterLatency = json.get_ptr("after_latency_ms");
  auto jTotalLatency = json.get_ptr("total_latency_ms");
  auto jRequestLatency = json.get_ptr("request_payload_latency");
  checkLogic(
      jBeforeLatency != nullptr || jAfterLatency != nullptr ||
          jTotalLatency != nullptr || jRequestLatency != nullptr,
      "LatencyInjectionRoute must specify either 'before_latency_ms', "
      "'after_latency_ms', 'total_latency_ms' or 'request_payload_latency'");

  std::chrono::milliseconds beforeLatency{0};
  std::chrono::milliseconds afterLatency{0};
  std::chrono::milliseconds totalLatency{0};
  if (jBeforeLatency) {
    checkLogic(
        jBeforeLatency->isInt(),
        "LatencyInjectionRoute: 'before_latency_ms' must be an interger.");
    beforeLatency = std::chrono::milliseconds(jBeforeLatency->asInt());
  }
  if (jAfterLatency) {
    checkLogic(
        jAfterLatency->isInt(),
        "LatencyInjectionRoute: 'after_latency_ms' must be an interger.");
    afterLatency = std::chrono::milliseconds(jAfterLatency->asInt());
  }
  if (jTotalLatency) {
    checkLogic(
        jTotalLatency->isInt(),
        "LatencyInjectionRoute: 'total_latency_ms' must be an integer.");
    totalLatency = std::chrono::milliseconds(jTotalLatency->asInt());
  }

  bool requestLatency = false;
  if (jRequestLatency) {
    checkLogic(
        jRequestLatency->isBool(),
        "LatencyInjectionRoute: 'request_payload_latency' is not a bool");
    if (jRequestLatency->getBool()) {
      requestLatency = true;
    }
  }

  // Note that maxRequestLatency is in usecs.
  std::chrono::microseconds maxRequestLatency{0};
  auto jMaxRequestLatency = json.get_ptr("max_request_latency_us");
  if (jMaxRequestLatency) {
    checkLogic(
        jMaxRequestLatency->isInt(),
        "LatencyInjectionRoute: 'max_request_latency_us' is not an int");
    maxRequestLatency = std::chrono::microseconds(jMaxRequestLatency->asInt());
  }

  if (beforeLatency.count() == 0 && afterLatency.count() == 0 &&
      totalLatency.count() == 0 && !requestLatency) {
    // if we are not injecting any latency, optimize this rh away.
    return child;
  }

  return makeRouteHandleWithInfo<RouterInfo, LatencyInjectionRoute>(
      std::move(child),
      beforeLatency,
      afterLatency,
      totalLatency,
      requestLatency,
      maxRequestLatency);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
