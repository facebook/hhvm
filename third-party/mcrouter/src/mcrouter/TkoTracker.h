/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include <folly/Range.h>
#include <folly/experimental/StringKeyedUnorderedMap.h>

#include "mcrouter/TkoCounters.h"
#include "mcrouter/lib/carbon/Result.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ProxyDestinationBase;
class TkoTrackerMap;

class PoolTkoTracker {
 public:
  PoolTkoTracker(uint32_t failOpenEnterNumTkos, uint32_t failOpenExitNumTkos)
      : failOpenEnterNumTkos_(failOpenEnterNumTkos),
        failOpenExitNumTkos_(failOpenExitNumTkos) {}

  /**
   * Returns pair with first one is true if we are in the fail-open state,
   * false, otherwise. The second bool is true
   * if we enter the fail-open state after this call.
   */
  std::pair<bool, bool> incNumDestinationsTko();

  /**
   * Return true if we exited fail-open state after this call.
   */
  bool decNumDestinationsTko();

 private:
  const uint32_t failOpenEnterNumTkos_;
  const uint32_t failOpenExitNumTkos_;
  bool failOpen_{false};
  std::atomic<size_t> numDestinationsTko_{0};
};

/**
 * We record the number of consecutive failures for each destination.
 * Once it goes over a certain threshold, we mark the destination as TKO
 * (stands for total knockout :), meaning that we should not send regular
 * requests to it.  The calling code can use this status to start sending probe
 * requests and only unmark it as TKO once a probe comes back successfully.
 *
 * We distinguish between soft and hard TKOs.  Hard TKOs imply that a box cannot
 * be connected to, and results in an instant failure.  We can have an unlimited
 * number of these. Soft TKOs are the result of soft errors and timeouts, as
 * described above.  These may be limited in number.  A box may transition for
 * soft to hard TKO, but once hard TKO the box must send a successful reply to
 * be unmarked.
 *
 * Perf implications: recordSuccess() with no previous failures and isTko()
 * are lock-free, so the common (no error results) path is fast.
 *
 * Races are not a big issue: it's OK to miss a few events. What's of critical
 * importance is that once a destination is marked TKO, only the responsible
 * proxy (the one sending probes) can change its TKO state. Once we are in TKO
 * the responsible thread effectively has a mutex over all state in TkoTracker,
 * and so races aren't possible.
 */
class TkoTracker {
 public:
  /**
   * @return Is the destination currently marked Hard TKO?
   */
  bool isHardTko() const;

  /**
   * @return Is the destination currently marked Soft TKO?
   */
  bool isSoftTko() const;

  /**
   * Tells whether or not the destination is marked as TKO.
   */
  bool isTko() const {
    // See sumFailures_ description for more details.
    return sumFailures_.load(std::memory_order_relaxed) > tkoThreshold_;
  }

  /**
   * The reason why the destination is marked as TKO.
   * NOTE: If this box is not TKO'd, returns mc_res_unkown.
   */
  carbon::Result tkoReason() const {
    return tkoReason_.load(std::memory_order_relaxed);
  }

  /**
   * @return The current number of consecutive failures.
   *         This is basically a number of recordHardFailure/recordSoftFailure
   *         calls after last recordSuccess.
   */
  size_t consecutiveFailureCount() const {
    return consecutiveFailureCount_;
  }

  /**
   * @return number of TKO destinations for current router
   */
  const TkoCounters& globalTkos() const;

  /**
   * Can be called from any proxy thread.
   * Signal that a "soft" failure occurred. We need to see
   * tko_threshold "soft" failures in a row to mark a host TKO. Will not TKO
   * a host if currentSoftTkos would exceed maxSoftTkos
   *
   * @param pdstn   A pointer to the calling proxydestination for tracking
   *                responsibility.
   * @param result  The result that caused the soft failure.
   *
   * @return true if we just reached tko_threshold with this result,
   *         marking the host TKO.  In this case, the calling proxy
   *         is responsible for sending probes and calling recordSuccess()
   *         once a probe is successful.
   */
  bool recordSoftFailure(ProxyDestinationBase* pdstn, carbon::Result result);

  /**
   * Can be called from any proxy thread.
   * Signal that a "hard" failure occurred - marks the host TKO
   * right away.
   *
   * @param pdstn   A pointer to the calling proxydestination for tracking
   *                responsibility.
   * @param result  The result that caused the hard failure.
   *
   * @return true if we just reached tko_threshold with this result,
   *         marking the host TKO.  In this case, the calling proxy
   *         is responsible for sending probes and calling recordSuccess()
   *         once a probe is successful. Note, transition from soft to hard
   *         TKO does not result in a change of responsibility.
   */
  bool recordHardFailure(ProxyDestinationBase* pdstn, carbon::Result result);

