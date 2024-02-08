/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <folly/Range.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

template <class RouterInfo>
class Proxy;
template <class RouterInfo>
class ProxyRoute;
template <class RouterInfo>
class ServiceInfo;

class PoolFactory;
struct CommonAccessPointAttributes;

/**
 * Topmost struct for mcrouter configs.
 */
template <class RouterInfo>
class ProxyConfig {
 public:
  ProxyRoute<RouterInfo>& proxyRoute() const {
    return *proxyRoute_;
  }

  std::shared_ptr<ServiceInfo<RouterInfo>> serviceInfo() const {
    return serviceInfo_;
  }

  std::string getConfigMd5Digest() const {
    return configMd5Digest_;
  }

  std::shared_ptr<typename RouterInfo::RouteHandleIf> getRouteHandleForAsyncLog(
      folly::StringPiece asyncLogName) const;

  std::shared_ptr<typename RouterInfo::RouteHandleIf> getRouteHandleForPool(
      folly::StringPiece poolName) const;

  const folly::F14NodeMap<
      std::string,
      std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>>&
  getPools() const {
    return pools_;
  }

  // pool source name -> (allow_partial_reconfig, [(pool_config,[pool_names])])
  const folly::F14NodeMap<
      std::string,
      std::pair<
          bool,
          std::vector<std::pair<
              std::shared_ptr<CommonAccessPointAttributes>,
              std::vector<std::string>>>>>&
  getPartialConfigs() const {
    return partialConfigs_;
  }

  bool allowPartialConfig(folly::StringPiece poolSourceName) const {
    auto it = partialConfigs_.find(poolSourceName);
    if (it == partialConfigs_.end()) {
      return false;
    }
    return it->second.first;
  }

  bool updateAccessPoints(
      const std::string& pool,
      std::shared_ptr<const AccessPoint>& oldAccessPoint,
      std::shared_ptr<const AccessPoint>& newAccessPoint) {
    auto it = accessPoints_.find(pool);
    if (it != accessPoints_.end()) {
      auto apIt = it->second.find(oldAccessPoint);
      if (apIt != it->second.end()) {
        it->second.erase(apIt);
      } else {
        return false;
      }
      it->second.insert(newAccessPoint);
      return true;
    }
    return false;
  }

  folly::F14NodeMap<
      std::string,
      folly::F14FastSet<std::shared_ptr<const AccessPoint>>>&
  getAccessPoints() {
    return accessPoints_;
  }

  size_t calcNumClients() const;

 private:
  // This map (accessPoints_) needs to be destroyed as the last object in the
  // config (after all RouteHandles) because its keys are being referenced
  // by object in the Config.
  folly::F14NodeMap<
      std::string,
      folly::F14FastSet<std::shared_ptr<const AccessPoint>>>
      accessPoints_;

  // pool source name -> (allow_partial_reconfig, [(pool_config,[pool_names])])
  folly::F14NodeMap<
      std::string,
      std::pair<
          bool,
          std::vector<std::pair<
              std::shared_ptr<CommonAccessPointAttributes>,
              std::vector<std::string>>>>>
      partialConfigs_;

  folly::F14NodeMap<
      std::string,
      std::vector<std::shared_ptr<typename RouterInfo::RouteHandleIf>>>
      pools_;
  std::shared_ptr<ProxyRoute<RouterInfo>> proxyRoute_;
  std::shared_ptr<ServiceInfo<RouterInfo>> serviceInfo_;
  std::string configMd5Digest_;
  folly::F14NodeMap<
      std::string,
      std::shared_ptr<typename RouterInfo::RouteHandleIf>>
      asyncLogRoutes_;
  folly::F14NodeMap<
      std::string,
      std::shared_ptr<typename RouterInfo::RouteHandleIf>>
      tierRoutes_;

  /**
   * Parses config and creates ProxyRoute
   *
   * @param jsonC config in format of JSON with comments and templates
   */
  ProxyConfig(
      Proxy<RouterInfo>& proxy,
      const folly::dynamic& json,
      std::string configMd5Digest,
      PoolFactory& poolFactory,
      size_t index);

  friend class ProxyConfigBuilder;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "ProxyConfig-inl.h"
