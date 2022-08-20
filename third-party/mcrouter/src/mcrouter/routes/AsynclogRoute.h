/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <utility>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Async logs a failed request. It assumes the required data is available in
 * the reply.
 */
template <class RouterInfo>
class AsynclogRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  std::string routeName() const {
    return "asynclog:" + asynclogName_;
  }

  AsynclogRoute(std::shared_ptr<RouteHandleIf> rh, std::string asynclogName)
      : rh_(std::move(rh)), asynclogName_(std::move(asynclogName)) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(*rh_, req);
  }

  memcache::McDeleteReply route(const memcache::McDeleteRequest& req) const {
    return fiber_local<RouterInfo>::runWithLocals([this, &req]() {
      fiber_local<RouterInfo>::setAsynclogName(asynclogName_);
      return rh_->route(req);
    });
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    return rh_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> rh_;
  const std::string asynclogName_;
};

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeAsynclogRoute(
    typename RouterInfo::RouteHandlePtr rh,
    std::string asynclogName) {
  return makeRouteHandleWithInfo<RouterInfo, AsynclogRoute>(
      std::move(rh), std::move(asynclogName));
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
