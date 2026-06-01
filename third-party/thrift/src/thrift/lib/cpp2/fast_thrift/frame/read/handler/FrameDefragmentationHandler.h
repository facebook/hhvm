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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FragmentState.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>

#include <folly/ExceptionWrapper.h>

namespace apache::thrift::fast_thrift::frame::read::handler {

/**
 * FrameDefragmentationHandler - Reassembles fragmented RSocket frames.
 *
 * This handler implements the InboundHandler concept. It intercepts frames
 * with hasFollows() == true, accumulates fragments per stream ID in an
 * F14NodeMap, and emits complete ParsedFrame instances when the final
 * fragment (hasFollows() == false) arrives.
 *
 * Non-fragmented frames pass through unchanged (fast path).
 *
 * Input:  ParsedFrame (possibly fragmented)
 * Output: ParsedFrame (complete, defragmented)
 *
 * Thread Safety: Not thread-safe. Assumes single-threaded EventBase access.
 *
 * RSocket Fragment Semantics:
 * - First fragment: Original frame type (REQUEST_*, etc.) with
 * hasFollows()=true
 * - Continuation fragments: PAYLOAD frames with hasFollows()=true
 * - Final fragment: PAYLOAD frame with hasFollows()=false
 * - Only first fragment contains metadata (if any)
 * - Reassembled frame preserves original frame type and flags
 */
class FrameDefragmentationHandler {
 public:
  static constexpr size_t kDefaultMaxPendingBytes = 16 * 1024 * 1024; // 16MB

  explicit FrameDefragmentationHandler(
      size_t maxPendingBytes = kDefaultMaxPendingBytes) noexcept
      : maxPendingBytes_(maxPendingBytes) {}

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    pending_.clear();
    totalPendingBytes_ = 0;
  }

  // === InboundHandler ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame = msg.get<ParsedFrame>();

    // Fast path: complete frame, no fragmentation in progress anywhere
    // Avoids map lookup in the common case
    if (!frame.hasFollows() && pending_.empty()) {
      return ctx.fireRead(std::move(msg));
    }

    // CANCEL/ERROR: clear any pending fragments for this stream, forward frame
    if (isTerminalFrameType(frame.type())) {
      if (auto it = pending_.find(frame.streamId()); it != pending_.end()) {
        totalPendingBytes_ -= it->second.accumulatedBytes;
        pending_.erase(it);
      }
      return ctx.fireRead(std::move(msg));
    }

    // All fragment handling delegated to focused method
    return handleFragment(ctx, std::move(msg), frame);
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === Accessors (for testing) ===

  size_t pendingCount() const noexcept { return pending_.size(); }

  bool hasPendingFragment(uint32_t streamId) const noexcept {
    return pending_.contains(streamId);
  }

  size_t totalPendingBytes() const noexcept { return totalPendingBytes_; }

  size_t maxPendingBytes() const noexcept { return maxPendingBytes_; }

 private:
  /**
   * Handle potentially fragmented frames.
   *
   * This method is only called when:
   * - Frame has FOLLOWS flag, OR
   * - There are pending fragments for some stream
   *
   * All paths are explicit about what state they represent.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result handleFragment(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&& msg,
      ParsedFrame& frame) noexcept {
    auto streamId = frame.streamId();
    auto it = pending_.find(streamId);

    // Complete frame: pending_ not empty globally, but not for THIS stream
    if (it == pending_.end() && !frame.hasFollows()) {
      return ctx.fireRead(std::move(msg));
    }

    // First fragment: start accumulation
    if (it == pending_.end()) {
      auto payloadSize = frame.payloadSize();
      if (totalPendingBytes_ + payloadSize > maxPendingBytes_) {
        return apache::thrift::fast_thrift::channel_pipeline::Result::
            Backpressure;
      }
      initPendingFragment(ctx, streamId, frame);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    // Continuation/final fragment: check backpressure before appending
    auto dataSize = frame.dataSize();
    if (totalPendingBytes_ + dataSize > maxPendingBytes_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }

    // Append fragment data (applies to both continuation and final)
    appendToPending(it->second, frame);

    // Final fragment: assemble and emit
    if (!frame.hasFollows()) {
      DCHECK(it->second.payload != nullptr);
      auto bytesToRelease = it->second.accumulatedBytes;
      auto assembled = assembleFrame(std::move(it->second));
      pending_.erase(it);
      totalPendingBytes_ -= bytesToRelease;
      return ctx.fireRead(
          apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
              std::move(assembled)));
    }

    // Continuation fragment: buffered, waiting for more
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  /**
   * Trim bytes from the front of an IOBuf chain (zero-copy).
   * Advances past header/metadata bytes to reach payload or data.
   */
  static std::unique_ptr<folly::IOBuf> trimBufferFront(
      std::unique_ptr<folly::IOBuf> buf, size_t bytes) {
    while (bytes > 0 && buf) {
      if (buf->length() > bytes) {
        buf->trimStart(bytes);
        break;
      }
      bytes -= buf->length();
      buf = buf->pop();
    }
    return buf;
  }

  /**
   * Initialize pending state for the first fragment of a stream.
   * Stores original frame type/flags and moves the payload buffer directly
   * (zero-copy) instead of cursor-cloning.
   */
  template <typename Context>
  void initPendingFragment(
      Context& ctx, uint32_t streamId, ParsedFrame& frame) {
    FragmentState state;
    state.originalType = frame.type();
    state.originalFlags = frame.metadata.flags_;
    state.streamId = streamId;
    state.metadataSize = frame.metadataSize();

    auto payloadSize = frame.payloadSize();
    if (payloadSize > 0) {
      state.payload = trimBufferFront(
          std::move(frame.buffer), frame.metadata.payloadOffset);
      state.accumulatedBytes = payloadSize;
      totalPendingBytes_ += payloadSize;
    } else {
      // Empty first fragment - initialize with empty buffer for safe appending
      state.payload = ctx.allocate(0);
    }

    pending_.emplace(streamId, std::move(state));
  }

  /**
   * Append a continuation fragment's data to pending state (zero-copy).
   * Moves the buffer and trims header/metadata instead of cursor-cloning.
   * Per RSocket spec, continuation fragments only have data (no metadata).
   */
  void appendToPending(FragmentState& state, ParsedFrame& frame) {
    DCHECK(state.payload != nullptr);
    auto dataSize = frame.dataSize();
    if (dataSize > 0) {
      auto buf = trimBufferFront(
          std::move(frame.buffer),
          frame.metadata.payloadOffset + frame.metadata.metadataSize);
      state.appendToChain(std::move(buf), dataSize);
      totalPendingBytes_ += dataSize;
    }
  }

  /**
   * Assemble a complete ParsedFrame from accumulated fragments.
   * Consumes the FragmentState.
   */
  ParsedFrame assembleFrame(FragmentState&& state) {
    auto payloadSize = static_cast<uint32_t>(state.accumulatedBytes);

    ParsedFrame result;
    result.metadata.descriptor = &getDescriptor(state.originalType);
    result.metadata.streamId = state.streamId;
    // Clear follows bit since frame is now complete
    result.metadata.flags_ = state.originalFlags & ~detail::kFollowsBit;
    result.metadata.payloadSize = payloadSize;
    result.metadata.payloadOffset = 0;
    result.metadata.metadataSize = state.metadataSize;
    result.buffer = std::move(state.payload);

    return result;
  }

  DirectStreamMap<FragmentState> pending_;

  // Backpressure configuration and tracking
  size_t maxPendingBytes_;
  size_t totalPendingBytes_{0};
};

} // namespace apache::thrift::fast_thrift::frame::read::handler
