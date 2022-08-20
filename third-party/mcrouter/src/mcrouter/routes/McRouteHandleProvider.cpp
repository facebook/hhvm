/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McRouteHandleProvider.h"

#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/routes/AllAsyncRouteFactory.h"
#include "mcrouter/routes/AllFastestRouteFactory.h"
#include "mcrouter/routes/AllInitialRouteFactory.h"
#include "mcrouter/routes/AllMajorityRouteFactory.h"
#include "mcrouter/routes/AllSyncRouteFactory.h"
#include "mcrouter/routes/BlackholeRoute.h"
#include "mcrouter/routes/CarbonLookasideRoute.h"
#include "mcrouter/routes/DevNullRoute.h"
#include "mcrouter/routes/ErrorRoute.h"
#include "mcrouter/routes/FailoverRoute.h"
#include "mcrouter/routes/FailoverWithExptimeRouteFactory.h"
#include "mcrouter/routes/HashRouteFactory.h"
#include "mcrouter/routes/HostIdRouteFactory.h"
#include "mcrouter/routes/KeySplitRoute.h"
#include "mcrouter/routes/L1L2CacheRouteFactory.h"
#include "mcrouter/routes/L1L2SizeSplitRoute.h"
#include "mcrouter/routes/LatencyInjectionRoute.h"
#include "mcrouter/routes/LatestRoute.h"
#include "mcrouter/routes/LoadBalancerRoute.h"
#include "mcrouter/routes/LoggingRoute.h"
#include "mcrouter/routes/McBucketRoute.h"
#include "mcrouter/routes/McExtraRouteHandleProvider.h"
#include "mcrouter/routes/MigrateRouteFactory.h"
#include "mcrouter/routes/MissFailoverRoute.h"
#include "mcrouter/routes/ModifyExptimeRoute.h"
#include "mcrouter/routes/ModifyKeyRoute.h"
#include "mcrouter/routes/OperationSelectorRoute.h"
#include "mcrouter/routes/OriginalClientHashRoute.h"
#include "mcrouter/routes/OutstandingLimitRoute.h"
#include "mcrouter/routes/RandomRouteFactory.h"
#include "mcrouter/routes/RoutingGroupRoute.h"
#include "mcrouter/routes/ShadowRoute.h"
#include "mcrouter/routes/StagingRoute.h"

namespace folly {
struct dynamic;
}

