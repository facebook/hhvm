/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <folly/Portability.h>
#include <folly/Range.h>
#include <folly/container/F14Map.h>

#include "mcrouter/config.h"
#include "mcrouter/routes/RoutePolicyMap.h"
#include "mcrouter/routes/RouteSelectorMap.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class RoutingPrefix;

/* A bridge between proxy_* config structures and corresponding route handles.
   It is initialized by RouteHandleMapBuilder. */
template <class RouteHandleIf>
class RouteHandleMap {
 public:
  RouteHandleMap(
      const RouteSelectorMap<RouteHandleIf>& routeSelectors,
      const RoutingPrefix& defaultRoute,
      bool sendInvalidRouteToDefault);

  /**
   * @return pointer to a precalculated vector of route handles that a request
   * with the given prefix and key should be forwarded to. nullptr if vector for
   * this prefix wasn't precalculated.
   */
  const std::vector<std::shared_ptr<RouteHandleIf>>* getTargetsForKeyFast(
      folly::StringPiece prefix,
      folly::StringPiece key) const;

  /**
   * @return A vector of route handles that a request with
   * the given prefix and key should be forwarded to. Works for
   * prefixes with arbitrary amount of '*'
   */
  FOLLY_NOINLINE std::vector<std::shared_ptr<RouteHandleIf>>
  getTargetsForKeySlow(folly::StringPiece prefix, folly::StringPiece key) const;

 private:
  const std::vector<std::shared_ptr<RouteHandleIf>> emptyV_;
  const RoutingPrefix& defaultRoute_;
  bool sendInvalidRouteToDefault_;
  std::shared_ptr<RoutePolicyMap<RouteHandleIf>> defaultRouteMap_;

  std::shared_ptr<RoutePolicyMap<RouteHandleIf>> allRoutes_;
  folly::F14NodeMap<std::string, std::shared_ptr<RoutePolicyMap<RouteHandleIf>>>
      byRegion_;
  folly::F14NodeMap<std::string, std::shared_ptr<RoutePolicyMap<RouteHandleIf>>>
      byRoute_;

  void foreachRoutePolicy(
      folly::StringPiece prefix,
      std::function<void(const std::shared_ptr<RoutePolicyMap<RouteHandleIf>>&)>
          f) const;

  FOLLY_NOINLINE const std::vector<std::shared_ptr<RouteHandleIf>>*
  getTargetsForKeyFallback(folly::StringPiece prefix, folly::StringPiece key)
      const;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "RouteHandleMap-inl.h"
