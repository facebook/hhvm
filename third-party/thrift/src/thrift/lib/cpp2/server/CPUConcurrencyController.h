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

#include <atomic>
#include <chrono>
#include <variant>

#include <folly/Synchronized.h>
#include <folly/executors/FunctionScheduler.h>
#include <folly/observer/Observer.h>
#include <folly/observer/SimpleObservable.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/thrift/gen-cpp2/serverdbginfo_types.h>

namespace apache::thrift {

enum class CPULoadSource {
  CONTAINER_AND_HOST,
  CONTAINER_ONLY,
  HOST_ONLY,
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    int64_t,
    getCPULoadCounter,
    std::chrono::milliseconds refreshPeriodMs,
    CPULoadSource cpuLoadSource);
} // namespace detail

/**
 * CPUConcurrencyController — adaptive load shedding for Thrift servers.
 *
 * Runs a periodic feedback control loop (every `refreshPeriodMs`, default 50ms)
 * that reads CPU utilization and adjusts a server concurrency/rate limit to
 * keep CPU near a configurable target (`cpuTarget`, default 90%).
 *
 * The controller has three phases:
 *
 * 1. Bootstrap (stable-estimate collection)
 *    While CPU < target and outside the refractory period, the controller
 *    collects `collectionSampleSize` (default 500) estimates of the concurrency
 *    that would yield `cpuTarget` CPU:
 *        estimate = (currentUsage / cpuLoad) * cpuTarget
 *    Once enough samples are collected, the `initialEstimatePercentile`-th
 *    percentile (default 75th) is chosen as the "stable estimate" and applied
 *    as the limit. This gives the controller a reasonable starting point.
 *
 * 2. Overload (CPU >= cpuTarget)
 *    The limit is decreased by `decreaseMultiplier` (default 5%), at least 1
 *    unit, clamped to `concurrencyLowerBound`. The timestamp is recorded for
 *    the refractory-period check.
 *
 * 3. Underload (CPU < cpuTarget, bootstrap complete)
 *    The limit is increased by `additiveMultiplier` (default 5%), at least 1
 *    unit, clamped to `concurrencyUpperBound`, when any of:
 *      - Current usage is near the limit (within `increaseDistanceRatio`), or
 *      - `bumpOnError` is set and a load-shed event occurred recently, or
 *      - We are outside the refractory period and below the stable estimate.
 *
 * Control loop (runs every refreshPeriodMs):
 *
 * // clang-format off
 *   cycleOnce()
 *       |
 *       v
 *   load >= cpuTarget? --YES--> decrease limit, record overload
 *       |NO
 *       v
 *   usage==0 or load<=0? --YES--> (no-op)
 *       |NO
 *       v
 *   bootstrap incomplete? --YES--> collect sample,
 *       |NO                        set stable estimate when full
 *       v
 *   near limit or shed
 *   or below estimate? --YES--> increase limit
 *       |NO
 *       v
 *     (no-op)
 * // clang-format on
 *
 * This produces AIMD-like (Additive Increase, Multiplicative Decrease) behavior
 * with a refractory period (`refractoryPeriodMs`, default 1s) that suppresses
 * sample collection and stable-estimate convergence right after overload.
 *
 * The enforcement method (`Config::method`) selects which server knob is
 * controlled: MAX_REQUESTS, MAX_QPS, CONCURRENCY_LIMIT, or EXECUTION_RATE.
 * In DRY_RUN mode the controller tracks limits without enforcing them.
 *
 * Config is delivered via `folly::observer::Observer<Config>` and can change at
 * runtime. A config change fully resets the controller (clears the stable
 * estimate and re-enters the bootstrap phase).
 */
class CPUConcurrencyController {
 public:
  using LoadFunc =
      std::function<int64_t(std::chrono::milliseconds, CPULoadSource)>;

  enum class Mode : uint8_t {
    DISABLED,
    DRY_RUN,
    ENABLED,
  };

  enum class Method : uint8_t {
    MAX_REQUESTS,
    MAX_QPS,
    CONCURRENCY_LIMIT,
    EXECUTION_RATE,
  };

  struct Config {
    struct UseStaticLimit {};

    // Operating mode
    Mode mode = Mode::DISABLED;
    // CPU concurrency enforcement method
    Method method = Method::MAX_QPS;
    // CPU target in the range [0, 100]
    uint8_t cpuTarget = 90;
    // Source of CPU load metrics (container-only, host-only, or
    // container_and_host = max(conatiner, host))
    CPULoadSource cpuLoadSource = CPULoadSource::CONTAINER_ONLY;

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
    // Use instead of `increaseDistanceRatio`. If bumpOnError = true,
    // then we will bump our concurrency limit only when we are
    // under CPU target AND seen a load shedding event in last interval.
    bool bumpOnError = false;

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
    std::variant<int32_t, UseStaticLimit> concurrencyUpperBound = 1 << 16;
    // Don't go below this concurrency limit, ever.
    uint32_t concurrencyLowerBound = 1;
    // Exponential moving average (EMA) smoothing coefficient for CPU load
    // readings. Controls how much weight is given to the latest CPU sample
    // vs. the historical smoothed value.
    //   smoothedLoad = alpha * rawLoad + (1 - alpha) * previousSmoothedLoad
    // Range [0, 1]:
    //   1.0 = no smoothing (use raw reading, current default behavior)
    //   0.0 = ignore new readings entirely (not useful)
    //   0.1 = heavy smoothing (good for bursty/spiky CPU profiles)
    //   0.5 = moderate smoothing
    double cpuLoadSmoothingCoeff = 1.0;

