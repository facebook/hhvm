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

#include <limits>

#include <folly/executors/MeteredExecutor.h>
#include <folly/lang/Align.h>
#include <folly/synchronization/RelaxedAtomic.h>

#include <thrift/lib/cpp2/server/ConcurrencyControllerBase.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

class ParallelConcurrencyController : public ConcurrencyControllerBase {
 public:
  ParallelConcurrencyController(RequestPileInterface& pile, folly::Executor& ex)
      : pile_(pile), executor_(ex) {}

  void setExecutionLimitRequests(uint64_t limit) override;

  using ConcurrencyControllerBase::setObserver;

  uint64_t getExecutionLimitRequests() const override {
    return executionLimit_.load();
  }

  uint64_t requestCount() const override {
    return counters_.load().requestInExecution;
  }

  void onEnqueued() override;

  void onRequestFinished(ServerRequestData&) override;

  void stop() override;

  uint64_t numPendingDequeRequest() const override {
    return counters_.load().pendingDequeCalls;
  }

 private:
  struct Counters {
    constexpr Counters() noexcept = default;
    // Number of requests that are being executed
    // by the executor
    uint32_t requestInExecution{0};
    // Number of requests that sit in the queue waiting
    // to be dequeued by the ConcurrencyController
    uint32_t pendingDequeCalls{0};
  };
  static_assert(std::atomic<Counters>::is_always_lock_free);

  folly::relaxed_atomic<Counters> counters_{};
  folly::relaxed_atomic<uint64_t> executionLimit_{
      std::numeric_limits<uint64_t>::max()};

  bool executorSupportPriority{true};
  RequestPileInterface& pile_;
  folly::Executor& executor_;

  bool trySchedule(bool onEnqueued = false);

  void executeRequest();

  bool isRequestActive(const ServerRequest& req);

  void onExecuteFinish(bool dequeueSuccess);

  std::string describe() const override;
};

} // namespace apache::thrift
