/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/Optional.h>

#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/routes/DefaultShadowSelectorPolicy.h"
#include "mcrouter/lib/routes/SelectionRoute.h"
#include "mcrouter/routes/ErrorRoute.h"

namespace facebook {
namespace memcache {

/**
 * Constructs a SelectionRoute for the given "RouterInfo".
 *
 * @param children                List of children route handles.
 * @param selector                Selector responsible for choosing to which
 *                                of the children the request should be sent
 *                                to.
 * @param outOfRangeDestination   The destination to which the request will be
 *                                routed if selector.select() returns a value
 *                                that is >= than children.size().
 */
template <
    class RouterInfo,
    class Selector,
    class ShadowSelectorPolicy = DefaultShadowSelectorPolicy>
typename RouterInfo::RouteHandlePtr createSelectionRoute(
    std::vector<typename RouterInfo::RouteHandlePtr> children,
    Selector selector,
    typename RouterInfo::RouteHandlePtr outOfRangeDestination = nullptr,
    std::vector<typename RouterInfo::RouteHandlePtr> shadowChildren = {},
    folly::Optional<Selector> shadowSelector = folly::none,
    std::vector<uint16_t> shadowProbabilities = {},
    const folly::Optional<ShadowSelectorPolicy>& shadowSelectorPolicy =
        folly::none,
    const uint32_t seed = mcrouter::nowUs()) {
  if (!outOfRangeDestination) {
    outOfRangeDestination =
        mcrouter::createErrorRoute<RouterInfo>("Invalid destination index.");
  }
  if (children.empty() && shadowChildren.empty()) {
    return std::move(outOfRangeDestination);
  }
  if (shadowChildren.empty()) {
    return makeRouteHandleWithInfo<RouterInfo, SelectionRoute, Selector>(
        std::move(children),
        std::move(selector),
        std::move(outOfRangeDestination));
  }
  return makeRouteHandleWithInfo<
      RouterInfo,
      SelectionRouteWithShadow,
      Selector,
      ShadowSelectorPolicy>(
      std::move(children),
      std::move(selector),
      std::move(outOfRangeDestination),
      std::move(shadowChildren),
      std::move(shadowSelector),
      std::move(shadowProbabilities),
      std::move(shadowSelectorPolicy),
      seed);
}

} // namespace memcache
} // namespace facebook
