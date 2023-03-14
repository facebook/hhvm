/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <string>

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

class ProxyDestinationBase;

struct ProxyDestinationKey {
  const AccessPoint& accessPoint;
  const std::chrono::milliseconds timeout;
  const uint32_t idx{0};

  explicit ProxyDestinationKey(const ProxyDestinationBase& dst);
  ProxyDestinationKey(
      const AccessPoint& ap,
      std::chrono::milliseconds timeout,
      uint32_t idx);

  std::string str() const;
  size_t hash() const;

  bool operator==(const ProxyDestinationKey& other) const;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
