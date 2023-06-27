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

#include <thrift/lib/cpp2/transport/core/RequestStateMachine.h>

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>

namespace apache {
namespace thrift {

RequestStateMachine::RequestStateMachine(
    bool includeInRecentRequests,
    AdaptiveConcurrencyController& controller,
    CPUConcurrencyController& cpuController)
    : includeInRecentRequests_(includeInRecentRequests),
      adaptiveConcurrencyController_(controller),
      cpuController_(cpuController) {
  if (includeInRecentRequests_) {
    adaptiveConcurrencyController_.requestStarted(started());
    cpuController_.requestStarted();
  }
}

RequestStateMachine::~RequestStateMachine() {
  if (includeInRecentRequests_ && getStartedProcessing()) {
    adaptiveConcurrencyController_.requestFinished(
        started(), std::chrono::steady_clock::now());
  }
}

[[nodiscard]] bool RequestStateMachine::tryCancel(folly::EventBase* eb) {
  eb->dcheckIsInEventBaseThread();
  if (cancelled_.load(std::memory_order_relaxed)) {
    return false;
  }
  cancelled_.store(true, std::memory_order_relaxed);
  return true;
}

[[nodiscard]] bool RequestStateMachine::tryStartProcessing() {
  if (cancelled_.load(std::memory_order_relaxed) ||
      startProcessingOrQueueTimeout_.exchange(
          true, std::memory_order_relaxed)) {
    return false;
  }
  infoStartedProcessing_.store(true, std::memory_order_relaxed);
  dequeued_.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);
  return true;
}

[[nodiscard]] bool RequestStateMachine::tryStopProcessing() {
  if (!startProcessingOrQueueTimeout_.exchange(
          true, std::memory_order_relaxed)) {
    cpuController_.requestShed();
    dequeued_.store(
        std::chrono::steady_clock::now(), std::memory_order_relaxed);
    return true;
  }
  return false;
}

folly::Optional<std::chrono::milliseconds> RequestStateMachine::queueingTime()
    const {
  using namespace std::chrono;
  if (auto dequeuedTime = dequeued();
      dequeuedTime != steady_clock::time_point::min()) {
    return duration_cast<milliseconds>(dequeuedTime - started());
  }
  return folly::none;
}

} // namespace thrift
} // namespace apache
