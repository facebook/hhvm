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

#include <folly/Synchronized.h>
#include <folly/experimental/FunctionScheduler.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/experimental/observer/SimpleObservable.h>
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

class CPUConcurrencyController {
 public:
  enum class Mode : uint8_t {
    DISABLED,
    DRY_RUN,
    ENABLED,
  };

  enum class Method : uint8_t {
    CONCURRENCY_LIMITS,
    TOKEN_BUCKET,
  };

  struct Config {
    // Operating mode
    Mode mode = Mode::DISABLED;
    // CPU concurrency enforcement method
    Method method = Method::TOKEN_BUCKET;
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
    uint32_t concurrencyUpperBound = 1 << 16;
    // Don't go below this concurrency limit, ever.
    uint32_t concurrencyLowerBound = 1;

    bool enabled() const;

    // Returns a string representation of the Config mode
    std::string_view modeName() const;

    // Returns a string representation of the Config mode
    std::string_view methodName() const;

    std::string_view concurrencyUnit() const;

    // Returns a string description of the Config
    std::string describe() const;

    // Returns a string representation of the CPU load source
    std::string_view cpuLoadSourceName() const;
  };

  CPUConcurrencyController(
      folly::observer::Observer<Config> config,
      apache::thrift::server::ServerConfigs& serverConfigs,
      apache::thrift::ThriftServerConfig& thriftServerConfig);

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
  void requestShed();

  int64_t getStableEstimate() const {
    return stableEstimate_.load(std::memory_order_relaxed);
  }

  bool isRefractoryPeriod() const {
    return isRefractoryPeriodInternal(config());
  }

  int64_t getLoad() const { return getLoadInternal(config()); }

  bool enabled() const { return (*config_.rlock())->enabled(); }

  std::shared_ptr<const Config> config() const { return config_.copy(); }

  serverdbginfo::CPUConcurrencyControllerDbgInfo getDbgInfo() const;

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

  folly::Synchronized<std::shared_ptr<const Config>> config_;
  std::atomic<bool> enabled_;

  folly::observer::CallbackHandle configSchedulerCallback_;
  folly::observer::SimpleObservable<std::optional<uint32_t>>
      activeRequestsLimit_{std::nullopt};
  folly::observer::SimpleObservable<std::optional<uint32_t>> qpsLimit_{
      std::nullopt};
  uint32_t dryRunLimit_{0};
  apache::thrift::server::ServerConfigs& serverConfigs_;
  apache::thrift::ThriftServerConfig& thriftServerConfig_;

  folly::FunctionScheduler scheduler_;
  folly::Synchronized<std::shared_ptr<EventHandler>> eventHandler_{nullptr};

  std::chrono::steady_clock::time_point lastOverloadStart_{
      std::chrono::steady_clock::now()};

  std::vector<uint32_t> stableConcurrencySamples_;
  std::atomic<int64_t> stableEstimate_{-1};

  // Keeps track of total requests seen. We use this to compute
  // an estimate of RPS since the last time interval. This helps us
  // estimate RPS at target load, as well as whether we should increase
  // while underloaded.
  folly::relaxed_atomic<int32_t> totalRequestCount_{0};
  folly::relaxed_atomic<bool> recentShedRequest_{false};
  std::chrono::steady_clock::time_point lastTotalRequestReset_{
      std::chrono::steady_clock::now()};
};

class BaseThriftServer;

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    folly::observer::Observer<CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig,
    BaseThriftServer*);
} // namespace detail
} // namespace apache::thrift
