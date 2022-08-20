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

#include <folly/experimental/FunctionScheduler.h>
#include <folly/experimental/observer/Observer.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    int64_t,
    getCPULoadCounter,
    std::chrono::milliseconds refreshPeriodMs,
    bool useHostLevelMetrics);
} // namespace detail

class CPUConcurrencyController {
 public:
  enum class Mode {
    DISABLED,
    ENABLED_CONCURRENCY_LIMITS,
    ENABLED_TOKEN_BUCKET
  };
  struct Config {
    Mode mode = Mode::DISABLED;
    // CPU target in the range [0, 100]
    uint8_t cpuTarget = 90;
    // Whether or not we will use host level metrics versus container metrics.
    bool useHostLevelMetrics = false;

    // How often to cycle the algorithm and update the current
    // concurrency limit.
    std::chrono::milliseconds refreshPeriodMs{50};
    // Increase current max request limit by X% when we
    // are underloaded and close to utilizing it.
    double additiveMultiplier = 0.05;
    // Decrease current max request limit by X% when we
    // are overloaded (above cpu target).
    double decreaseMultiplier = 0.05;
    // If we are within X% of our concurrency limit, then we will
    // adjust it by `additiveMultiplier`.
    double increaseDistanceRatio = 0.20;

    // How long to wait after an overload event to ensure we aren't
    // estimating concurrency improperly.
    std::chrono::milliseconds refractoryPeriodMs{1000};
    // What factor to adjust our initial concurrency estimate by
    double initialEstimateFactor = 1.0;
    // What percentile of initial samples do we consider our estimate
    double initialEstimatePercentile = 0.75;
    // How many samples to collect for the initial estimate
    uint32_t collectionSampleSize = 500;
    // Don't go above this concurrency limit, ever.
    int64_t concurrencyUpperBound = 1 << 16;
    // Don't go below this concurrency limit, ever.
    int64_t concurrencyLowerBound = 1;
  };

  CPUConcurrencyController(
      folly::observer::Observer<Config> config,
      apache::thrift::server::ServerConfigs& serverConfigs)
      : config_(std::move(config)), serverConfigs_(serverConfigs) {
    scheduler_.setThreadName("CPUConcurrencyController-loop");
    scheduler_.start();
    configSchedulerCallback_ =
        config_.addCallback([this](folly::observer::Snapshot<Config>) {
          this->cancel();
          this->schedule();
        });
  }

  ~CPUConcurrencyController() {
    // Cancel to avoid using CPUConcurrencyController members while its
    // partially destructed
    configSchedulerCallback_.cancel();
    cancel();
  }

  void requestStarted();

  int64_t getStableEstimate() const {
    return stableEstimate_.load(std::memory_order_relaxed);
  }

  bool isRefractoryPeriod() const {
    return (std::chrono::steady_clock::now() - lastOverloadStart_) <=
        std::chrono::milliseconds(config().refractoryPeriodMs);
  }

  int64_t getLoad() const {
    return std::clamp<int64_t>(
        detail::getCPULoadCounter(
            config().refreshPeriodMs, config().useHostLevelMetrics),
        0,
        100);
  }

 private:
  void cycleOnce();

  void schedule();
  void cancel();

  uint32_t getLimit() const;
  void setLimit(uint32_t newLimit);
  uint32_t getLimitUsage();

  const Config& config() const { return **config_; }

  folly::observer::Observer<Config> config_;
  folly::observer::CallbackHandle configSchedulerCallback_;
  apache::thrift::server::ServerConfigs& serverConfigs_;

  folly::FunctionScheduler scheduler_;

  std::chrono::steady_clock::time_point lastOverloadStart_{
      std::chrono::steady_clock::now()};

  std::vector<int64_t> stableConcurrencySamples_;
  std::atomic<int64_t> stableEstimate_{-1};

  // Keeps track of total requests seen. We use this to compute
  // an estimate of RPS since the last time interval. This helps us
  // estimate RPS at target load, as well as whether we should increase
  // while underloaded.
  folly::relaxed_atomic<int32_t> totalRequestCount_{0};
  std::chrono::steady_clock::time_point lastTotalRequestReset_{
      std::chrono::steady_clock::now()};
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    folly::observer::Observer<CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig);
} // namespace detail
} // namespace apache::thrift
