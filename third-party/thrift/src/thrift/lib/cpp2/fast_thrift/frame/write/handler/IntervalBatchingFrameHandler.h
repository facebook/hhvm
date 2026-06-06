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
 * IntervalBatchingFrameHandler - Outbound handler for interval-based write
 * batching.
 *
 * Mirrors WriteBatcher's batching semantics from thrift async within the
 * fast_thrift channel pipeline. Batches multiple IOBuf frames together using
 * IOBufQueue for O(1) appends, and flushes based on:
 * - Time interval: HHWheelTimer fires after configurable batchingInterval
 * - Frame count: Early flush when batchingSize frames are buffered
 * - Byte size: Early flush when batchingByteSize bytes are buffered
 *
 * When batchingInterval == 0, behaves like a LoopCallback-only batcher
 * (flushes at end of current event loop iteration).
 *
 * Input:  std::unique_ptr<folly::IOBuf> (individual frames)
 * Output: std::unique_ptr<folly::IOBuf> (coalesced batch)
 */

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/IntervalBatchingHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>

#include <functional>
#include <stdexcept>

namespace apache::thrift::fast_thrift::frame::write::handler {

template <WriteCompletionTracker Tracker = NoOpWriteCompletionTracker>
class IntervalBatchingFrameHandlerT : private folly::EventBase::LoopCallback,
                                      private folly::HHWheelTimer::Callback {
 public:
  // Detected by makeHandlerNode; pipeline drives onWriteReady through this.
  channel_pipeline::WriteReadyHook writeReadyHook_;

  explicit IntervalBatchingFrameHandlerT(
      IntervalBatchingHandlerConfig config = {}) noexcept
      : config_(std::move(config)) {}

  ~IntervalBatchingFrameHandlerT() override {
    cancelLoopCallbackIfScheduled();
    cancelTimeout();
  }

  IntervalBatchingFrameHandlerT(const IntervalBatchingFrameHandlerT&) = delete;
  IntervalBatchingFrameHandlerT& operator=(
      const IntervalBatchingFrameHandlerT&) = delete;
  IntervalBatchingFrameHandlerT(IntervalBatchingFrameHandlerT&&) = delete;
  IntervalBatchingFrameHandlerT& operator=(IntervalBatchingFrameHandlerT&&) =
      delete;

  // ===========================================================================
  // HandlerLifecycle
  // ===========================================================================

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    eventBase_ = ctx.eventBase();
    flushFn_ = [this, &ctx]() { flushAndPropagateErrors(ctx); };
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    clearPendingState();
    eventBase_ = nullptr;
    flushFn_ = nullptr;
  }

  // ===========================================================================
  // OutboundHandler
  // ===========================================================================

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onWrite(
      Context& /*ctx*/, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto frame = msg.take<std::unique_ptr<folly::IOBuf>>();

    if (!frame) {
      return channel_pipeline::Result::Success;
    }

    size_t frameSize = frame->computeChainDataLength();

    bool wasEmpty = bufferedWritesQueue_.empty();
    bufferedWritesQueue_.append(std::move(frame));

    totalBytesBuffered_ += frameSize;
    ++bufferedWritesCount_;
    tracker_.onWrite();

    // While downstream is backpressured we still buffer, but don't schedule
    // any flush — onWriteReady will drain when downstream is ready.
    if (backpressured_) {
      return channel_pipeline::Result::Backpressure;
    }

    if (wasEmpty) {
      scheduleFlush();
    }

    if (shouldEarlyFlush()) {
      earlyFlush();
    }

    return channel_pipeline::Result::Success;
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    clearPendingState();
  }

  template <typename Context>
  void onWriteReady(Context& ctx) noexcept {
    backpressured_ = false;
    ctx.cancelAwaitWriteReady();
    flushAndPropagateErrors(ctx);
  }

  // Receives the per-pipeline event fired by TransportHandlerT. The tracker
  // (which owns the per-pipeline event type via its EventFactory) drives
  // per-batch attribution. With NoOpWriteCompletionTracker the dispatch is
  // inline no-op.
  template <typename Context>
  void onEvent(
      Context& ctx, const channel_pipeline::TypeErasedBox& box) noexcept {
    tracker_.onEvent(ctx, box);
  }

