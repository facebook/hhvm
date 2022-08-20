/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "mcrouter/lib/Operation.h"
#include "mcrouter/lib/RouteHandleTraverser.h"

namespace facebook {
namespace memcache {

/**
 * Sends the request to a random destination from list of children.
 */
template <class RouteHandleIf>
class RandomRoute {
 public:
  static std::string routeName() {
    return "random";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(children_, req);
  }

  explicit RandomRoute(std::vector<std::shared_ptr<RouteHandleIf>> children)
      : children_(std::move(children)),
        gen_(std::ranlux24_base(
            std::chrono::system_clock::now().time_since_epoch().count())) {
    assert(!children_.empty());
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) {
    return children_[gen_() % children_.size()]->route(req);
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> children_;
  std::ranlux24_base gen_;
};

} // namespace memcache
} // namespace facebook
