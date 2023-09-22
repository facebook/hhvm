/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <folly/Range.h>
#include <folly/container/F14Set.h>

#include "mcrouter/lib/fbi/cpp/LowerBoundPrefixMap.h"
#include "mcrouter/lib/fbi/cpp/Trie.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouteHandleIf>
class PrefixSelectorRoute;

/**
 * @brief This class precalculates targets (vector of route handles) we should
 *        route to for some set of PrefixSelectorRoutes.
 *
 * Sometimes we want to route to multiple clusters in one request.
 * This happens if routing prefix contains some '*'. Most common cases are
 * /star/star/ and /region/star/. On the other hand different clusters can have
 * same OperationSelectorRoutes and we don't want to route to same
 * OperationSelectorRoute twice.
 *
 * Before to achieve this we iterated through all clusters that match
 * given routing prefix and built a set of unique OperationSelectorRoutes
 * on each request. This class precalculates these sets for some routing prefix.
 *
 * To use this class one should:
 * 1) create RoutePolicyMap with PrefixSelectorRoutes for all clusters that
 *    match the routing prefix e.g. for /star/star/ these will be all clusters.
 *    Order is important: targets for corresponding cluster will be in same
 *    order as in clusters vector passed to constructor. So local clusters
 *    should be at the beggining of vector.
 * 2) use getTargetsForKey to get precalculated vector of targets.
 *    Complexity is O(min(longest key prefix in config, key length))
 */
template <class RouteHandleIf>
class RoutePolicyMapV2 {
 public:
  using SharedRoutePtr = std::shared_ptr<RouteHandleIf>;
  using SharedClusterPtr = std::shared_ptr<PrefixSelectorRoute<RouteHandleIf>>;

  RoutePolicyMapV2() = default;
  explicit RoutePolicyMapV2(const std::vector<SharedClusterPtr>& clusters);

  // NOTE: change to span when the migration is over
  const std::vector<SharedRoutePtr>& getTargetsForKey(
      std::string_view key) const {
    // has to be there by construction
    auto found = ut_.findPrefix(key);
    // RoutePolicyMap always has wildcard which matches everything.
    CHECK(found != ut_.end());
    return found->value();
  }

  bool empty() const {
    return ut_.empty();
  }

 private:
  static std::vector<SharedRoutePtr> populateWildCards(
      const std::vector<SharedClusterPtr>& clusters);

  static std::vector<SharedRoutePtr> populateRoutesForKey(
      std::string_view key,
      const std::vector<SharedClusterPtr>& clusters);

  using LBRouteMap = LowerBoundPrefixMap<std::vector<SharedRoutePtr>>;

  LBRouteMap ut_;
};

template <class RouteHandleIf>
class RoutePolicyMap {
 public:
  explicit RoutePolicyMap(
      const std::vector<std::shared_ptr<PrefixSelectorRoute<RouteHandleIf>>>&
          clusters,
      bool useV2);

  /**
   * @return vector of route handles that a request with given key should be
   *         forwarded to.
   */
  const std::vector<std::shared_ptr<RouteHandleIf>>& getTargetsForKey(
      folly::StringPiece key) const;

 private:
  RoutePolicyMapV2<RouteHandleIf> v2_;
  const std::vector<std::shared_ptr<RouteHandleIf>> emptyV_;
  /**
   * This Trie contains targets for each key prefix. It is built like this:
   * 1) targets for empty string are wildcards.
   * 2) targets for string of length n+1 S[0..n] are targets for S[0..n-1] with
   *    OperationSelectorRoutes for key prefix == S[0..n] overridden.
   */
  Trie<std::vector<std::shared_ptr<RouteHandleIf>>> ut_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "RoutePolicyMap-inl.h"
