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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/IntervalBatchingHandlerConfig.h>

#include <functional>

namespace apache::thrift::fast_thrift::frame::write::handler {

class IntervalBatchingFrameHandler : private folly::EventBase::LoopCallback,
                                     private folly::HHWheelTimer::Callback {
 public:
  explicit IntervalBatchingFrameHandler(
      IntervalBatchingHandlerConfig config = {}) noexcept
      : config_(std::move(config)) {}

  ~IntervalBatchingFrameHandler() override {
    cancelLoopCallbackIfScheduled();
    cancelTimeout();
  }

  IntervalBatchingFrameHandler(const IntervalBatchingFrameHandler&) = delete;
  IntervalBatchingFrameHandler& operator=(const IntervalBatchingFrameHandler&) =
      delete;
  IntervalBatchingFrameHandler(IntervalBatchingFrameHandler&&) = delete;
  IntervalBatchingFrameHandler& operator=(IntervalBatchingFrameHandler&&) =
      delete;

  // ===========================================================================
  // HandlerLifecycle
  // ===========================================================================

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    eventBase_ = ctx.eventBase();
    flushFn_ = [this, &ctx]() { doFlush(ctx); };
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
    drain();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

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
  void doFlush(Context& ctx) noexcept {
    auto batchToSend = bufferedWritesQueue_.move();
    if (!batchToSend) {
      return;
    }

    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    earlyFlushRequested_ = false;

    (void)ctx.fireWrite(
        channel_pipeline::TypeErasedBox(std::move(batchToSend)));
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
  }

  IntervalBatchingHandlerConfig config_;
  folly::EventBase* eventBase_{nullptr};

  folly::IOBufQueue bufferedWritesQueue_{folly::IOBufQueue::cacheChainLength()};
  size_t bufferedWritesCount_{0};
  size_t totalBytesBuffered_{0};
  bool earlyFlushRequested_{false};

  std::function<void()> flushFn_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        IntervalBatchingFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "IntervalBatchingFrameHandler must satisfy OutboundHandler concept");
} // namespace apache::thrift::fast_thrift::frame::write::handler
