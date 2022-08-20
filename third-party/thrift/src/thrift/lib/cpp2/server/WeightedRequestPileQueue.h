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
#include <optional>
#include <variant>

#include <folly/concurrency/UnboundedQueue.h>
#include <folly/synchronization/RelaxedAtomic.h>

namespace apache::thrift::server {

enum class RequestPileQueueAcceptResult {
  Success = 1,
  FirstSuccess = 2,
  Failed = 3,
};

// This control block can be applied in case
// where we would like to contrain on CPU or Memomry
class OneDimensionalControlBlock {
 public:
  using Weights = uint64_t;
  constexpr static Weights defaultWeights = 1;

  // weight should always be non-zero
  RequestPileQueueAcceptResult accept(Weights weight = defaultWeights);

  void setLimit(Weights limit) { limit_.store(limit); }

  void onDequeue(Weights limit) { counter_ -= limit; }

 private:
  folly::relaxed_atomic<uint64_t> counter_{0};
  folly::relaxed_atomic<uint64_t> limit_{std::numeric_limits<uint64_t>::max()};
};

// This control block can be applied in case
// where we would like to constrain on both CPU and memory
class TwoDimensionalControlBlock {
 public:
  struct Weights {
    uint64_t first;
    uint64_t second;

    friend bool operator<(const Weights& w1, const Weights& w2) {
      return w1.first < w2.first || w1.second < w2.second;
    }
  };

  constexpr static Weights defaultWeights = {1, 1};

  // weights should always be non-zero values for
  // both dimensions
  RequestPileQueueAcceptResult accept(Weights weights = defaultWeights);

  void setLimit(Weights limits) {
    auto [l1, l2] = limits;
    limit1_.store(l1);
    limit2_.store(l2);
  }

  void onDequeue(Weights limits) {
    auto [l1, l2] = limits;
    counter1_ -= l1;
    counter2_ -= l2;
  }

 private:
  folly::relaxed_atomic<uint64_t> counter1_{0};
  folly::relaxed_atomic<uint64_t> counter2_{0};

  folly::relaxed_atomic<uint64_t> limit1_{std::numeric_limits<uint64_t>::max()};
  folly::relaxed_atomic<uint64_t> limit2_{std::numeric_limits<uint64_t>::max()};
};

// WeightedRequestPileQueue
// This is a building block for weighted RequestPile.
// This can be a replacement for what we are currently using
// in RoundRobinRequestPile by defaulting the weight to 1.
// It's an UnboundedQueue augmented with a control block with provide
// weighted constraints. Different control block plugins can provide
// different features (we here provide 2 control blocks, one for 1D weights
// one for 2D). In the scenario where no dequeue is called until the queue
// is not empty, it also provides an API that tells the caller whether an
// enqueue is the first enqueue before which the queue is empty.

// Usage:
//
// This is a normal MPMC queue
// WeightedRequestPileQueue<int> queue;
//
// This is a MPMC queue with control enabled
// WeightedRequestPileQueue<int, true> queue;
// // weight is defaulted to 1
// queue.enqueue(1);
// // set weight sum limit to 100
// queue.setLimit(100);
// // enqueue an element with weighht == 200
// // should reject
// queue.enqueue(1, 200);
template <
    typename T,
    bool EnableControl = false,
    typename ControlBlock = OneDimensionalControlBlock>
class WeightedRequestPileQueue {
 public:
  using Weights = typename ControlBlock::Weights;

  struct Item {
    T data;
    Weights weight;
  };

  enum class DequeueRejReason {
    NoElement = 1,
    OverCapacity = 2,
  };

  using DequeueResult = std::variant<T, DequeueRejReason>;

  // thread-safe
  // should only be used when EnableControl is true
  template <
      bool Q = EnableControl,
      typename = typename std::enable_if<Q, void>::type>
  void setLimit(Weights limits);

  RequestPileQueueAcceptResult enqueue(T&& elem);

  // should only be used when EnableControl is true
  template <
      bool Q = EnableControl,
      typename = typename std::enable_if<Q, void>::type>
  RequestPileQueueAcceptResult enqueue(T&& elem, Weights weights);

  // returns an estimate of number of elements in the queue
  size_t size();

  // This version of dequeue shouldn't be called
  // following any call of tryDequeueWithCapacity(Weights capacity)
  std::optional<T> tryDequeue() noexcept;

  // This is not a lock-free method
  // a std::mutex is used
  // The lock can be removed in the future
  // if the queue is MPMC
  template <
      bool Q = EnableControl,
      typename = typename std::enable_if<Q, void>::type>
  DequeueResult tryDequeueWithCapacity(Weights capacity) noexcept;

 private:
  using Queue = folly::UMPMCQueue<
      Item,
      /* MayBlock  */ false,
      /* log2(SegmentSize=1024) */ 10>;

  Queue queue_;
  ControlBlock controlBlock_{};

  // for dequeueWithCapacity
  bool dequeueWithCapacityCalled_{false};
  std::optional<Item> firstElementHolder_;
  std::mutex dequeueLock_;
};

} // namespace apache::thrift::server

#include <thrift/lib/cpp2/server/WeightedRequestPileQueue-inl.h>
