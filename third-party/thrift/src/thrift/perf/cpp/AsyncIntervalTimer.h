/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_TEST_PERF_ASYNCINTERVALTIMER_H_
#define THRIFT_TEST_PERF_ASYNCINTERVALTIMER_H_ 1

#include <thrift/lib/cpp/TLogging.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/test/loadgen/RNG.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <folly/portability/Unistd.h>

namespace apache {
namespace thrift {

/**
 * AsyncIntervalTimer helps async tasks run at a desired rate. It is very
 * similar to IntervalTimer at this stage
 *
 * The key changes here are to take into account that each thread has a set of
 * AsyncRunners and the accounting is done per AsyncWorker. This means we need
 * to ensure they start at random times in the initialization window (the first
 * interval of traffic) rather than all kicking off immediately and using a
 * central rate limiter. The second key change is of cource to limit each thread
 * to its proportion of the queries.
 */
class AsyncIntervalTimer {
 public:
  /**
   * Create a new AsyncIntervalTimer-Similar to IntervalTimer
   *
   * @param intervalNsec  The desired number of ns each interval should take.
   * @param maxBacklog    If we can't keep up with the requested rate, reset
   *                      when we fall more than maxBacklog microseconds
   *                      behind.  If the rate does eventually recover, this
   *                      will setting helps reduce the amount of time that the
   *                      timer goes too fast in order to catch up to the
   *                      average rate.
   */
  explicit AsyncIntervalTimer(
      uint64_t intervalNsec,
      std::chrono::microseconds maxBacklog = std::chrono::seconds{3})
      : numTimes_(0),
        intervalNsec_(intervalNsec),
        intervalStart_(),
        maxBacklog_(maxBacklog) {}

  void setIntervalNsec(uint64_t interval) {
    std::unique_lock guard(mutex_);
    intervalNsec_ = interval;
    if (intervalStart_ > std::chrono::steady_clock::time_point{}) {
      intervalStart_ = std::chrono::steady_clock::now();
    }
    numTimes_ = 0;
  }

  /**
   * Change the rate, 0 means run as fast as possible.
   */
  void setRatePerSec(uint64_t rate, uint32_t threads) {
    std::unique_lock guard(mutex_);
    double shift = loadgen::RNG::getReal(0, 1);
    if (rate == 0)
      intervalNsec_ = 0;
    else
      intervalNsec_ = (concurrency::Util::NS_PER_S * threads) / rate;

    // With interval per operation per thread, you know the time gap in which
    // all the threads should start, pick a random spot in that time gap
    if (intervalStart_ > std::chrono::steady_clock::time_point{}) {
      intervalStart_ = std::chrono::steady_clock::now();
    } else {
      intervalStart_ +=
          std::chrono::nanoseconds((uint64_t)(intervalNsec_ * shift));
    }
    numTimes_ = 0;
  }

  /**
   * Start the timer.
   *
   * Call this method before the first interval.
   */
  void start() {
    std::unique_lock guard(mutex_);
    intervalStart_ = std::chrono::steady_clock::now();
  }

  /**
   * Sleep until the next interval should start.
   *
   * @return Returns true during normal operations, and false if the maxBacklog
   *         was hit and the timer has reset the average rate calculation.
   */
  bool sleep() {
    // Go as fast as possible when intervalNsec_ is 0
    if (intervalNsec_ == 0) {
      return true;
    }

    std::chrono::steady_clock::time_point waitUntil, now;
    {
      std::unique_lock guard(mutex_);

      // intervalStart_ is when the just previous interval started (or when it
      // was supposed to start, if we aren't able to keep up with the requested
      // rate).
      //
      // Update it to be when the next interval is supposed to start
      numTimes_++;
      now = std::chrono::steady_clock::now();

      waitUntil =
          intervalStart_ + std::chrono::nanoseconds(intervalNsec_ * numTimes_);

      if (now > waitUntil) {
        // If we can't keep up with the requested rate, we'll keep falling
        // farther and farther behind.
        //
        // If we fall farther than maxBacklog_ behind, reset intervalStart_ to
        // the current time.  This way, if the operations eventually do speed up
        // and we are able to meet the requested rate, we won't exceed it for
        // too long trying to catch up.
        auto delta = now - waitUntil;
        if (delta > maxBacklog_) {
          intervalStart_ = now;
          numTimes_ = 0;
          return false;
        }
        return true;
      }
    }

    std::this_thread::sleep_for(waitUntil - now);

    return true;
  }

 private:
  uint64_t numTimes_;
  uint64_t intervalNsec_;
  std::chrono::steady_clock::time_point intervalStart_;
  std::chrono::microseconds maxBacklog_;
  std::mutex mutex_;
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_TEST_PERF_ASYNCINTERVALTIMER_H_
