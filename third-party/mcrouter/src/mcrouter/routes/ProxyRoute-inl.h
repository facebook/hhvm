/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>

#include "mcrouter/Proxy.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/routes/BigValueRoute.h"
#include "mcrouter/routes/LoggingRoute.h"
#include "mcrouter/routes/McRouteHandleBuilder.h"
#include "mcrouter/routes/RootRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouterInfo>
typename RouterInfo::RouteHandlePtr wrapWithBigValueRoute(
    typename RouterInfo::RouteHandlePtr ch,
    const McrouterOptions& routerOpts) {
  if (routerOpts.big_value_split_threshold == 0) {
    return std::move(ch);
  }
  BigValueRouteOptions options(
      routerOpts.big_value_split_threshold,
      routerOpts.big_value_batch_size,
      routerOpts.big_value_hide_reply_flag);
  return makeBigValueRoute<RouterInfo>(std::move(ch), std::move(options));
}

} // namespace detail

template <class RouterInfo>
ProxyRoute<RouterInfo>::ProxyRoute(
    Proxy<RouterInfo>& proxy,
    const RouteSelectorMap<typename RouterInfo::RouteHandleIf>& routeSelectors,
    bool disableBroadcastDeleteRpc)
    : proxy_(proxy),
      root_(makeRouteHandle<typename RouterInfo::RouteHandleIf, RootRoute>(
          proxy_,
          routeSelectors,
          disableBroadcastDeleteRpc)) {
  if (proxy_.getRouterOptions().big_value_split_threshold != 0) {
    root_ = detail::wrapWithBigValueRoute<RouterInfo>(
        std::move(root_), proxy_.getRouterOptions());
  }
  if (proxy_.getRouterOptions().enable_logging_route) {
    root_ = createLoggingRoute<RouterInfo>(std::move(root_));
  }
}

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
ProxyRoute<RouterInfo>::getAllDestinations() const {
  std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>> rh;

  // We're on the proxy thread, but this should ideally be grabbed
  // from fiber_local::getSharedCtx().  Hard to do due to circular
  // include dependecies.
  //
  // Important: keep the shared_ptr alive for the duration of the loop.
  auto config = proxy_.getConfigUnsafe();
  for (auto& it : config->getPools()) {
    rh.insert(rh.end(), it.second.begin(), it.second.end());
  }
  return rh;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
