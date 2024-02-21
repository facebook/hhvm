/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <folly/Range.h>
#include <folly/container/F14Set.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/json/dynamic.h>

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

class ProxyBase;
template <class Transport>
class ProxyDestination;
class ProxyDestinationBase;
struct ProxyDestinationKey;
class PoolTkoTracker;

/**
 * Manages lifetime of ProxyDestinations. Main goal is to reuse same
 * ProxyDestinations (thus do not close opened connecitons) during
 * router reconfigururation.
 *
 * ProxyDestination can be either 'used' ot 'unused'. 'Used' means it is present
 * in current configuration. 'Unused' means it is not present in current
 * configuration (e.g. pool was removed and mcrouter reconfigured).
 *
 * Also ProxyDestination can be 'active' or 'inactive'. 'Active' means there is
 * opened connection to this destination and there were requests during last
 * reset_inactive_connection_interval ms routed to this destination.
 * 'Inactive' means there were no requests and connection may be closed.
 *
 * Note: There's one ProxyDestinationMap per proxy thread.
 */
class ProxyDestinationMap {
 public:
  explicit ProxyDestinationMap(ProxyBase* proxy);

  /**
   * If ProxyDestination is already stored in this object - returns it;
   * otherwise creates a new one.
   *
   * @throws std::logic_error If Transport is not compatible with
   *                          AccessPoint::getProtocol().
   */
  template <class Transport>
  std::shared_ptr<ProxyDestination<Transport>> emplace(
      std::shared_ptr<AccessPoint> ap,
      std::chrono::milliseconds timeout,
      uint32_t qosClass,
      uint32_t qosPath,
      const std::shared_ptr<PoolTkoTracker>& poolTkoTracker,
      uint32_t idx);

  template <class Transport>
  std::shared_ptr<const AccessPoint> replace(
      const AccessPoint& tmpOldAccessPoint,
      std::shared_ptr<const AccessPoint> newAccessPoint,
      std::chrono::milliseconds timeout);

  std::shared_ptr<PoolTkoTracker> createPoolTkoTracker(
      std::string poolName,
      uint32_t numTkosFailOpenEnter,
      uint32_t numTkosFailOpenExit);

  /**
   * Remove destination from both active and inactive lists
   */
  void removeDestination(ProxyDestinationBase& destination);

  /**
   * Mark destination as 'active', so it won't be closed on next
   * resetAllInactive call
   */
  void markAsActive(ProxyDestinationBase& destination);

  /**
   * Close all 'inactive' destinations i.e. destinations which weren't marked
   * 'active' after last removeAllInactive call.
   */
  void resetAllInactive();

  /**
   * Set timer which resets inactive connections.
   * @param interval timer interval, should be greater than zero.
   */
  void setResetTimer(std::chrono::milliseconds interval);

  /**
   * Calls f(const ProxyDestination&) for each destination stored
   * in ProxyDestinationMap. The whole map is locked during the call.
   *
   * TODO: replace with getStats()
   */
  template <typename Func>
  void foreachDestinationSynced(Func&& f) const {
    std::lock_guard<std::mutex> lock(destinationsLock_);
    for (const auto* dst : destinations_) {
      f(*dst);
    }
  }

  ~ProxyDestinationMap();

 private:
  struct StateList;
  struct DestHasher {
    using is_transparent = void;
    size_t operator()(const ProxyDestinationBase* dest) const;
    size_t operator()(const ProxyDestinationKey& key) const;
  };

  struct DestEq {
    using is_transparent = void;
    bool operator()(
        const ProxyDestinationBase* x,
        const ProxyDestinationBase* y) const;
    bool operator()(const ProxyDestinationKey& x, const ProxyDestinationBase* y)
        const;
  };

  ProxyBase* proxy_;
  folly::F14ValueSet<ProxyDestinationBase*, DestHasher, DestEq> destinations_;
  mutable std::mutex destinationsLock_;

  std::unique_ptr<StateList> active_;
  std::unique_ptr<StateList> inactive_;

  uint32_t inactivityTimeout_;
  std::unique_ptr<folly::AsyncTimeout> resetTimer_;

  /**
   * Schedules timeout for resetting inactive connections.
   *
   * @param initial  true iff this an initial attempt to schedule timer.
   */
  void scheduleTimer(bool initialAttempt);
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "mcrouter/ProxyDestinationMap-inl.h"
