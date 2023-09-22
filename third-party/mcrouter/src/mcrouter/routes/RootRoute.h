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

#include <folly/Likely.h>
#include <folly/fibers/FiberManager.h>

#include "mcrouter/ProxyBase.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/carbon/RequestReplyUtil.h"
#include "mcrouter/lib/carbon/RoutingGroups.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/routes/RouteHandleMap.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouteHandleIf>
class RootRoute {
 public:
  static std::string routeName() {
    return "root";
  }

  RootRoute(
      ProxyBase& proxy,
      const RouteSelectorMap<RouteHandleIf>& routeSelectors,
      bool disableBroadcastDeleteRpc = false)
      : opts_(proxy.getRouterOptions()),
        rhMap_(
            routeSelectors,
            opts_.default_route,
            opts_.send_invalid_route_to_default,
            opts_.enable_route_policy_v2),
        disableBroadcastDeleteRpc_(disableBroadcastDeleteRpc) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<RouteHandleIf>& t) const {
    const auto* rhPtr = rhMap_.getTargetsForKeyFast(
        req.key_ref()->routingPrefix(), req.key_ref()->routingKey());
    if (LIKELY(rhPtr != nullptr)) {
      for (const auto& rh : *rhPtr) {
        if (t(*rh, req)) {
          return true;
        }
      }
      return false;
    }
    auto v = rhMap_.getTargetsForKeySlow(
        req.key_ref()->routingPrefix(), req.key_ref()->routingKey());
    for (const auto& rh : v) {
      if (t(*rh, req)) {
        return true;
      }
    }
    return false;
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    /* If we have to send to more than one prefix,
       wait for the first in the list to reply and let others
       run in the background.

       This is a good default for /star/star/ requests. */
    const auto* rhPtr = rhMap_.getTargetsForKeyFast(
        req.key_ref()->routingPrefix(), req.key_ref()->routingKey());

    auto reply = UNLIKELY(rhPtr == nullptr)
        ? routeImpl(
              rhMap_.getTargetsForKeySlow(
                  req.key_ref()->routingPrefix(), req.key_ref()->routingKey()),
              req)
        : routeImpl(*rhPtr, req);

    if (isErrorResult(*reply.result_ref()) && opts_.group_remote_errors) {
      reply = ReplyT<Request>(carbon::Result::REMOTE_ERROR);
    }

    return reply;
  }

 private:
  const McrouterOptions& opts_;
  RouteHandleMap<RouteHandleIf> rhMap_;
  bool disableBroadcastDeleteRpc_;

  template <class Request>
  ReplyT<Request> routeImpl(
      const std::vector<std::shared_ptr<RouteHandleIf>>& rh,
      const Request& req,
      carbon::GetLikeT<Request> = 0) const {
    auto reply = doRoute(rh, req);
    if (UNLIKELY(
            isErrorResult(*reply.result_ref()) && opts_.miss_on_get_errors &&
            !rh.empty())) {
      /* rh.empty() case: for backwards compatibility,
         always surface invalid routing errors */
      auto originalResult = *reply.result_ref();
      reply = createReply(DefaultReply, req);
      carbon::setMessageIfPresent(
          reply,
          folly::to<std::string>(
              "Error reply transformed into miss due to miss_on_get_errors. "
              "Original reply result: ",
              carbon::resultToString(originalResult)));
    }
    return reply;
  }

  template <class Request>
  ReplyT<Request> routeImpl(
      const std::vector<std::shared_ptr<RouteHandleIf>>& rh,
      const Request& req,
      carbon::ArithmeticLikeT<Request> = 0) const {
    auto reply = opts_.allow_only_gets ? createReply(DefaultReply, req)
                                       : doRoute(rh, req);
    if (isErrorResult(*reply.result_ref()) &&
        !opts_.disable_miss_on_arith_errors) {
      reply = createReply(DefaultReply, req);
    }
    return reply;
  }

  template <class Request>
  ReplyT<Request> routeImpl(
      const std::vector<std::shared_ptr<RouteHandleIf>>& rh,
      const Request& req,
      carbon::OtherThanT<Request, carbon::GetLike<>, carbon::ArithmeticLike<>> =
          0) const {
    if (!opts_.allow_only_gets) {
      return doRoute(rh, req);
    }

    return createReply(DefaultReply, req);
  }

  template <class Request>
  ReplyT<Request> doRoute(
      const std::vector<std::shared_ptr<RouteHandleIf>>& rh,
      const Request& req) const {
    if (FOLLY_LIKELY(rh.size() == 1)) {
      return rh[0]->route(req);
    }
    if (!rh.empty()) {
      // Broadcast delete via Distribution, route only
      // to the first (local) route handle
      if constexpr (folly::IsOneOf<Request, McDeleteRequest>::value) {
        if (disableBroadcastDeleteRpc_ &&
            req.key_ref()->routingPrefix() == kBroadcastPrefix) {
          return rh[0]->route(req);
        }
      }

      auto reqCopy = std::make_shared<const Request>(req);
      for (size_t i = 1, e = rh.size(); i < e; ++i) {
        auto r = rh[i];
        folly::fibers::addTask([r, reqCopy]() { r->route(*reqCopy); });
      }
      return rh[0]->route(req);
    }
    return createReply<Request>(ErrorReply);
  }
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