  /**
   * Resets all consecutive failures accumulated so far
   * (unmarking any TKO status).
   *
   * @param pdstn  a pointer to the calling proxydestination for tracking
   *               responsibility.
   *
   * @return true if `pdstn` was responsible for sending probes
   */
  bool recordSuccess(ProxyDestinationBase* pdstn);

  /**
   * Should be called when ProxyDestination is going to be destroyed
   *
   * @return true if `pdstn` was responsible for sending probes
   */
  bool removeDestination(ProxyDestinationBase* pdstn);

  ~TkoTracker();

  void setPoolTracker(const std::shared_ptr<PoolTkoTracker>& poolTracker) {
    poolTracker_ = poolTracker;
  }

 private:
  // The string is stored in TkoTrackerMap::trackers_
  folly::StringPiece key_;
  const size_t tkoThreshold_;
  std::shared_ptr<PoolTkoTracker> poolTracker_;
  TkoTrackerMap& trackerMap_;

  TkoCounters tkoCounters_;

  /**
   * sumFailures_ is used for a few things depending on the state of the
   * destination. For a destination that is not TKO, it tracks the number of
   * consecutive soft failures to a destination.
   * If a destination is soft TKO, it contains the numerical representation of
   * the pointer to the proxy thread that is responsible for sending it probes.
   * If a destination is hard TKO, it contains the same value as for soft TKO,
   * but with the LSB set to 1 instead of 0.
   * In summary, allowed values are:
   *   0, 1, .., tkoThreshold_ - 1, pdstn, pdstn | 0x1, where pdstn is the
   *   address of any of the proxy threads for this destination.
   */
  std::atomic<uintptr_t> sumFailures_{0};

  std::atomic<size_t> consecutiveFailureCount_{0};

  std::atomic<carbon::Result> tkoReason_{carbon::Result::UNKNOWN};

  /**
   * Decrement the global counter of current soft TKOs
   */
  void decrementSoftTkoCount(ProxyDestinationBase* pdstn);

  /**
   * Increment the global counter of current soft TKOs.
   */
  bool incrementSoftTkoCount(ProxyDestinationBase* pdstn);

  /**
   * Decrement the global counter of current hard TKOs
   */
  void decrementHardTkoCount(ProxyDestinationBase* pdstn);

  /**
   * Increment the global counter of current hard TKOs.
   */
  bool incrementHardTkoCount(ProxyDestinationBase* pdstn);

  /* Modifies the value of sumFailures atomically. Fails only
     in the case that another proxy takes responsibility, in which case all
     other proxies may not modify state */
  bool setSumFailures(uintptr_t value);

  /* Return true if this thread is responsible for the TKO state */
  bool isResponsible(ProxyDestinationBase* pdstn) const;

  /**
   * @param tkoThreshold    Require this many soft failures to mark
   *                        the destination TKO.
   * @param trackerMap  Number of TKO destination for current router.
   */
  TkoTracker(size_t tkoThreshold, TkoTrackerMap& trackerMap);

  friend class TkoTrackerMap;
};

/**
 * Manages the TkoTrackers for all servers
 */
class TkoTrackerMap {
 public:
  TkoTrackerMap() = default;
  TkoTrackerMap(const TkoTrackerMap&) = delete;
  TkoTrackerMap(TkoTrackerMap&&) = delete;

  std::shared_ptr<PoolTkoTracker> createPoolTkoTracker(
      std::string poolName,
      uint32_t numEnterTkos,
      uint32_t numExitTkos);
  /**
   * Creates/updates TkoTracker for `pdstn` and updates `pdstn->tko` pointer.
   */
  void updateTracker(
      ProxyDestinationBase& pdstn,
      const size_t tkoThreshold,
      const std::shared_ptr<PoolTkoTracker>& poolTkoTracker);

  /**
   * @return  number of servers that recently returned error replies.
   */
  size_t getSuspectServersCount() const;

  /**
   * @return  servers that recently returned error replies.
   *   Format: {
   *     server ip => ( is server marked as TKO?, number of failures )
   *   }
   *   Only servers with positive number of failures will be returned.
   */
  std::unordered_map<std::string, std::pair<bool, size_t>> getSuspectServers()
      const;

  const TkoCounters& globalTkos() const {
    return globalTkos_;
  }

 private:
  mutable std::mutex mx_;
  folly::F14NodeMap<std::string, std::weak_ptr<TkoTracker>> trackers_;
  folly::F14NodeMap<std::string, std::weak_ptr<PoolTkoTracker>> poolTrackers_;

  // Total number of boxes marked as TKO.
  TkoCounters globalTkos_;

  void removeTracker(folly::StringPiece key) noexcept;

  // Thread-safe function that iterates over all tko trackers
  void foreachTkoTracker(
      const std::function<void(folly::StringPiece, const TkoTracker&)> func)
      const;

  friend class TkoTracker;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
