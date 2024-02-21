/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/json/DynamicConverter.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/DynamicUtil.h"
#include "mcrouter/lib/SelectionRouteFactory.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/routes/ErrorRoute.h"
#include "mcrouter/routes/LatestRoute.h"
#include "mcrouter/routes/LoadBalancerRoute.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

std::vector<size_t> parseShardsPerServerJson(const folly::dynamic& json);

std::vector<std::vector<size_t>> parseAllShardsJson(
    const folly::dynamic& allShardsJson);

inline size_t getMaxShardId(const std::vector<std::vector<size_t>>& shardsMap) {
  size_t maxShardId = 0;
  for (const auto& shards : shardsMap) {
    for (auto shardId : shards) {
      maxShardId = std::max(maxShardId, shardId);
    }
  }
  return maxShardId;
}
template <class T>
size_t getMaxShardId(const std::unordered_map<uint32_t, T>& shardsMap) {
  uint32_t maxShardId = 0;
  for (const auto& item : shardsMap) {
    auto shardId = item.first;
    maxShardId = std::max(maxShardId, shardId);
  }
  return maxShardId;
}

template <class MapType>
MapType prepareMap(size_t numDistinctShards, size_t maxShardId);
template <>
inline std::vector<uint16_t> prepareMap(
    size_t /* numDistinctShards */,
    size_t maxShardId) {
  return std::vector<uint16_t>(
      maxShardId + 1, std::numeric_limits<uint16_t>::max());
}
template <>
inline std::unordered_map<uint32_t, uint32_t> prepareMap(
    size_t numDistinctShards,
    size_t /* maxShardId */) {
  return std::unordered_map<uint32_t, uint32_t>(numDistinctShards);
}

inline bool containsShard(const std::vector<uint16_t>& vec, size_t shard) {
  return vec.at(shard) != std::numeric_limits<uint16_t>::max();
}
inline bool containsShard(
    const std::unordered_map<uint32_t, uint32_t>& map,
    size_t shard) {
  return (map.find(shard) != map.end());
}

template <class RouterInfo>
const folly::dynamic& getPoolJson(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  assert(json.isObject());

  const auto* poolJson = json.get_ptr("pool");
  checkLogic(poolJson, "ShardSelectionRoute: 'pool' not found");
  return factory.parsePool(*poolJson);
}

template <class RouterInfo>
bool hasShardsJson(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const folly::StringPiece shardString = "shards") {
  const auto jPools = [&json]() {
    auto poolsJson = json.get_ptr("pools");
    checkLogic(
        poolsJson && poolsJson->isArray(),
        "EagerShardSelectionRoute: 'pools' not found");
    return *poolsJson;
  }();

  for (const auto& jPool : jPools) {
    const auto& poolJson = getPoolJson<RouterInfo>(factory, jPool);
    const auto* shardsJson = poolJson.get_ptr(shardString);
    if (!shardsJson) {
      shardsJson = jPool.get_ptr(shardString);
    }
    if (!shardsJson) {
      return false;
    }
  }
  return true;
}

template <class RouterInfo>
const folly::dynamic& getShardsJson(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const folly::StringPiece shardString = "shards") {
  assert(json.isObject());

  // first, look for shards inside the pool.
  const auto& poolJson = getPoolJson<RouterInfo>(factory, json);
  const auto* shardsJson = poolJson.get_ptr(shardString);

  // if not found, look outside.
  // TODO: kill this fallback logic when every client is at version >= 28
  if (!shardsJson) {
    shardsJson = json.get_ptr(shardString);
  }
  checkLogic(
      shardsJson && shardsJson->isArray(),
      "ShardSelectionRoute: '{}' not found or not an array",
      shardString);
  return *shardsJson;
}

/**
 * Build a map from shardId -> destinationId.
 */
template <class MapType>
MapType getShardsMap(const folly::dynamic& json, size_t numDestinations) {
  assert(json.isArray());

  checkLogic(
      numDestinations < std::numeric_limits<uint16_t>::max(),
      "ShardSelectionRoute: Only up to {} destinations are supported. "
      "Current number of destinations: {}",
      std::numeric_limits<uint16_t>::max() - 1,
      numDestinations);

  // Validate and get a list of shards.
  auto allShards = parseAllShardsJson(json);

  auto shardsMap =
      prepareMap<MapType>(allShards.size(), getMaxShardId(allShards));

  // We don't need to validate here, as it was validated before.
  size_t i = 0;
  for (const auto& vecShard : allShards) {
    for (const auto& shard : vecShard) {
      if (!containsShard(shardsMap, shard)) {
        shardsMap[shard] = i;
      } else {
        // Shard is served by two destinations, picking one randomly
        if (folly::Random::oneIn(2)) {
          shardsMap[shard] = i;
        }
      }
    }
    ++i;
  }

  return shardsMap;
}

