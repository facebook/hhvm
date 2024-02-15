/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/dynamic.h>
#include <folly/json.h>

#include "mcrouter/PoolFactory.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/PrefixSelectorRoute.h"
#include "mcrouter/routes/ProxyRoute.h"
#include "mcrouter/routes/RouteSelectorMap.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
ProxyConfig<RouterInfo>::ProxyConfig(
    Proxy<RouterInfo>& proxy,
    const folly::dynamic& json,
    std::string configMd5Digest,
    PoolFactory& poolFactory,
    size_t index)
    : configMd5Digest_(std::move(configMd5Digest)) {
  McRouteHandleProvider<RouterInfo> provider(proxy, poolFactory);
  RouteHandleFactory<typename RouterInfo::RouteHandleIf> factory(
      provider, proxy.getId());

  checkLogic(json.isObject(), "Config is not an object");

  if (auto jNamedHandles = json.get_ptr("named_handles")) {
    if (jNamedHandles->isArray()) {
      for (const auto& it : *jNamedHandles) {
        factory.create(it);
      }
    } else if (jNamedHandles->isObject()) {
      for (const auto& it : jNamedHandles->items()) {
        // NOTE: addNamed causes `factory` to hold a reference to it.second.
        // `factory` is local to this function, so the JSON value outlives it.
        factory.addNamed(it.first.stringPiece(), it.second);
      }
    } else {
      throwLogic(
          "named_handles is {} expected array/object",
          jNamedHandles->typeName());
    }
  }

  RouteSelectorMap<typename RouterInfo::RouteHandleIf> routeSelectors;

  auto jRoute = json.get_ptr("route");
  auto jRoutes = json.get_ptr("routes");
  checkLogic(
      !jRoute || !jRoutes,
      "Invalid config: both 'route' and 'routes' are specified");
  if (jRoute) {
    routeSelectors[proxy.getRouterOptions().default_route] = std::make_shared<
        PrefixSelectorRoute<typename RouterInfo::RouteHandleIf>>(
        factory, *jRoute);
  } else if (jRoutes) { // jRoutes
    checkLogic(
        jRoutes->isArray() || jRoutes->isObject(),
        "Config: routes is not array/object");
    if (jRoutes->isArray()) {
      for (const auto& it : *jRoutes) {
        checkLogic(it.isObject(), "RoutePolicy is not an object");
        auto jCurRoute = it.get_ptr("route");
        auto jAliases = it.get_ptr("aliases");
        checkLogic(jCurRoute, "RoutePolicy: no route");
        checkLogic(jAliases, "RoutePolicy: no aliases");
        checkLogic(jAliases->isArray(), "RoutePolicy: aliases is not an array");
        auto routeSelector = std::make_shared<
            PrefixSelectorRoute<typename RouterInfo::RouteHandleIf>>(
            factory, *jCurRoute);
        for (const auto& alias : *jAliases) {
          checkLogic(alias.isString(), "RoutePolicy: alias is not a string");
          routeSelectors[alias.stringPiece()] = routeSelector;
        }
      }
    } else { // object
      for (const auto& it : jRoutes->items()) {
        checkLogic(it.first.isString(), "RoutePolicy: alias is not a string");
        routeSelectors[it.first.stringPiece()] = std::make_shared<
            PrefixSelectorRoute<typename RouterInfo::RouteHandleIf>>(
            factory, it.second);
      }
    }
  } else {
    throwLogic("No route/routes in config");
  }

  asyncLogRoutes_ = provider.releaseAsyncLogRoutes();
  tierRoutes_ = provider.releaseTierRoutes();
  pools_ = provider.releasePools();
  if (index == 0) {
    // only need to keep partial config info in one proxy
    partialConfigs_ = provider.releasePartialConfigs();
  }
  accessPoints_ = provider.releaseAccessPoints();

  auto jRoutingOptions = json.get_ptr("routing_options");
  auto readBool = [&jRoutingOptions](std::string_view key, bool defaultValue) {
    if (!jRoutingOptions) {
      return defaultValue;
    }
    if (auto* j = jRoutingOptions->get_ptr(key)) {
      return parseBool(*j, key);
    }
    return defaultValue;
  };

  bool enableCrossRegionDeleteRpc =
      readBool("enable_cross_region_delete_rpc", true);
  bool enableDeleteDistribution = readBool("enable_delete_distribution", false);
  checkLogic(
      enableDeleteDistribution || enableCrossRegionDeleteRpc,
      "ProxyConfig: cannot disable cross-region delete rpc if distribution is disabled");

  bool enableAsyncDlBroadcast = readBool("enable_async_dl_broadcast", false);

  proxyRoute_ = std::make_shared<ProxyRoute<RouterInfo>>(
      proxy,
      routeSelectors,
      RootRouteRolloutOpts{
          .enableDeleteDistribution = enableDeleteDistribution,
          .enableCrossRegionDeleteRpc = enableCrossRegionDeleteRpc,
          .enableAsyncDlBroadcast = enableAsyncDlBroadcast});
  serviceInfo_ = std::make_shared<ServiceInfo<RouterInfo>>(proxy, *this);
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
ProxyConfig<RouterInfo>::getRouteHandleForAsyncLog(
    folly::StringPiece asyncLogName) const {
  auto it = asyncLogRoutes_.find(asyncLogName);
  return it != asyncLogRoutes_.end() ? it->second : nullptr;
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
ProxyConfig<RouterInfo>::getRouteHandleForPool(
    folly::StringPiece poolName) const {
  auto it = tierRoutes_.find(poolName);
  return it != tierRoutes_.end() ? it->second : nullptr;
}

template <class RouterInfo>
size_t ProxyConfig<RouterInfo>::calcNumClients() const {
  size_t result = 0;
  for (const auto& it : pools_) {
    result += it.second.size();
  }
  return result;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
