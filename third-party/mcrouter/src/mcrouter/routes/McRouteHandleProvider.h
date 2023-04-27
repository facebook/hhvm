/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <folly/Range.h>
#include <folly/json.h>

#include "mcrouter/PoolFactory.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/TkoTracker.h"
#include "mcrouter/lib/config/RouteHandleProviderIf.h"
#include "mcrouter/lib/network/SecurityOptions.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

FOLLY_ATTR_WEAK MemcacheRouterInfo::RouteHandlePtr makeSRRoute(
    RouteHandleFactory<MemcacheRouterInfo::RouteHandleIf>&,
    const folly::dynamic& json,
    ProxyBase& proxy);

FOLLY_ATTR_WEAK MemcacheRouterInfo::RouteHandlePtr makeAxonLogRoute(
    MemcacheRouterInfo::RouteHandlePtr rh,
    ProxyBase& proxy,
    const folly::dynamic& json);

template <class RouteHandleIf>
class ExtraRouteHandleProviderIf;
class ProxyBase;

struct CommonAccessPointAttributes {
  folly::dynamic json;
  folly::StringPiece poolName;
  mc_protocol_t protocol;
  SecurityMech mech = SecurityMech::NONE;
  folly::Optional<SecurityMech> mechOverride;
  folly::Optional<SecurityMech> withinDcMech;
  folly::Optional<SecurityMech> crossDcMech;
  folly::Optional<uint16_t> crossDcPort;
  folly::Optional<uint16_t> withinDcPort;
  std::optional<folly::StringPiece> serviceIdOverride;
  uint16_t port;
  bool enableCompression;

  bool operator==(const CommonAccessPointAttributes& other) const {
    return protocol == other.protocol && mech == other.mech &&
        mechOverride == other.mechOverride &&
        withinDcMech == other.withinDcMech &&
        crossDcMech == other.crossDcMech && crossDcPort == other.crossDcPort &&
        withinDcPort == other.withinDcPort && port == other.port &&
        enableCompression == other.enableCompression &&
        serviceIdOverride == other.serviceIdOverride;
  }
};

const folly::dynamic& getConfigJsonFromCommonAccessPointAttributes(
    const std::shared_ptr<CommonAccessPointAttributes>& apAttr);

std::shared_ptr<CommonAccessPointAttributes> getCommonAccessPointAttributes(
    const folly::dynamic& json,
    CarbonRouterInstanceBase& router);

std::shared_ptr<AccessPoint> createAccessPoint(
    folly::StringPiece apString,
    uint32_t failureDomain,
    CarbonRouterInstanceBase& router,
    const CommonAccessPointAttributes& apAttr);

/**
 * RouteHandleProviderIf implementation that can create mcrouter-specific
 * routes.
 */