template <class RouterInfo>
using ShardDestinationsMapCustomFn = std::function<
    void(uint32_t, std::vector<typename RouterInfo::RouteHandlePtr>&)>;

template <class RouterInfo>
using ShardDestinationsMap = std::
    unordered_map<uint32_t, std::vector<typename RouterInfo::RouteHandlePtr>>;

template <class RouterInfo>
ShardDestinationsMap<RouterInfo> getShardDestinationsMap(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const folly::StringPiece shardName = "shards") {
  const auto jPools = [&json]() {
    auto poolsJson = json.get_ptr("pools");
    checkLogic(
        poolsJson && poolsJson->isArray(),
        "EagerShardSelectionRoute: 'pools' not found");
    return *poolsJson;
  }();
  ShardDestinationsMap<RouterInfo> shardMap;

  for (const auto& jPool : jPools) {
    auto poolJson = getPoolJson<RouterInfo>(factory, jPool);
    auto destinations = factory.createList(poolJson);
    auto shardsJson = getShardsJson<RouterInfo>(factory, jPool, shardName);
    checkLogic(
        shardsJson.size() == destinations.size(),
        folly::sformat(
            "EagerShardSelectionRoute: '{}' must have the same number of "
            "entries as servers in 'pool'. Servers size: {}. Shards size: {}.",
            shardName,
            destinations.size(),
            shardsJson.size()));
    if (destinations.empty()) {
      continue;
    }

    size_t j = 0;
    for (const auto& v : shardsJson) {
      for (const auto& shard : parseShardsPerServerJson(v)) {
        auto rh = destinations[j];
        auto it = shardMap.find(shard);
        if (it == shardMap.end()) {
          it =
              shardMap
                  .emplace(
                      shard, std::vector<typename RouterInfo::RouteHandlePtr>())
                  .first;
        }
        it->second.push_back(std::move(rh));
      }
      ++j;
    }
  }
  for (auto& it : shardMap) {
    it.second.shrink_to_fit();
  }
  return shardMap;
}

template <class RouterInfo, class MapType>
void buildChildrenLatestRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const ShardDestinationsMap<RouterInfo>& shardMap,
    std::vector<typename RouterInfo::RouteHandlePtr>& destinations,
    MapType& shardToDestinationIndexMap,
    ShardDestinationsMapCustomFn<RouterInfo> customFn = nullptr) {
  LatestRouteOptions options =
      parseLatestRouteJson(json, factory.getThreadId());
  destinations.reserve(shardMap.size());
  std::for_each(shardMap.begin(), shardMap.end(), [&](auto& item) {
    auto shardId = item.first;
    auto childrenRouteHandles = folly::copy(item.second);
    size_t numChildren = childrenRouteHandles.size();
    if (customFn) {
      customFn(shardId, childrenRouteHandles);
    }
    destinations.push_back(createLatestRoute<RouterInfo>(
        json,
        std::move(childrenRouteHandles),
        options,
        std::vector<double>(numChildren, 1.0)));
    shardToDestinationIndexMap[shardId] = destinations.size() - 1;
  });
}

template <class RouterInfo, class MapType>
void buildChildrenLoadBalancerRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& /* factory */,
    const folly::dynamic& json,
    const ShardDestinationsMap<RouterInfo>& shardMap,
    std::vector<typename RouterInfo::RouteHandlePtr>& destinations,
    MapType& shardToDestinationIndexMap,
    ShardDestinationsMapCustomFn<RouterInfo> customFn = nullptr) {
  LoadBalancerRouteOptions<RouterInfo> options =
      parseLoadBalancerRouteJson<RouterInfo>(json);
  destinations.reserve(shardMap.size());
  std::for_each(shardMap.begin(), shardMap.end(), [&](auto& item) {
    auto shardId = item.first;
    auto childrenRouteHandles = folly::copy(item.second);
    if (customFn) {
      customFn(shardId, childrenRouteHandles);
    }
    destinations.push_back(createLoadBalancerRoute<RouterInfo>(
        std::move(childrenRouteHandles), options));
    shardToDestinationIndexMap[shardId] = destinations.size() - 1;
  });
}

