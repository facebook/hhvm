/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

namespace proxygen {

/**
 * Simple base class that a subclass may implement in order to create a data
 * structure suitable for use by PeriodicStats.  It is not a hard requirement
 * but as this class implements the PeriodicStats template interface expected
 * from wrapped data any other class can simply implement this class to buy-in
 * without caring about any other internals.
 *
 * Users must call setLastUpdateTime() appropriately when generating new data
 * for use in PeriodicStats.  This is not done by default in the constructor
 * in case an implementation takes a not trivial amount of time after object
 * construction.
 */
class PeriodicStatsDataBase {
 public:
  PeriodicStatsDataBase() = default;
  ~PeriodicStatsDataBase() = default;

  /**
   * Refreshes the time (from epoch) when this record was created (so for
   * which the utilization metrics are valid).
   */
  void refreshLastUpdateTime() {
    time_ = getEpochTime();
  }
  void setLastUpdateTime(std::chrono::milliseconds updateTime) {
    time_ = updateTime;
  }

  /**
   * Gets the time (from epoch) when this record was created (so for
   * which the utilization metrics are valid).
   */
  std::chrono::milliseconds getLastUpdateTime() const {
    return time_;
  }

  // Helper method to get ms since epoch.
  static std::chrono::milliseconds getEpochTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
  }

 protected:
  // Refresh management fields
  std::chrono::milliseconds time_{0};
};

} // namespace proxygen
