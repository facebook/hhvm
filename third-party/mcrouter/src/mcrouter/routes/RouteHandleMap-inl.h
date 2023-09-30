/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <folly/fibers/FiberManager.h>
#include <folly/hash/Hash.h>

#include "mcrouter/Proxy.h"
#include "mcrouter/RoutingPrefix.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/route.h"
#include "mcrouter/routes/PrefixSelectorRoute.h"
#include "mcrouter/routes/RoutePolicyMap.h"
#include "mcrouter/routes/RouteSelectorMap.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

constexpr std::string_view kBroadcastPrefix = "/*/*/";
constexpr const char* kFallbackCluster = "fallback";

namespace detail {

template <class RouteHandleIf>
using RouteSelectorVector =
    std::vector<std::shared_ptr<PrefixSelectorRoute<RouteHandleIf>>>;

template <class RouteHandleIf>
struct VectorHash {
  size_t operator()(const RouteSelectorVector<RouteHandleIf>& v) const {
    size_t ret = 0;
    for (const auto& it : v) {
      ret = folly::hash::hash_combine(ret, it);
    }
    return ret;
  }
};

template <class RouteHandleIf>
using UniqueVectorMap = std::unordered_map<
    RouteSelectorVector<RouteHandleIf>,
    std::shared_ptr<RoutePolicyMap<RouteHandleIf>>,
    VectorHash<RouteHandleIf>>;

template <class RouteHandleIf>
std::shared_ptr<RoutePolicyMap<RouteHandleIf>> makePolicyMap(
    UniqueVectorMap<RouteHandleIf>& uniqueVectors,
    const RouteSelectorVector<RouteHandleIf>& v,
    bool enableRoutePolicyV2) {
  auto it = uniqueVectors.find(v);
  if (it != uniqueVectors.end()) {
    return it->second;
  }
  return uniqueVectors[v] = std::make_shared<RoutePolicyMap<RouteHandleIf>>(
             v, enableRoutePolicyV2);
}

} // namespace detail

template <class RouteHandleIf>
RouteHandleMap<RouteHandleIf>::RouteHandleMap(
    const RouteSelectorMap<RouteHandleIf>& routeSelectors,
    const RoutingPrefix& defaultRoute,
    bool sendInvalidRouteToDefault,
    bool enableRoutePolicyV2)
    : defaultRoute_(defaultRoute),
      sendInvalidRouteToDefault_(sendInvalidRouteToDefault),
      enableRoutePolicyV2_(enableRoutePolicyV2) {
  checkLogic(
      routeSelectors.find(defaultRoute_) != routeSelectors.end(),
      "invalid default route: {}",
      defaultRoute_.str());

  detail::RouteSelectorVector<RouteHandleIf> allRoutes;
  folly::StringKeyedUnorderedMap<detail::RouteSelectorVector<RouteHandleIf>>
      byRegion;
  folly::StringKeyedUnorderedMap<detail::RouteSelectorVector<RouteHandleIf>>
      byRoute;
  // add defaults first
  for (const auto& it : routeSelectors) {
    RoutingPrefix prefix(it.first);
    if (prefix.str() == defaultRoute_.str()) {
      allRoutes.push_back(it.second);
    }

    if (prefix.getRegion() == defaultRoute_.getRegion()) {
      byRegion[prefix.getRegion()].push_back(it.second);
    }
  }

  // then add rest
  for (const auto& it : routeSelectors) {
    RoutingPrefix prefix(it.first);
    if (prefix.str() != defaultRoute_.str()) {
      allRoutes.push_back(it.second);
    }

    if (prefix.getRegion() != defaultRoute_.getRegion()) {
      byRegion[prefix.getRegion()].push_back(it.second);
    }

    byRoute[it.first].push_back(it.second);
  }

  // create corresponding RoutePolicyMaps
  detail::UniqueVectorMap<RouteHandleIf> uniqueVectors;
  allRoutes_ = makePolicyMap(uniqueVectors, allRoutes, enableRoutePolicyV2_);
  for (const auto& it : byRegion) {
    byRegion_.emplace(
        it.first,
        makePolicyMap(uniqueVectors, it.second, enableRoutePolicyV2_));
  }
  for (const auto& it : byRoute) {
    byRoute_.emplace(
        it.first,
        makePolicyMap(uniqueVectors, it.second, enableRoutePolicyV2_));
  }

  assert(byRoute_.find(defaultRoute_) != byRoute_.end());
  defaultRouteMap_ = byRoute_[defaultRoute_];
}

