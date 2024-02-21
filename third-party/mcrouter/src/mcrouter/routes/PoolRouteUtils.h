/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/routes/OutstandingLimitRoute.h"
#include "mcrouter/routes/SlowWarmUpRoute.h"
#include "mcrouter/routes/SlowWarmUpRouteSettings.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ProxyBase;

template <class RouterHandleIf>
class ExtraRouteHandleProviderIf;

/**
 * Wraps pool "destinations" with route handles according to config in "json".
 *
 * @param factory         The route handle factory.
 * @param destinations    The list of destinations of the pool.
 * @param poolName        The name of the pool that "destinations" belong to.
 * @param json            Json containing basic PoolRoute settings:
 *                           - "max_outstanding" (optional),
 *                           - "slow_warmup" (optional),
 *                           - "shadows", "shadow_policy" (optional)
 * @param proxy           Instance of ProxyBase.
 * @param extraProvider   Extra route handle provider.
 *
 * @return The (possibly) modified list of destinations.
 *
 * @throws Logic error if config is invalid.
 */
template <class RouterInfo>
std::vector<typename RouterInfo::RouteHandlePtr> wrapPoolDestinations(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    std::vector<typename RouterInfo::RouteHandlePtr>&& destinations,
    folly::StringPiece poolName,
    const folly::dynamic& json,
    ProxyBase& proxy,
    ExtraRouteHandleProviderIf<RouterInfo>& extraProvider) {
  try {
    if (json.isObject()) {
      if (auto maxOutstandingJson = json.get_ptr("max_outstanding")) {
        auto v = parseInt(*maxOutstandingJson, "max_outstanding", 0, 1000000);
        if (v) {
          for (auto& destination : destinations) {
            destination = makeOutstandingLimitRoute<RouterInfo>(
                std::move(destination), v);
          }
        }
      }

      if (auto slowWarmUpJson = json.get_ptr("slow_warmup")) {
        checkLogic(
            slowWarmUpJson->isObject(), "slow_warmup must be a json object");

        auto failoverTargetJson = slowWarmUpJson->get_ptr("failoverTarget");
        checkLogic(
            failoverTargetJson,
            "couldn't find 'failoverTarget' property in slow_warmup");
        auto failoverTarget = factory.create(*failoverTargetJson);

        std::shared_ptr<SlowWarmUpRouteSettings> slowWarmUpSettings;
        if (auto settingsJson = slowWarmUpJson->get_ptr("settings")) {
          checkLogic(
              settingsJson->isObject(),
              "'settings' in slow_warmup must be a json object.");
          slowWarmUpSettings =
              std::make_shared<SlowWarmUpRouteSettings>(*settingsJson);
        } else {
          slowWarmUpSettings = std::make_shared<SlowWarmUpRouteSettings>();
        }

        for (auto& destination : destinations) {
          destination = makeSlowWarmUpRoute<RouterInfo>(
              std::move(destination), failoverTarget, slowWarmUpSettings);
        }
      }

      if (json.count("shadows")) {
        destinations = makeShadowRoutes(
            factory, json, std::move(destinations), proxy, extraProvider);
      }
    }

    return std::move(destinations);
  } catch (const std::exception& e) {
    throwLogic("PoolRoute {}: {}", poolName, e.what());
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