template <class RouterInfo, class MapType>
void buildChildrenCustomRoutesFromMap(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const ShardDestinationsMap<RouterInfo>& shardMap,
    const RouteHandleWithChildrenFactoryFn<RouterInfo>& createCustomRh,
    std::vector<typename RouterInfo::RouteHandlePtr>& destinations,
    MapType& shardToDestinationIndexMap,
    ShardDestinationsMapCustomFn<RouterInfo> customFn = nullptr) {
  destinations.reserve(shardMap.size());
  std::for_each(shardMap.begin(), shardMap.end(), [&](auto& item) {
    auto shardId = item.first;
    auto childrenRouteHandles = folly::copy(item.second);
    if (customFn) {
      customFn(shardId, childrenRouteHandles);
    }
    destinations.push_back(
        createCustomRh(factory, json, std::move(childrenRouteHandles)));
    shardToDestinationIndexMap[shardId] = destinations.size() - 1;
  });
}

template <class RouterInfo, class MapType>
void buildChildrenCustomJsonmRoutes(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const ShardDestinationsMap<RouterInfo>& shardMap,
    std::vector<typename RouterInfo::RouteHandlePtr>& destinations,
    MapType& shardToDestinationIndexMap) {
  destinations.reserve(shardMap.size());
  std::for_each(shardMap.begin(), shardMap.end(), [&](auto& item) {
    auto shardId = item.first;
    auto childrenList = folly::copy(item.second);
    // push children to factory. Factory will use when it sees "$children_list$"
    factory.pushChildrenList(std::move(childrenList));
    destinations.push_back(factory.create(json));
    factory.popChildrenList();
    shardToDestinationIndexMap[shardId] = destinations.size() - 1;
  });
}

} // namespace detail

// routes
template <class RouterInfo, class ShardSelector, class MapType>
typename RouterInfo::RouteHandlePtr createShardSelectionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "ShardSelectionRoute config should be an object");

  const auto& poolJson = detail::getPoolJson<RouterInfo>(factory, json);
  auto destinations = factory.createList(poolJson);
  if (destinations.empty()) {
    LOG(WARNING) << "ShardSelectionRoute: Empty list of destinations found. "
                 << "Using ErrorRoute.";
    return mcrouter::createErrorRoute<RouterInfo>(
        "ShardSelectionRoute has an empty list of destinations");
  }

  const auto& shardsJson = detail::getShardsJson<RouterInfo>(factory, json);
  checkLogic(
      shardsJson.size() == destinations.size(),
      folly::sformat(
          "ShardSelectionRoute: 'shards' must have the same number of "
          "entries as servers in 'pool'. Servers size: {}. Shards size: {}.",
          destinations.size(),
          shardsJson.size()));

  auto selector = ShardSelector(
      detail::getShardsMap<MapType>(shardsJson, destinations.size()));

  typename RouterInfo::RouteHandlePtr outOfRangeDestination = nullptr;
  if (auto outOfRangeJson = json.get_ptr("out_of_range")) {
    outOfRangeDestination = factory.create(*outOfRangeJson);
  }

  return createSelectionRoute<RouterInfo, ShardSelector>(
      std::move(destinations),
      std::move(selector),
      std::move(outOfRangeDestination));
}

