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
#include <fmt/core.h>
#include <folly/lang/Assume.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    folly::observer::Observer<CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig,
    BaseThriftServer*) {
  return folly::observer::makeStaticObserver(
      CPUConcurrencyController::Config{});
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    int64_t, getCPULoadCounter, std::chrono::milliseconds, CPULoadSource) {
  return -1;
}
} // namespace detail

CPUConcurrencyController::CPUConcurrencyController(
    folly::observer::Observer<Config> config,
    apache::thrift::server::ServerConfigs& serverConfigs,
    apache::thrift::ThriftServerConfig& thriftServerConfig)
    : config_{(*config).getShared()},
      enabled_{(*config_.rlock())->enabled()},
      serverConfigs_(serverConfigs),
      thriftServerConfig_(thriftServerConfig) {
  scheduler_.setThreadName("CPUConcurrencyController-loop");
  scheduler_.start();
  configSchedulerCallback_ = config.addCallback(
      [this](folly::observer::Snapshot<Config> configSnapshot) {
        auto newConfig = configSnapshot.getShared();
        this->config_.withWLock([&newConfig, this](auto& config) {
          config = newConfig;
          this->enabled_.store(config->enabled(), std::memory_order_relaxed);
        });
        this->eventHandler_.withRLock([&newConfig](const auto& eventHandler) {
          if (eventHandler) {
            eventHandler->configUpdated(newConfig);
          }
        });
        this->cancel();
        this->schedule(std::move(newConfig));
      });
}

CPUConcurrencyController::~CPUConcurrencyController() {
  // Cancel to avoid using CPUConcurrencyController members while its
  // partially destructed
  configSchedulerCallback_.cancel();
  cancel();
}

void CPUConcurrencyController::setEventHandler(
    std::shared_ptr<EventHandler> eventHandler) {
  eventHandler_.withWLock(
      [newEventHandler = std::move(eventHandler)](auto& eventHandler) {
        eventHandler = std::move(newEventHandler);
      });
}

void CPUConcurrencyController::requestStarted() {
  if (!enabled_fast()) {
    return;
  }

  totalRequestCount_ += 1;
}

void CPUConcurrencyController::requestShed() {
  if (!enabled_fast()) {
    return;
  }

  recentShedRequest_.store(true);
}

void CPUConcurrencyController::cycleOnce() {
  // Get a snapshot of the current config
  auto config = this->config();
  auto eventHandler = eventHandler_.copy();
  if (!config->enabled()) {
    return;
  }

  auto limit = this->getLimit(config);
  auto currentLimitUsage = this->getLimitUsage(config);
  auto load = getLoadInternal(config);
  if (eventHandler) {
    eventHandler->onCycle(limit, currentLimitUsage, load);
  }
  if (load >= config->cpuTarget) {
    lastOverloadStart_ = std::chrono::steady_clock::now();
    auto newLim =
        limit -
        std::max<uint32_t>(
            static_cast<uint32_t>(limit * config->decreaseMultiplier), 1);
    this->setLimit(
        config, std::max<uint32_t>(newLim, config->concurrencyLowerBound));
    if (eventHandler) {
      eventHandler->limitDecreased();
    }
  } else {
    if (currentLimitUsage == 0 || load <= 0) {
      return;
    }

    // Estimate stable concurrency only if we haven't been overloaded recently
    // (and thus current concurrency/CPU is not a good indicator).
    if (!isRefractoryPeriodInternal(config) &&
        config->collectionSampleSize > stableConcurrencySamples_.size()) {
      auto concurrencyEstimate =
          static_cast<double>(currentLimitUsage) / load * config->cpuTarget;

      stableConcurrencySamples_.push_back(
          config->initialEstimateFactor * concurrencyEstimate);
      if (stableConcurrencySamples_.size() >= config->collectionSampleSize) {
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
                       config->initialEstimatePercentile);
        std::nth_element(
            stableConcurrencySamples_.begin(),
            pct,
            stableConcurrencySamples_.end());
        auto result = std::clamp<uint32_t>(
            *pct, config->concurrencyLowerBound, config->concurrencyUpperBound);
        stableEstimate_.store(result, std::memory_order_relaxed);
        this->setLimit(config, result);
        if (eventHandler) {
          if (result > limit) {
            eventHandler->limitIncreased();
          } else if (result < limit) {
            eventHandler->limitDecreased();
          }
        }
        return;
      }
    }

    // We prevent unbounded increase of limits by only changing when
    // necessary (i.e., we are getting near breaching existing limit).
    bool nearExistingLimit = !config->bumpOnError &&
        (currentLimitUsage >= (1.0 - config->increaseDistanceRatio) * limit);

    // Bump concurrency limit if we saw some load shedding errors recently.
    bool recentLoadShed =
        config->bumpOnError && recentShedRequest_.exchange(false);

    // We try and push the limit back to stable estimate if we are not
    // overloaded as after an overload event the limit will be set aggressively
    // and may cause steady-state load shedding due to bursty traffic.
    bool shouldConvergeStable =
        !isRefractoryPeriodInternal(config) && limit < getStableEstimate();

    if (nearExistingLimit || recentLoadShed || shouldConvergeStable) {
      auto newLim =
          limit +
          std::max<uint32_t>(
              static_cast<uint32_t>(limit * config->additiveMultiplier), 1);
      this->setLimit(
          config, std::min<uint32_t>(config->concurrencyUpperBound, newLim));
      if (eventHandler) {
        eventHandler->limitIncreased();
      }
    }
  }
}

