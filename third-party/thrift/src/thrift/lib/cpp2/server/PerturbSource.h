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

#include <folly/coro/AsyncScope.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {

// PerturbSource can be used in the RoundRobinRequestPile as a source to
// periodically reshuffle the buckets that ids are mapped to.
class PerturbSource {
 public:
  PerturbSource(std::chrono::milliseconds interval = std::chrono::seconds(30)) {
    auto task =
        co_withExecutor(folly::getGlobalCPUExecutor(), updatePerturb(interval));
    asyncScope_.add(folly::coro::co_withCancellation(
        cancellation_.getToken(), std::move(task)));
  }

  ~PerturbSource() {
    cancellation_.requestCancellation();
    folly::coro::blockingWait(co_withExecutor(
        folly::getGlobalCPUExecutor(), asyncScope_.joinAsync()));
  }

  std::size_t perturbedId(std::size_t id) {
    return folly::hash::hash_combine(
        id, perturb_.load(std::memory_order_relaxed));
  }

 private:
  folly::coro::Task<void> updatePerturb(std::chrono::milliseconds interval) {
    while (!(co_await folly::coro::co_current_cancellation_token)
                .isCancellationRequested()) {
      perturb_.fetch_add(1, std::memory_order_relaxed);
      co_await folly::coro::sleepReturnEarlyOnCancel(interval);
    }
  }

  folly::coro::AsyncScope asyncScope_;
  folly::CancellationSource cancellation_;
  std::atomic<size_t> perturb_{0};
};

} // namespace apache::thrift

#endif
