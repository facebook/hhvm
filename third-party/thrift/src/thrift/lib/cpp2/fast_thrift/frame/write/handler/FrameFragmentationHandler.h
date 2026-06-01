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

/**
 * FrameFragmentationHandler - Outbound handler for HOL blocking mitigation.
 *
 * Uses SRPT (Shortest Remaining Processing Time) scheduling to minimize mean
 * stream completion latency. Streams with the fewest remaining bytes are
 * flushed first, so small responses (the common case — single-fragment
 * request-response) complete immediately rather than being blocked behind
 * bulk transfers.
 *
 * See: FrameFragmentationHandler.md for design documentation.
 */

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/PerStreamState.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/SrptHeap.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <deque>
#include <functional>

namespace apache::thrift::fast_thrift::frame::write::handler {

/**
 * Outbound frame structure for fragmentation.
 */
struct OutboundFrame {
  uint32_t streamId{0};
  FrameType frameType{FrameType::RESERVED};
  uint16_t flags{0};
  std::unique_ptr<folly::IOBuf> payload;

  size_t payloadSize() const noexcept {
    return payload ? payload->computeChainDataLength() : 0;
  }
};

/// KeyFn for SrptHeap: extracts remaining bytes from PerStreamState.
struct RemainingBytesFn {
  size_t operator()(const PerStreamState& s) const noexcept {
    return s.remaining();
  }
};

/**
 * FrameFragmentationHandler - Composable outbound handler for HOL mitigation.
 */
class FrameFragmentationHandler : public folly::EventBase::LoopCallback {
 public:
  apache::thrift::fast_thrift::channel_pipeline::WriteReadyHook writeReadyHook_;

  explicit FrameFragmentationHandler(
      FragmentationHandlerConfig config = {}) noexcept
      : config_(config) {}

  ~FrameFragmentationHandler() override { cancelLoopCallbackIfScheduled(); }

  FrameFragmentationHandler(const FrameFragmentationHandler&) = delete;
  FrameFragmentationHandler& operator=(const FrameFragmentationHandler&) =
      delete;
  FrameFragmentationHandler(FrameFragmentationHandler&&) = delete;
  FrameFragmentationHandler& operator=(FrameFragmentationHandler&&) = delete;

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    eventBase_ = ctx.getEventBase();
    flushFn_ = [this, &ctx]() { (void)doFlush(ctx); };
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    cancelLoopCallbackIfScheduled();
    immediateQueue_.clear();
    streams_.clear();
    pendingBytes_ = 0;
    pendingFrames_ = 0;
    eventBase_ = nullptr;
    flushFn_ = nullptr;
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame = msg.get<OutboundFrame>();
    size_t frameSize = frame.payloadSize();
    uint32_t streamId = frame.streamId;

    // Terminal frames: cancel pending fragments, forward immediately.
    if (isTerminalFrameType(frame.frameType)) {
      cancelStream(streamId);
      return ctx.fireWrite(std::move(msg));
    }

    // Fast path: frame doesn't need fragmentation AND no pending fragments
    // from other streams → pass straight through, zero overhead.
    // Covers ~99% of traffic (small request-response at 200+ QPS).
    if (frameSize <= config_.minSizeToFragment ||
        (frameSize <= config_.maxFragmentSize && streams_.empty())) {
      return ctx.fireWrite(std::move(msg));
    }

    // Medium path: fits in one fragment but there ARE pending fragments
    // from other streams → queue behind them to preserve ordering.
    if (frameSize <= config_.maxFragmentSize) {
      immediateQueue_.push_back(std::move(msg));
    } else {
      // Slow path: needs fragmentation → SrptHeap
      addToStreamQueue(streamId, std::move(msg));
    }

    pendingBytes_ += frameSize;
    pendingFrames_++;

    if (pendingBytes_ > config_.maxPendingBytes ||
        pendingFrames_ > config_.maxPendingFrames) {
      return flushNow(ctx);
    }

    scheduleFlushIfNeeded();
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  template <typename Context>
  void onPipelineDeactivated(Context& ctx) noexcept {
    cancelLoopCallbackIfScheduled();
    immediateQueue_.clear();
    streams_.clear();
    ctx.deactivate();
  }

  template <typename Context>
  void onWriteReady(Context& ctx) noexcept {
    ctx.cancelAwaitWriteReady();
    backpressured_ = false;
    (void)doFlush(ctx);
  }

