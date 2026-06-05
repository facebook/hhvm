/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <utility>

#include <folly/json/dynamic.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook::memcache::mcrouter {

/**
 * Stamps the distribution target region fiber-local, then delegates to its
 * child so a downstream DistributionRoute submits the write to DL.
 *
 *   target_region = ""           -> broadcast to all regions
 *   target_region = "<region>"   -> directed cross-region
 *
 * The stamp is applied ONLY when the request has an empty routing prefix and
 * no distribution target is already set on the fiber. This guarantees:
 *  - a directed (/<region>/.../) or broadcast (/(star)/(star)/) request is
 *    left for RootRoute to govern -- we never convert a directed SET into a
 *    broadcast,
 *  - a target already chosen upstream (e.g. RootRoute's directed stamp) is
 *    never overwritten.
 *
 * Config:
 *   - child(route handle) - the route handle to delegate to
 *   - target_region(string) - the distribution target ("" = broadcast)
 */
template <class RouterInfo>
class SetDistributionTargetRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  SetDistributionTargetRoute(RouteHandlePtr rh, std::string targetRegion)
      : rh_(std::move(rh)), targetRegion_(std::move(targetRegion)) {}

  std::string routeName() const {
    // Quote the value so the broadcast case ("") is legible in route dumps
    // (renders `target_region=""` rather than a trailing `=`).
    return "set-distribution-target|target_region=\"" + targetRegion_ + "\"";
  }

  template <class Request>
  bool traverse(const Request& req, RouteHandleTraverser<RouteHandleIf>& t)
      const {
    return t(*rh_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    // (a) A distribution target was already chosen upstream (e.g. RootRoute's
    //     directed stamp) -> defer to it, do not overwrite.
    if (fiber_local<RouterInfo>::getDistributionTargetRegion().has_value()) {
      return rh_->route(req);
    }
    // (b) Directed (/<region>/.../) or broadcast (/(star)/(star)/) routing
    //     prefix -> let RootRoute govern; never convert a directed request
    //     into a broadcast.
    if (!req.key_ref()->routingPrefix().empty()) {
      return rh_->route(req);
    }
    // (c) Empty routing prefix -> stamp the configured target and route the
    //     child within the scope so DistributionRoute consumes it
    //     synchronously.
    return fiber_local<RouterInfo>::runWithLocals([this, &req]() {
      fiber_local<RouterInfo>::setDistributionTargetRegion(targetRegion_);
      return rh_->route(req);
    });
  }

 private:
  const RouteHandlePtr rh_;
  const std::string targetRegion_;
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeSetDistributionTargetRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "SetDistributionTargetRoute is not an object");
  checkLogic(json.count("child"), "SetDistributionTargetRoute: no child route");
  auto* jTargetRegion = json.get_ptr("target_region");
  checkLogic(
      jTargetRegion && jTargetRegion->isString(),
      "SetDistributionTargetRoute: target_region must be a string");
  return makeRouteHandleWithInfo<RouterInfo, SetDistributionTargetRoute>(
      factory.create(json["child"]), jTargetRegion->getString());
}

} // namespace facebook::memcache::mcrouter
