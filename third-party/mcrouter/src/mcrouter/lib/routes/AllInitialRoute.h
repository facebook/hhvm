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

#include <folly/fibers/FiberManager.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/routes/AllAsyncRoute.h"

namespace facebook {
namespace memcache {

/**
 * Sends the same request to all child route handles.
 * Returns the reply from the first route handle in the list;
 * all other requests complete asynchronously.
 */
template <class RouteHandleIf>
class AllInitialRoute {
 public:
  static std::string routeName() {
    return "all-initial";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    if (t(*firstChild_, req)) {
      return true;
    }
    return asyncRoute_.traverse(req, t);
  }

  explicit AllInitialRoute(std::vector<std::shared_ptr<RouteHandleIf>> rh)
      : firstChild_(getFirstAndCheck(rh)),
        asyncRoute_(std::vector<std::shared_ptr<RouteHandleIf>>(
            rh.begin() + 1,
            rh.end())) {}

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    asyncRoute_.route(req);
    return firstChild_->route(req);
  }

 private:
  const std::shared_ptr<RouteHandleIf> firstChild_;
  const AllAsyncRoute<RouteHandleIf> asyncRoute_;

  static std::shared_ptr<RouteHandleIf> getFirstAndCheck(
      std::vector<std::shared_ptr<RouteHandleIf>>& rh) {
    assert(rh.size() > 1);
    return std::move(rh[0]);
  }
};
} // namespace memcache
} // namespace facebook
