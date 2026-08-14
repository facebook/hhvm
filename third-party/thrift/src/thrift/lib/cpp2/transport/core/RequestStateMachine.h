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
#include <cstdint>

#include <folly/Optional.h>
#include <folly/Portability.h>

namespace folly {
class EventBase;
}

namespace apache::thrift {

class AdaptiveConcurrencyController;
class CPUConcurrencyController;

// Why a request stopped requiring processing. `NotTerminated` means no
// terminal path has won yet.
enum class RequestTerminationCause : std::uint8_t {
  NotTerminated = 0,
  Unknown,
  TaskTimeout,
  QueueTimeout,
  RequestFinished,
};

class RequestStateMachine {
 public:
  RequestStateMachine(
      bool includeInRecentRequests,
      AdaptiveConcurrencyController& controller,
      CPUConcurrencyController* cpuController);

  ~RequestStateMachine();

  // Returns true if the request has not been terminated (via tryTerminate())
  //
  // Note: using this method from a thread other than that of eventBase_
  //       presents a data race condition. As such, the isActive() API returning
  //       true should be considered a weak promise that a request is active
  //       and should not be relied upon for the purposes of synchronization.
  bool isActive() const {
    return getTerminationCause() == RequestTerminationCause::NotTerminated;
  }

  // Why the request stopped requiring processing, or `NotTerminated` if it has
  // not been terminated yet.
  RequestTerminationCause getTerminationCause() const {
    return terminationCause_.load(std::memory_order_relaxed);
  }

  // Terminate a request when the caller cannot classify the cause.
  [[nodiscard]] bool tryTerminate(folly::EventBase* eb);

  // Instruct whether request no longer requires processing. The first caller
  // to win permanently records `cause`, which must not be `NotTerminated`.
  // This API may only be called from IO worker thread of the request.
  // @return: whether this call terminated the request. False means the request
  // had already been terminated before the call.
  // Suggested usages of the API:
  // * queue/task timeout has sent load shedding response, and no further
  //   response is needed
  // * client has closed its connection and does not expect a response
  [[nodiscard]] bool tryTerminate(
      folly::EventBase* eb, RequestTerminationCause cause);

  // The tryStartProcessing() API is used to mark the request as started
  // processing. This method is ultimately called by request processors. A
  // return value of true indicates that request processing can begin. A return
  // value of false indicates that request processing should be aborted.
  [[nodiscard]] bool tryStartProcessing();

  // The tryStopProcessing() API is used to mark the request as stopped by queue
  // timeout callbacks. A return value of true indicates that queue timeout
  // handling can begin. A return value of false indicates that queue timeout
  // handling should be aborted.
  [[nodiscard]] bool tryStopProcessing();

  bool getStartedProcessing() const {
    return infoStartedProcessing_.load(std::memory_order_relaxed);
  }

  [[nodiscard]] bool includeInRecentRequests() const {
    return includeInRecentRequests_;
  }

  std::chrono::steady_clock::time_point started() const { return started_; }

  std::chrono::steady_clock::time_point dequeued() const {
    return dequeued_.load(std::memory_order_relaxed);
  }

  folly::Optional<std::chrono::milliseconds> queueingTime() const;

 private:
  std::atomic<bool> startProcessingOrQueueTimeout_{false};
  std::atomic<RequestTerminationCause> terminationCause_{
      RequestTerminationCause::NotTerminated};

  std::atomic<bool> infoStartedProcessing_{false};
  const bool includeInRecentRequests_;
  const std::chrono::steady_clock::time_point started_{
      std::chrono::steady_clock::now()};
  std::atomic<std::chrono::steady_clock::time_point> dequeued_{
      std::chrono::steady_clock::time_point::min()};
  AdaptiveConcurrencyController& adaptiveConcurrencyController_;
  CPUConcurrencyController* cpuController_;
};

} // namespace apache::thrift
