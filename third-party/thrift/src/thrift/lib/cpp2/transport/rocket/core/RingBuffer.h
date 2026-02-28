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
#include <folly/Likely.h>
#include <folly/lang/Align.h>

namespace apache::thrift::rocket {

/**
 * A specialized ring buffer implementation optimized for high-performance
 * transport code in Thrift Rocket RPC. It is not thread-safe and should only be
 * used by a single thread.
 *
 * Unlike std::queue or std::deque, this RingBuffer:
 * - Has a fixed capacity that must be a power of 2
 * - Provides no-throw guarantees for critical operations
 * - Offers specialized batch consumption via the consume() method
 * - Does not automatically resize
 * - Optimizes for FIFO access patterns common in RPC request/response handling
 * - Uses bitwise operations for fast index calculations avoiding modulo
 * - Prevent allocation of new memory for each push operation
 * - Have predictable memory allocation and cache behavior
 *
 * This implementation prioritizes performance over flexibility, making it
 * suitable for high-throughput, low-latency scenarios where
 * memory allocation must be controlled and predictable.
 *
 * Additionally, this RingBuffer is designed to be more cache-friendly than
 * existing STL implementations like std::queue or std::deque. By using a
 * contiguous block of memory and minimizing pointer indirection, it improves
 * cache locality, which can lead to better performance in high-throughput
 * scenarios.
 *
 * In Thrift Rocket RPC, this RingBuffer is used to for batching operations of
 * operations like writing to a socket, or serializing/deserializing frames to
 * amortize the cost of these operations.
 */
template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t log_size)
      : capacity_([log_size]() {
          if (log_size < 1 || log_size > 31) {
            throw std::invalid_argument("log_size must be between 1 and 31");
          }
          return 1 << log_size;
        }()),
        data_(new T[capacity_]) {}

  RingBuffer(RingBuffer&& other) = delete;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(RingBuffer&&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  ~RingBuffer() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      consume([](T& obj) { obj.~T(); }, size());
    }
    delete[] data_;
  }

  template <typename... Args>
  bool emplace_back(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>) {
    const bool has_space = size() < capacity();
    if (FOLLY_LIKELY(has_space)) {
      new (&data_[tail_index()]) T(std::forward<Args>(args)...);
      increment_tail();
    }
    return has_space;
  }

  T& front() noexcept { return *std::launder(&data_[head_index()]); }

  const T& front() const noexcept {
    return *std::launder(&data_[head_index()]);
  }

  void pop_front() noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      data_[head_index()].~T();
    }
    increment_head();
  }

  /**
   * The consume method allows for batch processing of elements in the ring
   * buffer. It takes a consumer function and a maximum number of elements to
   * process. The method iterates over the elements from the head of the buffer
   * up to the specified maximum or the current size, whichever is smaller. For
   * each element, it invokes the consumer function, passing the element by
   * reference. If the element type is not trivially destructible, it explicitly
   * calls the destructor on each element after processing. The method returns
   * the number of elements consumed. This is specialized for batch processing
   * elements.
   */
  template <typename Consumer>
  size_t consume(Consumer&& consumer, size_t max) noexcept(
      std::is_nothrow_invocable_v<Consumer, T&>) {
    static_assert(std::is_invocable_v<Consumer, T&>, "Consumer must accept T&");
    const auto limit = std::min(max, size());
    auto head = head_;
    size_t i = 0;
    for (; i < limit; ++i) {
      T& data = *std::launder(&data_[index(head + i)]);
      consumer(data);
      if constexpr (!std::is_trivially_destructible_v<T>) {
        data.~T();
      }
    }
    head_ = head + limit;
    return i;
  }

  size_t size() const noexcept { return tail_ - head_; }
  size_t capacity() const noexcept { return capacity_; }
  bool empty() const noexcept { return size() == 0; }

 private:
  size_t head_ = 0;
  size_t tail_ = 0;
  const size_t capacity_;
  T* data_;

  size_t index(size_t source) const noexcept {
    return source & (capacity_ - 1);
  }

  size_t tail_index() const noexcept { return index(tail_); }
  size_t head_index() const noexcept { return index(head_); }

  void increment_tail() noexcept { ++tail_; }
  void increment_head() noexcept { ++head_; }
};

} // namespace apache::thrift::rocket
