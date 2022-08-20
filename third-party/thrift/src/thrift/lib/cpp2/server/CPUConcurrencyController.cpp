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
    int64_t, getCPULoadCounter, std::chrono::milliseconds, bool) {
  return -1;
}
} // namespace detail

void CPUConcurrencyController::cycleOnce() {
  if (config().mode == Mode::DISABLED) {
    return;
  }

  auto load = getLoad();
  if (load >= config().cpuTarget) {
    lastOverloadStart_ = std::chrono::steady_clock::now();
    auto lim = this->getLimit();
    auto newLim =
        lim -
        std::max<int64_t>(
            static_cast<int64_t>(lim * config().decreaseMultiplier), 1);
    this->setLimit(std::max<int64_t>(newLim, config().concurrencyLowerBound));
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
        auto result = std::clamp<int64_t>(
            *pct,
            config().concurrencyLowerBound,
            config().concurrencyUpperBound);
        stableEstimate_.store(result, std::memory_order_relaxed);
        this->setLimit(result);
        return;
      }
    }

    // We prevent unbounded increase of limits by only changing when
    // necessary (i.e., we are getting near breaching existing limit). We try
    // and push the limit back to stable estimate if we are not overloaded as
    // after an overload event the limit will be set aggressively and may cause
    // steady-state load shedding due to bursty traffic.
    auto lim = this->getLimit();
    bool nearExistingLimit =
        currentLimitUsage >= (1.0 - config().increaseDistanceRatio) * lim;
    bool shouldConvergeStable =
        !isRefractoryPeriod() && lim < getStableEstimate();
    if (nearExistingLimit || shouldConvergeStable) {
      auto newLim =
          lim +
          std::max<int64_t>(
              static_cast<int64_t>(lim * config().additiveMultiplier), 1);
      this->setLimit(std::min<int64_t>(config().concurrencyUpperBound, newLim));
    }
  }
}

void CPUConcurrencyController::schedule() {
  using time_point = std::chrono::steady_clock::time_point;
  if (config().mode == Mode::DISABLED) {
    return;
  }

  LOG(INFO) << "Enabling CPUConcurrencyController. CPU Target: "
            << static_cast<int32_t>(this->config().cpuTarget)
            << " Refresh Period Ms: " << this->config().refreshPeriodMs.count();
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
  stableConcurrencySamples_.clear();
  stableEstimate_.exchange(-1);
}

void CPUConcurrencyController::requestStarted() {
  if (config().mode == Mode::DISABLED) {
    return;
  }

  totalRequestCount_ += 1;
}

uint32_t CPUConcurrencyController::getLimit() const {
  uint32_t limit = 0;
  switch (config().mode) {
    case Mode::ENABLED_CONCURRENCY_LIMITS:
      limit = serverConfigs_.getMaxRequests();
      break;
    case Mode::ENABLED_TOKEN_BUCKET:
      limit = serverConfigs_.getMaxQps();
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
      serverConfigs_.setMaxRequests(newLimit);
      break;
    case Mode::ENABLED_TOKEN_BUCKET:
      serverConfigs_.setMaxQps(newLimit);
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
