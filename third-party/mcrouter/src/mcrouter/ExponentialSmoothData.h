/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <cmath>

namespace facebook {
namespace memcache {
namespace mcrouter {

template <size_t WindowSize>
class ExponentialSmoothData {
 public:
  ExponentialSmoothData() = default;
  ExponentialSmoothData(const ExponentialSmoothData& other) {
    currentValue_.store(
        other.currentValue_.load(std::memory_order_relaxed),
        std::memory_order_relaxed);
  }

  static_assert(WindowSize > 0, "WindowSize should be > 0");

  void insertSample(double sample) {
    auto value = currentValue_.load(std::memory_order_relaxed);
    if (!std::isnan(value)) {
      currentValue_.store(
          (sample + (WindowSize - 1) * value) / WindowSize,
          std::memory_order_relaxed);
    } else {
      currentValue_.store(sample, std::memory_order_relaxed);
    }
  }

  double value() const {
    auto value = currentValue_.load(std::memory_order_relaxed);
    return !std::isnan(value) ? value : 0.0;
  }

  bool hasValue() const {
    return !std::isnan(currentValue_.load(std::memory_order_relaxed));
  }

 private:
  std::atomic<double> currentValue_{std::nan("")};
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
