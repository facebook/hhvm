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
    const std::vector<std::shared_ptr<PrefixSelectorRoute<RouteHandleIf>>>&
        clusters,
    bool useV2) {
  if (useV2) {
    v2_ = RoutePolicyMapV2<RouteHandleIf>(clusters);
    return;
  }

  // wildcards of all clusters
  std::vector<std::shared_ptr<RouteHandleIf>> wildcards;
  wildcards.reserve(clusters.size());
  // Trie with aggregated policies from all clusters
  Trie<std::vector<std::pair<size_t, std::shared_ptr<RouteHandleIf>>>> t;

  size_t clusterId = 0;
  for (auto& policy : clusters) {
    wildcards.push_back(policy->wildcard);

    for (auto& it : policy->policies) {
      auto clusterHandlePair = std::make_pair(clusterId, it.second);
      auto existing = t.find(it.first);
      if (existing != t.end()) {
        existing->second.push_back(std::move(clusterHandlePair));
      } else {
        t.emplace(it.first, {clusterHandlePair});
      }
    }
    ++clusterId;
  }

  ut_.emplace("", std::move(wildcards));
  // we iterate over keys in lexicographic order, so all prefixes of key will go
  // before key itself
  for (auto& it : t) {
    auto existing = ut_.findPrefix(it.first);
    // at least empty string should be there
    assert(existing != ut_.end());
    ut_.emplace(it.first, detail::overrideItems(existing->second, it.second));
  }
  for (auto& it : ut_) {
    it.second = detail::orderedUnique(it.second);
  }
}

template <class RouteHandleIf>
const std::vector<std::shared_ptr<RouteHandleIf>>&
RoutePolicyMap<RouteHandleIf>::getTargetsForKey(folly::StringPiece key) const {
  // empty means not initialized - i.e. the flag was not enabled.
  if (!v2_.empty()) {
    return v2_.getTargetsForKey(key);
  }
  auto result = ut_.findPrefix(key);
  return result == ut_.end() ? emptyV_ : result->second;
}

template <class RouteHandleIf>
RoutePolicyMapV2<RouteHandleIf>::RoutePolicyMapV2(
    const std::vector<SharedClusterPtr>& clusters) {
  typename LBRouteMap::Builder builder;

  // not enough but it's not a problem, we will avoid at least some
  // reallocations.
  builder.reserve(clusters.size() + 1);
  builder.insert({"", populateWildCards(clusters)});

  // Combining routes for each key
  folly::F14FastSet<std::string_view> seen;
  for (const auto& cluster : clusters) {
    for (const auto& [key, _] : cluster->policies) {
      if (seen.insert(key).second) {
        builder.insert({key, populateRoutesForKey(key, clusters)});
      }
    }
  }

  ut_ = std::move(builder).build();
}

template <class RouteHandleIf>
// static
auto RoutePolicyMapV2<RouteHandleIf>::populateWildCards(
    const std::vector<SharedClusterPtr>& clusters)
    -> std::vector<SharedRoutePtr> {
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
auto RoutePolicyMapV2<RouteHandleIf>::populateRoutesForKey(
    std::string_view key,
    const std::vector<SharedClusterPtr>& clusters)
    -> std::vector<SharedRoutePtr> {
  std::vector<std::shared_ptr<RouteHandleIf>> res;
  res.reserve(clusters.size());

  folly::F14FastSet<const RouteHandleIf*> seen;
  seen.reserve(clusters.size());

  for (const auto& cluster : clusters) {
    auto found = cluster->policies.findPrefix(key);
    const SharedRoutePtr& ptr =
        found == cluster->policies.end() ? cluster->wildcard : found->second;
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
