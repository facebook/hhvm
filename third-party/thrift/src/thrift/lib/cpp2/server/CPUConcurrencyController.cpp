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

#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>

#include <algorithm>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    folly::observer::Observer<CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig) {
  return folly::observer::makeStaticObserver(
      CPUConcurrencyController::Config{});
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    int64_t, getCPULoadCounter, std::chrono::milliseconds, CPULoadSource) {
  return -1;
}
} // namespace detail

namespace {

struct ModeInfo {
  std::string_view name;
  std::string_view concurrencyUnit;
};

ModeInfo getModeInfo(CPUConcurrencyController::Mode mode) {
  if (mode == CPUConcurrencyController::Mode::ENABLED_CONCURRENCY_LIMITS) {
    return {"CONCURRENCY_LIMITS", "Active Requests"};
  } else if (mode == CPUConcurrencyController::Mode::ENABLED_TOKEN_BUCKET) {
    return {"TOKEN_BUCKET", "QPS"};
  }

  DCHECK(false);
  return {"UNKNOWN", "UNKNOWN"};
}

} // namespace

CPUConcurrencyController::CPUConcurrencyController(
    folly::observer::Observer<Config> config,
    apache::thrift::server::ServerConfigs& serverConfigs,
    apache::thrift::ThriftServerConfig& thriftServerConfig)
    : config_(std::move(config)),
      serverConfigs_(serverConfigs),
      thriftServerConfig_(thriftServerConfig) {
  thriftServerConfig_.setMaxRequests(
      activeRequestsLimit_.getObserver(),
      apache::thrift::AttributeSource::OVERRIDE_INTERNAL);
  thriftServerConfig_.setMaxQps(
      qpsLimit_.getObserver(),
      apache::thrift::AttributeSource::OVERRIDE_INTERNAL);

  scheduler_.setThreadName("CPUConcurrencyController-loop");
  scheduler_.start();
  configSchedulerCallback_ = config_.getUnderlyingObserver().addCallback(
      [this](folly::observer::Snapshot<Config>) {
        this->cancel();
        this->schedule();
      });
}

CPUConcurrencyController::~CPUConcurrencyController() {
  // Cancel to avoid using CPUConcurrencyController members while its
  // partially destructed
  configSchedulerCallback_.cancel();
  cancel();
}

void CPUConcurrencyController::cycleOnce() {
  if (!enabled()) {
    return;
  }

  auto load = getLoad();
  if (load >= config().cpuTarget) {
    lastOverloadStart_ = std::chrono::steady_clock::now();
    auto lim = this->getLimit();
    auto newLim =
        lim -
        std::max<uint32_t>(
            static_cast<uint32_t>(lim * config().decreaseMultiplier), 1);
    this->setLimit(std::max<uint32_t>(newLim, config().concurrencyLowerBound));
  } else {
    auto currentLimitUsage = this->getLimitUsage();
    if (currentLimitUsage == 0 || load <= 0) {
      return;
    }

    // Estimate stable concurrency only if we haven't been overloaded recently
    // (and thus current concurrency/CPU is not a good indicator).
    if (!isRefractoryPeriod() &&
        config().collectionSampleSize > stableConcurrencySamples_.size()) {
      auto concurrencyEstimate =
          static_cast<double>(currentLimitUsage) / load * config().cpuTarget;

      stableConcurrencySamples_.push_back(
          config().initialEstimateFactor * concurrencyEstimate);
      if (stableConcurrencySamples_.size() >= config().collectionSampleSize) {
        // Take percentile to hopefully account for inaccuracies. We don't
        // need a very accurate value as we will adjust this later in the
        // algorithm.
        //
        // The purpose of stable concurrency estimate is to optimistically
        // avoid causing too much request ingestion during initial overload, and
        // thus causing high tail latencies during initial overload.
        auto pct = stableConcurrencySamples_.begin() +
            static_cast<size_t>(
                       stableConcurrencySamples_.size() *
                       config().initialEstimatePercentile);
        std::nth_element(
            stableConcurrencySamples_.begin(),
            pct,
            stableConcurrencySamples_.end());
        auto result = std::clamp<uint32_t>(
            *pct,
            config().concurrencyLowerBound,
            config().concurrencyUpperBound);
        stableEstimate_.store(result, std::memory_order_relaxed);
        this->setLimit(result);
        return;
      }
    }

    auto lim = this->getLimit();

    // We prevent unbounded increase of limits by only changing when
    // necessary (i.e., we are getting near breaching existing limit).
    bool nearExistingLimit = !config().bumpOnError &&
        (currentLimitUsage >= (1.0 - config().increaseDistanceRatio) * lim);

    // Bump concurrency limit if we saw some load shedding errors recently.
    bool recentLoadShed =
        config().bumpOnError && recentShedRequest_.exchange(false);

    // We try and push the limit back to stable estimate if we are not
    // overloaded as after an overload event the limit will be set aggressively
    // and may cause steady-state load shedding due to bursty traffic.
    bool shouldConvergeStable =
        !isRefractoryPeriod() && lim < getStableEstimate();

    if (nearExistingLimit || recentLoadShed || shouldConvergeStable) {
      auto newLim =
          lim +
          std::max<uint32_t>(
              static_cast<uint32_t>(lim * config().additiveMultiplier), 1);
      this->setLimit(
          std::min<uint32_t>(config().concurrencyUpperBound, newLim));
    }
  }
}

