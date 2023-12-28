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

#include <cstddef>
#include <cstdint>

#include <folly/Utility.h>
#include <folly/experimental/observer/Observer.h>

namespace apache {
namespace thrift {

/**
 * Tracks memory consumption.
 */
class MemoryTracker : folly::MoveOnly {
 public:
  using Observer = folly::observer::Observer<size_t>;
  using AtomicObserver = folly::observer::ReadMostlyAtomicObserver<size_t>;

  // A basic tracker which does not check for limits
  MemoryTracker()
      : MemoryTracker(
            folly::observer::makeStaticObserver<size_t>(0),
            folly::observer::makeStaticObserver<size_t>(0)) {}

  // A tracker with a memory limit
  explicit MemoryTracker(Observer limitObserver)
      : MemoryTracker(
            std::move(limitObserver),
            folly::observer::makeStaticObserver<size_t>(0)) {}

  // A tracker with a memory limit which is only checked for memory increments
  // of a minimum size.
  MemoryTracker(Observer limitObserver, Observer minObserver)
      : limitObserver_(std::move(limitObserver)),
        minObserver_(std::move(minObserver)) {}

  // Increases the memory consumption counter by `size`. Returns `true` if usage
  // is below limit or `size` is less than minimum.
  bool increment(size_t size) {
    usage_ += size;
    auto limit = *limitObserver_;
    auto min = *minObserver_;
    return !limit || size < min || usage_ <= limit;
  }

  void decrement(size_t size) {
    DCHECK_GE(usage_, size);
    usage_ -= std::min(size, usage_);
  }

  size_t getUsage() { return usage_; }

  size_t getMemLimit() { return *limitObserver_; }

  size_t getMinIncrementSize() { return *minObserver_; }

 private:
  size_t usage_{0};
  AtomicObserver limitObserver_;
  AtomicObserver minObserver_;
};

} // namespace thrift
} // namespace apache
