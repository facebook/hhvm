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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace apache::thrift::fast_thrift::frame::write {

/**
 * Per-stream fragment state for lazy fragmentation.
 *
 * Tracks the state needed to fragment a large payload across multiple
 * round-robin flush cycles.
 *
 * Design: Zero-copy fragmentation using IOBufQueue::split().
 * Instead of cloning chunks from a static payload, we directly split
 * off fragments from the queue. This avoids any data copying.
 *
 * See: FrameFragmentationHandler.md for design documentation.
 */
struct PerStreamState {
  // Stream identifier
  uint32_t streamId{0};

  // Payload queue - we split fragments directly from this (zero-copy)
  folly::IOBufQueue payloadQueue{folly::IOBufQueue::cacheChainLength()};

  // Original frame type (REQUEST_*, PAYLOAD, etc.)
  FrameType frameType{FrameType::RESERVED};

  // Original flags from the frame (metadata bit, etc.)
  uint16_t originalFlags{0};

  // ============================================================================
  // Construction
  // ============================================================================

  PerStreamState() = default;
  ~PerStreamState() = default;

  // Move-only
  PerStreamState(PerStreamState&&) = default;
  PerStreamState& operator=(PerStreamState&&) = default;
  PerStreamState(const PerStreamState&) = delete;
  PerStreamState& operator=(const PerStreamState&) = delete;

  /**
   * Initialize state with a new payload.
   * Takes ownership of the IOBuf and appends it to the queue.
   */
  void init(std::unique_ptr<folly::IOBuf> payload) noexcept {
    payloadQueue.reset();
    if (payload) {
      payloadQueue.append(std::move(payload));
    }
  }

  // ============================================================================
  // Fragment Iteration
  // ============================================================================

  /**
   * Returns true if more data remains to be sent.
   */
  [[nodiscard]] bool hasMore() const noexcept { return !payloadQueue.empty(); }

  /**
   * Returns the number of bytes remaining to be sent.
   */
  [[nodiscard]] size_t remaining() const noexcept {
    return payloadQueue.chainLength();
  }

  /**
   * Extract the next fragment up to maxSize bytes.
   *
   * Zero-copy: Uses IOBufQueue::split() which either:
   * - Returns existing IOBuf directly if it's exactly the right size
   * - Splits an IOBuf in the chain (pointer manipulation, no memcpy)
   *
   * Returns a pair of:
   *   - fragment: IOBuf containing the next chunk (ownership transferred)
   *   - follows: true if more fragments remain after this one
   */
  [[nodiscard]] std::pair<std::unique_ptr<folly::IOBuf>, bool> nextFragment(
      size_t maxSize) noexcept {
    if (payloadQueue.empty()) {
      return {nullptr, false};
    }

    size_t chunkSize = std::min(maxSize, remaining());

    // Split off the front of the queue (zero-copy)
    auto fragment = payloadQueue.split(chunkSize);

    // Determine if more fragments follow
    bool follows = hasMore();

    return {std::move(fragment), follows};
  }

  /**
   * Reset state for reuse with a new payload.
   */
  void reset(
      std::unique_ptr<folly::IOBuf> newPayload,
      FrameType newFrameType,
      uint16_t newFlags) noexcept {
    frameType = newFrameType;
    originalFlags = newFlags;
    init(std::move(newPayload));
  }
};

} // namespace apache::thrift::fast_thrift::frame::write
