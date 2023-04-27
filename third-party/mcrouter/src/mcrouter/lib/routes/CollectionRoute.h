/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

/**
 * This class represents the Collection API that abstracts out the
 * routine of visiting children and "collecting" results via Collector
 * abstraction.
 *
 * At a high level, this is for routes that:
 * - Have 0-n children
 * - Can visit 0-n of their children
 * - How many children they visit is determined based on the state of visits to
 * other children
 *
 * Examples:
 * - AllSyncRoute
 * - AllFastestRoute
 * - AllInitialRoute
 * - AllMajorityRoute
 *
 * To use this class, implement two classes:
 * - A child of CollectionRoute, which implements
 * create[RouteName]Route(factory, json)
 * - A child of Collector, which implements:
 *   - A constructor
 *   - initialReplyImpl : Takes no argument and returns an optional reply.
 * Evaluated first
 *   - iterImpl : Takes a Reply and returns an optional Reply. If it is:
 *      - none  => keep visiting children
 *      - Reply => return reply upstream, stop visiting other children
 *   - finalReplyImpl : If no visited children in iterImpl returned a reply,
 * return a reply here.
 *
 */

#include <memory>
#include <string>
#include <vector>

#include <folly/fibers/AddTasks.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"

namespace facebook {
namespace memcache {

template <
    class Request,
    template <class, class...>
    class DerivedCollector,
    class... Args>
class Collector {
 public:
  using Reply = ReplyT<Request>;
  using ChildCollector = DerivedCollector<Request, Args...>;

  Collector(const Collector&) = delete;
  Collector& operator=(Collector const&) = delete;

  folly::Optional<Reply> initialReply() const {
    return static_cast<const ChildCollector*>(this)->initialReplyImpl();
  }

  folly::Optional<Reply> iter(const Reply& reply) {
    return static_cast<ChildCollector*>(this)->iterImpl(reply);
  }

  Reply finalReply() const {
    return static_cast<const ChildCollector*>(this)->finalReplyImpl();
  }

  void initialize(const Request& initialRequest, size_t childrenCount) {
    assert(!initialized_);
    initialRequest_ = std::make_unique<Request>(initialRequest);
    childrenCount_ = childrenCount;
    initialized_ = true;
  }

 protected:
  Collector() {}

  size_t getChildrenCount() const {
    return childrenCount_;
  }

  const Request& getInitialRequest() const {
    return *initialRequest_->get();
  }

 private:
  bool initialized_{false};
  std::unique_ptr<Request> initialRequest_{nullptr};
  size_t childrenCount_{0};
};

template <
    class RouterInfo,
    template <class, class...>
    class Collector,
    class... Args>
class CollectionRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = typename RouterInfo::RouteHandlePtr;

 public:
  explicit CollectionRoute(
      const std::vector<RouteHandlePtr> children,
      const Args&... args)
      : children_(std::move(children)),
        args_(std::make_tuple(std::move(args)...)) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(children_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    using Reply = ReplyT<Request>;

    std::vector<std::function<Reply()>> funcs;
    funcs.reserve(children_.size());
    auto reqCopy = std::make_shared<Request>(req);
    for (auto& rh : children_) {
      funcs.push_back([reqCopy, rh]() { return rh->route(*reqCopy); });
    }

    auto taskIt = folly::fibers::addTasks(funcs.begin(), funcs.end());

    return folly::apply(
        [&](const Args&... args) {
          Collector<Request, Args...> collector(args...);
          collector.initialize(req, children_.size());

          auto res = collector.initialReply();
          if (res.hasValue()) {
            return res.value();
          }

          while (taskIt.hasNext()) {
            res = collector.iter(taskIt.awaitNext());

            if (res.hasValue()) {
              return res.value();
            }
          }

          return collector.finalReply();
        },
        args_);
  }

 private:
  const std::vector<RouteHandlePtr> children_;
  const std::tuple<Args...> args_;
};

} // end namespace memcache
} // end namespace facebook
