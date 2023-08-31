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

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/transport/rocket/flush/FlushManager.h>

namespace apache::thrift::rocket {
void FlushManager::runLoopCallback() noexcept {
  // always reschedule until the end of event loop.
  if (!std::exchange(rescheduled_, true)) {
    evb_.runInLoop(this, true /* thisIteration */);
    return;
  }
  rescheduled_ = false;

  auto cbs = std::move(flushList_);
  while (!cbs.empty()) {
    auto callback = &cbs.front();
    cbs.pop_front();
    callback->runLoopCallback();
  }
  pendingFlushes_ = 0;
}

void FlushManager::timeoutExpired() noexcept {
  if (!isLoopCallbackScheduled()) {
    evb_.runInLoop(this);
  }
}

void FlushManager::resetFlushPolicy() {
  flushPolicy_.reset();
  cancelTimeout();
  timeoutExpired();
}

void FlushManager::enqueueFlush(
    folly::EventBase::LoopCallback& writeLoopCallback) {
  // add write callback to flush list and schedule flush manager callback
  flushList_.push_back(writeLoopCallback);
  pendingFlushes_++;
  if (!isLoopCallbackScheduled() &&
      (!flushPolicy_.has_value() ||
       pendingFlushes_ > flushPolicy_->maxPendingFlushes)) {
    evb_.runInLoop(this);
    cancelTimeout();
  }
  if (flushPolicy_.has_value() && !isLoopCallbackScheduled() &&
      !isScheduled()) {
    evb_.scheduleTimeoutHighRes(this, flushPolicy_->maxFlushLatency);
  }
}

folly::EventBaseLocal<FlushManager>& FlushManager::getEventBaseLocal() {
  static folly::Indestructible<folly::EventBaseLocal<FlushManager>> evbLocal;
  return *evbLocal;
}

} // namespace apache::thrift::rocket