template <class RouterInfo, class ShardFilter>
typename RouterInfo::RouteHandlePtr createShardFilterRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(json.isObject(), "ShardFilterRoute config should be an object");
  auto jMatchChild = json.get_ptr("match_child");
  checkLogic(
      jMatchChild != nullptr,
      "ShardFilterRoute: 'match_child' property is missing");
  auto jDefaultChild = json.get_ptr("default_child");
  checkLogic(
      jMatchChild != nullptr,
      "ShardFilterRoute: 'default_child' property is missing");
  auto jShardRange = json.get_ptr("shard_range");
  checkLogic(
      jShardRange != nullptr,
      "ShardFilterRoute: 'shard_range' property is missing");
  auto allShards = detail::parseAllShardsJson(*jShardRange);
  for (auto shard : allShards) {
    checkLogic(
        shard.size() == 1 || shard.size() == 2,
        "Each shard_range member must be of format (int, int)");
  }
  double weightThreshold = 0.0;
  if (auto jWeightThreshold = json.get_ptr("weight_threshold")) {
    checkLogic(
        jWeightThreshold->isDouble(), "weight_threshold is not a double");
    checkLogic(
        jWeightThreshold->asDouble() <= 1.0,
        "weight_threshold must be within [0.0, 1.0]");
    checkLogic(
        jWeightThreshold->asDouble() >= 0.0,
        "weight_threshold must be within [0.0, 1.0]");
    weightThreshold = jWeightThreshold->asDouble();
  }
  // Check that shard ranges do not overlap
  checkLogic(!intervalOverlap(allShards), "Ranges must not overlap");
  auto selector = ShardFilter(std::move(allShards), weightThreshold);

  typename RouterInfo::RouteHandlePtr outOfRangeDestination = nullptr;
  if (auto outOfRangeJson = json.get_ptr("out_of_range")) {
    outOfRangeDestination = factory.create(*outOfRangeJson);
  }
  std::vector<typename RouterInfo::RouteHandlePtr> destinations;
  destinations.reserve(2);
  destinations.push_back(factory.create(*jMatchChild));
  destinations.push_back(factory.create(*jDefaultChild));
  return createSelectionRoute<RouterInfo, ShardFilter>(
      std::move(destinations),
      std::move(selector),
      std::move(outOfRangeDestination));
}

template <class RouterInfo, class ShardSelector, class MapType>
typename RouterInfo::RouteHandlePtr createEagerShardSelectionRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const ChildrenFactoryMap<RouterInfo>& childrenFactoryMap) {
  checkLogic(
      json.isObject(), "EagerShardSelectionRoute config should be an object");

  const auto childrenType = [&json]() {
    auto jChildType = json.get_ptr("children_type");
    checkLogic(
        jChildType && jChildType->isString(),
        "EagerShardSelectionRoute: 'children_type' not found or is not a "
        "string");
    return jChildType->stringPiece();
  }();

  const auto& childrenSettings = [&json]() {
    auto jSettings = json.get_ptr("children_settings");
    checkLogic(
        jSettings && jSettings->isObject(),
        "EagerShardSelectionRoute: 'children_settings' not found or not an "
        "object");
    return *jSettings;
  }();

  auto shardMap = detail::getShardDestinationsMap<RouterInfo>(factory, json);
  if (shardMap.empty()) {
    return mcrouter::createErrorRoute<RouterInfo>(
        "EagerShardSelectionRoute has an empty list of destinations");
  }

  MapType shardToDestinationIndexMap = detail::prepareMap<MapType>(
      shardMap.size(), detail::getMaxShardId(shardMap));
  std::vector<typename RouterInfo::RouteHandlePtr> destinations;
  if (childrenType == "LoadBalancerRoute") {
    detail::buildChildrenLoadBalancerRoutes<RouterInfo, MapType>(
        factory,
        childrenSettings,
        shardMap,
        destinations,
        shardToDestinationIndexMap);
  } else if (childrenType == "LatestRoute") {
    detail::buildChildrenLatestRoutes<RouterInfo, MapType>(
        factory,
        childrenSettings,
        shardMap,
        destinations,
        shardToDestinationIndexMap);
  } else if (childrenType == "CustomJsonmRoute") {
    detail::buildChildrenCustomJsonmRoutes<RouterInfo, MapType>(
        factory,
        childrenSettings,
        shardMap,
        destinations,
        shardToDestinationIndexMap);
  } else {
    auto it = childrenFactoryMap.find(childrenType.str());
    if (it != childrenFactoryMap.end()) {
      detail::buildChildrenCustomRoutesFromMap<RouterInfo, MapType>(
          factory,
          childrenSettings,
          shardMap,
          it->second,
          destinations,
          shardToDestinationIndexMap);
    } else {
      throwLogic(
          "EagerShardSelectionRoute: 'children_type' {} not supported",
          childrenType);
    }
  }

  ShardSelector selector(std::move(shardToDestinationIndexMap));

  typename RouterInfo::RouteHandlePtr outOfRangeDestination = nullptr;
  if (auto outOfRangeJson = json.get_ptr("out_of_range")) {
    outOfRangeDestination = factory.create(*outOfRangeJson);
  }

  return createSelectionRoute<RouterInfo, ShardSelector>(
      std::move(destinations),
      std::move(selector),
      std::move(outOfRangeDestination));
}

