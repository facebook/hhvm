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
#include <folly/logging/xlog.h>
#include <folly/synchronization/RelaxedAtomic.h>

#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerBase.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

class ParallelConcurrencyControllerBase : public ConcurrencyControllerBase {
 public:
  explicit ParallelConcurrencyControllerBase(RequestPileInterface& pile)
      : pile_(pile) {}

  void setExecutionLimitRequests(uint64_t limit) override final;

  using ConcurrencyControllerBase::setObserver;

  uint64_t getExecutionLimitRequests() const override final {
    return executionLimit_.load();
  }

  void setQpsLimit(uint64_t) override final {}

  uint64_t getQpsLimit() const override final { return 0; }

  uint64_t requestCount() const override final {
    return counters_.load().requestInExecution;
  }

  void onEnqueued() override final;

  void onRequestFinished(ServerRequestData&) override;

  void stop() override final;

  uint64_t numPendingDequeRequest() const override final {
    return counters_.load().pendingDequeCalls;
  }

 protected:
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

  bool trySchedule(bool onEnqueued = false);
  void executeRequest(std::optional<ServerRequest> req);

  virtual void scheduleOnExecutor() = 0;

  bool isRequestActive(const ServerRequest& req);

  void onExecuteFinish(bool dequeueSuccess);
};

class ParallelConcurrencyController : public ParallelConcurrencyControllerBase {
 public:
  ParallelConcurrencyController(RequestPileInterface& pile, folly::Executor& ex)
      : ParallelConcurrencyControllerBase(pile), executor_(ex) {}
  std::string describe() const override;

 private:
  folly::Executor& executor_;

  void scheduleOnExecutor() override;
};

} // namespace apache::thrift
