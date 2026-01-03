/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <quic/priority/PriorityQueue.h>

namespace proxygen::coro::detail {

/**
 * A thin wrapper around quic::PriorityQueue for managing stream egress.
 *
 * This class:
 * - Takes uint64_t streamId instead of quic::PriorityQueue::Identifier
 * - Tracks the count of streams in the queue
 * - Provides a simpler interface for stream-only operations
 *
 * The underlying PriorityQueue may be shared with other users (e.g.,
 * datagrams), so this wrapper only tracks streams inserted through its own
 * interface.
 */
class StreamPriorityQueue {
 public:
  using Priority = quic::PriorityQueue::Priority;

  explicit StreamPriorityQueue(quic::PriorityQueue& queue) noexcept
      : queue_(queue) {
  }

  /**
   * Insert a stream into the queue or update its priority if already present.
   * Increments the stream count if the stream was not already in the queue.
   */
  void insert(uint64_t streamId, Priority priority) {
    auto id = quic::PriorityQueue::Identifier::fromStreamID(streamId);
    if (!queue_.contains(id)) {
      count_++;
    }
    queue_.insertOrUpdate(id, priority);
  }

  /**
   * Update the priority of a stream if it exists in the queue.
   * Does not change the stream count.
   */
  void update(uint64_t streamId, Priority priority) {
    auto id = quic::PriorityQueue::Identifier::fromStreamID(streamId);
    queue_.updateIfExist(id, priority);
  }

  /**
   * Remove a stream from the queue.
   * Decrements the stream count if the stream was in the queue.
   */
  void erase(uint64_t streamId) {
    auto id = quic::PriorityQueue::Identifier::fromStreamID(streamId);
    if (queue_.contains(id)) {
      count_--;
    }
    queue_.erase(id);
  }

  /**
   * Consume bytes for fairness accounting.
   * Pass-through to the underlying queue.
   */
  void consume(uint64_t bytes) {
    queue_.consume(bytes);
  }

  /**
   * Returns the number of streams currently in the queue.
   */
  [[nodiscard]] uint64_t count() const noexcept {
    return count_;
  }

  /**
   * Returns true if there are streams in the queue.
   */
  [[nodiscard]] bool hasStreams() const noexcept {
    return count_ > 0;
  }

  /**
   * Peek at the next scheduled stream ID without modifying state.
   * Returns std::nullopt if the queue is empty or if the head of the queue
   * is not a stream (e.g., a datagram).
   */
  [[nodiscard]] std::optional<uint64_t> peek() const noexcept {
    if (queue_.empty()) {
      return std::nullopt;
    }
    auto id = queue_.peekNextScheduledID();
    if (!id.isStreamID()) {
      return std::nullopt;
    }
    return id.asStreamID();
  }

 private:
  quic::PriorityQueue& queue_;
  uint64_t count_{0};
};

} // namespace proxygen::coro::detail
