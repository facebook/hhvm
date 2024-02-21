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

#include <folly/json/dynamic.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/routes/NullRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * For get-like requests, sends the same request sequentially
 * to each destination in the list in order until the first hit reply.
 * If all replies result in errors/misses, returns the reply from the
 * last destination in the list.
 */
template <class RouterInfo>
class MissFailoverRoute {
 private:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;

 public:
  explicit MissFailoverRoute(
      std::vector<std::shared_ptr<RouteHandleIf>> targets,
      bool returnBestOnError = false)
      : targets_(std::move(targets)), returnBestOnError_(returnBestOnError) {
    assert(targets_.size() > 1);
  }

  static std::string routeName() {
    return "miss-failover";
  }

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    return t(targets_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req, carbon::GetLikeT<Request> = 0)
      const {
    return routeImpl(req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req, carbon::DeleteLikeT<Request> = 0)
      const {
    return routeImpl(req);
  }

  template <class Request>
  ReplyT<Request> route(
      const Request& req,
      carbon::OtherThanT<Request, carbon::GetLike<>, carbon::DeleteLike<>> =
          0) const {
    return targets_[0]->route(req);
  }

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> targets_;
  const bool returnBestOnError_;

  bool shouldFailover(const carbon::Result replyResult) const {
    return !isHitResult(replyResult) && (replyResult != carbon::Result::OK);
  }

  template <class Request>
  ReplyT<Request> routeImpl(const Request& req) const {
    auto reply = targets_[0]->route(req);
    if (!shouldFailover(*reply.result_ref())) {
      return reply;
    }

    // Failover
    return fiber_local<RouterInfo>::runWithLocals(
        [this, &req, bestReply = std::move(reply)]() mutable {
          fiber_local<RouterInfo>::addRequestClass(RequestClass::kFailover);
          for (size_t i = 1, s = targets_.size(); i < s; ++i) {
            auto failoverReply = targets_[i]->route(req);
            if (!shouldFailover(*failoverReply.result_ref())) {
              return failoverReply;
            }
            if (returnBestOnError_) {
              // Prefer returning a miss from a healthy host rather than
              // an error from the last broken host.
              if (!worseThan(
                      *failoverReply.result_ref(), *bestReply.result_ref())) {
                // This reply is "better" than we already have.
                bestReply = std::move(failoverReply);
              }
            } else {
              // Return reply from the last host, no matter if it's broken.
              bestReply = std::move(failoverReply);
            }
          }
          return std::move(bestReply);
        });
  }
};

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeMissFailoverRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> targets,
    bool returnBestOnError) {
  if (targets.empty()) {
    return createNullRoute<typename RouterInfo::RouteHandleIf>();
  }

  if (targets.size() == 1) {
    return std::move(targets[0]);
  }

  return makeRouteHandleWithInfo<RouterInfo, MissFailoverRoute>(
      std::move(targets), returnBestOnError);
}

} // namespace detail

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeMissFailoverRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  std::vector<typename RouterInfo::RouteHandlePtr> children;
  bool returnBestOnError = false;
  if (json.isObject()) {
    if (auto jChildren = json.get_ptr("children")) {
      children = factory.createList(*jChildren);
    }
    if (auto jReturnBest = json.get_ptr("return_best_on_error")) {
      checkLogic(
          jReturnBest->isBool(),
          "ModifyKeyRoute: return_best_on_error is not a bool");
      returnBestOnError = jReturnBest->asBool();
    }

  } else {
    children = factory.createList(json);
  }
  return detail::makeMissFailoverRoute<RouterInfo>(
      std::move(children), returnBestOnError);
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