    bool enabled() const;

    // Returns a string representation of the Config mode
    std::string_view modeName() const;

    // Returns a string representation of the Config mode
    std::string_view methodName() const;

    std::string_view concurrencyUnit() const;

    // Returns a string representation of the CPU load source
    std::string_view cpuLoadSourceName() const;

    // Returns a string representation of the concurrencyUpperBound value
    std::string concurrencyUpperBoundName() const;

    // Returns a string description of the Config
    std::string describe() const;
  };

  CPUConcurrencyController(
      folly::observer::Observer<Config> config,
      apache::thrift::server::ServerConfigs& serverConfigs,
      apache::thrift::ThriftServerConfig& thriftServerConfig,
      std::optional<LoadFunc> loadFunc = std::nullopt);

  ~CPUConcurrencyController();

  class EventHandler {
   public:
    virtual ~EventHandler() = default;
    virtual void configUpdated(std::shared_ptr<const Config>) {}
    virtual void onCycle(int64_t limit, int64_t limitUsage, int64_t load) = 0;
    virtual void limitIncreased() = 0;
    virtual void limitDecreased() = 0;
  };

  void setEventHandler(std::shared_ptr<EventHandler> eventHandler);

  void requestStarted();
  bool requestShed(std::optional<Method> method = std::nullopt);

  int64_t getStableEstimate() const {
    return stableEstimate_.load(std::memory_order_relaxed);
  }

  bool isRefractoryPeriod() const {
    return isRefractoryPeriodInternal(config());
  }

  int64_t getLoad() const { return getLoadInternal(config()); }

  uint32_t getConcurrencyUpperBound() const {
    return getConcurrencyUpperBoundInternal(config());
  }

  bool enabled() const { return (*config_.rlock())->enabled(); }

  std::shared_ptr<const Config> config() const { return config_.copy(); }

  serverdbginfo::CPUConcurrencyControllerDbgInfo getDbgInfo() const;

  /* Allows to set custom LoadFunc for this CPU-CC. If function is not set we'll
   * falback to THRIFT_PLUGGABLE_FUNCTION called getCPULoadCounter() */
  void setLoadFunc(LoadFunc loadFunc) { loadFunc_ = std::move(loadFunc); }

 private:
  void cycleOnce();

  void schedule(std::shared_ptr<const Config> config);
  void cancel();

  bool enabled_fast() const;

  uint32_t getLimit(const std::shared_ptr<const Config>& config) const;
  void setLimit(const std::shared_ptr<const Config>& config, uint32_t newLimit);
  uint32_t getLimitUsage(const std::shared_ptr<const Config>& config);
  bool isRefractoryPeriodInternal(
      const std::shared_ptr<const Config>& config) const;
  int64_t getLoadInternal(const std::shared_ptr<const Config>& config) const;
  uint32_t getConcurrencyUpperBoundInternal(
      const std::shared_ptr<const Config>& config) const;

  folly::Synchronized<std::shared_ptr<const Config>> config_;
  std::atomic<bool> enabled_;
  std::atomic<Method> method_;

  folly::observer::CallbackHandle configSchedulerCallback_;
  folly::observer::SimpleObservable<std::optional<uint32_t>>
      activeRequestsLimit_{std::nullopt};
  folly::observer::SimpleObservable<std::optional<uint32_t>> qpsLimit_{
      std::nullopt};
  folly::observer::SimpleObservable<std::optional<uint32_t>> concurrencyLimit_{
      std::nullopt};
  folly::observer::SimpleObservable<std::optional<uint32_t>> executionRate_{
      std::nullopt};
  uint32_t dryRunLimit_{0};
  apache::thrift::server::ServerConfigs& serverConfigs_;
  apache::thrift::ThriftServerConfig& thriftServerConfig_;

  folly::FunctionScheduler scheduler_;
  folly::Synchronized<std::shared_ptr<EventHandler>> eventHandler_{nullptr};

  folly::relaxed_atomic<std::chrono::steady_clock::time_point>
      lastOverloadStart_{std::chrono::steady_clock::now()};

  std::vector<uint32_t> stableConcurrencySamples_;
  std::atomic<int64_t> stableEstimate_{-1};

  // EMA-smoothed CPU load. Negative value means uninitialized (first sample
  // will seed it at zero). Only accessed from the control loop thread.
  double smoothedLoad_{-1.0};

  // Keeps track of total requests seen. We use this to compute
  // an estimate of RPS since the last time interval. This helps us
  // estimate RPS at target load, as well as whether we should increase
  // while underloaded.
  folly::relaxed_atomic<int32_t> totalRequestCount_{0};
  folly::relaxed_atomic<bool> recentShedRequest_{false};
  std::chrono::steady_clock::time_point lastTotalRequestReset_{
      std::chrono::steady_clock::now()};

  std::optional<LoadFunc> loadFunc_;
};

class ThriftServer;

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    folly::observer::Observer<CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig,
    ThriftServer*);
} // namespace detail
} // namespace apache::thrift
