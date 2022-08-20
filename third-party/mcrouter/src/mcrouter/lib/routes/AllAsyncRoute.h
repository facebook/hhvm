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

#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {

/**
 * Sends the same request to all child route handles.
 * Does not wait for response.
 */
template <class RouteHandleIf>
class AllAsyncRoute {
 public:
  static std::string routeName() {
    return "all-async";
  }

  explicit AllAsyncRoute(std::vector<std::shared_ptr<RouteHandleIf>> rh)
      : children_(std::move(rh)) {
    assert(!children_.empty());
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(children_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto reqCopy = std::make_shared<Request>(req);
    for (auto& rh : children_) {
      folly::fibers::addTask([rh, reqCopy]() { rh->route(*reqCopy); });
    }
    return NullRoute<RouteHandleIf>::route(req);
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> children_;
};
} // namespace memcache
} // namespace facebook
