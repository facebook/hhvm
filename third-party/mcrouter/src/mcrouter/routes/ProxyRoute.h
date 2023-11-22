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

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyDestination.h"
#include "mcrouter/ProxyDestinationMap.h"
#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/routes/AllSyncRoute.h"
#include "mcrouter/routes/BigValueRouteIf.h"
#include "mcrouter/routes/RootRoute.h"
#include "mcrouter/routes/RouteSelectorMap.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class Proxy;

/**
 * This is the top-most level of Mcrouter's RouteHandle tree.
 */
template <class RouterInfo>
class ProxyRoute {
 public:
  static std::string routeName() {
    return "proxy";
  }

  ProxyRoute(
      Proxy<RouterInfo>& proxy,
      const RouteSelectorMap<typename RouterInfo::RouteHandleIf>&
          routeSelectors,
      RootRouteRolloutOpts rolloutOpts);

  template <class Request>
  bool traverse(
      const Request& req,
      const RouteHandleTraverser<typename RouterInfo::RouteHandleIf>& t) const {
    return t(*root_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    auto reply = root_->route(req);

    auto& requestContext = fiber_local<RouterInfo>::getSharedCtx();
    requestContext->setFinalResult(*reply.result_ref());

    if (isErrorResult(*reply.result_ref())) {
      proxy_.stats().increment(final_result_error_stat);
    }

    return reply;
  }

  McFlushAllReply route(const McFlushAllRequest& req) const {
    // route to all destinations in the config.
    return AllSyncRoute<typename RouterInfo::RouteHandleIf>(
               getAllDestinations())
        .route(req);
  }

 private:
  Proxy<RouterInfo>& proxy_;
  std::shared_ptr<typename RouterInfo::RouteHandleIf> root_;

  std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
  getAllDestinations() const;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "ProxyRoute-inl.h"
