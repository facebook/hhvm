/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <mutex>
#include <stdexcept>

#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestinationBase.h"
#include "mcrouter/ProxyDestinationKey.h"
#include "mcrouter/lib/network/AccessPoint.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class Transport>
std::shared_ptr<const AccessPoint> ProxyDestinationMap::replace(
    const AccessPoint& tmpAccessPoint,
    std::shared_ptr<const AccessPoint> newAccessPoint,
    const std::chrono::milliseconds timeout) {
  // Find the destination with old access point using
  // temp access point. If it is found, the old access point is replaced
  // with new access point
  std::lock_guard<std::mutex> lck(destinationsLock_);
  auto it = destinations_.find(ProxyDestinationKey(tmpAccessPoint, timeout, 0));
  if (it != destinations_.end()) {
    auto destination = std::dynamic_pointer_cast<ProxyDestination<Transport>>(
        (*it)->selfPtr().lock());
    // Erase the destination before the access point is replaced because
    // the internal logic to identify the destination inside destinations_
    // uses access point info as Key
    destinations_.erase(destination.get());
    auto oldAccessPoint = destination->replaceAP(newAccessPoint);
    // After the access point is replaced, insert the destination back into
    // destinations with new access point used as Key
    destinations_.emplace(destination.get());
    return oldAccessPoint;
  }
  return nullptr;
}
template <class Transport>
std::shared_ptr<ProxyDestination<Transport>> ProxyDestinationMap::emplace(
    std::shared_ptr<AccessPoint> ap,
    std::chrono::milliseconds timeout,
    uint32_t qosClass,
    uint32_t qosPath,
    const std::shared_ptr<PoolTkoTracker>& poolTkoTracker,
    uint32_t idx) {
  std::shared_ptr<ProxyDestination<Transport>> destination;
  {
    std::lock_guard<std::mutex> lck(destinationsLock_);
    auto it = destinations_.find(ProxyDestinationKey(*ap, timeout, idx));
    if (it != destinations_.end()) {
      destination = std::dynamic_pointer_cast<ProxyDestination<Transport>>(
          (*it)->selfPtr().lock());
      assert(destination); // if destination is in map, selfPtr should be OK
      return destination;
    }

    destination = ProxyDestination<Transport>::create(
        *proxy_, std::move(ap), timeout, qosClass, qosPath, idx);
    destinations_.emplace(destination.get());
  }

  // Update shared area of ProxyDestinations with same key from different
  // threads. This shared area is represented with TkoTracker class.
  proxy_->router().tkoTrackerMap().updateTracker(
      *destination, proxy_->router().opts().failures_until_tko, poolTkoTracker);

  return destination;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
