/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/fibers/AddTasks.h>

#include <mcrouter/lib/RouteHandleTraverser.h>
#include <mcrouter/lib/config/RouteHandleBuilder.h>
#include <mcrouter/lib/config/RouteHandleFactory.h>
#include <mcrouter/lib/fbi/cpp/util.h>

namespace hellogoodbye {

/**
 * This is a special route handle that synchronoysly duplicates requests.
 * The first reply received is returned to the client.
 */
template <class RouterInfo>
class DuplicateRoute {
 public:
  static std::string routeName() {
    return "duplicate";
  }

  explicit DuplicateRoute(
      typename RouterInfo::RouteHandlePtr child,
      size_t numCopies)
      : child_(std::move(child)), numCopies_(numCopies) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const facebook::memcache::RouteHandleTraverser<
          typename RouterInfo::RouteHandleIf>& t) const {
    if (child_) {
      return t(*child_, req);
    }
    return false;
  }

  template <class Request>
  typename Request::reply_type route(const Request& req) {
    using Reply = typename Request::reply_type;

    // Allocate one fiber for each new request.
    std::vector<std::function<Reply()>> funcs;
    funcs.reserve(numCopies_);
    for (size_t i = 0; i < numCopies_; ++i) {
      funcs.push_back([&req, child = child_]() { return child->route(req); });
    }
    auto taskIt = folly::fibers::addTasks(funcs.begin(), funcs.end());

    // Return the first reply received
    return taskIt.awaitNext();
  }

 private:
  const typename RouterInfo::RouteHandlePtr child_;
  const size_t numCopies_;
};

// Factory function
template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeDuplicateRoute(
    facebook::memcache::RouteHandleFactory<typename RouterInfo::RouteHandleIf>&
        factory,
    const folly::dynamic& json) {
  typename RouterInfo::RouteHandlePtr target;
  size_t numCopies = 2;

  if (json.isObject()) {
    facebook::memcache::checkLogic(
        json.count("target") == 1, "DuplicateRoute: no target route");
    if (auto targetJson = json.get_ptr("target")) {
      target = factory.create(*targetJson);
    }
    if (auto copiesJson = json.get_ptr("copies")) {
      facebook::memcache::checkLogic(
          copiesJson->isInt(), "DuplicateRoute: 'copies' is not an integer");
      numCopies = copiesJson->getInt();
      facebook::memcache::checkLogic(
          numCopies >= 2, "DuplicateRoute: 'copies' must be 2 or more");
    }
  } else if (json.isString()) {
    target = factory.create(json);
  }

  return facebook::memcache::
      makeRouteHandleWithInfo<RouterInfo, DuplicateRoute>(
          std::move(target), numCopies);
}

} // namespace hellogoodbye
