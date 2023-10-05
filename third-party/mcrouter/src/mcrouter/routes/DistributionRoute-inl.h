/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/dynamic.h>

#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"

namespace facebook::memcache::mcrouter {

inline DistributionRouteSettings parseDistributionRouteSettings(
    const folly::dynamic& json) {
  DistributionRouteSettings settings;
  settings.replay = false;
  if (auto* jReplay = json.get_ptr("replay")) {
    settings.replay = parseBool(*jReplay, "replay");
  }
  if (auto* jDistributedDeleteRpcEnabled =
          json.get_ptr("distributed_delete_rpc_enabled")) {
    settings.distributedDeleteRpcEnabled = parseBool(
        *jDistributedDeleteRpcEnabled, "distributed_delete_rpc_enabled");
  }
  return settings;
}

// standalone handle
template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeDistributionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "DistributionRoute is not an object");
  checkLogic(json.count("child"), "DistributionRoute: no child route");
  auto settings = parseDistributionRouteSettings(json);
  return makeRouteHandleWithInfo<RouterInfo, DistributionRoute>(
      factory.create(json["child"]), settings);
}

// wrapper handle
template <class RouterInfo>
typename RouterInfo::RouteHandlePtr makeDistributionRoute(
    typename RouterInfo::RouteHandlePtr rh,
    const folly::dynamic& json) {
  auto settings = parseDistributionRouteSettings(json);
  return makeRouteHandleWithInfo<RouterInfo, DistributionRoute>(
      std::move(rh), settings);
}

} // namespace facebook::memcache::mcrouter
