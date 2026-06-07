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
 * BatchingFrameHandler - Outbound handler for write coalescing.
 *
 * Batches multiple IOBuf frames together into a chain to reduce syscall
 * overhead. Flushes based on configurable thresholds:
 * - Byte threshold: Flush when accumulated bytes exceed maxPendingBytes
 * - Frame threshold: Flush when frame count exceeds maxPendingFrames
 * - Event loop tick: Schedule LoopCallback to flush at end of iteration
 *
 * This handler implements the channel_pipeline OutboundHandler concept.
 *
 * Input:  std::unique_ptr<folly::IOBuf> (individual frames)
 * Output: std::unique_ptr<folly::IOBuf> (chained batch of frames)
 */

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/BatchingHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <functional>
#include <stdexcept>

namespace apache::thrift::fast_thrift::frame::write::handler {

/**
 * BatchingFrameHandler - Composable outbound handler for write coalescing.
 *
 * Templated on a `WriteCompletionTracker`-satisfying type (see
 * WriteCompletionTracker.h). The default `NoOpWriteCompletionTracker` makes
 * the three hook sites (onWrite, doFlush, onEvent) fully no-op and the
 * compiler elides them. Pipelines that need per-write completion notifications
 * instantiate `BatchingFrameHandlerT<RealTracker>` and the same hooks drive
 * the tracker.
 *
 * Thread Safety: Not thread-safe. Assumes single-threaded EventBase access.
 */
template <WriteCompletionTracker Tracker = NoOpWriteCompletionTracker>
class BatchingFrameHandlerT : public folly::EventBase::LoopCallback {
 public:
  apache::thrift::fast_thrift::channel_pipeline::WriteReadyHook writeReadyHook_;

  explicit BatchingFrameHandlerT(BatchingHandlerConfig config = {}) noexcept
      : config_(config) {}

  ~BatchingFrameHandlerT() override { cancelLoopCallbackIfScheduled(); }

  BatchingFrameHandlerT(const BatchingFrameHandlerT&) = delete;
  BatchingFrameHandlerT& operator=(const BatchingFrameHandlerT&) = delete;
  BatchingFrameHandlerT(BatchingFrameHandlerT&&) = delete;
  BatchingFrameHandlerT& operator=(BatchingFrameHandlerT&&) = delete;

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
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto frame = msg.take<std::unique_ptr<folly::IOBuf>>();

    if (!frame) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    size_t frameSize = frame->computeChainDataLength();

    // Append to batch chain (zero-copy)
    appendToBatch(std::move(frame));
    pendingBytes_ += frameSize;
    ++pendingFrames_;
    tracker_.onWrite();

    // Downstream is backpressured: buffer and propagate backpressure upstream.
    // Do not attempt to flush until onWriteReady is called.
    if (backpressured_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }

    // Check flush thresholds
    if (pendingBytes_ >= config_.maxPendingBytes ||
        pendingFrames_ >= config_.maxPendingFrames) {
      return flushNow(ctx);
    }

    // Schedule flush for end of event loop iteration
    scheduleFlushIfNeeded();
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
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

  // Event subscription is sourced from the tracker, which owns the per-pipeline
  // event type via its templated EventFactory. With NoOpWriteCompletionTracker
  // EventId is NoEvent and kSubscribedEvents is empty, so nothing is wired and
  // the event path compiles out.
  using EventId = typename Tracker::EventId;
  static constexpr auto kSubscribedEvents = Tracker::kSubscribedEvents;

  // Receives the per-pipeline event fired by TransportHandlerT on each
  // socket-level write completion. The tracker discriminates on the event's
  // kind and drives per-batch attribution (see WriteCompletionTracker.h).
  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId ev,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    tracker_.onEvent(ctx, ev, box);
  }

  // ===========================================================================
  // LoopCallback
  // ===========================================================================

  void runLoopCallback() noexcept override {
    isScheduled_ = false;
    if (flushFn_) {
      flushFn_();
    }
  }

  // ===========================================================================
  // Accessors (for testing)
  // ===========================================================================

  size_t pendingBytes() const noexcept { return pendingBytes_; }
  size_t pendingFrames() const noexcept { return pendingFrames_; }
  bool isScheduled() const noexcept { return isScheduled_; }
  bool hasPendingData() const noexcept { return batch_ != nullptr; }
  bool isBackpressured() const noexcept { return backpressured_; }
  Tracker& tracker() noexcept { return tracker_; }

 private:
  /**
   * Append a frame to the batch chain.
   *
   * Uses IOBuf::prependChain() for zero-copy chaining.
   * New frames are appended to the end of the chain.
   */
  void appendToBatch(std::unique_ptr<folly::IOBuf> frame) noexcept {
    if (!batch_) {
      batch_ = std::move(frame);
    } else {
      // Append to end of chain (prependChain adds to tail)
      batch_->prependChain(std::move(frame));
    }
  }

  void scheduleFlushIfNeeded() noexcept {
    if (!isScheduled_ && eventBase_) {
      eventBase_->runInLoop(this, true);
      isScheduled_ = true;
    }
  }

  void cancelLoopCallbackIfScheduled() noexcept {
    if (isScheduled_) {
      this->cancelLoopCallback();
      isScheduled_ = false;
    }
  }

  void clearPendingState() noexcept {
    cancelLoopCallbackIfScheduled();
    batch_.reset();
    pendingBytes_ = 0;
    pendingFrames_ = 0;
    backpressured_ = false;
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result flushNow(
      Context& ctx) noexcept {
    cancelLoopCallbackIfScheduled();
    return doFlush(ctx);
  }

  template <typename Context>
  void flushAndPropagateErrors(Context& ctx) noexcept {
    if (doFlush(ctx) ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      ctx.fireException(
          folly::make_exception_wrapper<std::runtime_error>(
              "BatchingFrameHandler: downstream write failed"));
    }
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result doFlush(
      Context& ctx) noexcept {
    if (!batch_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    // Move batch to TypeErasedBox and send downstream
    auto batchToSend = std::move(batch_);
    pendingBytes_ = 0;
    pendingFrames_ = 0;
    tracker_.onFlush();

    auto result = ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            std::move(batchToSend)));

    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure) {
      backpressured_ = true;
      ctx.awaitWriteReady();
    }

    return result;
  }

  BatchingHandlerConfig config_;
  folly::EventBase* eventBase_{nullptr};

  bool isScheduled_{false};
  bool backpressured_{false};

  size_t pendingBytes_{0};
  size_t pendingFrames_{0};

  // The accumulated batch of frames (IOBuf chain)
  std::unique_ptr<folly::IOBuf> batch_;

  // Type-erased flush function captured in handlerAdded
  std::function<void()> flushFn_;

  // Per-write tracker mixin; NoOp by default.
  [[no_unique_address]] Tracker tracker_{};
};

// Default specialization preserves the existing class name for callers that
// don't opt into per-write tracking.
using BatchingFrameHandler = BatchingFrameHandlerT<NoOpWriteCompletionTracker>;

} // namespace apache::thrift::fast_thrift::frame::write::handler
