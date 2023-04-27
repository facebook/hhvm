/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/FailoverErrorsSettings.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/FailoverRateLimiter.h"
#include "mcrouter/routes/FailoverRoute.h"
#include "mcrouter/routes/ModifyExptimeRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
std::vector<typename RouterInfo::RouteHandlePtr> getFailoverChildren(
    typename RouterInfo::RouteHandlePtr normal,
    std::vector<typename RouterInfo::RouteHandlePtr> failover,
    int32_t failoverExptime) {
  std::vector<typename RouterInfo::RouteHandlePtr> children;
  children.reserve(1 + failover.size());
  children.push_back(std::move(normal));
  for (auto& frh : failover) {
    if (failoverExptime > 0) {
      auto rh = makeRouteHandle<
          typename RouterInfo::RouteHandleIf,
          ModifyExptimeRouteMin>(std::move(frh), failoverExptime);
      children.push_back(std::move(rh));
    } else {
      children.push_back(std::move(frh));
    }
  }
  return children;
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr createFailoverWithExptimeRoute(
    typename RouterInfo::RouteHandlePtr normal,
    std::vector<typename RouterInfo::RouteHandlePtr> failover,
    int32_t failoverExptime,
    FailoverErrorsSettings failoverErrors,
    std::unique_ptr<FailoverRateLimiter> rateLimiter) {
  auto children = detail::getFailoverChildren<RouterInfo>(
      std::move(normal), std::move(failover), failoverExptime);
  return makeFailoverRouteInOrder<RouterInfo, FailoverRoute>(
      std::move(children),
      std::move(failoverErrors),
      std::move(rateLimiter),
      /* failoverTagging */ false,
      /* enableLeasePairing */ false,
      "",
      nullptr);
}

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeFailoverWithExptimeRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "FailoverWithExptimeRoute is not an object");
  auto jnormal = json.get_ptr("normal");
  checkLogic(jnormal, "FailoverWithExptimeRoute: normal not found");
  auto normal = factory.create(*jnormal);

  int32_t failoverExptime = 60;
  if (auto jexptime = json.get_ptr("failover_exptime")) {
    checkLogic(
        jexptime->isInt(),
        "FailoverWithExptimeRoute: "
        "failover_exptime is not an integer");
    failoverExptime = jexptime->getInt();
  }

  std::vector<typename RouterInfo::RouteHandlePtr> failover;
  if (auto jfailover = json.get_ptr("failover")) {
    failover = factory.createList(*jfailover);
  }

  auto children = detail::getFailoverChildren<RouterInfo>(
      std::move(normal), std::move(failover), failoverExptime);
  return makeFailoverRouteDefault<RouterInfo, FailoverRoute>(
      json, std::move(children));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
