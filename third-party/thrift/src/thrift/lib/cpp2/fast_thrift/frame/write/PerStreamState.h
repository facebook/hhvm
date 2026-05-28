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
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <utility>

namespace apache::thrift::fast_thrift::frame::write {

/**
 * One logical frame awaiting fragmentation. `originalFrame` is the
 * `ComposedFrame` the caller handed us, with its `data` buffer detached
 * into `dataQueue`. The first fragment goes out as `originalFrame` (with
 * the first chunk reattached and `follows` stamped); subsequent fragments
 * are minted as fresh `ComposedFrame{.frameType = PAYLOAD, ...}`
 * continuations from `dataQueue`.
 *
 * `originalComplete` is the originating frame's `isComplete()` value at
 * enqueue time — propagated to the final PAYLOAD continuation so
 * REQUEST_CHANNEL streams close properly on the receiver.
 */
struct PendingFrame {
  ComposedFrame originalFrame{};
  folly::IOBufQueue dataQueue{folly::IOBufQueue::cacheChainLength()};
  bool firstEmitted{false};
  bool originalComplete{false};

  PendingFrame() = default;
  ~PendingFrame() = default;
  PendingFrame(PendingFrame&&) noexcept = default;
  PendingFrame& operator=(PendingFrame&&) noexcept = default;
  PendingFrame(const PendingFrame&) = delete;
  PendingFrame& operator=(const PendingFrame&) = delete;

  [[nodiscard]] size_t remaining() const noexcept {
    return dataQueue.chainLength();
  }
};

/**
 * Per-stream fragmentation state. Owns a FIFO of `PendingFrame`s;
 * SRPT scheduling reads `remaining()` (sum across the FIFO, cached).
 *
 * `nextFragment()` extracts and materializes the next on-wire frame for
 * this stream, dispatching first-vs-continuation typing in one place so
 * the handler stays focused on scheduling.
 */
struct PerStreamState {
  uint32_t streamId{0};
  std::deque<PendingFrame> frames;
  size_t totalRemaining{0};

  PerStreamState() = default;
  ~PerStreamState() = default;
  PerStreamState(PerStreamState&&) noexcept = default;
  PerStreamState& operator=(PerStreamState&&) noexcept = default;
  PerStreamState(const PerStreamState&) = delete;
  PerStreamState& operator=(const PerStreamState&) = delete;

  void enqueue(
      ComposedFrame&& originalFrame,
      std::unique_ptr<folly::IOBuf> data,
      bool originalComplete) noexcept {
    const size_t size = data ? data->computeChainDataLength() : 0;
    PendingFrame pf;
    pf.originalFrame = std::move(originalFrame);
    pf.originalComplete = originalComplete;
    if (data) {
      pf.dataQueue.append(std::move(data));
    }
    totalRemaining += size;
    frames.push_back(std::move(pf));
  }

  [[nodiscard]] bool hasMore() const noexcept { return !frames.empty(); }
  [[nodiscard]] size_t remaining() const noexcept { return totalRemaining; }

  /**
   * Materialized fragment ready to emit. The handler fires `outFrame`,
   * decrements `payloadBytes` from its pending counter, and decrements
   * the frame counter iff `currentFrameDone`.
   */
  struct Fragment {
    ComposedFrame outFrame{};
    size_t payloadBytes{0};
    bool currentFrameDone{false};
  };

  /**
   * Pull the next fragment from the head pending frame. Mutates state:
   * splits `maxSize` bytes off the head's data queue, restores the
   * original `ComposedFrame` for first fragments or mints a PAYLOAD
   * continuation for subsequent ones, and pops the head frame if its
   * data is exhausted. Precondition: `hasMore()`.
   */
  Fragment nextFragment(size_t maxSize) noexcept {
    auto& head = frames.front();
    const size_t chunkSize = std::min(maxSize, head.dataQueue.chainLength());
    auto chunk = head.dataQueue.split(chunkSize);
    const bool follows = !head.dataQueue.empty();

    totalRemaining -= chunkSize;

    Fragment frag;
    frag.payloadBytes = chunkSize;
    frag.currentFrameDone = !follows;

    // `complete` only fires on the final emitted fragment. For unfragmented
    // first-emit (follows=false) this preserves the caller's bit; for any
    // fragmented case it clears it on intermediate frames and re-asserts
    // it on the last one.
    if (!head.firstEmitted) {
      head.firstEmitted = true;
      frag.outFrame = std::move(head.originalFrame);
      frag.outFrame.data = std::move(chunk);
      frag.outFrame.follows = follows;
      frag.outFrame.complete = !follows && head.originalComplete;
    } else {
      frag.outFrame.frameType = FrameType::PAYLOAD;
      frag.outFrame.streamId = streamId;
      frag.outFrame.data = std::move(chunk);
      frag.outFrame.follows = follows;
      frag.outFrame.complete = !follows && head.originalComplete;
      frag.outFrame.next = true;
    }

    if (frag.currentFrameDone) {
      frames.pop_front();
    }
    return frag;
  }
};

} // namespace apache::thrift::fast_thrift::frame::write
