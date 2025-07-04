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
#include <thrift/lib/cpp2/server/RequestExpirationDelegate.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

class ParallelConcurrencyControllerBase : public ConcurrencyControllerBase,
                                          public RequestExpirationDelegate {
 public:
  enum class RequestExecutionMode : uint8_t {
    // Requests are executed in the order they are enqueued, but coroutines are
    // reenqueued to the CPU Executor directly allowing any thread to execute
    // them. This is the default mode.
    Parallel,

    // Requests are executed in the order they are enqueued, but coroutines are
    // reenqueued to the CPU Executor through a SerialExecutor bound to the
    // request. This lows contention on the Executor when using coroutines.
    Serial,
  };

  explicit ParallelConcurrencyControllerBase(
      RequestPileInterface& pile,
      RequestExecutionMode requestExecutionMode =
          RequestExecutionMode::Parallel)
      : requestExecutionMode_(requestExecutionMode), pile_(pile) {}

  void setExecutionLimitRequests(uint64_t limit) final;

  using ConcurrencyControllerBase::setObserver;

  uint64_t getExecutionLimitRequests() const final {
    return executionLimit_.load();
  }

  void setQpsLimit(uint64_t) final {}

  uint64_t getQpsLimit() const final { return 0; }

  uint64_t requestCount() const final {
    return counters_.load().requestInExecution;
  }

  void onEnqueued() final;

  void onRequestFinished(ServerRequestData&) override;

  void stop() final;

  uint64_t numPendingDequeRequest() const final {
    return counters_.load().pendingDequeCalls;
  }

  void processExpiredRequest(ServerRequest&& request) override;

  using ServerRequestLoggingFunction =
      std::function<void(const ServerRequest&)>;

  void setOnExpireFunction(ServerRequestLoggingFunction fn) {
    onExpireFunction_ = std::move(fn);
  }

  void setOnExecuteFunction(ServerRequestLoggingFunction fn) {
    onExecuteFunction_ = std::move(fn);
  }

 protected:
  const RequestExecutionMode requestExecutionMode_;

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

  ServerRequestLoggingFunction onExpireFunction_;
  ServerRequestLoggingFunction onExecuteFunction_;
};

class ParallelConcurrencyController : public ParallelConcurrencyControllerBase {
 public:
  using RequestExecutionMode =
      ParallelConcurrencyControllerBase::RequestExecutionMode;

  /**
   *
   * @param requestExecutor: If set to RequestExecutor::Serial, the requests
   * will be executed using a a folly::SerialExecutor. Consider using this if
   * your code uses coroutines.
   */
  ParallelConcurrencyController(
      RequestPileInterface& pile,
      folly::Executor& ex,
      RequestExecutionMode requestExecutionMode =
          RequestExecutionMode::Parallel)
      : ParallelConcurrencyControllerBase(pile, requestExecutionMode),
        executor_(ex) {}
  std::string describe() const override;

  serverdbginfo::ConcurrencyControllerDbgInfo getDbgInfo() const override;

 private:
  folly::Executor& executor_;

  void scheduleOnExecutor() override;
};

} // namespace apache::thrift