void CPUConcurrencyController::schedule() {
  using time_point = std::chrono::steady_clock::time_point;
  if (!enabled()) {
    return;
  }

  auto modeInfo = getModeInfo(config().mode);
  LOG(INFO) << "Enabling CPUConcurrencyController. Mode: " << modeInfo.name
            << " CPU Target: " << static_cast<int32_t>(this->config().cpuTarget)
            << " Refresh Period (Ms): "
            << this->config().refreshPeriodMs.count()
            << " Concurrency Upper Bound (" << modeInfo.concurrencyUnit
            << "): " << this->config().concurrencyUpperBound;
  this->setLimit(this->config().concurrencyUpperBound);
  scheduler_.addFunctionGenericNextRunTimeFunctor(
      [this] { this->cycleOnce(); },
      [this](time_point, time_point now) {
        return now + this->config().refreshPeriodMs;
      },
      "cpu-shed-loop",
      "cpu-shed-interval",
      this->config().refreshPeriodMs);
}

void CPUConcurrencyController::cancel() {
  scheduler_.cancelAllFunctionsAndWait();
  activeRequestsLimit_.setValue(std::nullopt);
  qpsLimit_.setValue(std::nullopt);
  stableConcurrencySamples_.clear();
  stableEstimate_.exchange(-1);
}

void CPUConcurrencyController::requestStarted() {
  if (!enabled()) {
    return;
  }

  totalRequestCount_ += 1;
}

void CPUConcurrencyController::requestShed() {
  if (!enabled()) {
    return;
  }

  recentShedRequest_.store(true);
}

uint32_t CPUConcurrencyController::getLimit() const {
  uint32_t limit = 0;
  switch (config().mode) {
    case Mode::ENABLED_CONCURRENCY_LIMITS:
      // Using ServiceConfigs instead of ThriftServerConfig, because the value
      // may come from AdaptiveConcurrencyController
      limit = serverConfigs_.getMaxRequests();
      break;
    case Mode::ENABLED_TOKEN_BUCKET:
      limit = thriftServerConfig_.getMaxQps().get();
      break;
    default:
      DCHECK(false);
  }

  // Fallback to concurrency upper bound if no limit is set yet.
  // This is most sensible value until we collect enough samples
  // to estimate a better upper bound;
  return limit ? limit : config().concurrencyUpperBound;
}

void CPUConcurrencyController::setLimit(uint32_t newLimit) {
  switch (config().mode) {
    case Mode::ENABLED_CONCURRENCY_LIMITS:
      activeRequestsLimit_.setValue(newLimit);
      break;
    case Mode::ENABLED_TOKEN_BUCKET:
      qpsLimit_.setValue(newLimit);
      break;
    default:
      DCHECK(false);
  }
}

uint32_t CPUConcurrencyController::getLimitUsage() {
  using namespace std::chrono;
  switch (config().mode) {
    case Mode::ENABLED_CONCURRENCY_LIMITS:
      // Note: estimating concurrency from this is fairly lossy as it's a
      // gauge metric and we can't use techniques to measure it over a duration.
      // We may be able to get much better estimates if we switch to use QPS
      // and a token bucket for rate limiting.
      // TODO: We should exclude fb303 methods.
      return serverConfigs_.getActiveRequests();
    case Mode::ENABLED_TOKEN_BUCKET: {
      auto now = steady_clock::now();
      auto milliSince =
          duration_cast<milliseconds>(now - lastTotalRequestReset_).count();
      if (milliSince == 0) {
        return 0;
      }
      auto totalRequestSince = totalRequestCount_.load();
      totalRequestCount_ = 0;
      lastTotalRequestReset_ = now;
      return totalRequestSince * 1000L / milliSince;
    }
    default:
      DCHECK(false);
      return 0;
  }
}
} // namespace apache::thrift
