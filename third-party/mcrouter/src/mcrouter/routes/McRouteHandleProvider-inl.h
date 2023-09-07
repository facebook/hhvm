/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>

#include <folly/Conv.h>
#include <folly/Range.h>

#include "mcrouter/CarbonRouterInstanceBase.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestination.h"
#include "mcrouter/ProxyDestinationMap.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/fbi/cpp/ParsingUtil.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/AccessPoint.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/FailureDomains.h"
#include "mcrouter/lib/network/SecurityOptions.h"
#include "mcrouter/lib/network/ThriftTransport.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/AllFastestRouteFactory.h"
#include "mcrouter/routes/AsynclogRoute.h"
#include "mcrouter/routes/DestinationRoute.h"
#include "mcrouter/routes/ExtraRouteHandleProviderIf.h"
#include "mcrouter/routes/FailoverRoute.h"
#include "mcrouter/routes/HashRouteFactory.h"
#include "mcrouter/routes/McBucketRoute.h"
#include "mcrouter/routes/PoolRouteUtils.h"
#include "mcrouter/routes/RateLimitRoute.h"
#include "mcrouter/routes/RateLimiter.h"
#include "mcrouter/routes/ShadowRoute.h"
#include "mcrouter/routes/ShardHashFunc.h"
#include "mcrouter/routes/ShardSplitRoute.h"
#include "mcrouter/routes/ShardSplitter.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

static constexpr uint32_t kMaxTotalFanout = 32 * 1024;

extern template MemcacheRouterInfo::RouteHandlePtr
createHashRoute<MemcacheRouterInfo>(
    const folly::dynamic& json,
    std::vector<MemcacheRouterInfo::RouteHandlePtr> rh,
    size_t threadId);

extern template MemcacheRouterInfo::RouteHandlePtr
makeAllFastestRoute<MemcacheRouterInfo>(
    RouteHandleFactory<MemcacheRouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

extern template MemcacheRouterInfo::RouteHandlePtr
makeFailoverRouteWithFailoverErrorSettings<
    MemcacheRouterInfo,
    FailoverRoute,
    FailoverErrorsSettings>(
    const folly::dynamic& json,
    std::vector<MemcacheRouterInfo::RouteHandlePtr> children,
    FailoverErrorsSettings failoverErrors,
    const folly::dynamic* jFailoverPolicy);

extern template std::tuple<
    std::vector<MemcacheRouterInfo::RouteHandlePtr>,
    std::optional<folly::dynamic>>
McRouteHandleProvider<MemcacheRouterInfo>::makePool(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const PoolFactory::PoolJson& json);

extern template MemcacheRouterInfo::RouteHandlePtr
McRouteHandleProvider<MemcacheRouterInfo>::makePoolRoute(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json);

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf> makeLoggingRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json);

template <class RouteHandleIf>
std::shared_ptr<RouteHandleIf> makeNullRoute(
    RouteHandleFactory<RouteHandleIf>& factory,
    const folly::dynamic& json);

template <class RouterInfo>
McRouteHandleProvider<RouterInfo>::McRouteHandleProvider(
    ProxyBase& proxy,
    PoolFactory& poolFactory)
    : proxy_(proxy),
      poolFactory_(poolFactory),
      extraProvider_(buildExtraProvider()),
      routeMap_(buildCheckedRouteMap()),
      routeMapWithProxy_(buildCheckedRouteMapWithProxy()),
      routeMapForWrapper_(buildCheckedRouteMapForWrapper()) {}

template <class RouterInfo>
McRouteHandleProvider<RouterInfo>::~McRouteHandleProvider() {
  /* Needed for forward declaration of ExtraRouteHandleProviderIf in .h */
}

template <class RouterInfo>
std::unique_ptr<ExtraRouteHandleProviderIf<RouterInfo>>
McRouteHandleProvider<RouterInfo>::buildExtraProvider() {
  return RouterInfo::buildExtraProvider();
}

