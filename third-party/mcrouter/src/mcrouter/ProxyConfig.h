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

  const folly::F14NodeMap<
      std::string,
      folly::F14FastSet<std::shared_ptr<const AccessPoint>>>&
  getAccessPoints() const {
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