  /**
   * Synchronously flush all pending writes.
   * Cancels any scheduled callbacks and flushes immediately.
   */
  void drain() noexcept {
    if (bufferedWritesQueue_.empty()) {
      return;
    }
    cancelLoopCallbackIfScheduled();
    cancelTimeout();
    flushPendingWrites();
  }

  // ===========================================================================
  // Accessors (for testing)
  // ===========================================================================

  size_t pendingBytes() const noexcept { return totalBytesBuffered_; }
  size_t pendingFrames() const noexcept { return bufferedWritesCount_; }
  bool empty() const noexcept { return bufferedWritesQueue_.empty(); }
  bool isBackpressured() const noexcept { return backpressured_; }
  Tracker& tracker() noexcept { return tracker_; }

 private:
  // ===========================================================================
  // LoopCallback
  // ===========================================================================

  void runLoopCallback() noexcept final { flushPendingWrites(); }

  // ===========================================================================
  // HHWheelTimer::Callback
  // ===========================================================================

  void timeoutExpired() noexcept final { flushPendingWrites(); }

  // ===========================================================================
  // Internals
  // ===========================================================================

  void scheduleFlush() noexcept {
    if (config_.batchingInterval != std::chrono::milliseconds::zero()) {
      eventBase_->timer().scheduleTimeout(this, config_.batchingInterval);
    } else {
      eventBase_->runInLoop(this, true);
    }
  }

  bool shouldEarlyFlush() const noexcept {
    if (config_.batchingInterval == std::chrono::milliseconds::zero()) {
      return false;
    }
    return bufferedWritesCount_ >= config_.batchingSize ||
        (config_.batchingByteSize != 0 &&
         totalBytesBuffered_ >= config_.batchingByteSize);
  }

  void earlyFlush() noexcept {
    if (earlyFlushRequested_) {
      return;
    }
    earlyFlushRequested_ = true;
    cancelTimeout();
    eventBase_->runInLoop(this, true);
  }

  void flushPendingWrites() noexcept {
    if (flushFn_) {
      flushFn_();
    }
  }

  template <typename Context>
  void flushAndPropagateErrors(Context& ctx) noexcept {
    if (doFlush(ctx) == channel_pipeline::Result::Error) {
      ctx.fireException(
          folly::make_exception_wrapper<std::runtime_error>(
              "IntervalBatchingFrameHandler: downstream write failed"));
    }
  }

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result doFlush(Context& ctx) noexcept {
    auto batchToSend = bufferedWritesQueue_.move();
    if (!batchToSend) {
      return channel_pipeline::Result::Success;
    }

    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    earlyFlushRequested_ = false;
    tracker_.onFlush();

    auto result =
        ctx.fireWrite(channel_pipeline::TypeErasedBox(std::move(batchToSend)));

    if (result == channel_pipeline::Result::Backpressure) {
      backpressured_ = true;
      ctx.awaitWriteReady();
    }

    return result;
  }

  void cancelLoopCallbackIfScheduled() noexcept {
    if (isLoopCallbackScheduled()) {
      cancelLoopCallback();
    }
  }

  void clearPendingState() noexcept {
    cancelLoopCallbackIfScheduled();
    cancelTimeout();
    bufferedWritesQueue_.move(); // discard
    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    earlyFlushRequested_ = false;
    backpressured_ = false;
  }

  IntervalBatchingHandlerConfig config_;
  folly::EventBase* eventBase_{nullptr};

  folly::IOBufQueue bufferedWritesQueue_{folly::IOBufQueue::cacheChainLength()};
  size_t bufferedWritesCount_{0};
  size_t totalBytesBuffered_{0};
  bool earlyFlushRequested_{false};
  bool backpressured_{false};

  std::function<void()> flushFn_;

  // Per-write tracker mixin; NoOp by default.
  [[no_unique_address]] Tracker tracker_{};
};

// Default specialization preserves the existing class name for callers that
// don't opt into per-write tracking.
using IntervalBatchingFrameHandler =
    IntervalBatchingFrameHandlerT<NoOpWriteCompletionTracker>;

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        IntervalBatchingFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "IntervalBatchingFrameHandler must satisfy OutboundHandler concept");
} // namespace apache::thrift::fast_thrift::frame::write::handler
