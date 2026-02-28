/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "mcrouter/routes/PrefixSelectorRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

template <class RouteHandleIf>
std::vector<std::shared_ptr<RouteHandleIf>> orderedUnique(
    const std::vector<std::shared_ptr<RouteHandleIf>>& v) {
  std::unordered_set<std::shared_ptr<RouteHandleIf>> seen;
  std::vector<std::shared_ptr<RouteHandleIf>> result;
  for (auto& el : v) {
    if (el && seen.find(el) == seen.end()) {
      seen.insert(el);
      result.push_back(el);
    }
  }
  result.shrink_to_fit();
  return result;
}

template <class RouteHandleIf>
std::vector<std::shared_ptr<RouteHandleIf>> overrideItems(
    std::vector<std::shared_ptr<RouteHandleIf>> original,
    const std::vector<std::pair<size_t, std::shared_ptr<RouteHandleIf>>>&
        overrides) {
  for (auto& it : overrides) {
    original[it.first] = it.second;
  }
  return original;
}

} // namespace detail

template <class RouteHandleIf>
RoutePolicyMap<RouteHandleIf>::RoutePolicyMap(
    const std::vector<SharedClusterPtr>& clusters) {
  typename LBRouteMap::Builder builder;

  // not enough but it's not a problem, we will avoid at least some
  // reallocations.
  builder.reserve(clusters.size() + 1);
  builder.insert({"", populateWildCards(clusters)});

  // Combining routes for each key
  folly::F14FastSet<std::string_view> seen;
  for (const auto& cluster : clusters) {
    for (const auto& policy : cluster->policies) {
      std::string_view key = policy.key();
      if (seen.insert(key).second) {
        builder.insert({std::string(key), populateRoutesForKey(key, clusters)});
      }
    }
  }

  ut_ = std::move(builder).build();
}

template <class RouteHandleIf>
// static
auto RoutePolicyMap<RouteHandleIf>::populateWildCards(
    std::span<const SharedClusterPtr> clusters) -> std::vector<SharedRoutePtr> {
  std::vector<SharedRoutePtr> res;
  res.reserve(clusters.size());

  folly::F14FastSet<const RouteHandleIf*> seen;
  for (const auto& cluster : clusters) {
    const SharedRoutePtr& ptr = cluster->wildcard;
    if (ptr && seen.insert(ptr.get()).second) {
      res.push_back(cluster->wildcard);
    }
  }

  // This code is cold but the vector stays around for a while
  res.shrink_to_fit();
  return res;
}

template <class RouteHandleIf>
// static
auto RoutePolicyMap<RouteHandleIf>::populateRoutesForKey(
    std::string_view key,
    std::span<const SharedClusterPtr> clusters) -> std::vector<SharedRoutePtr> {
  std::vector<std::shared_ptr<RouteHandleIf>> res;
  res.reserve(clusters.size());

  folly::F14FastSet<const RouteHandleIf*> seen;
  seen.reserve(clusters.size());

  for (const auto& cluster : clusters) {
    auto found = cluster->policies.findPrefix(key);
    const SharedRoutePtr& ptr =
        found == cluster->policies.end() ? cluster->wildcard : found->value();
    if (ptr && seen.insert(ptr.get()).second) {
      res.push_back(ptr);
    }
  }

  // This code is cold but the vector stays around for a while
  res.shrink_to_fit();
  return res;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
