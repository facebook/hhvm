/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/TimeoutManager.h>
#include <memory>
#include <proxygen/lib/utils/Time.h>

namespace proxygen {

/**
 * AsyncTimeoutSet exists for efficiently managing a group of timeouts events
 * that always have the same timeout interval.
 *
 * AsyncTimeoutSet takes advantage of the fact that the timeouts are always
 * scheduled in sorted order.  (Since each timeout has the same interval, when
 * a new timeout is scheduled it will always be the last timeout in the set.)
 * This avoids the need to perform any additional sorting of the timeouts
 * within a single AsyncTimeoutSet.
 *
 * AsyncTimeoutSet is useful whenever you have a large group of objects that
 * each need their own timeout, but with the same interval for each object.
 * For example, managing idle timeouts for thousands of connection, or
 * scheduling health checks for a large group of servers.
 *
 * Note, this class may not be needed given libevent's
 * event_base_init_common_timeout(). We should look into using that.
 */
class AsyncTimeoutSet
    : private folly::AsyncTimeout
    , public folly::DelayedDestruction {
 public:
  using UniquePtr = std::unique_ptr<AsyncTimeoutSet, Destructor>;

  /**
   * A callback to be notified when a timeout has expired.
   *
   * AsyncTimeoutSet::Callback is very similar to AsyncTimeout.  The primary
   * distinction is that AsyncTimeout can choose its timeout interval each
   * time it is scheduled.  On the other hand, AsyncTimeoutSet::Callback
   * always uses the timeout interval defined by the AsyncTimeoutSet where it
   * is scheduled.
   */
  class Callback {
   public:
    Callback() {
    }

    virtual ~Callback();

    /**
     * timeoutExpired() is invoked when the timeout has expired.
     */
    virtual void timeoutExpired() noexcept = 0;

    /**
     * Cancel the timeout, if it is running.
     *
     * If the timeout is not scheduled, cancelTimeout() does nothing.
     */
    void cancelTimeout() {
      if (timeoutSet_ == nullptr) {
        // We're not scheduled, so there's nothing to do.
        return;
      }
      cancelTimeoutImpl();
    }

    /**
     * Return true if this timeout is currently scheduled, and false otherwise.
     */
    bool isScheduled() const {
      return timeoutSet_ != nullptr;
    }

   private:
    // Get the time remaining until this timeout expires
    std::chrono::milliseconds getTimeRemaining(
        std::chrono::milliseconds now) const {
      if (now >= expiration_) {
        return std::chrono::milliseconds(0);
      }
      return expiration_ - now;
    }

    void setScheduled(AsyncTimeoutSet* timeoutSet, Callback* prev);
    void cancelTimeoutImpl();

    std::shared_ptr<folly::RequestContext> context_;

    AsyncTimeoutSet* timeoutSet_{nullptr};
    Callback* prev_{nullptr};
    Callback* next_{nullptr};
    std::chrono::milliseconds expiration_{0};

    // Give AsyncTimeoutSet direct access to our members so it can take care
    // of scheduling/cancelling.
    friend class AsyncTimeoutSet;
  };

  /**
   * Clock interface.  Can be used for different time implementations (eg:
   * monotonic, system) or mocking.
   */
  class TimeoutClock {
   public:
    TimeoutClock() {
    }

    virtual ~TimeoutClock() {
    }

    virtual std::chrono::milliseconds millisecondsSinceEpoch() = 0;
  };

  /**
   * Create a new AsyncTimeoutSet with the specified interval.
   *
   * If timeout clock is unspecified, it will use the default (system clock)
   */
  AsyncTimeoutSet(
      folly::TimeoutManager* timeoutManager,
      std::chrono::milliseconds intervalMS,
      std::chrono::milliseconds atMostEveryN = std::chrono::milliseconds(0),
      TimeoutClock* timeoutClock = nullptr);

  /**
   * Create a new AsyncTimeoutSet with the given 'internal' settting. For
   * details on what the InternalEnum specifies, see the documentation in
   * AsyncTimeout.h
   */
  AsyncTimeoutSet(
      folly::TimeoutManager* timeoutManager,
      InternalEnum internal,
      std::chrono::milliseconds intervalMS,
      std::chrono::milliseconds atMostEveryN = std::chrono::milliseconds(0));

  /**
   * Destroy the AsyncTimeoutSet.
   *
   * Normally a AsyncTimeoutSet should only be destroyed when there are no
   * more callbacks pending in the set.  If there are timeout callbacks pending
   * for this set, destroying the AsyncTimeoutSet will automatically cancel
   * them.  If you destroy a AsyncTimeoutSet with callbacks pending, your
   * callback code needs to be aware that the callbacks will never be invoked.
   */
  void destroy() override;

  /**
   * Get the interval for this AsyncTimeoutSet.
   *
   * Returns the timeout interval in milliseconds.  All callbacks scheduled
   * with scheduleTimeout() will be invoked after this amount of time has
   * passed since the call to scheduleTimeout().
   */
  std::chrono::milliseconds getInterval() const {
    return interval_;
  }

  /**
   * Schedule the specified Callback to be invoked after the AsyncTimeoutSet's
   * specified timeout interval.
   *
   * If the callback is already scheduled, this cancels the existing timeout
   * before scheduling the new timeout.
   */
  void scheduleTimeout(Callback* callback);

  /**
   * Limit how frequently this AsyncTimeoutSet will fire.
   */
  void fireAtMostEvery(const std::chrono::milliseconds& ms) {
    atMostEveryN_ = ms;
  }

  /**
   * Get a pointer to the next Callback scheduled to be invoked (may be null).
   */
  Callback* front() {
    return head_;
  }
  const Callback* front() const {
    return head_;
  }

 protected:
  /**
   * Protected destructor.
   *
   * Use destroy() instead.  See the comments in DelayedDestruction for more
   * details.
   */
  ~AsyncTimeoutSet() override;

 private:
  // Forbidden copy constructor and assignment operator
  AsyncTimeoutSet(AsyncTimeoutSet const&) = delete;
  AsyncTimeoutSet& operator=(AsyncTimeoutSet const&) = delete;

  // Private methods to be invoked by AsyncTimeoutSet::Callback
  void headChanged();

  // Methods inherited from AsyncTimeout
  void timeoutExpired() noexcept override;

  TimeoutClock& timeoutClock_;
  Callback* head_;
  Callback* tail_;
  std::chrono::milliseconds interval_;
  std::chrono::milliseconds atMostEveryN_;
  bool inTimeoutExpired_{false};
};

} // namespace proxygen
