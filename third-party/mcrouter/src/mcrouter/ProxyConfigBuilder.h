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
#include <folly/dynamic.h>
#include <folly/json.h>

#include "mcrouter/PoolFactory.h"
#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class RouterInfo>
class ProxyConfig;
template <class RouterInfo>
class Proxy;

class ConfigApi;

class ProxyConfigBuilder {
 public:
  ProxyConfigBuilder(
      const McrouterOptions& opts,
      ConfigApi& configApi,
      folly::StringPiece jsonC);

  template <class RouterInfo>
  std::shared_ptr<ProxyConfig<RouterInfo>> buildConfig(
      Proxy<RouterInfo>& proxy,
      size_t index) const {
    return std::shared_ptr<ProxyConfig<RouterInfo>>(new ProxyConfig<RouterInfo>(
        proxy, json_, configMd5Digest_, *poolFactory_, index));
  }

  const folly::dynamic& preprocessedConfig() const {
    return json_;
  }

 private:
  folly::dynamic json_;
  std::unique_ptr<PoolFactory> poolFactory_;
  std::string configMd5Digest_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
