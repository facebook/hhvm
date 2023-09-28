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
#include <folly/Function.h>
#include <folly/experimental/observer/Observer.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

namespace apache {
namespace thrift {

using Clock = std::chrono::steady_clock;

// Adaptive concurrency controller implements an algorithm that uses
// observed latencies to protect servers against overload. The algorithm was
// initially published by Netflix here (notice that original whitepaper is
// fairly light on details) :
// https://netflixtechblog.medium.com/performance-under-load-3e6fa9a60581
// Based on the above, an implementation by Tony Allen, a Lyft engineer at the
// time, was put into Envoy - a sidecar for HTTP servers. Tony has put
// together a much more details explanation of the approach here:
// https://www.youtube.com/watch?v=CQvmSXlnyeQ
//
// Basic description of the algorithm:
// The algorithm consist of two loosely coupled components:
// 1. Determines service's target latency (targetRtt)
// 2. Adjusts allowed concurrency so that observed latency (sampledRtt) is not
// exceeding the target latency.
//
// The latter is a continious feedback loop based mechanism that uses current
// concurrency limit together with observed sampled latency to calculate new
// concurrency roughly following the following gradient based formula:
//     gradient = targetRtt / sampledRtt
//     concurrency = concurrency * gradient
//     concurrency = concurrency + headroom
//
// The headroom value allows for request bursts and is also the driving factor
// behind increasing the concurrency limit when sampledRtt is below or near
// targetRtt. The value must be present in the calculation, since it forces the
// concurrency limit to increase until there is a deviation from the targetRtt
// latency. In its absence, the concurrency limit could remain stagnant at an
// unnecessarily small value if sampleRTT ~= targetRtt. Therefore, the headroom
// value is unconfigurable and is set to the square-root of the new limit.
//
// To compute either sampledRtt or targetRtt, the controller relies on sampling
// latencies of pre-defined number of requests and computing the target as their
// percentile (default is p95 but this is configurable). However, to compute
// targetRtt the concurrency of the server is first set to pre-defined value of
// *minConncurrency*. The sampling windows for sampledRtt and targetRtt never
// overlap and the algorithm makes sure of that.
//
// To sample latency, the controller periodically sets a timepoint, and starts
// collecting latencies of requests that started their execution after that
// time. Once pre-defined number of samples is collected, the collection window
// is closed, data is recaculated and next collection window is scheduled.
class AdaptiveConcurrencyController {
 public:
  class Config {
   public:
    bool isEnabled() const { return minConcurrency != 0; }

    // adaptive concurrency is enabled if minConcurrency is != 0
    size_t minConcurrency = 0;

    // ratio to use to buffer targetRtt from observed latency
    double targetRttFactor = 2.0;
    // alternatively, a fixed targetRtt can be configured. Takes
    // effect if not 0.
    std::chrono::milliseconds targetRttFixed{};
    // minimum (floor) for the targetRtt value. If the computed
    // ideal rtt latency is below this, targetRtt will be overridden
    // to this value.
    std::chrono::milliseconds minTargetRtt{};
    // percentile to be used for computing the targetRtt from the sampled
    // set of request latencies. NOTE that this has no effect if
    // targetRttFixed is non-zero.
    double targetRttPercentile = 0.95;

    double recalcPeriodJitter = 0.5;

    std::chrono::milliseconds samplingInterval{500};
    std::chrono::milliseconds recalcInterval{5 * 60 * 1000};
  };

  explicit AdaptiveConcurrencyController(
      folly::observer::Observer<Config> config,
      folly::observer::Observer<uint32_t> maxRequestsLimit,
      apache::thrift::ThriftServerConfig& thriftServerConfig);

  // server should call this methods for requests
  // it wants to use as input to the algorithm
  void requestStarted(Clock::time_point start);
  void requestFinished(Clock::time_point start, Clock::time_point finish);

  bool enabled() const { return getMaxRequests(); }
  // concurrency that should be used by the server, 0 if disabled
  size_t getMaxRequests() const;
  size_t getOriginalMaxRequests() const;

  // info helpers, to use in monitoring
  std::chrono::microseconds targetRtt() const;
  std::chrono::microseconds sampledRtt() const;
  std::chrono::microseconds minTargetRtt() const;

  size_t getMinConcurrency() const;
  // Similar to getMaxRequests() except that it wont drop
  // to minConcurrency when controller is in the rtt calibration state.
  size_t getConcurrency() const;

  void setConfigUpdateCallback(
      std::function<void(folly::observer::Snapshot<Config>)> callback);

 private:
  // testing helpers
  friend class AdaptiveConcurrencyTestHelper;
  Clock::time_point samplingPeriodStart() const;
  void samplingPeriodStart(Clock::time_point v);
  Clock::time_point rttRecalcStart() const;
  Clock::time_point nextRttRecalcStart() const;
  void nextRttRecalcStart(Clock::time_point);

  void recalculate();
  const Config& config() const { return **config_; }
  bool inSamplingPeriod(std::chrono::steady_clock::time_point) const;

  constexpr static size_t nSamples = 200;
  using Duration = Clock::duration;
  std::array<Duration, nSamples> latencySamples_;

  // The following two atomics are used in RMW operations
  // Ensure these are on different cachelines to reduce contention
  std::atomic<size_t> latencySamplesIdx_{0};
  alignas(folly::cacheline_align_v) std::atomic<size_t> latencySamplesCnt_{0};

  // rttRecalcStart_ is used to indicate the point in time when the next RTT
  // recalculation should occur.
  std::atomic<Clock::time_point> rttRecalcStart_{Clock::time_point{}};

  std::atomic<Clock::time_point> nextRttRecalcStart_{Clock::time_point{}};
  std::atomic<Clock::time_point> samplingPeriodStart_{Clock::time_point{}};

  folly::observer::Observer<Config> config_;
  folly::observer::Observer<uint32_t> maxRequestsLimit_;

  folly::observer::CallbackHandle enablingCallback_;
  std::function<void(folly::observer::Snapshot<Config>)> configUpdateCallback_;

  folly::observer::SimpleObservable<std::optional<uint32_t>> maxRequestsOb_{
      std::nullopt};
  apache::thrift::ThriftServerConfig& thriftServerConfig_;

  std::atomic<Duration> targetRtt_{Duration{}};
  std::atomic<Duration> minRtt_{Duration{}};
  std::atomic<Duration> sampledRtt_{Duration{}};
  // the following concurrency limits are related but are not always
  // identical. Both keep track of the concurrency limit for the server.
  // maxRequests_, however, will be set to minRequests during rtt recalibration
  // and will be restored back concurrencyLimit_.
  std::atomic<size_t> concurrencyLimit_{0};
  std::atomic<size_t> maxRequests_{0};
  size_t originalMaxRequestsLimit_;
};

namespace detail {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    folly::observer::Observer<AdaptiveConcurrencyController::Config>,
    makeAdaptiveConcurrencyConfig);

} // namespace detail

} // namespace thrift
} // namespace apache
