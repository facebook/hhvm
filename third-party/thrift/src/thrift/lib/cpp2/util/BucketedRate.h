/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <chrono>
#include <cmath>
#include <vector>

#include <glog/logging.h>

#include <folly/lang/Bits.h>

namespace apache::thrift {

/**
 * Compute a rate (in tick per second) with an algorithm based on buckets.
 *
 * Each bucket count how many ticks happened in a specific time frame.
 * Those buckets are organized in a ring buffer way, we reuse the previous
 * buckets after they have expired.
 *
 * All buckets are initialized with 0, which means the rate slowly ramp up
 * toward its true value.
 */
template <class Clock>
class BucketedRate {
 public:
  /**
   * Construct a Rate estimator based on a series of bucket that each count
   * the number of ticks in a specific time window.
   * `window` represent the duration of the total window we consider
   * `bucketCount` represent the number of buckets to use to subdivise this
   * window.
   * e.g. window=8s, bucketCount=8 means that we'll have 8 buckets of 1s each.
   */
  explicit BucketedRate(
      std::chrono::duration<double> window, uint32_t bucketCount = 8)
      : buckets_(bucketCount, 0),
        bucketWidth_(window / bucketCount),
        lastIndex_(divide_chrono(Clock::now(), window)) {
    CHECK(folly::isPowTwo(bucketCount));
  }

  /**
   * Indicate that a tick happened
   */
  void tick() {
    const auto now = Clock::now();
    const auto index = discardOutdatedBuffer(now);
    const auto i = (index & (buckets_.size() - 1)); // = index % buckets_.size()
    buckets_[i]++;
  }

  /**
   * Return the rate of ticks in second as a double.
   * It uses the current absence of tick in [lastArrivalTime_, now] as
   * a parameter to compute more accurate value. (i.e. if we don't see a tick
   * for a long time, the reported rate will still decrease)
   */
  double rate() {
    auto now = Clock::now();
    auto index = discardOutdatedBuffer(now);

    auto sum = 0;
    for (auto x : buckets_) {
      sum += x;
    }

    // offset represent the amount of time elapsed in the last bucket
    // (e.g. 1s from a 5s bucket)
    auto offset = now - (index * bucketWidth_);
    // compute the window that we consider for calculating the rate, indeed the
    // last bucket may be only partially filled and we don't want to consider it
    // if we only have spent 1ms of a 5s bucket.
    auto elapsedWindow = std::chrono::duration_cast<std::chrono::nanoseconds>(
        bucketWidth_ * (buckets_.size() - 1) + offset.time_since_epoch());

    return static_cast<double>(sum) * 1e9 / elapsedWindow.count();
  }

 private:
  // Clear any outdated buckets (i.e. between last entry and now)
  // and return the current index in the ring buffer
  uint32_t discardOutdatedBuffer(std::chrono::time_point<Clock> t) {
    auto index = divide_chrono(t, bucketWidth_);
    if (index != lastIndex_) {
      for (uint32_t j = lastIndex_ + 1; j <= index; j++) {
        auto i = (j & (buckets_.size() - 1)); // = j % buckets_.size()
        buckets_[i] = 0;
      }
      lastIndex_ = index;
    }
    return index;
  }

  // Helper for dividing a time_point by a duration
  static uint32_t divide_chrono(
      std::chrono::time_point<Clock> t, std::chrono::duration<double> w) {
    auto t_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(t)
                    .time_since_epoch()
                    .count();
    auto w_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(w).count();
    return static_cast<double>(t_ns) / w_ns;
  }

  std::vector<uint32_t> buckets_;
  const std::chrono::duration<double> bucketWidth_;
  uint32_t lastIndex_;
};

} // namespace apache::thrift