template <class RouterInfo>
class McRouteHandleProvider
    : public RouteHandleProviderIf<typename RouterInfo::RouteHandleIf> {
 public:
  using RouteHandleIf = typename RouterInfo::RouteHandleIf;
  using RouteHandlePtr = std::shared_ptr<RouteHandleIf>;
  using RouteHandleFactoryFunc = std::function<RouteHandlePtr(
      RouteHandleFactory<RouteHandleIf>&,
      const folly::dynamic&)>;
  using RouteHandleFactoryMap = std::
      unordered_map<folly::StringPiece, RouteHandleFactoryFunc, folly::Hash>;
  using RouteHandleFactoryFuncWithProxy = std::function<RouteHandlePtr(
      RouteHandleFactory<RouteHandleIf>&,
      const folly::dynamic&,
      ProxyBase&)>;
  using RouteHandleFactoryMapWithProxy = std::unordered_map<
      folly::StringPiece,
      RouteHandleFactoryFuncWithProxy,
      folly::Hash>;
  using RouteHandleFactoryFuncForWrapper = std::function<
      RouteHandlePtr(RouteHandlePtr, ProxyBase&, const folly::dynamic&)>;
  using RouteHandleFactoryMapForWrapper = std::unordered_map<
      folly::StringPiece,
      RouteHandleFactoryFuncForWrapper,
      folly::Hash>;

  McRouteHandleProvider(ProxyBase& proxy, PoolFactory& poolFactory);

  std::vector<RouteHandlePtr> create(
      RouteHandleFactory<RouteHandleIf>& factory,
      folly::StringPiece type,
      const folly::dynamic& json) final;

  const folly::dynamic& parsePool(const folly::dynamic& json) final;

  folly::StringKeyedUnorderedMap<RouteHandlePtr> releaseAsyncLogRoutes() {
    return std::move(asyncLogRoutes_);
  }

  folly::StringKeyedUnorderedMap<RouteHandlePtr> releaseSRRoutes() {
    return std::move(srRoutes_);
  }

  folly::StringKeyedUnorderedMap<std::vector<RouteHandlePtr>> releasePools() {
    return std::move(pools_);
  }

  folly::StringKeyedUnorderedMap<std::pair<
      bool,
      std::vector<std::pair<
          std::shared_ptr<CommonAccessPointAttributes>,
          std::vector<std::string>>>>>
  releasePartialConfigs() {
    return std::move(partialConfigs_);
  }
  folly::StringKeyedUnorderedMap<
      std::unordered_set<std::shared_ptr<const AccessPoint>>>
  releaseAccessPoints() {
    return std::move(accessPoints_);
  }

  ~McRouteHandleProvider() override;

 private:
  ProxyBase& proxy_;
  PoolFactory& poolFactory_;
  std::unique_ptr<ExtraRouteHandleProviderIf<RouterInfo>> extraProvider_;

  // poolName -> AsynclogRoute
  folly::StringKeyedUnorderedMap<RouteHandlePtr> asyncLogRoutes_;

  // poolName -> SRRoute
  folly::StringKeyedUnorderedMap<RouteHandlePtr> srRoutes_;

  // pool source name -> (allow_partial_reconfig, [(pool_config,[pool_names])])
  folly::StringKeyedUnorderedMap<std::pair<
      bool,
      std::vector<std::pair<
          std::shared_ptr<CommonAccessPointAttributes>,
          std::vector<std::string>>>>>
      partialConfigs_;

  // poolName -> destinations
  folly::StringKeyedUnorderedMap<std::vector<RouteHandlePtr>> pools_;

  // poolName -> AccessPoints
  folly::StringKeyedUnorderedMap<
      std::unordered_set<std::shared_ptr<const AccessPoint>>>
      accessPoints_;

  const RouteHandleFactoryMap routeMap_;
  const RouteHandleFactoryMapWithProxy routeMapWithProxy_;
  const RouteHandleFactoryMapForWrapper routeMapForWrapper_;

  const std::vector<RouteHandlePtr>& makePool(
      RouteHandleFactory<RouteHandleIf>& factory,
      const PoolFactory::PoolJson& json);

  RouteHandlePtr makePoolRoute(
      RouteHandleFactory<RouteHandleIf>& factory,
      const folly::dynamic& json);

  RouteHandlePtr createSRRoute(
      RouteHandleFactory<RouteHandleIf>& factory,
      const folly::dynamic& json,
      const RouteHandleFactoryFuncWithProxy& factoryFunc);

  RouteHandlePtr createAsynclogRoute(
      RouteHandlePtr route,
      std::string asynclogName);

  RouteHandlePtr bucketize(RouteHandlePtr route, const folly::dynamic& json);

  RouteHandlePtr wrapAxonLogRoute(
      RouteHandlePtr route,
      ProxyBase& proxy,
      const folly::dynamic& json);

  template <class Transport>
  std::pair<RouteHandlePtr, std::shared_ptr<const AccessPoint>>
  createDestinationRoute(
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
      uint32_t idx);

  RouteHandleFactoryMap buildRouteMap();
  RouteHandleFactoryMapWithProxy buildRouteMapWithProxy();
  RouteHandleFactoryMapForWrapper buildRouteMapForWrapper();

  // This can be removed when the buildRouteMap specialization for
  // MemcacheRouterInfo is removed.
  RouteHandleFactoryMap buildCheckedRouteMap();
  RouteHandleFactoryMapWithProxy buildCheckedRouteMapWithProxy();
  RouteHandleFactoryMapForWrapper buildCheckedRouteMapForWrapper();

  std::unique_ptr<ExtraRouteHandleProviderIf<RouterInfo>> buildExtraProvider();
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "McRouteHandleProvider-inl.h"
