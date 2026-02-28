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

#include <thrift/lib/cpp2/server/ServiceHealthPoller.h>

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

#include <folly/GLog.h>
#include <folly/Portability.h>
#include <folly/coro/Collect.h>
#include <folly/coro/CurrentExecutor.h>
#include <folly/coro/Sleep.h>
#include <folly/futures/Future.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {

using ServiceHealth = PolledServiceHealth::ServiceHealth;

/* static */ folly::coro::AsyncGenerator<ServiceHealth>
ServiceHealthPoller::poll(
    std::vector<PolledServiceHealth*> handlers,
    folly::observer::Observer<std::chrono::milliseconds> pollingLivenessMs) {
  DCHECK(!handlers.empty());

  const auto computeServiceHealth =
      [handlers = std::move(handlers)]() -> folly::coro::Task<ServiceHealth> {
    std::vector<folly::coro::Task<ServiceHealth>> tasks;
    tasks.reserve(handlers.size());
    for (auto* handler : handlers) {
      tasks.emplace_back(handler->co_getServiceHealth());
    }
    std::vector<folly::Try<ServiceHealth>> reportedHealths =
        co_await folly::coro::collectAllTryRange(std::move(tasks));

    for (const auto& reportedHealth : reportedHealths) {
      if (reportedHealth.hasException()) {
        FB_LOG_EVERY_MS(WARNING, 10'000)
            << "PolledServiceHealth::co_getServiceHealth threw an exception... defaulting to ServiceHealth::ERROR: "
            << folly::exceptionStr(reportedHealth.exception());
      }
      if (reportedHealth.value_or(ServiceHealth::ERROR) ==
          ServiceHealth::ERROR) {
        co_return ServiceHealth::ERROR;
      }
    }
    co_return ServiceHealth::OK;
  };

  folly::observer::AtomicObserver<std::chrono::milliseconds> liveness{
      std::move(pollingLivenessMs)};

  while (true) {
    co_await folly::coro::co_safe_point;
    co_yield co_await computeServiceHealth();
    try {
      co_await folly::coro::sleep(*liveness);
    } catch (const folly::FutureNoTimekeeper&) {
      // Sleep is cancelled on singleton vault destruction. The vault may be
      // (temporarily) destroyed when forking.
      FB_LOG_EVERY_MS(WARNING, 10'000)
          << "Failed to wait between service health computations - No Timekeeper";
    }
  }
}

} // namespace apache::thrift

#endif
