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

#include <thrift/lib/cpp2/util/Ewma.h>

namespace apache::thrift {

/**
 * Compute a rate (in tick per second) based on an EWMA of the inter-arrival
 * time of the ticks.
 * i.e. if the inter-arrival time is 100ms, it means the rate is 10 tick/s.
 */
template <class Clock>
class EwmaRate {
 public:
  EwmaRate(std::chrono::duration<double> window) noexcept
      : ewma_(window, 1e9), lastArrivalTime_(Clock::now()) {}

  /**
   * Indicate that a tick happened
   */
  void tick() {
    auto now = Clock::now();
    double delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       now - lastArrivalTime_)
                       .count();
    lastArrivalTime_ = now;
    ewma_.add(delta);
  }

  /**
   * Return the rate of ticks in second as a double.
   * It uses the current absence of tick in [lastArrivalTime_, now] as
   * a parameter to compute more accurate value. (i.e. if we don't see a tick
   * for a long time, the reported rate will still decrease)
   */
  double rate() const {
    auto now = Clock::now();
    double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    now - lastArrivalTime_)
                    .count();
    double ewma = ewma_.estimate();
    if (dt > ewma) { // if too much time has passed, we decay the estimation
      double tau = ewma_.getWindowNs();
      double w = exp(-dt / tau);
      ewma = ewma * w + (1 - w) * dt;
    }

    return 1e9 / ewma; // tick/sec
  }

 private:
  Ewma<Clock> ewma_;
  std::chrono::time_point<Clock> lastArrivalTime_;
};

} // namespace apache::thrift
