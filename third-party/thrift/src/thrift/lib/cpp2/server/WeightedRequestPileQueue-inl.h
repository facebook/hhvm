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

#include <mutex>

#include <thrift/lib/cpp2/server/WeightedRequestPileQueue.h>

namespace apache::thrift::server {

template <typename T, bool EnableControl, typename ControlBlock>
template <bool Q, typename>
void WeightedRequestPileQueue<T, EnableControl, ControlBlock>::setLimit(
    Weights limits) {
  controlBlock_.setLimit(limits);
}

template <typename T, bool EnableControl, typename ControlBlock>
RequestPileQueueAcceptResult
WeightedRequestPileQueue<T, EnableControl, ControlBlock>::enqueue(T&& elem) {
  if constexpr (!EnableControl) {
    queue_.enqueue({std::forward<T>(elem), ControlBlock::defaultWeights});
    return RequestPileQueueAcceptResult::Success;
  } else {
    return enqueue(std::forward<T>(elem), ControlBlock::defaultWeights);
  }
}

template <typename T, bool EnableControl, typename ControlBlock>
template <bool Q, typename>
RequestPileQueueAcceptResult
WeightedRequestPileQueue<T, EnableControl, ControlBlock>::enqueue(
    T&& elem, Weights weights) {
  auto res = controlBlock_.accept(weights);
  if (res == RequestPileQueueAcceptResult::Failed) {
    return RequestPileQueueAcceptResult::Failed;
  }

  queue_.enqueue({std::forward<T>(elem), weights});

  return res;
}

template <typename T, bool EnableControl, typename ControlBlock>
std::optional<T> WeightedRequestPileQueue<T, EnableControl, ControlBlock>::
    tryDequeue() noexcept {
  if (auto res = queue_.try_dequeue()) {
    if constexpr (EnableControl) {
      controlBlock_.onDequeue(res->weight);
    }
    return std::move(res->data);
  }
  return std::nullopt;
}

template <typename T, bool EnableControl, typename ControlBlock>
template <bool Q, typename>
typename WeightedRequestPileQueue<T, EnableControl, ControlBlock>::DequeueResult
WeightedRequestPileQueue<T, EnableControl, ControlBlock>::
    tryDequeueWithCapacity(Weights capacity) noexcept {
  std::lock_guard g(dequeueLock_);
  if (firstElementHolder_) {
    if (capacity < firstElementHolder_->weight) {
      return DequeueRejReason::OverCapacity;
    } else {
      auto data = std::move(firstElementHolder_->data);
      firstElementHolder_ = std::nullopt;
      return data;
    }
  }

  if (auto res = queue_.try_dequeue()) {
    if (capacity < res->weight) {
      firstElementHolder_ = std::move(res.value());
      return DequeueRejReason::OverCapacity;
    } else {
      return res->data;
    }
  } else {
    return DequeueRejReason::NoElement;
  }
}

template <typename T, bool EnableControl, typename ControlBlock>
size_t WeightedRequestPileQueue<T, EnableControl, ControlBlock>::size() {
  return queue_.size() + (firstElementHolder_.has_value() ? 1 : 0);
}

} // namespace apache::thrift::server