void CPUConcurrencyController::schedule(std::shared_ptr<const Config> config) {
  using time_point = std::chrono::steady_clock::time_point;
  if (!config->enabled()) {
    return;
  }

  VLOG(4) << "Enabling CPUConcurrencyController: " << config->describe();
  thriftServerConfig_.setMaxRequests(
      activeRequestsLimit_.getObserver(),
      apache::thrift::AttributeSource::OVERRIDE_INTERNAL);
  thriftServerConfig_.setMaxQps(
      qpsLimit_.getObserver(),
      apache::thrift::AttributeSource::OVERRIDE_INTERNAL);

  this->setLimit(config, config->concurrencyUpperBound);
  scheduler_.addFunctionGenericNextRunTimeFunctor(
      [this] { this->cycleOnce(); },
      [config](time_point, time_point now) {
        return now + config->refreshPeriodMs;
      },
      "cpu-shed-loop",
      "cpu-shed-interval",
      config->refreshPeriodMs);
}

void CPUConcurrencyController::cancel() {
  scheduler_.cancelAllFunctionsAndWait();
  activeRequestsLimit_.setValue(std::nullopt);
  qpsLimit_.setValue(std::nullopt);
  dryRunLimit_ = 0;
  stableConcurrencySamples_.clear();
  stableEstimate_.exchange(-1);
}

bool CPUConcurrencyController::enabled_fast() const {
  return enabled_.load(std::memory_order_relaxed);
}

uint32_t CPUConcurrencyController::getLimit(
    const std::shared_ptr<const Config>& config) const {
  uint32_t limit = 0;
  if (config->mode == Mode::DRY_RUN) {
    limit = dryRunLimit_;
  } else {
    switch (config->method) {
      case Method::CONCURRENCY_LIMITS:
        // Using ServiceConfigs instead of ThriftServerConfig, because the value
        // may come from AdaptiveConcurrencyController
        limit = serverConfigs_.getMaxRequests();
        break;
      case Method::TOKEN_BUCKET:
        limit = thriftServerConfig_.getMaxQps().get();
        break;
      default:
        DCHECK(false);
    }
  }

  // Fallback to concurrency upper bound if no limit is set yet.
  // This is most sensible value until we collect enough samples
  // to estimate a better upper bound;
  return limit ? limit : config->concurrencyUpperBound;
}

void CPUConcurrencyController::setLimit(
    const std::shared_ptr<const Config>& config, uint32_t newLimit) {
  if (config->mode == Mode::DRY_RUN) {
    dryRunLimit_ = newLimit;
  } else if (config->mode == Mode::ENABLED) {
    switch (config->method) {
      case Method::CONCURRENCY_LIMITS:
        activeRequestsLimit_.setValue(newLimit);
        break;
      case Method::TOKEN_BUCKET:
        qpsLimit_.setValue(newLimit);
        break;
      default:
        DCHECK(false);
    }
  }
}

uint32_t CPUConcurrencyController::getLimitUsage(
    const std::shared_ptr<const Config>& config) {
  using namespace std::chrono;
  switch (config->method) {
    case Method::CONCURRENCY_LIMITS:
      // Note: estimating concurrency from this is fairly lossy as it's a
      // gauge metric and we can't use techniques to measure it over a duration.
      // We may be able to get much better estimates if we switch to use QPS
      // and a token bucket for rate limiting.
      // TODO: We should exclude fb303 methods.
      return serverConfigs_.getActiveRequests();
    case Method::TOKEN_BUCKET: {
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

bool CPUConcurrencyController::isRefractoryPeriodInternal(
    const std::shared_ptr<const Config>& config) const {
  return (std::chrono::steady_clock::now() - lastOverloadStart_) <=
      std::chrono::milliseconds(config->refractoryPeriodMs);
}

int64_t CPUConcurrencyController::getLoadInternal(
    const std::shared_ptr<const Config>& config) const {
  return std::clamp<int64_t>(
      detail::getCPULoadCounter(config->refreshPeriodMs, config->cpuLoadSource),
      0,
      100);
}

/**
 * CPUConcurrencyController Config helper methods
 */
bool CPUConcurrencyController::Config::enabled() const {
  return mode != Mode::DISABLED;
}

std::string_view CPUConcurrencyController::Config::modeName() const {
  switch (mode) {
    case Mode::DISABLED:
      return "DISABLED";
    case Mode::DRY_RUN:
      return "DRY_RUN";
    case Mode::ENABLED:
      return "ENABLED";
  }
  folly::assume_unreachable();
}

std::string_view CPUConcurrencyController::Config::methodName() const {
  switch (method) {
    case Method::CONCURRENCY_LIMITS:
      return "CONCURRENCY_LIMITS";
    case Method::TOKEN_BUCKET:
      return "TOKEN_BUCKET";
  }
  folly::assume_unreachable();
}

std::string_view CPUConcurrencyController::Config::concurrencyUnit() const {
  switch (method) {
    case Method::CONCURRENCY_LIMITS:
      return "Active Requests";
    case Method::TOKEN_BUCKET:
      return "QPS";
  }
  folly::assume_unreachable();
}

std::string CPUConcurrencyController::Config::describe() const {
  return fmt::format(
      "CPUConcurrencyController Config: "
      "Mode: {}, "
      "Method: {}, "
      "CPU Target: {}, "
      "Refresh Period (ms): {}, "
      "Concurrency Upper Bound({}): {}",
      modeName(),
      methodName(),
      cpuTarget,
      refreshPeriodMs.count(),
      concurrencyUnit(),
      concurrencyUpperBound);
}

} // namespace apache::thrift
