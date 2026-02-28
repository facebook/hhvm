/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

#include <folly/Random.h>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {
namespace mcrouter {

class SlowWarmUpRouteSettings {
 public:
  SlowWarmUpRouteSettings() = default;
  explicit SlowWarmUpRouteSettings(const folly::dynamic& json);
  SlowWarmUpRouteSettings(
      double enableTs,
      double disableTs,
      double startFraction,
      double stepPercent,
      size_t minReqs)
      : enableThreshold_(enableTs),
        disableThreshold_(disableTs),
        start_(startFraction),
        step_(stepPercent),
        minRequests_(minReqs) {}

  double enableThreshold() const {
    return enableThreshold_;
  }
  double disableThreshold() const {
    return disableThreshold_;
  }

  double start() const {
    return start_;
  }
  double step() const {
    return step_;
  }

  size_t minRequests() const {
    return minRequests_;
  }

 private:
  // Threshold to start warming up.
  double enableThreshold_{0.7};
  // Threshold to stop warming up (must be greater than enable threshold).
  double disableThreshold_{0.9};

  // Fraction of requests to send to target when hit rate is 0%.
  double start_{0.1};
  // Increment step for each 1% of hit rate.
  double step_{1.0};

  // Mininum number of requests to start computing hit rate.
  size_t minRequests_{100};
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