template <
    class RouterInfo,
    class ShardSelector,
    class MapType,
    class ShadowSelectorPolicy>
typename RouterInfo::RouteHandlePtr createEagerShardSelectionShadowRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const ChildrenFactoryMap<RouterInfo>& childrenFactoryMap,
    const ChildrenFactoryMap<RouterInfo>& shadowChildrenFactoryMap,
    const folly::Optional<ShadowSelectorPolicy>& shadowSelectorPolicy,
    const uint32_t seed) {
  checkLogic(
      json.isObject(),
      "EagerShardSelectionShadowRoute config should be an object");

  const auto childrenType = [&json]() {
    auto jChildType = json.get_ptr("children_type");
    checkLogic(
        jChildType && jChildType->isString(),
        "EagerShardSelectionShadowRoute: 'children_type' not found or is not a "
        "string");
    return jChildType->stringPiece();
  }();

  const auto& childrenSettings = [&json]() {
    auto jSettings = json.get_ptr("children_settings");
    checkLogic(
        jSettings && jSettings->isObject(),
        "EagerShardSelectionShadowRoute: 'children_settings' not found or not an "
        "object");
    return *jSettings;
  }();

  std::vector<typename RouterInfo::RouteHandlePtr> destinations;
  auto shardMap = detail::getShardDestinationsMap<RouterInfo>(factory, json);
  MapType shardToDestinationIndexMap = detail::prepareMap<MapType>(
      shardMap.size(), detail::getMaxShardId(shardMap));
  if (!shardMap.empty()) {
    if (childrenType == "LoadBalancerRoute") {
      detail::buildChildrenLoadBalancerRoutes<RouterInfo, MapType>(
          factory,
          childrenSettings,
          shardMap,
          destinations,
          shardToDestinationIndexMap);
    } else if (childrenType == "LatestRoute") {
      detail::buildChildrenLatestRoutes<RouterInfo, MapType>(
          factory,
          childrenSettings,
          shardMap,
          destinations,
          shardToDestinationIndexMap);
    } else if (childrenType == "CustomJsonmRoute") {
      detail::buildChildrenCustomJsonmRoutes<RouterInfo, MapType>(
          factory,
          childrenSettings,
          shardMap,
          destinations,
          shardToDestinationIndexMap);
    } else {
      auto it = childrenFactoryMap.find(childrenType.str());
      if (it != childrenFactoryMap.end()) {
        detail::buildChildrenCustomRoutesFromMap<RouterInfo, MapType>(
            factory,
            childrenSettings,
            shardMap,
            it->second,
            destinations,
            shardToDestinationIndexMap);
      } else {
        throwLogic(
            "EagerShardSelectionRoute: 'children_type' {} not supported",
            childrenType);
      }
    }
  }

  // Out Of Range Destinations
  typename RouterInfo::RouteHandlePtr outOfRangeDestination = nullptr;
  if (auto outOfRangeJson = json.get_ptr("out_of_range")) {
    outOfRangeDestination = factory.create(*outOfRangeJson);
  }

  // If shard map is empty, the selector will mark everything out_of_range
  ShardSelector selector(std::move(shardToDestinationIndexMap));

  /**
   * Shadow settings
   */

  // Shadow children type
  const auto shadowChildrenType = [&json]() {
    auto jChildType = json.get_ptr("shadow_children_type");
    checkLogic(
        jChildType && jChildType->isString(),
        "EagerShardSelectionShadowRoute: 'shadow_children_type' not found or is not a "
        "string");
    return jChildType->stringPiece();
  }();

  // Shadow children settings
  const auto& shadowChildrenSettings = [&json]() {
    auto jSettings = json.get_ptr("shadow_children_settings");
    checkLogic(
        jSettings && jSettings->isObject(),
        "EagerShardSelectionShadowRoute: 'shadow_children_settings' not found or not an "
        "object");
    return *jSettings;
  }();

  // Weight of hosts in shadow shard map. If > 1.0, hosts in shadow shard map
  // will receive more shadow requests. If < 1.0, hosts in shadow shard map
  // will receive less shadow requests.
  double shadowWeights = 1.0;
  if (auto jShadowWeights = json.get_ptr("shadow_weights")) {
    checkLogic(jShadowWeights->isDouble(), "shadow_weights is not a double");
    shadowWeights = jShadowWeights->asDouble();
  }

  constexpr folly::StringPiece shadowShardName = "shadow_shards";
  if (detail::hasShardsJson<RouterInfo>(factory, json, shadowShardName) ==
      false) {
    // Shadow shards property is missing, create a regular selection route
    return createSelectionRoute<RouterInfo, ShardSelector>(
        std::move(destinations),
        std::move(selector),
        std::move(outOfRangeDestination));
  }

  // Create Shadow Shard Map
  auto shadowShardMap = detail::getShardDestinationsMap<RouterInfo>(
      factory, json, shadowShardName);
  if (shadowShardMap.empty()) {
    // Shard and Shadow Shard maps are empty, create error route.
    if (shardMap.empty()) {
      return mcrouter::createErrorRoute<RouterInfo>(
          "EagerShardSelectionShadowRoute has an empty list of destinations");
    } else {
      // Shadow shards are empty, create a regular selection route
      return createSelectionRoute<RouterInfo, ShardSelector>(
          std::move(destinations),
          std::move(selector),
          std::move(outOfRangeDestination));
    }
  }

  // Builds vector containing the probabilities
  std::vector<uint16_t> shadowProbabilities;
  detail::ShardDestinationsMapCustomFn<RouterInfo> probabilityBuilderFn =
      [&shadowProbabilities, &shardMap, &shadowWeights](
          uint32_t shardId,
          std::vector<typename RouterInfo::RouteHandlePtr>&
              childrenRouteHandles) {
        auto it = shardMap.find(shardId);
        size_t shardSize = 0;
        if (it != shardMap.end()) {
          // Implementation goes here
          shardSize = it->second.size();
        }
        // Entries in vector are from (0,100] representing % of time that if
        // selected by shadowSelector that request will be shadowed to child
        // route handle.
        uint16_t probability =
            (((uint16_t)(100 * childrenRouteHandles.size() * shadowWeights)) /
             (shardSize + childrenRouteHandles.size()));
        shadowProbabilities.push_back(probability);
      };

  MapType shadowShardToDestinationIndexMap = detail::prepareMap<MapType>(
      shadowShardMap.size(), detail::getMaxShardId(shadowShardMap));
  std::vector<typename RouterInfo::RouteHandlePtr> shadowDestinations;
  if (shadowChildrenType == "LoadBalancerRoute") {
    detail::buildChildrenLoadBalancerRoutes<RouterInfo, MapType>(
        factory,
        shadowChildrenSettings,
        shadowShardMap,
        shadowDestinations,
        shadowShardToDestinationIndexMap,
        probabilityBuilderFn);
  } else if (shadowChildrenType == "LatestRoute") {
    detail::buildChildrenLatestRoutes<RouterInfo, MapType>(
        factory,
        shadowChildrenSettings,
        shadowShardMap,
        shadowDestinations,
        shadowShardToDestinationIndexMap,
        probabilityBuilderFn);
  } else if (shadowChildrenType == "CustomJsonmRoute") {
    throwLogic(
        "EagerShardSelectionShadowRoute: {} not supported", shadowChildrenType);
  } else {
    auto it = shadowChildrenFactoryMap.find(shadowChildrenType.str());
    if (it != shadowChildrenFactoryMap.end()) {
      detail::buildChildrenCustomRoutesFromMap<RouterInfo, MapType>(
          factory,
          shadowChildrenSettings,
          shadowShardMap,
          it->second,
          shadowDestinations,
          shadowShardToDestinationIndexMap,
          probabilityBuilderFn);
    } else {
      throwLogic(
          "EagerShardSelectionRoute: 'shadow_children_type' {} not supported",
          shadowChildrenType);
    }
  }

  ShardSelector shadowSelector(std::move(shadowShardToDestinationIndexMap));
  shadowProbabilities.shrink_to_fit();
  return createSelectionRoute<RouterInfo, ShardSelector, ShadowSelectorPolicy>(
      std::move(destinations),
      std::move(selector),
      std::move(outOfRangeDestination),
      std::move(shadowDestinations),
      std::move(shadowSelector),
      std::move(shadowProbabilities),
      std::move(shadowSelectorPolicy),
      seed);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