template <class RouteHandleIf>
void RouteHandleMap<RouteHandleIf>::foreachRoutePolicy(
    folly::StringPiece prefix,
    std::function<void(const std::shared_ptr<RoutePolicyMap<RouteHandleIf>>&)>
        f) const {
  // if no route is provided or the default route matches the glob
  // then stick at the start so that we always send to the local cluster first
  if (prefix.empty() || match_pattern_route(prefix, defaultRoute_)) {
    auto it = byRoute_.find(defaultRoute_);
    if (it != byRoute_.end()) {
      f(it->second);
    }
  }

  if (prefix.empty()) {
    return;
  }

  bool selectAll = (prefix == kBroadcastPrefix);
  for (const auto& it : byRoute_) {
    if (it.first != defaultRoute_.str() &&
        (selectAll || match_pattern_route(prefix, it.first))) {
      f(it.second);
    }
  }
}

template <class RouteHandleIf>
std::vector<std::shared_ptr<RouteHandleIf>>
RouteHandleMap<RouteHandleIf>::getTargetsForKeySlow(
    folly::StringPiece prefix,
    folly::StringPiece key) const {
  struct Ctx {
    const RouteHandleMap* rhMap;
    // we need to ensure first policy is for local cluster
    std::shared_ptr<RouteHandleIf> first;
    std::unordered_set<std::shared_ptr<RouteHandleIf>> seen;
    Ctx(const RouteHandleMap* rhMap_) : rhMap(rhMap_) {}
  } c(this);

  auto result = folly::fibers::runInMainContext(
      [&c, prefix, key]() -> std::vector<std::shared_ptr<RouteHandleIf>> {
        c.rhMap->foreachRoutePolicy(
            prefix,
            [&c, key](const std::shared_ptr<RoutePolicyMap<RouteHandleIf>>& r) {
              const auto& policies = r->getTargetsForKey(key);
              for (const auto& policy : policies) {
                c.seen.insert(policy);
                if (!c.first) {
                  c.first = policy;
                }
              }
            });

        if (!c.first) {
          return {};
        }

        std::vector<std::shared_ptr<RouteHandleIf>> rh;
        rh.reserve(c.seen.size());
        rh.push_back(c.first);
        if (c.seen.size() > 1) {
          c.seen.erase(c.first);
          rh.insert(rh.end(), c.seen.begin(), c.seen.end());
        }
        return rh;
      });
  if (result.empty() && sendInvalidRouteToDefault_) {
    return defaultRouteMap_->getTargetsForKey(key);
  }
  return result;
}

template <class RouteHandleIf>
const std::vector<std::shared_ptr<RouteHandleIf>>*
RouteHandleMap<RouteHandleIf>::getTargetsForKeyFast(
    folly::StringPiece prefix,
    folly::StringPiece key) const {
  const std::vector<std::shared_ptr<RouteHandleIf>>* result = nullptr;
  if (FOLLY_LIKELY(prefix.empty())) {
    // empty prefix => route to default route
    result = &defaultRouteMap_->getTargetsForKey(key);
  } else if (prefix == kBroadcastPrefix) {
    // route to all routes
    result = &allRoutes_->getTargetsForKey(key);
  } else {
    auto starPos = prefix.find("*");
    if (starPos == std::string::npos) {
      // no stars at all
      auto it = byRoute_.find(prefix);
      if (it != byRoute_.end()) {
        result = &it->second->getTargetsForKey(key);
      } else {
        // cluster in question isn't in config, try the fallback
        result = getTargetsForKeyFallback(prefix, key);
      }
    } else if (prefix.endsWith("/*/") && starPos == prefix.size() - 2) {
      // route to all clusters of some region (/region/*/)
      auto region = prefix.subpiece(1, prefix.size() - 4);
      auto it = byRegion_.find(region);
      result =
          it == byRegion_.end() ? &emptyV_ : &it->second->getTargetsForKey(key);
    }
  }
  if (sendInvalidRouteToDefault_ && result != nullptr && result->empty()) {
    return &defaultRouteMap_->getTargetsForKey(key);
  }
  return result;
}

template <class RouteHandleIf>
const std::vector<std::shared_ptr<RouteHandleIf>>*
RouteHandleMap<RouteHandleIf>::getTargetsForKeyFallback(
    folly::StringPiece prefix,
    folly::StringPiece key) const {
  auto clusterStart = prefix.find('/', 1);
  if (clusterStart == std::string::npos) {
    return &emptyV_;
  }

  constexpr size_t kMaxBufLen = 128;
  char routingPrefixBuf[kMaxBufLen];

  // Construct /<region>/fallback/ prefix
  auto len = snprintf(
      routingPrefixBuf,
      kMaxBufLen,
      "%.*s%s/",
      static_cast<int>(clusterStart + 1),
      prefix.data(),
      kFallbackCluster);
  if (len > 0 && static_cast<size_t>(len) < kMaxBufLen) {
    auto it = byRoute_.find(folly::StringPiece(routingPrefixBuf, len));
    return it == byRoute_.end() ? &emptyV_ : &it->second->getTargetsForKey(key);
  }
  return &emptyV_;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