template <>
std::unique_ptr<ExtraRouteHandleProviderIf<MemcacheRouterInfo>>
McRouteHandleProvider<MemcacheRouterInfo>::buildExtraProvider();

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McRouteHandleProvider<RouterInfo>::createAsynclogRoute(
    RouteHandlePtr target,
    std::string asynclogName) {
  if (!proxy_.router().opts().asynclog_disable) {
    target = makeAsynclogRoute<RouterInfo>(std::move(target), asynclogName);
  }
  asyncLogRoutes_.emplace(std::move(asynclogName), target);
  return target;
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McRouteHandleProvider<RouterInfo>::wrapAxonLogRoute(
    RouteHandlePtr route,
    ProxyBase& proxy,
    const folly::dynamic& json) {
  bool needAxonlog = false;
  if (auto* jNeedAxonlog = json.get_ptr("axonlog")) {
    needAxonlog = parseBool(*jNeedAxonlog, "axonlog");
  }
  if (needAxonlog) {
    auto it = routeMapForWrapper_.find("AxonLogRoute");
    checkLogic(
        it != routeMapForWrapper_.end(),
        "AxonLogRoute is not implemented for this router");
    return it->second(std::move(route), proxy, json);
  }
  return route;
}

template <class RouterInfo>
std::tuple<
    std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>,
    std::optional<folly::dynamic>>
McRouteHandleProvider<RouterInfo>::makePool(
    RouteHandleFactory<RouteHandleIf>& factory,
    const PoolFactory::PoolJson& jpool) {
  auto existingIt = pools_.find(jpool.name);
  auto existingWeightsIt = poolWeights_.find(jpool.name);
  if (existingIt != pools_.end() && existingWeightsIt != poolWeights_.end()) {
    return {existingIt->second, existingWeightsIt->second};
  }

  auto name = jpool.name.str();
  const auto& json = jpool.json;
  auto& opts = proxy_.router().opts();
  // region & cluster
  folly::StringPiece region, cluster;
  if (auto jregion = json.get_ptr("region")) {
    if (!jregion->isString()) {
      MC_LOG_FAILURE(
          opts,
          memcache::failure::Category::kInvalidConfig,
          "Pool {}: pool_region is not a string",
          name);
    } else {
      region = jregion->stringPiece();
    }
  }
  if (auto jcluster = json.get_ptr("cluster")) {
    if (!jcluster->isString()) {
      MC_LOG_FAILURE(
          opts,
          memcache::failure::Category::kInvalidConfig,
          "Pool {}: pool_cluster is not a string",
          name);
    } else {
      cluster = jcluster->stringPiece();
    }
  }

  try {
    std::chrono::milliseconds timeout{opts.server_timeout_ms};
    if (auto jTimeout = json.get_ptr("server_timeout")) {
      timeout = parseTimeout(*jTimeout, "server_timeout");
    }

    std::chrono::milliseconds connectTimeout = timeout;
    if (auto jConnectTimeout = json.get_ptr("connect_timeout")) {
      connectTimeout = parseTimeout(*jConnectTimeout, "connect_timeout");
    }

    if (!region.empty() && !cluster.empty()) {
      auto& route = opts.default_route;
      if (region == route.getRegion() && cluster == route.getCluster()) {
        if (opts.within_cluster_timeout_ms != 0) {
          timeout = std::chrono::milliseconds(opts.within_cluster_timeout_ms);
        }
      } else if (region == route.getRegion()) {
        if (opts.cross_cluster_timeout_ms != 0) {
          timeout = std::chrono::milliseconds(opts.cross_cluster_timeout_ms);
        }
      } else {
        if (opts.cross_region_timeout_ms != 0) {
          timeout = std::chrono::milliseconds(opts.cross_region_timeout_ms);
        }
      }
    }

    bool keepRoutingPrefix = false;
    if (auto jKeepRoutingPrefix = json.get_ptr("keep_routing_prefix")) {
      keepRoutingPrefix = parseBool(*jKeepRoutingPrefix, "keep_routing_prefix");
    }

    uint32_t qosClass = opts.default_qos_class;
    uint32_t qosPath = opts.default_qos_path;
    if (auto jQos = json.get_ptr("qos")) {
      checkLogic(jQos->isObject(), "qos must be an object.");
      if (auto jClass = jQos->get_ptr("class")) {
        qosClass = parseInt(*jClass, "qos.class", 0, 4);
      }
      if (auto jPath = jQos->get_ptr("path")) {
        qosPath = parseInt(*jPath, "qos.path", 0, 3);
      }
    }

    auto apAttr = getCommonAccessPointAttributes(json, proxy_.router());

    bool disableRequestDeadlineCheck =
        proxy_.router().opts().disable_request_deadline_check;
    if (auto jRequestDeadline =
            json.get_ptr("disable_request_deadline_check")) {
      disableRequestDeadlineCheck =
          parseBool(*jRequestDeadline, "disable_request_deadline_check");
    }
    auto jservers = json.get_ptr("servers");
    checkLogic(jservers, "servers not found");
    checkLogic(jservers->isArray(), "servers is not an array");
    uint32_t numTkoThresholdUpper = 0;
    uint32_t numTkoThresholdLower = 0;
    std::shared_ptr<PoolTkoTracker> poolTkoTracker;
    if (auto poolTkoTrackerConfig = json.get_ptr("tko_tracker")) {
      if (auto jNumTkoThresholdUpper =
              poolTkoTrackerConfig->get_ptr("num_tko_threshold_upper")) {
        numTkoThresholdUpper = jNumTkoThresholdUpper->asInt();
      } else if (
          auto jPercentTkoThresholdUpper =
              poolTkoTrackerConfig->get_ptr("percent_tko_threshold_upper")) {
        numTkoThresholdUpper =
            (jPercentTkoThresholdUpper->asInt() * jservers->size()) / 100;
      }

      if (auto jNumTkoThresholdLower =
              poolTkoTrackerConfig->get_ptr("num_tko_threshold_lower")) {
        numTkoThresholdLower = jNumTkoThresholdLower->asInt();
      } else if (
          auto jPercentTkoThresholdLower =
              poolTkoTrackerConfig->get_ptr("percent_tko_threshold_lower")) {
        numTkoThresholdLower =
            (jPercentTkoThresholdLower->asInt() * jservers->size()) / 100;
      }
      checkLogic(
          numTkoThresholdUpper > 0 && numTkoThresholdLower > 0,
          "Both tko threshold upper and lower must be configured");
      checkLogic(
          numTkoThresholdLower <= numTkoThresholdUpper,
          "tko upper threshold must be greater than or equal to lower"
          " threshold");
      poolTkoTracker = proxy_.destinationMap()->createPoolTkoTracker(
          name, numTkoThresholdUpper, numTkoThresholdLower);
    }

    // servers
    auto jhostnames = json.get_ptr("hostnames");
    auto jfailureDomains = json.get_ptr("failure_domains");
    auto jAdditionalFanout = json.get_ptr("additional_fanout");
    checkLogic(
        !jfailureDomains || jfailureDomains->isArray(),
        "failure_domains is not an array");
    checkLogic(
        !jhostnames || jhostnames->isArray(), "hostnames is not an array");
    checkLogic(
        !jhostnames || jhostnames->size() == jservers->size(),
        "hostnames expected to be of the same size as servers, "
        "expected {}, got {}",
        jservers->size(),
        jhostnames ? jhostnames->size() : 0);

    checkLogic(
        !jfailureDomains || jfailureDomains->size() == jservers->size(),
        "failure_domains expected to be of the same size as servers, "
        "expected {}, got {}",
        jservers->size(),
        jfailureDomains ? jfailureDomains->size() : 0);

    checkLogic(
        !jAdditionalFanout || jAdditionalFanout->isInt(),
        "additional_fanout is not an integer");
    uint32_t additionalFanout = 0;
    if (jAdditionalFanout) {
      additionalFanout = jAdditionalFanout->getInt();
    }
    checkLogic(
        static_cast<uint64_t>(additionalFanout + 1) *
                static_cast<uint64_t>(proxy_.router().opts().num_proxies) <=
            kMaxTotalFanout,
        "(additional_fanout={} + 1) * num_proxies={} must be <= {}",
        additionalFanout,
        proxy_.router().opts().num_proxies,
        kMaxTotalFanout);

    checkLogic(
        additionalFanout == 0 || !proxy_.router().opts().thread_affinity,
        "additional_fanout is not supported with thread_affinity");

    int32_t poolStatIndex = proxy_.router().getStatsEnabledPoolIndex(name);

    std::vector<RouteHandlePtr> destinations;
    destinations.reserve(jservers->size());
    auto jExistingWeights = json.get_ptr("weights");
    auto jWeights = jExistingWeights;
    if (jWeights &&
        (!jWeights->isArray() || jWeights->size() != jservers->size())) {
      jWeights = nullptr;
    }
    folly::dynamic jNewWeights = folly::dynamic::array;
    for (size_t i = 0; i < jservers->size(); ++i) {
      const auto& server = jservers->at(i);
      checkLogic(
          server.isString() || server.isObject(),
          "server #{} is not a string/object",
          i);
      auto addDestination = [&](auto&& dest) {
        destinations.push_back(std::move(dest));
        if (jWeights) {
          jNewWeights.push_back(jWeights->at(i));
        }
      };
      if (server.isObject()) {
        addDestination(factory.create(server));
        continue;
      }

      uint32_t failureDomain = 0;

      if (jfailureDomains && jfailureDomains->at(i).isInt()) {
        failureDomain = jfailureDomains->at(i).asInt();
      } else if (jfailureDomains && jfailureDomains->at(i).isString()) {
        failureDomain = getFailureDomainHash(jfailureDomains->at(i).asString());
      } else {
        proxy_.stats().increment(dest_with_no_failure_domain_count_stat);
      }

      for (uint32_t idx = 0; idx < (1 + additionalFanout); ++idx) {
        auto ap = createAccessPoint(
            server.stringPiece(), failureDomain, proxy_.router(), *apAttr);

        auto it = accessPoints_.find(name);
        if (it == accessPoints_.end()) {
          folly::F14FastSet<std::shared_ptr<const AccessPoint>> accessPoints;
          it = accessPoints_.emplace(name, std::move(accessPoints)).first;
        }
        folly::StringPiece nameSp = it->first;

        if (ap->getProtocol() == mc_thrift_protocol) {
          checkLogic(
              ap->getSecurityMech() == SecurityMech::NONE ||
                  ap->getSecurityMech() == SecurityMech::TLS ||
                  ap->getSecurityMech() == SecurityMech::TLS13_FIZZ ||
                  ap->getSecurityMech() == SecurityMech::TLS_TO_PLAINTEXT,
              "Security mechanism must be 'plain', 'tls', 'fizz' or "
              "'tls_to_plain' for ThriftTransport, got {}",
              securityMechToString(ap->getSecurityMech()));

          using Transport = ThriftTransport<RouterInfo>;
          auto destResult = createDestinationRoute<Transport>(
              std::move(ap),
              timeout,
              connectTimeout,
              qosClass,
              qosPath,
              nameSp,
              i,
              poolStatIndex,
              disableRequestDeadlineCheck,
              poolTkoTracker,
              keepRoutingPrefix,
              idx);
          it->second.insert(destResult.second);
          addDestination(std::move(destResult.first));
        } else {
          using Transport = AsyncMcClient;
          auto destResult = createDestinationRoute<Transport>(
              std::move(ap),
              timeout,
              connectTimeout,
              qosClass,
              qosPath,
              nameSp,
              i,
              poolStatIndex,
              disableRequestDeadlineCheck,
              poolTkoTracker,
              keepRoutingPrefix,
              idx);
          it->second.insert(destResult.second);
          addDestination(std::move(destResult.first));
        }
      }
    } // servers

    if (auto jPoolConfigPath = json.get_ptr("pool_config_path")) {
      auto enablePartialReconfig = json["enable_partial_reconfig"].asBool() &&
          apAttr->protocol >= mc_caret_protocol;
      auto poolConfigPath = jPoolConfigPath->getString();
      folly::dynamic poolConfigJson = folly::dynamic::object;
      for (auto propName : json.keys()) {
        const auto jProp = json.get_ptr(propName);
        if (jProp->isArray() || propName == "services") {
          continue;
        }
        poolConfigJson[propName] = *jProp;
      }
      poolConfigJson["enable_partial_reconfig"] = enablePartialReconfig;

      auto it = partialConfigs_.find(poolConfigPath);

      if (it == partialConfigs_.end()) {
        apAttr->json = folly::dynamic::object;
        apAttr->json[name] = poolConfigJson;
        std::vector<std::string> poolNames = {name};
        std::vector<std::pair<
            std::shared_ptr<CommonAccessPointAttributes>,
            std::vector<std::string>>>
            poolConfigGroups = {std::make_pair(apAttr, poolNames)};
        partialConfigs_.emplace(
            poolConfigPath,
            std::make_pair(enablePartialReconfig, poolConfigGroups));
      } else {
        it->second.first = it->second.first && enablePartialReconfig;
        bool found = false;
        auto& poolConfigGroups = it->second.second;
        for (auto& p : poolConfigGroups) {
          if (*p.first == *apAttr) {
            p.second.push_back(name);
            p.first->json[name] = poolConfigJson;
            found = true;
            break;
          }
        }
        if (!found) {
          std::vector<std::string> poolNames = {name};
          apAttr->json = folly::dynamic::object;
          apAttr->json[name] = poolConfigJson;
          poolConfigGroups.emplace_back(apAttr, poolNames);
        }
      }
    }
    /**
     * For backwards compatibility, return invalidly sized "weights" array here
     * anyway
     */
    return {
        pools_.emplace(name, std::move(destinations)).first->second,
        poolWeights_
            .emplace(
                name,
                jWeights ? std::optional{jNewWeights}
                         : (jExistingWeights ? std::optional{*jExistingWeights}
                                             : std::nullopt))
            .first->second};
  } catch (const std::exception& e) {
    throwLogic("Pool {}: {}", name, e.what());
  }
}

template <class RouterInfo>
template <class Transport>
std::pair<
    typename McRouteHandleProvider<RouterInfo>::RouteHandlePtr,
    std::shared_ptr<const AccessPoint>>
McRouteHandleProvider<RouterInfo>::createDestinationRoute(
    std::shared_ptr<AccessPoint> ap,
    std::chrono::milliseconds timeout,
    std::chrono::milliseconds connectTimeout,
    uint32_t qosClass,
    uint32_t qosPath,
    folly::StringPiece poolName,
    size_t indexInPool,
    int32_t poolStatIndex,
    bool disableRequestDeadlineCheck,
    const std::shared_ptr<PoolTkoTracker>& poolTkoTracker,
    bool keepRoutingPrefix,
    uint32_t idx) {
  auto pdstn = proxy_.destinationMap()->template emplace<Transport>(
      std::move(ap), timeout, qosClass, qosPath, poolTkoTracker, idx);
  pdstn->updateShortestTimeout(connectTimeout, timeout);
  auto resAp = pdstn->accessPoint();

  return {
      makeDestinationRoute<RouterInfo, Transport>(
          std::move(pdstn),
          poolName,
          indexInPool,
          poolStatIndex,
          timeout,
          disableRequestDeadlineCheck,
          keepRoutingPrefix),
      std::move(resAp)};
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McRouteHandleProvider<RouterInfo>::createSRRoute(
    RouteHandleFactory<typename RouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json,
    const RouteHandleFactoryFuncWithProxy& factoryFunc) {
  checkLogic(json.isObject(), "SRRoute should be object");
  auto route = factoryFunc(factory, json, proxy_);
  // Track the SRRoute created so that we can save it to SRRoute map later
  auto srRoute = route;

  if (auto maxOutstandingJson = json.get_ptr("max_outstanding")) {
    auto v = parseInt(
        *maxOutstandingJson,
        "max_outstanding",
        0,
        std::numeric_limits<int64_t>::max());
    if (v) {
      route = makeOutstandingLimitRoute<RouterInfo>(std::move(route), v);
    }
  }

  if (!(proxy_.router().opts().disable_shard_split_route)) {
    if (auto jsplits = json.get_ptr("shard_splits")) {
      route = makeShardSplitRoute<RouterInfo>(
          std::move(route), ShardSplitter(*jsplits));
    }
  }

  bool needAsynclog = true;
  if (auto* jNeedAsynclog = json.get_ptr("asynclog")) {
    needAsynclog = parseBool(*jNeedAsynclog, "asynclog");
  }
  if (needAsynclog) {
    folly::StringPiece asynclogName;
    if (auto jAsynclogName = json.get_ptr("asynclog_name")) {
      asynclogName = parseString(*jAsynclogName, "asynclog_name");
    } else if (auto jServiceName = json.get_ptr("service_name")) {
      asynclogName = parseString(*jServiceName, "service_name");
    } else {
      throwLogic(
          "AsynclogRoute over SRRoute: 'service_name' property is missing");
    }
    route = createAsynclogRoute(std::move(route), asynclogName.toString());
  }

  route = wrapAxonLogRoute(std::move(route), proxy_, json);

  if (json.count("shadows")) {
    route = std::move(makeShadowRoutes(
        factory, json, {std::move(route)}, proxy_, *extraProvider_)[0]);
  }
  route = bucketize(std::move(route), json);

  if (auto jSRRouteName = json.get_ptr("service_name")) {
    auto srRouteName = parseString(*jSRRouteName, "service_name");
    tierRoutes_.emplace(srRouteName, srRoute);
  }

  return route;
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McRouteHandleProvider<RouterInfo>::bucketize(
    std::shared_ptr<typename RouterInfo::RouteHandleIf> route,
    const folly::dynamic& json) {
  bool bucketize = false;
  if (auto* jNeedBucketization = json.get_ptr("bucketize")) {
    bucketize = parseBool(*jNeedBucketization, "bucketize");
  }
  if (bucketize) {
    return makeMcBucketRoute<RouterInfo>(std::move(route), json);
  }
  return route;
}

template <class RouterInfo>
std::shared_ptr<typename RouterInfo::RouteHandleIf>
McRouteHandleProvider<RouterInfo>::makePoolRoute(
    RouteHandleFactory<RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(
      json.isObject() || json.isString(),
      "PoolRoute should be object or string");
  const folly::dynamic* jpool;
  if (json.isObject()) {
    jpool = json.get_ptr("pool");
    checkLogic(jpool, "PoolRoute: pool not found");
  } else { // string
    jpool = &json;
  }

  auto poolJson = poolFactory_.parsePool(*jpool);
  auto [destinations, weights] = makePool(factory, poolJson);

  try {
    destinations = wrapPoolDestinations<RouterInfo>(
        factory,
        std::move(destinations),
        poolJson.name,
        json,
        proxy_,
        *extraProvider_);

    // add weights and override whatever we have in PoolRoute::hash
    folly::dynamic jhashWithWeights = folly::dynamic::object();
    if (weights) {
      jhashWithWeights = folly::dynamic::object(
          "hash_func", WeightedCh3HashFunc::type())("weights", *weights);
    }

    if (auto jTags = poolJson.json.get_ptr("tags")) {
      jhashWithWeights["tags"] = *jTags;
    }

    if (json.isObject()) {
      if (auto jhash = json.get_ptr("hash")) {
        checkLogic(
            jhash->isObject() || jhash->isString(),
            "hash is not object/string");
        if (jhash->isString()) {
          jhashWithWeights["hash_func"] = *jhash;
        } else { // object
          for (const auto& it : jhash->items()) {
            jhashWithWeights[it.first] = it.second;
          }
        }
      }
      if (auto* jNeedBucketization = json.get_ptr("bucketize")) {
        if (parseBool(*jNeedBucketization, "bucketize")) {
          jhashWithWeights["bucketize"] = true;
        }
      }
      // When setting useBucketHashSelector, the PoolRoute is constructed with
      // BucketHashSelector but skip init McBucketRoute as direct parent. This
      // allow shadow PoolRoute to use the McBucketRoute from the associated
      // normal route.
      if (auto* jNeedBucketizationShadow =
              json.get_ptr("useBucketHashSelector")) {
        if (parseBool(*jNeedBucketizationShadow, "useBucketHashSelector")) {
          jhashWithWeights["bucketize"] = true;
        }
      }
    }

    if (poolJson.json.isObject()) {
      if (auto* jErrorOnEmpty = poolJson.json.get_ptr("error_on_empty")) {
        jhashWithWeights["error_on_empty"] =
            parseBool(*jErrorOnEmpty, "error_on_empty");
      }
    }
    jhashWithWeights["name"] = poolJson.name;

    auto route = createHashRoute<RouterInfo>(
        jhashWithWeights, std::move(destinations), factory.getThreadId());
    auto poolRoute = route;

    auto asynclogName = poolJson.name;
    bool needAsynclog = true;
    if (json.isObject()) {
      if (auto jrates = json.get_ptr("rates")) {
        route = createRateLimitRoute(std::move(route), RateLimiter(*jrates));
      }
      if (!(proxy_.router().opts().disable_shard_split_route)) {
        if (auto jsplits = json.get_ptr("shard_splits")) {
          route = makeShardSplitRoute<RouterInfo>(
              std::move(route), ShardSplitter(*jsplits));
        }
      }
      if (auto jasynclog = json.get_ptr("asynclog")) {
        needAsynclog = parseBool(*jasynclog, "asynclog");
      }
      if (auto jasynclogName = json.get_ptr("asynclog_name")) {
        asynclogName = parseString(*jasynclogName, "asynclog_name");
      } else if (auto jname = json.get_ptr("name")) {
        asynclogName = parseString(*jname, "name");
      }
    }
    if (needAsynclog) {
      route = createAsynclogRoute(std::move(route), asynclogName.str());
    }
    if (json.isObject()) {
      // Wrap AxonLogRoute if configured
      route = wrapAxonLogRoute(std::move(route), proxy_, json);
      route = bucketize(std::move(route), json);

      if (auto jPoolId = json.get_ptr("pool_id")) {
        auto poolId = parseString(*jPoolId, "pool_id");
        tierRoutes_.emplace(poolId, poolRoute);
      }
    }
    return route;
  } catch (const std::exception& e) {
    throwLogic("PoolRoute {}: {}", poolJson.name, e.what());
  }
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMap
McRouteHandleProvider<RouterInfo>::buildRouteMap() {
  return RouterInfo::buildRouteMap();
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapWithProxy
McRouteHandleProvider<RouterInfo>::buildRouteMapWithProxy() {
  return RouterInfo::buildRouteMapWithProxy();
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapForWrapper
McRouteHandleProvider<RouterInfo>::buildRouteMapForWrapper() {
  return RouterInfo::buildRouteMapForWrapper();
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMap
McRouteHandleProvider<RouterInfo>::buildCheckedRouteMap() {
  typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMap
      checkedRouteMap;

  // Wrap all factory functions with a nullptr check. Note that there are still
  // other code paths that could lead to a nullptr being returned from a
  // route handle factory function, e.g., in makeShadow() and makeFailover()
  // extra provider functions. So those code paths must be checked by other
  // means.
  for (auto& it : buildRouteMap()) {
    checkedRouteMap.emplace(
        it.first,
        [factoryFunc = std::move(it.second), rhName = it.first](
            RouteHandleFactory<RouteHandleIf>& factory,
            const folly::dynamic& json) {
          try {
            auto rh = factoryFunc(factory, json);
            checkLogic(rh != nullptr, "make{} returned nullptr", rhName);
            return rh;
          } catch (const std::exception& e) {
            throw std::logic_error(folly::sformat(
                "make{} throws when contructing: {}", rhName, e.what()));
          }
        });
  }

  return checkedRouteMap;
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapWithProxy
McRouteHandleProvider<RouterInfo>::buildCheckedRouteMapWithProxy() {
  typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapWithProxy
      checkedRouteMapWithProxy;

  // Wrap all factory functions with a nullptr check. Note that there are still
  // other code paths that could lead to a nullptr being returned from a
  // route handle factory function, e.g., in makeShadow() and makeFailover()
  // extra provider functions. So those code paths must be checked by other
  // means.
  for (auto& it : buildRouteMapWithProxy()) {
    checkedRouteMapWithProxy.emplace(
        it.first,
        [factoryFunc = std::move(it.second), rhName = it.first](
            RouteHandleFactory<RouteHandleIf>& factory,
            const folly::dynamic& json,
            ProxyBase& proxy) {
          try {
            auto rh = factoryFunc(factory, json, proxy);
            checkLogic(rh != nullptr, "make{} returned nullptr", rhName);
            return rh;
          } catch (const std::exception& e) {
            throw std::logic_error(folly::sformat(
                "make{} throws when contructing: {}", rhName, e.what()));
          }
        });
  }

  return checkedRouteMapWithProxy;
}

template <class RouterInfo>
typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapForWrapper
McRouteHandleProvider<RouterInfo>::buildCheckedRouteMapForWrapper() {
  typename McRouteHandleProvider<RouterInfo>::RouteHandleFactoryMapForWrapper
      checkedRouteMapForWrapper;

  // Wrap all factory functions with a nullptr check. Note that there are still
  // other code paths that could lead to a nullptr being returned from a
  // route handle factory function, e.g., in makeShadow() and makeFailover()
  // extra provider functions. So those code paths must be checked by other
  // means.
  for (auto& it : buildRouteMapForWrapper()) {
    checkedRouteMapForWrapper.emplace(
        it.first,
        [factoryFunc = std::move(it.second), rhName = it.first](
            RouteHandlePtr rh, ProxyBase& proxy, const folly::dynamic& json) {
          try {
            auto ret = factoryFunc(std::move(rh), proxy, json);
            checkLogic(ret != nullptr, "make{} returned nullptr", rhName);
            return ret;
          } catch (const std::exception& e) {
            throw std::logic_error(folly::sformat(
                "make{} throws when contructing: {}", rhName, e.what()));
          }
        });
  }

  return checkedRouteMapForWrapper;
}

// TODO(@aap): Remove this override as soon as all route handles are migrated
template <>
typename McRouteHandleProvider<MemcacheRouterInfo>::RouteHandleFactoryMap
McRouteHandleProvider<MemcacheRouterInfo>::buildRouteMap();

template <>
typename McRouteHandleProvider<
    MemcacheRouterInfo>::RouteHandleFactoryMapWithProxy
McRouteHandleProvider<MemcacheRouterInfo>::buildRouteMapWithProxy();

template <>
typename McRouteHandleProvider<
    MemcacheRouterInfo>::RouteHandleFactoryMapForWrapper
McRouteHandleProvider<MemcacheRouterInfo>::buildRouteMapForWrapper();

template <class RouterInfo>
std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>
McRouteHandleProvider<RouterInfo>::create(
    RouteHandleFactory<RouteHandleIf>& factory,
    folly::StringPiece type,
    const folly::dynamic& json) {
  if (type == "Pool") {
    return std::get<0>(makePool(factory, poolFactory_.parsePool(json)));
  } else if (type == "ShadowRoute") {
    return makeShadowRoutes(factory, json, proxy_, *extraProvider_);
  } else if (type == "SaltedFailoverRoute") {
    auto jPool = json.get_ptr("pool");
    // Create two children with first one for Normal Route and the second
    // one for failover route. The Normal route would be Pool Route with
    // Pool Name and Hash object shared with Failover Route. So insert
    // pool name and hash object into the Normal Route Json.
    folly::dynamic newJson = json;
    folly::dynamic children = folly::dynamic::array;
    folly::dynamic normalRoute = folly::dynamic::object;
    normalRoute.insert("type", "PoolRoute");
    if (jPool->isString()) {
      normalRoute.insert("pool", jPool->asString());
    } else if (jPool->isObject()) {
      normalRoute.insert("pool", *jPool);
    } else {
      throwLogic("pool needs to be either a string or an object");
    }
    auto jShadows = json.get_ptr("shadows");
    if (jShadows) {
      checkLogic(jShadows->isArray(), "shadows must be an array.");
      normalRoute.insert("shadows", *jShadows);
    }
    auto jShadowPolicy = json.get_ptr("shadow_policy");
    if (jShadowPolicy) {
      checkLogic(jShadowPolicy->isString(), "shadow_policy must be a string.");
      normalRoute.insert("shadow_policy", jShadowPolicy->asString());
    }
    if (auto jHash = json.get_ptr("hash")) {
      normalRoute.insert("hash", *jHash);
    }
    children.push_back(normalRoute);
    if (jPool->isString()) {
      children.push_back("Pool|" + jPool->asString());
    } else if (jPool->isObject()) {
      children.push_back(*jPool);
    } else {
      throwLogic("pool needs to be either a string or an object");
    }
    newJson.erase("children");
    newJson.insert("children", children);
    return {makeFailoverRoute(factory, newJson, *extraProvider_)};
  } else if (type == "FailoverRoute") {
    return {makeFailoverRoute(factory, json, *extraProvider_)};
  } else if (type == "PoolRoute") {
    return {makePoolRoute(factory, json)};
  } else if (type == "SRRoute") {
    auto it = routeMapWithProxy_.find(type);
    checkLogic(
        it != routeMapWithProxy_.end(),
        "SRRoute is not implemented for this router");
    return {createSRRoute(factory, json, it->second)};
  }

  auto it = routeMap_.find(type);
  if (it != routeMap_.end()) {
    return {it->second(factory, json)};
  }

  // Route handles that take ProxyBase as an argument
  auto itr = routeMapWithProxy_.find(type);
  if (itr != routeMapWithProxy_.end()) {
    return {itr->second(factory, json, proxy_)};
  }

  /* returns empty vector if type is unknown */
  auto ret = extraProvider_->tryCreate(factory, type, json);
  if (!ret.empty()) {
    return ret;
  }

  if (json.empty()) {
    throwLogic("Unknown type or Missing Name Handle : {}", type.str());
  }
  const auto& configMetadataMap = poolFactory_.getConfigMetadataMap();
  auto jType = json.get_ptr("type");
  auto typeMetadata = configMetadataMap.find(jType);
  if (typeMetadata != configMetadataMap.end()) {
    // The line numbers returned by the folly API are 0-based. Make them
    // 1-based.
    auto line = typeMetadata->second.value_range.begin.line + 1;
    throwLogic("Unknown RouteHandle: {} line: {}", type, line);
  } else {
    throwLogic("Unknown RouteHandle: {}", type);
  }
}

template <class RouterInfo>
const folly::dynamic& McRouteHandleProvider<RouterInfo>::parsePool(
    const folly::dynamic& json) {
  return poolFactory_.parsePool(json).json;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
