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

namespace apache::thrift {

/**
 * Compute the EWMA (Exponential Weighted Moving Average) of a
 * non-evenly-spaced time-series.
 *
 * The traditional formula for evenly-spaced time-series is:
 * x_n = w * x_n-1 + (1 - w) * x
 * x_n: estimation after n datapoint
 * w: learning rate (constant)
 * x: new datapoint
 *
 * But when the time-series is unevenly-spaced, the previous algorithm compute
 * a value which may represent a time window far greater that the expected one.
 * This algorithm used a slight modification of the previous one, it uses a
 * variable value of w, based on the elapsed time between the last two
 * datapoints.
 * We still have `x_n = w * x_n-1 + (1 - w) * x`
 * but w = exp(-dt/tau)
 * dt: time between the last two datapoints
 * tau: constant indicating the "window" of the average (longer window means
 * the average will need longer to converge)
 *
 * A very good source of theoritical knowledge about algorithm that works with
 * uneven time series can be found here:
 * http://www.eckner.com/papers/Algorithms%20for%20Unevenly%20Spaced%20Time%20Series.pdf
 */
template <class Clock>
class Ewma {
 public:
  explicit Ewma(std::chrono::duration<double> window, double initialValue = 0)
      : tau_(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                window / Ewma<Clock>::LN2l_)
                .count()),
        ewma_(initialValue),
        timestamp_(Clock::now()) {}

  void add(double x) {
    const auto now = Clock::now();
    const double dt =
        std::chrono::duration_cast<std::chrono::nanoseconds>(now - timestamp_)
            .count();
    const double w = exp(-dt / tau_);
    ewma_ = ewma_ * w + (1 - w) * x;
    timestamp_ = now;
  }

  double estimate() const { return ewma_; }

  double getWindowNs() const { return tau_; }

 private:
  static inline long double LN2l_ = logl(2);
  const double tau_;
  double ewma_;
  std::chrono::time_point<Clock> timestamp_;
};

} // namespace apache::thrift
