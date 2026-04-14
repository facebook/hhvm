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
 * LoopBatchingFrameHandler - Outbound handler for loop-iteration write
 * batching.
 *
 * Mirrors RocketClient's client-side batching pattern: accumulates all writes
 * enqueued within a single event loop iteration and flushes them together at
 * the end of the iteration via LoopCallback with double-scheduling (reschedule
 * once to push to the back of the loop queue, ensuring all writes from the
 * current iteration are captured).
 *
 * Input:  std::unique_ptr<folly::IOBuf> (individual frames)
 * Output: std::unique_ptr<folly::IOBuf> (coalesced batch)
 */

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

#include <functional>

namespace apache::thrift::fast_thrift::frame::write::handler {

class LoopBatchingFrameHandler : public folly::EventBase::LoopCallback {
 public:
  LoopBatchingFrameHandler() noexcept = default;

  ~LoopBatchingFrameHandler() override { cancelLoopCallbackIfScheduled(); }

  LoopBatchingFrameHandler(const LoopBatchingFrameHandler&) = delete;
  LoopBatchingFrameHandler& operator=(const LoopBatchingFrameHandler&) = delete;
  LoopBatchingFrameHandler(LoopBatchingFrameHandler&&) = delete;
  LoopBatchingFrameHandler& operator=(LoopBatchingFrameHandler&&) = delete;

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

    bufferedWritesQueue_.append(std::move(frame));
    scheduleFlushIfNeeded();
    return channel_pipeline::Result::Success;
  }

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {
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
    flushPendingWrites();
  }

  // ===========================================================================
  // LoopCallback
  // ===========================================================================

  void runLoopCallback() noexcept override {
    if (!std::exchange(rescheduled_, true)) {
      eventBase_->runInLoop(this, true);
      return;
    }

    rescheduled_ = false;
    scheduled_ = false;
    flushPendingWrites();
  }

  // ===========================================================================
  // Accessors (for testing)
  // ===========================================================================

  bool isScheduled() const noexcept { return scheduled_; }
  bool empty() const noexcept { return bufferedWritesQueue_.empty(); }

 private:
  void scheduleFlushIfNeeded() noexcept {
    if (!scheduled_ && eventBase_) {
      eventBase_->runInLoop(this, true);
      scheduled_ = true;
    }
  }

  void cancelLoopCallbackIfScheduled() noexcept {
    if (scheduled_) {
      this->cancelLoopCallback();
      scheduled_ = false;
      rescheduled_ = false;
    }
  }

  void clearPendingState() noexcept {
    cancelLoopCallbackIfScheduled();
    bufferedWritesQueue_.move(); // discard
  }

  template <typename Context>
  void doFlush(Context& ctx) noexcept {
    auto batchToSend = bufferedWritesQueue_.move();
    if (!batchToSend) {
      return;
    }

    (void)ctx.fireWrite(
        channel_pipeline::TypeErasedBox(std::move(batchToSend)));
  }

  void flushPendingWrites() noexcept {
    if (flushFn_) {
      flushFn_();
    }
  }

  folly::EventBase* eventBase_{nullptr};

  bool scheduled_{false};
  bool rescheduled_{false};

  folly::IOBufQueue bufferedWritesQueue_{folly::IOBufQueue::cacheChainLength()};

  std::function<void()> flushFn_;
};

} // namespace apache::thrift::fast_thrift::frame::write::handler
