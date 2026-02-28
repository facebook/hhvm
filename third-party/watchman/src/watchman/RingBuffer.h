/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/concurrency/container/LockFreeRingBuffer.h>

namespace watchman {

/**
 * Fixed-size, lock-free ring buffer. Used for low-latency event logging.
 */
template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(uint32_t capacity)
      : ring_{capacity}, lastClear_{ring_.currentHead()} {}

  void clear() {
    lastClear_.store(ring_.currentHead(), std::memory_order_release);
  }

  void write(const T& entry) {
    ring_.write(entry);
  }

  std::vector<T> readAll() const {
    auto lastClear = lastClear_.load(std::memory_order_acquire);

    std::vector<T> entries;

    auto head = ring_.currentHead();
    T entry;
    while (head.moveBackward() && head >= lastClear &&
           ring_.tryRead(entry, head)) {
      entries.push_back(std::move(entry));
    }
    std::reverse(entries.begin(), entries.end());
    return entries;
  }

 private:
  RingBuffer(RingBuffer&&) = delete;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(RingBuffer&&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  folly::LockFreeRingBuffer<T> ring_;
  std::atomic<typename folly::LockFreeRingBuffer<T>::Cursor> lastClear_;
};

} // namespace watchman
