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

struct RootRouteRolloutOpts {
  bool enablePolicyMapV2 = false;
  bool enableDeleteDistribution = false;
  bool enableCrossRegionDeleteRpc = true;
};

template <class RouterInfo>
class RootRoute {
 public:
  static std::string routeName() {
    return "root";
  }

  RootRoute(
      ProxyBase& proxy,
      const RouteSelectorMap<typename RouterInfo::RouteHandleIf>&
          routeSelectors,
      RootRouteRolloutOpts rolloutOpts)
      : opts_(proxy.getRouterOptions()),
        rhMap_(
            routeSelectors,
            opts_.default_route,
            opts_.send_invalid_route_to_default,
            rolloutOpts.enablePolicyMapV2),
        defaultRoute_(opts_.default_route),
        enableDeleteDistribution_(rolloutOpts.enableDeleteDistribution),
        enableCrossRegionDeleteRpc_(rolloutOpts.enableCrossRegionDeleteRpc) {}

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<typename RouterInfo::RouteHandleIf>& t) const {
    const auto* rhPtr = rhMap_.getTargetsForKeyFast(
        req.key_ref()->routingPrefix(), req.key_ref()->routingKey());
    if (FOLLY_LIKELY(rhPtr != nullptr)) {
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
    auto reply = getTargetsAndRoute(req.key_ref()->routingPrefix(), req);
    if (isErrorResult(*reply.result_ref()) && opts_.group_remote_errors) {
      reply = ReplyT<Request>(carbon::Result::REMOTE_ERROR);
    }

    return reply;
  }

  McDeleteReply route(const McDeleteRequest& req) const {
    // If distribution is enabled, route deletes to the default route where
    // DistributionRoute will route cross region.
    //
    // NOTE: if enableCrossRegionDeleteRpc flag is not set it defaults to true,
    // the distribution step will be followed by a duplicating RPC step, and the
    // return value will be the reply returned by the RPC step.
    McDeleteReply reply;
    if (enableDeleteDistribution_ && !req.key_ref()->routingPrefix().empty()) {
      auto routingPrefix = RoutingPrefix(req.key_ref()->routingPrefix());
      if (routingPrefix.getRegion() != defaultRoute_.getRegion() &&
          req.key_ref()->routingPrefix() != kBroadcastPrefix) {
        reply = fiber_local<RouterInfo>::runWithLocals(
            [this, &req, &routingPrefix]() {
              fiber_local<RouterInfo>::setDistributionTargetRegion(
                  routingPrefix.getRegion().str());
              return getTargetsAndRoute(defaultRoute_, req);
            });

        if (!enableCrossRegionDeleteRpc_) {
          return reply;
        }
      }
    }
    reply = getTargetsAndRoute(req.key_ref()->routingPrefix(), req);
    if (isErrorResult(*reply.result_ref()) && opts_.group_remote_errors) {
      reply = McDeleteReply(carbon::Result::REMOTE_ERROR);
    }
    return reply;
  }

 private:
  const McrouterOptions& opts_;
  RouteHandleMap<typename RouterInfo::RouteHandleIf> rhMap_;
  RoutingPrefix defaultRoute_;
  bool enableDeleteDistribution_;
  bool enableCrossRegionDeleteRpc_;

  template <class Request>
  FOLLY_ALWAYS_INLINE ReplyT<Request> getTargetsAndRoute(
      folly::StringPiece routingPrefix,
      const Request& req) const {
    const auto* rhPtr =
        rhMap_.getTargetsForKeyFast(routingPrefix, req.key_ref()->routingKey());

    return UNLIKELY(rhPtr == nullptr)
        ? routeImpl(
              rhMap_.getTargetsForKeySlow(
                  routingPrefix, req.key_ref()->routingKey()),
              req)
        : routeImpl(*rhPtr, req);
  }

  template <class Request>
  ReplyT<Request> routeImpl(
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
      const Request& req,
      carbon::GetLikeT<Request> = 0) const {
    auto reply = doRoute(rh, req);
    if (FOLLY_UNLIKELY(
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
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
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
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
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
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
      const Request& req) const {
    if (!rh.empty()) {
      return routeToAll(rh, req);
    }
    return createReply<Request>(ErrorReply);
  }

  template <class Request>
  ReplyT<Request> routeToAll(
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
      const Request& req) const {
    auto reqCopy = std::make_shared<const Request>(req);
    for (size_t i = 1, e = rh.size(); i < e; ++i) {
      auto r = rh[i];
      folly::fibers::addTask([r, reqCopy]() { r->route(*reqCopy); });
    }
    return rh[0]->route(req);
  }

  McDeleteReply routeToAll(
      const std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>&
          rh,
      const McDeleteRequest& req) const {
    if (enableCrossRegionDeleteRpc_) {
      auto reqCopy = std::make_shared<const McDeleteRequest>(req);
      for (size_t i = 1, e = rh.size(); i < e; ++i) {
        auto r = rh[i];
        folly::fibers::addTask([r, reqCopy]() { r->route(*reqCopy); });
      }
    }
    if (enableDeleteDistribution_ &&
        req.key_ref()->routingPrefix() == kBroadcastPrefix) {
      return fiber_local<RouterInfo>::runWithLocals([&req, &rh]() {
        // DistributionRoute will read empty string as "broadcast", i.e.
        // distribute to all regions:
        fiber_local<RouterInfo>::setDistributionTargetRegion("");
        return rh[0]->route(req);
      });
    } else {
      return rh[0]->route(req);
    }
  }
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
