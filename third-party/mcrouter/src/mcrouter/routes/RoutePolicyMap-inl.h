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
        clusters) {
  // wildcards of all clusters
  std::vector<std::shared_ptr<RouteHandleIf>> wildcards;
  wildcards.reserve(clusters.size());
  // Trie with aggregated policies from all clusters
  Trie<std::vector<std::pair<size_t, std::shared_ptr<RouteHandleIf>>>> t;

  for (size_t clusterId = 0; clusterId < clusters.size(); ++clusterId) {
    auto& policy = clusters[clusterId];
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
  auto result = ut_.findPrefix(key);
  return result == ut_.end() ? emptyV_ : result->second;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
