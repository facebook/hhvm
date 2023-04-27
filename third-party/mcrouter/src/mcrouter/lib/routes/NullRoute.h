/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {

template <class RouteHandleIf>
class RouteHandleFactory;

/**
 * Returns the default reply for each request right away
 */
template <class RouteHandleIf>
struct NullRoute {
  static std::string routeName() {
    return "null";
  }

  template <class Request>
  bool traverse(const Request&, const RouteHandleTraverser<RouteHandleIf>&)
      const {
    return false;
  }

  template <class Request>
  static ReplyT<Request> route(const Request& req) {
    return createReply(DefaultReply, req);
  }
};

namespace mcrouter {

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> createNullRoute() {
  return makeRouteHandle<RouteHandleIf, NullRoute>();
}

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> makeNullRoute(
    RouteHandleFactory<RouteHandleIf>&,
    const folly::dynamic&) {
  return createNullRoute<RouteHandleIf>();
}

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> makeNullOrSingletonRoute(
    std::vector<std::shared_ptr<RouteHandleIf>> rh) {
  assert(rh.size() <= 1);
  if (rh.empty()) {
    return createNullRoute<RouteHandleIf>();
  }
  return std::move(rh[0]);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
