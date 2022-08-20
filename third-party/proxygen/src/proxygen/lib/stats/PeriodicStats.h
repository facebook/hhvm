/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <folly/Function.h>
#include <folly/ThreadLocal.h>
#include <folly/experimental/FunctionScheduler.h>
#include <folly/synchronization/Rcu.h>

namespace proxygen {

/**
 * PeriodicStats:
 *
 * An abstract class that implements the internals of periodically retrieving
 * some collection of data, making it available to other threads in a safe
 * and performant way.  Can be used to build a thread-safe mechanism for
 * retrieving desired information (i.e. multiple readers, single writer).
 * In this way readers view a consistent representation of the underlying
 * data, as opposed to a model where each reader fetches the data separately,
 * potentially at different times, and thus potentially coming to different
 * conclusions (not to mention wasting resources performing the same
 * operations).
 *
 * Template class T must:
 * - be copyable (careful about storing pointers in T for this reason).
 * - implement method getLastUpdateTime() that returns a
 * chrono timepoint, since epoch, that denotes for what time T was valid.
 *
 * All public methods are thread-safe.
 */
template <class T>
class PeriodicStats {
 public:
  /**
   * Default constructor that will initialize data_ as appropriate.
   * Underlying thread that updates locally cached Data is not
   * started yet.
   */
  explicit PeriodicStats(T* data) : data_(data) {
  }
  virtual ~PeriodicStats() {
    stopRefresh();
    modifyData(nullptr, /*sync=*/true);
  }

  /**
   * A caller can set a custom callback, to be executed by the function
   * scheduler thread, after each refresh.  This can only be set while the
   * refreshing thread is not active.
   */
  void setRefreshCB(folly::Function<void()>&& cb) {
    std::lock_guard<std::mutex> guard(schedulerMutex_);
    CHECK(!scheduler_);
    refreshCb_ = std::move(cb);
  }

  // Returns the current refresh interval in ms of the underlying data.
  std::chrono::milliseconds getRefreshIntervalMs() {
    return refreshPeriodMs_;
  }

  /**
   * Refreshes the underlying data using the period specified by employing
   * the underlying function scheduler.  If already refreshing, the current
   * period will instead be updated and on next scheduling the updated period
   * applied.
   */
  void refreshWithPeriod(
      std::chrono::milliseconds periodMs,
      std::chrono::milliseconds initialDelayMs = std::chrono::milliseconds(0)) {
    CHECK_GE(periodMs.count(), 0);
    std::lock_guard<std::mutex> guard(schedulerMutex_);
    refreshPeriodMs_ = periodMs;
    if (!scheduler_) {
      scheduler_.reset(new folly::FunctionScheduler());
      scheduler_->setThreadName("periodic_stats");
      // Steady here implies that scheduling will be fixed as opposed to
      // offsetting from the current time which is desired to ensure minimal
      // use of synchronization for getCurrentData()
      scheduler_->setSteady(true);

      std::function<void()> updateFunc(
          std::bind(&PeriodicStats::updateCachedData, this));
      std::function<std::chrono::milliseconds()> intervalFunc(
          std::bind(&PeriodicStats::getRefreshIntervalMs, this));

      scheduler_->addFunctionGenericDistribution(updateFunc,
                                                 intervalFunc,
                                                 "periodic_stats",
                                                 "periodic_stats_interval",
                                                 initialDelayMs);

      scheduler_->start();
    }
  }

  /**
   * Stops refreshing the underlying data.  It is possible that an update is
   * occuring as this method is run in which case the guarantee becomes that no
   * subsequent refresh will be scheduled.
   */
  void stopRefresh() {
    std::lock_guard<std::mutex> guard(schedulerMutex_);
    scheduler_.reset();
  }

  /**
   * Readers utilize this method to get their thread local representation
   * of the current data.  The way in which this works is that while there is
   * a thread local representation of the data, there is also a global
   * representation that is guarded.  Asynchronously, the refresh thread will
   * update the global representation in a thread safe manner.  Then, as
   * readers wish to consume this data, they are instead presented with a
   * thread local representation that is updated if:
   *  A) their representation is uninitialized
   *  B) their representation is old
   *
   * Method is virtual for testing reasons.
   */
  virtual const T& getCurrentData() const {
    {
      std::scoped_lock guard(folly::rcu_default_domain());
      auto* loadedData = data_.load();
      if (loadedData->getLastUpdateTime() != tlData_->getLastUpdateTime()) {
        // Should be fine using the default assignment operator the compiler
        // gave us I think...this will stop being true if loadedData starts
        // storing pointers.
        *tlData_ = *loadedData;
      }
    }
    return *tlData_;
  }
  // Same as above except no local update will be performed, even if newer
  // data is available.
  virtual const T& getPreviousData() const {
    return *tlData_;
  }

 protected:
  /**
   * Returns a new instance of T data that will be owned and cached
   * by this class.  Subclasses can implement accordingly to override the
   * returned instance.
   *
   * If nullptr is returned, no update is performed.  Subclasses can leverage
   * this flow in case no new data is available or if they want the current
   * data to remain in use.
   */
  virtual T* getNewData() const = 0;

  // Wrapper for updating and retiring old cached data_ via RCU.
  // The 'sync' parameter controls whether the previous object is deleted
  // right away vs in a delayed fashion
  void modifyData(T* newData, bool sync = false) {
    auto* oldData = data_.exchange(newData);
    if (sync) {
      folly::rcu_synchronize();
      delete oldData;
    } else {
      folly::rcu_retire(oldData);
    }
  }

  /**
   * Wrapper for the internal function scheduler to call in order to update
   * data_ via getNewData() and modifyData().  Method virtual for test
   * purposes.
   */
  void updateCachedData() {
    auto* newData = getNewData();
    if (newData) {
      modifyData(newData);
      if (refreshCb_) {
        refreshCb_();
      }
    }
  }

  /**
   * data_ represents the source of truth for the class.
   * tlData_ is updated on a 'as-used' basis from data_ via RCU
   * synchronization.
   */
  std::atomic<T*> data_;
  folly::ThreadLocal<T> tlData_;

  // Refresh management fields

  /**
   * scheduler_ represents the underlying abstraction that will create
   * the thread that updates data_ asynchronously.
   *
   * The mutex specifically synchronizes access to scheduler_.  It is
   * leveraged to ensure all public APIs are thread-safe.
   *
   * The refreshPeriodMs_ is the amount of time that must elapse before
   * the underlying cached data is updated.  It is atomic as while it can
   * be read/set by the caller under the schedulerMutex_, it is also read
   * by the underlying function scheduler thread.
   *
   * The refreshCb_, if specified, provides the caller the functionality to
   * execute an arbitrary function on refresh.
   */
  std::unique_ptr<folly::FunctionScheduler> scheduler_;
  std::mutex schedulerMutex_;
  std::atomic<std::chrono::milliseconds> refreshPeriodMs_{
      std::chrono::milliseconds(0)};
  folly::Function<void()> refreshCb_;
};

} // namespace proxygen