namespace facebook {
namespace memcache {
namespace mcrouter {

const folly::dynamic& getConfigJsonFromCommonAccessPointAttributes(
    const std::shared_ptr<CommonAccessPointAttributes>& apAttr) {
  return apAttr->json;
}

std::shared_ptr<CommonAccessPointAttributes> getCommonAccessPointAttributes(
    const folly::dynamic& json,
    CarbonRouterInstanceBase& router) {
  auto ret = std::make_shared<CommonAccessPointAttributes>();
  CommonAccessPointAttributes& apAttr = *ret;
  if (auto jName = json.get_ptr("name")) {
    apAttr.poolName = jName->stringPiece();
  }
  auto& protocol = apAttr.protocol;
  protocol = mc_ascii_protocol;
  if (auto jProtocol = json.get_ptr("protocol")) {
    auto str = parseString(*jProtocol, "protocol");
    if (equalStr("ascii", str, folly::AsciiCaseInsensitive())) {
      protocol = mc_ascii_protocol;
    } else if (equalStr("caret", str, folly::AsciiCaseInsensitive())) {
      protocol = mc_caret_protocol;
    } else if (equalStr("thrift", str, folly::AsciiCaseInsensitive())) {
      protocol = mc_thrift_protocol;
    } else {
      throwLogic("Unknown protocol '{}'", str);
    }
  }

  auto& enableCompression = apAttr.enableCompression;
  enableCompression = router.opts().enable_compression;
  if (auto jCompression = json.get_ptr("enable_compression")) {
    enableCompression = parseBool(*jCompression, "enable_compression");
  }

  auto& mech = apAttr.mech;
  mech = SecurityMech::NONE;
  auto& mechOverride = apAttr.mechOverride;
  auto& withinDcMech = apAttr.withinDcMech;
  auto& crossDcMech = apAttr.crossDcMech;
  auto& crossDcPort = apAttr.crossDcPort;
  auto& withinDcPort = apAttr.withinDcPort;
  auto& port = apAttr.port;
  // default to 0, which doesn't override
  port = 0;
  if (router.configApi().enableSecurityConfig()) {
    if (auto jSecurityMech = json.get_ptr("security_mech_within_dc")) {
      auto mechStr = parseString(*jSecurityMech, "security_mech_within_dc");
      withinDcMech = parseSecurityMech(mechStr);
    }

    if (auto jSecurityMech = json.get_ptr("security_mech_cross_dc")) {
      auto mechStr = parseString(*jSecurityMech, "security_mech_cross_dc");
      crossDcMech = parseSecurityMech(mechStr);
    }

    if (withinDcMech.has_value() && crossDcMech.has_value() &&
        withinDcMech.value() == crossDcMech.value()) {
      // mech is used if nothing is specified in server ap
      mech = withinDcMech.value();
      // mechOverride overrides per-server values
      mechOverride = withinDcMech.value();
      withinDcMech.reset();
      crossDcMech.reset();
    } else {
      if (auto jSecurityMech = json.get_ptr("security_mech")) {
        auto mechStr = parseString(*jSecurityMech, "security_mech");
        mech = parseSecurityMech(mechStr);
      } else if (auto jUseSsl = json.get_ptr("use_ssl")) {
        // deprecated - prefer security_mech
        auto useSsl = parseBool(*jUseSsl, "use_ssl");
        if (useSsl) {
          mech = SecurityMech::TLS;
        }
      }
    }

    if (auto jPort = json.get_ptr("port_override_within_dc")) {
      withinDcPort = parseInt(*jPort, "port_override_within_dc", 1, 65535);
    }

    if (auto jPort = json.get_ptr("port_override_cross_dc")) {
      crossDcPort = parseInt(*jPort, "port_override_cross_dc", 1, 65535);
    }

    if (withinDcPort.has_value() && crossDcPort.has_value() &&
        withinDcPort.value() == crossDcPort.value()) {
      port = withinDcPort.value();
      withinDcPort.reset();
      crossDcPort.reset();
    } else {
      // parse port override only if withinDc & crossDc are not present
      if (auto jPort = json.get_ptr("port_override")) {
        port = parseInt(*jPort, "port_override", 1, 65535);
      }
    }
  }
  return ret;
}

std::shared_ptr<AccessPoint> createAccessPoint(
    folly::StringPiece apString,
    uint32_t failureDomain,
    CarbonRouterInstanceBase& router,
    const CommonAccessPointAttributes& apAttr) {
  auto& protocol = apAttr.protocol;
  auto& mech = apAttr.mech;
  auto& mechOverride = apAttr.mechOverride;
  auto& withinDcMech = apAttr.withinDcMech;
  auto& crossDcMech = apAttr.crossDcMech;
  auto& crossDcPort = apAttr.crossDcPort;
  auto& withinDcPort = apAttr.withinDcPort;
  auto& port = apAttr.port;
  auto& enableCompression = apAttr.enableCompression;

  auto ap = AccessPoint::create(
      apString, protocol, mech, port, enableCompression, failureDomain);
  checkLogic(ap != nullptr, "invalid server {}", apString);

  if (mechOverride.has_value()) {
    ap->setSecurityMech(mechOverride.value());
  }

  if (withinDcMech.has_value() || crossDcMech.has_value() ||
      withinDcPort.has_value() || crossDcPort.has_value()) {
    bool isInLocalDc = isInLocalDatacenter(ap->getHost());
    if (isInLocalDc) {
      if (withinDcMech.has_value()) {
        ap->setSecurityMech(withinDcMech.value());
      }
      if (withinDcPort.has_value()) {
        ap->setPort(withinDcPort.value());
      }
    } else {
      if (crossDcMech.has_value()) {
        ap->setSecurityMech(crossDcMech.value());
      }
      if (crossDcPort.has_value()) {
        ap->setPort(crossDcPort.value());
      }
    }
  }

  if (ap->compressed() && router.getCodecManager() == nullptr) {
    if (!initCompression(router)) {
      MC_LOG_FAILURE(
          router.opts(),
          failure::Category::kBadEnvironment,
          "Pool {}: Failed to initialize compression. "
          "Disabling compression for host: {}",
          apAttr.poolName,
          apString);
      ap->disableCompression();
    }
  }

  return ap;
}

using McRouteHandleFactory = RouteHandleFactory<McrouterRouteHandleIf>;
using MemcacheRouterInfo = facebook::memcache::MemcacheRouterInfo;

class MemcacheCarbonLookasideHelper;

// This is rather expensive to instantiate, therefore explicitly instantiated
// in separate file: `McrouteHandleProvider-CarbonLookasideRoute.cpp`.
extern template MemcacheRouterInfo::RouteHandlePtr
createCarbonLookasideRoute<MemcacheRouterInfo, MemcacheCarbonLookasideHelper>(
    RouteHandleFactory<MemcacheRouteHandleIf>& factory,
    const folly::dynamic& json);

McrouterRouteHandlePtr makeWarmUpRoute(
    McRouteHandleFactory& factory,
    const folly::dynamic& json);

MemcacheRouterInfo::RouteHandlePtr wrapAxonLogRoute(
    MemcacheRouterInfo::RouteHandlePtr route,
    ProxyBase& proxy,
    const folly::dynamic& json) {
  bool needAxonlog = false;
  if (auto* jNeedAxonlog = json.get_ptr("axonlog")) {
    needAxonlog = parseBool(*jNeedAxonlog, "axonlog");
  }
  if (needAxonlog) {
    checkLogic(
        makeAxonLogRoute, "AxonLogRoute is not implemented for this router");
    folly::StringPiece axonlogTier;
    int64_t axonlogBaseId;
    bool needAxonAllDelete{false};
    int64_t maxTask{-1};
    if (auto* jAxonLogTier = json.get_ptr("axonlog_tier")) {
      axonlogTier = parseString(*jAxonLogTier, "axonlog_tier");
    } else {
      throwLogic(
          "AxonLogRoute over {}: 'axonlog_tier' property is missing",
          route->routeName());
    }
    if (auto* jAxonlogBaseId = json.get_ptr("axonlog_base_id")) {
      axonlogBaseId = parseInt(
          *jAxonlogBaseId,
          "axonlog_base_id",
          1,
          std::numeric_limits<int64_t>::max());
    } else {
      throwLogic(
          "AxonLogRoute over {}: 'axonlog_base_id' property is missing",
          route->routeName());
    }
    if (auto* jNeedAxonAllDelete = json.get_ptr("axonlog_all_delete")) {
      needAxonAllDelete = parseBool(*jNeedAxonAllDelete, "axonlog_all_delete");
    }
    if (auto* jMaxAxonTask = json.get_ptr("axonlog_max_task")) {
      maxTask = parseInt(
          *jMaxAxonTask,
          "axonlog_max_task",
          0,
          std::numeric_limits<int64_t>::max());
    }
    return makeAxonLogRoute(
        std::move(route),
        proxy,
        axonlogTier,
        axonlogBaseId,
        needAxonAllDelete,
        maxTask);
  }
  return route;
}

MemcacheRouterInfo::RouteHandlePtr bucketize(
    MemcacheRouterInfo::RouteHandlePtr route,
    const folly::dynamic& json) {
  bool bucketize = false;
  if (auto* jNeedBucketization = json.get_ptr("bucketize")) {
    bucketize = parseBool(*jNeedBucketization, "bucketize");
  }
  if (bucketize) {
    return makeMcBucketRoute(std::move(route), json);
  }
  return route;
}

template <>
std::unique_ptr<ExtraRouteHandleProviderIf<MemcacheRouterInfo>>
McRouteHandleProvider<MemcacheRouterInfo>::buildExtraProvider() {
  return std::make_unique<McExtraRouteHandleProvider<MemcacheRouterInfo>>();
}

template <>
std::shared_ptr<MemcacheRouterInfo::RouteHandleIf>
McRouteHandleProvider<MemcacheRouterInfo>::createSRRoute(
    RouteHandleFactory<MemcacheRouterInfo::RouteHandleIf>& factory,
    const folly::dynamic& json) {
  checkLogic(makeSRRoute, "SRRoute is not implemented for this router");
  checkLogic(json.isObject(), "SRRoute should be object");
  auto route = makeSRRoute(factory, json, proxy_);
  // Track the SRRoute created so that we can save it to SRRoute map later
  auto srRoute = route;

  if (auto maxOutstandingJson = json.get_ptr("max_outstanding")) {
    auto v = parseInt(
        *maxOutstandingJson,
        "max_outstanding",
        0,
        std::numeric_limits<int64_t>::max());
    if (v) {
      route =
          makeOutstandingLimitRoute<MemcacheRouterInfo>(std::move(route), v);
    }
  }

  if (!(proxy_.router().opts().disable_shard_split_route)) {
    if (auto jsplits = json.get_ptr("shard_splits")) {
      route = makeShardSplitRoute<MemcacheRouterInfo>(
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
  route = bucketize(std::move(route), json);

  if (auto jSRRouteName = json.get_ptr("service_name")) {
    auto srRouteName = parseString(*jSRRouteName, "service_name");
    srRoutes_.emplace(srRouteName, srRoute);
  }

  return route;
}

template <>
typename McRouteHandleProvider<MemcacheRouterInfo>::RouteHandleFactoryMap
McRouteHandleProvider<MemcacheRouterInfo>::buildRouteMap() {
  RouteHandleFactoryMap map{
      {"AllAsyncRoute", &makeAllAsyncRoute<MemcacheRouterInfo>},
      {"AllFastestRoute", &makeAllFastestRoute<MemcacheRouterInfo>},
      {"AllInitialRoute", &makeAllInitialRoute<MemcacheRouterInfo>},
      {"AllMajorityRoute", &makeAllMajorityRoute<MemcacheRouterInfo>},
      {"AllSyncRoute", &makeAllSyncRoute<MemcacheRouterInfo>},
      {"BlackholeRoute", &makeBlackholeRoute<MemcacheRouterInfo>},
      {"CarbonLookasideRoute",
       &createCarbonLookasideRoute<
           MemcacheRouterInfo,
           MemcacheCarbonLookasideHelper>},
      {"DevNullRoute", &makeDevNullRoute<MemcacheRouterInfo>},
      {"ErrorRoute", &makeErrorRoute<MemcacheRouterInfo>},
      {"FailoverWithExptimeRoute",
       &makeFailoverWithExptimeRoute<MemcacheRouterInfo>},
      {"HashRoute",
       [](McRouteHandleFactory& factory, const folly::dynamic& json) {
         return makeHashRoute<McrouterRouterInfo>(factory, json);
       }},
      {"HostIdRoute", &makeHostIdRoute<MemcacheRouterInfo>},
      {"LatencyInjectionRoute", &makeLatencyInjectionRoute<MemcacheRouterInfo>},
      {"L1L2CacheRoute", &makeL1L2CacheRoute<MemcacheRouterInfo>},
      {"L1L2SizeSplitRoute", &makeL1L2SizeSplitRoute},
      {"KeySplitRoute", &makeKeySplitRoute},
      {"LatestRoute", &makeLatestRoute<MemcacheRouterInfo>},
      {"LoadBalancerRoute", &makeLoadBalancerRoute<MemcacheRouterInfo>},
      {"LoggingRoute", &makeLoggingRoute<MemcacheRouterInfo>},
      {"MigrateRoute", &makeMigrateRoute<MemcacheRouterInfo>},
      {"MissFailoverRoute", &makeMissFailoverRoute<MemcacheRouterInfo>},
      {"ModifyKeyRoute", &makeModifyKeyRoute<MemcacheRouterInfo>},
      {"ModifyExptimeRoute", &makeModifyExptimeRoute<MemcacheRouterInfo>},
      {"NullRoute", &makeNullRoute<MemcacheRouteHandleIf>},
      {"OperationSelectorRoute",
       &makeOperationSelectorRoute<MemcacheRouterInfo>},
      {"OriginalClientHashRoute",
       &makeOriginalClientHashRoute<MemcacheRouterInfo>},
      {"PoolRoute",
       [this](McRouteHandleFactory& factory, const folly::dynamic& json) {
         return makePoolRoute(factory, json);
       }},
      {"PrefixPolicyRoute", &makeOperationSelectorRoute<MemcacheRouterInfo>},
      {"RandomRoute", &makeRandomRoute<MemcacheRouterInfo>},
      {"RateLimitRoute",
       [](McRouteHandleFactory& factory, const folly::dynamic& json) {
         return makeRateLimitRoute(factory, json);
       }},
      {"RoutingGroupRoute", &makeRoutingGroupRoute<MemcacheRouterInfo>},
      {"StagingRoute", &makeStagingRoute},
      {"SRRoute",
       [this](McRouteHandleFactory& factory, const folly::dynamic& json) {
         return createSRRoute(factory, json);
       }},
      {"WarmUpRoute", &makeWarmUpRoute},
  };
  return map;
}

template <>
typename McRouteHandleProvider<
    MemcacheRouterInfo>::RouteHandleFactoryMapWithProxy
McRouteHandleProvider<MemcacheRouterInfo>::buildRouteMapWithProxy() {
  return RouteHandleFactoryMapWithProxy();
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