  void runLoopCallback() noexcept override {
    isScheduled_ = false;
    if (flushFn_) {
      flushFn_();
    }
  }

  size_t pendingStreamCount() const noexcept { return streams_.size(); }
  size_t immediateQueueSize() const noexcept { return immediateQueue_.size(); }
  size_t pendingBytes() const noexcept { return pendingBytes_; }
  bool isScheduled() const noexcept { return isScheduled_; }

  bool hasPendingStream(uint32_t streamId) const noexcept {
    return streams_.contains(streamId);
  }

 private:
  void addToStreamQueue(
      uint32_t streamId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame = msg.get<OutboundFrame>();

    auto* existing = streams_.find(streamId);
    if (existing) {
      existing->frameType = frame.frameType;
      existing->originalFlags = frame.flags;
      existing->init(std::move(frame.payload));
      streams_.update(streamId);
    } else {
      PerStreamState state;
      state.streamId = streamId;
      state.frameType = frame.frameType;
      state.originalFlags = frame.flags;
      state.init(std::move(frame.payload));
      streams_.insert(streamId, std::move(state));
    }
  }

  void scheduleFlushIfNeeded() noexcept {
    if (!isScheduled_ && eventBase_) {
      eventBase_->runInLoop(this);
      isScheduled_ = true;
    }
  }

  void cancelLoopCallbackIfScheduled() noexcept {
    if (isScheduled_) {
      this->cancelLoopCallback();
      isScheduled_ = false;
    }
  }

  /**
   * Cancel a stream and clean up its pending fragments.
   * Called when CANCEL or ERROR frame is sent for a stream.
   */
  void cancelStream(uint32_t streamId) noexcept {
    auto* state = streams_.find(streamId);
    if (state) {
      pendingBytes_ -= state->remaining();
      pendingFrames_--;
      streams_.erase(streamId);
    }
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result flushNow(
      Context& ctx) noexcept {
    cancelLoopCallbackIfScheduled();
    return doFlush(ctx);
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result doFlush(
      Context& ctx) noexcept {
    // Step 1: Drain immediate queue
    while (!immediateQueue_.empty()) {
      auto result = ctx.fireWrite(std::move(immediateQueue_.front()));
      immediateQueue_.pop_front();
      if (result ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure) {
        backpressured_ = true;
        ctx.awaitWriteReady();
        return result;
      }
    }

    // Step 2: SRPT — always flush the stream with least remaining bytes.
    // This minimizes mean flow completion time: small streams finish
    // immediately rather than being interleaved with bulk transfers.
    while (!streams_.empty()) {
      auto& state = streams_.peekMin();

      if (!state.hasMore()) {
        streams_.extractMin();
        continue;
      }

      auto [fragment, follows] = state.nextFragment(config_.maxFragmentSize);

      OutboundFrame fragmentFrame;
      fragmentFrame.streamId = state.streamId;
      fragmentFrame.frameType = state.frameType;
      fragmentFrame.flags = state.originalFlags;
      if (follows) {
        fragmentFrame.flags |= detail::kFollowsBit;
      } else {
        fragmentFrame.flags &= ~detail::kFollowsBit;
      }
      fragmentFrame.payload = std::move(fragment);

      // Update heap priority after removing bytes.
      uint32_t minStreamId = streams_.peekMinStreamId();
      streams_.update(minStreamId);

      auto result = ctx.fireWrite(
          apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
              std::move(fragmentFrame)));

      if (result ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure) {
        backpressured_ = true;
        ctx.awaitWriteReady();
        return result;
      }

      // If the stream is done after this fragment, remove it.
      if (!follows) {
        streams_.erase(minStreamId);
      }
    }

    pendingBytes_ = 0;
    pendingFrames_ = 0;
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  FragmentationHandlerConfig config_;
  folly::EventBase* eventBase_{nullptr};

  bool isScheduled_{false};
  bool backpressured_{false};

  size_t pendingBytes_{0};
  size_t pendingFrames_{0};

  std::deque<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
      immediateQueue_;
  // SRPT min-heap: always flush the stream with least remaining bytes first.
  // Eliminates round-robin's HOL blocking — small responses complete
  // immediately rather than being interleaved with bulk transfers.
  SrptHeap<PerStreamState, RemainingBytesFn> streams_;
  std::function<void()> flushFn_;
};

} // namespace apache::thrift::fast_thrift::frame::write::handler
