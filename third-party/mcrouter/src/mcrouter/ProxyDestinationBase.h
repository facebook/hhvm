/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <chrono>
#include <memory>

#include <folly/IntrusiveList.h>
#include <folly/concurrency/AtomicSharedPtr.h>

#include "mcrouter/ExponentialSmoothData.h"
#include "mcrouter/TkoLog.h"
#include "mcrouter/lib/carbon/Result.h"
#include "mcrouter/lib/network/Transport.h"

namespace folly {
class AsyncTimeout;
} // namespace folly

namespace facebook {
namespace memcache {

struct AccessPoint;
struct RpcStatsContext;

namespace mcrouter {

class ProxyBase;
class TkoTracker;

enum class GlobalTkoUpdateType : uint32_t {
  INC_SOFT_TKOS = 1,
  DEC_SOFT_TKOS = 2,
  INC_HARD_TKOS = 4,
  DEC_HARD_TKOS = 8,
  ENTER_FAIL_OPEN = 16,
  EXIT_FAIL_OPEN = 32,
};

class ProxyDestinationBase {
 public:
  using RequestQueueStats = Transport::RequestQueueStats;

  enum class State {
    New, // never connected
    Up, // currently connected
    Down, // currently down
    Closed, // closed due to inactive
    NumStates
  };

  struct Stats {
    State state{State::New};
    ExponentialSmoothData<16> avgLatency;
    std::unique_ptr<
        std::array<uint64_t, static_cast<size_t>(carbon::Result::NUM_RESULTS)>>
        results;
    size_t probesSent{0};
    double retransPerKByte{0.0};
    // If poolstats config is present, keep track of most recent
    // pool with this destination
    int32_t poolStatIndex_{-1};

    // last time this connection was closed due to inactivity
    uint64_t inactiveConnectionClosedTimestampUs{0};
  };

  ProxyDestinationBase(
      ProxyBase& proxy,
      std::shared_ptr<const AccessPoint> ap,
      std::chrono::milliseconds timeout,
      uint32_t qosClass,
      uint32_t qosPath,
      uint32_t idx);
  virtual ~ProxyDestinationBase();

  /**
   * Returns true if okay to send request using this client.
   *
   * @param tkoReason   Output argument that will have the TKO reason in case
   *                    this method returns true.
   *
   * @return  True iff it is okay to send a request using this client.
   *          False otherwise.
   */
  bool maySend(carbon::Result& tkoReason) const;

  /**
   * @return stats for ProxyDestination
   */
  const Stats& stats() const {
    return stats_;
  }

  /**
   * Destination host that this client talks to.
   */
  const std::shared_ptr<const AccessPoint> accessPoint() const {
    return accessPoint_.load();
  }

  /**
   * The proxy that owns this client.
   */
  ProxyBase& proxy() const {
    return proxy_;
  }

  void updateShortestTimeout(
      std::chrono::milliseconds connectTimeout,
      std::chrono::milliseconds writeTimeout);

  /**
   * If the connection was previously closed due to lack of activity,
   * log for how long it was closed.
   */
  void updateConnectionClosedInternalStat();

  std::shared_ptr<TkoTracker> tracker() const {
    return tracker_;
  }
  void setTracker(std::shared_ptr<TkoTracker> tracker) {
    tracker_ = std::move(tracker);
  }

  /**
   * Update some global tko related stats based on type
   */
  void updateTkoStats(GlobalTkoUpdateType type);

  void setPoolStatsIndex(int32_t index);
  void updatePoolStatConnections(bool connected);

  virtual RequestQueueStats getRequestStats() const = 0;

  /**
   * Closes transport connection.
   */
  virtual void resetInactive() = 0;

  std::chrono::milliseconds shortestConnectTimeout() const {
    return shortestConnectTimeout_;
  }

 protected:
  virtual void updateTransportTimeoutsIfShorter(
      std::chrono::milliseconds shortestConnectTimeout,
      std::chrono::milliseconds shortestWriteTimeout) = 0;
  virtual carbon::Result sendProbe() = 0;
  virtual std::weak_ptr<ProxyDestinationBase> selfPtr() = 0;

  void markAsActive();
  void setState(State st);

  void handleTko(const carbon::Result result, bool isProbeRequest);
  void onTransitionToState(State state);
  void onTransitionFromState(State state);

  Stats& stats() {
    return stats_;
  }
  std::chrono::milliseconds shortestWriteTimeout() const {
    return shortestWriteTimeout_;
  }
  uint32_t qosClass() const {
    return qosClass_;
  }
  uint32_t qosPath() const {
    return qosPath_;
  }
  uint32_t idx() const {
    return idx_;
  }
  bool probeInflight() const {
    return probeInflight_;
  }

  std::shared_ptr<const AccessPoint> replaceAP(
      std::shared_ptr<const AccessPoint> newAccessPoint) {
    return accessPoint_.exchange(newAccessPoint);
  }

 private:
  ProxyBase& proxy_;
  std::shared_ptr<TkoTracker> tracker_;

  // Destination host information
  folly::atomic_shared_ptr<const AccessPoint> accessPoint_;
  std::chrono::milliseconds shortestConnectTimeout_{0};
  std::chrono::milliseconds shortestWriteTimeout_{0};
  const uint32_t qosClass_{0};
  const uint32_t qosPath_{0};
  const uint32_t idx_{0};

  Stats stats_;

  // Fields related to probes (for un-TKO).
  std::unique_ptr<folly::AsyncTimeout> probeTimer_;
  int probeDelayNextMs{0};
  bool probeInflight_{false};

  void* stateList_{nullptr};
  folly::IntrusiveListHook stateListHook_;

  void onTkoEvent(TkoLogEvent event, carbon::Result result) const;

  void startSendingProbes();
  void stopSendingProbes();
  void scheduleNextProbe();

  void onTransitionImpl(State state, bool to);

  friend struct ProxyDestinationKey;
  friend class ProxyDestinationMap;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
