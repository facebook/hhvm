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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/IntervalBatchingHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BackpressurePolicy.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/util/BatchFlushScheduler.h>

#include <functional>
#include <stdexcept>
#include <type_traits>

namespace apache::thrift::fast_thrift::frame::write::handler {

template <
    WriteCompletionTracker Tracker = NoOpWriteCompletionTracker,
    BackpressurePolicy Backpressure = BackpressureEnabled,
    typename Ev = typename Tracker::EventId>
class IntervalBatchingFrameHandlerT : public Backpressure {
 public:
  // Backpressure state (writeReadyHook_, backpressured_) is inherited from the
  // policy and reached through `this->`. With BackpressureDisabled the policy
  // is empty and hook-less, so this handler is never linked into the
  // pipeline's writeReadyList_ and every touchpoint below compiles out.

  explicit IntervalBatchingFrameHandlerT(
      IntervalBatchingHandlerConfig config = {}) noexcept
      : config_(std::move(config)),
        scheduler_(
            util::BatchFlushScheduler::DeferredFlushMode::EndOfCurrentLoop) {}

  ~IntervalBatchingFrameHandlerT() { scheduler_.cancelAll(); }

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
    scheduler_.setEventBase(ctx.eventBase());
    scheduler_.setFlushFunction(
        [this, &ctx]() { flushAndPropagateErrors(ctx); });
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    clearPendingState();
    scheduler_.setEventBase(nullptr);
    scheduler_.clearFlushFunction();
    scheduler_.setFlushList(nullptr);
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
    if constexpr (Backpressure::kBackpressureEnabled) {
      if (this->backpressured_) {
        return channel_pipeline::Result::Backpressure;
      }
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

  // Never invoked when backpressure is disabled: the policy contributes no
  // writeReadyHook_, so makeHandlerNode never registers this handler and the
  // pipeline has no way to reach it. The method still has to exist to satisfy
  // the OutboundHandler concept.
  template <typename Context>
  void onWriteReady(Context& ctx) noexcept {
    if constexpr (Backpressure::kBackpressureEnabled) {
      this->backpressured_ = false;
      ctx.cancelAwaitWriteReady();
      flushAndPropagateErrors(ctx);
    }
  }

  // The tracker's own subscriptions plus FlushWrites when the pipeline's event
  // enum defines it. A pipeline with neither wires nothing and the whole event
  // path compiles out.
  using EventId = Ev;
  static constexpr auto kSubscribedEvents =
      makeBatcherSubscriptions<Tracker, Ev>();

  // FlushWrites: teardown is imminent, so push the buffered writes downstream
  // now, while the transport still accepts them. A refused write means the
  // socket is already gone and there is nothing left to preserve, so the result
  // is dropped rather than raised as an exception into a pipeline being torn
  // down.
  //
  // Anything else is a write completion, which the tracker turns into per-batch
  // attribution.
  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId ev,
      const channel_pipeline::TypeErasedBox& box) noexcept {
    if constexpr (HasFlushWritesEvent<EventId>) {
      if (ev == EventId::FlushWrites) {
        if (!bufferedWritesQueue_.empty()) {
          scheduler_.cancelAll();
          (void)doFlush(ctx);
        }
        return;
      }
    }
    if constexpr (!std::is_same_v<
                      typename Tracker::EventId,
                      channel_pipeline::NoEvent>) {
      tracker_.onEvent(ctx, ev, box);
    }
  }

  /**
   * Synchronously flush all pending writes.
   * Cancels any scheduled callbacks and flushes immediately.
   */
  void drain() noexcept {
    if (bufferedWritesQueue_.empty()) {
      return;
    }
    scheduler_.cancelAll();
    scheduler_.flushNow();
  }

  // ===========================================================================
  // Flush list
  // ===========================================================================

  // An intrusive list of deferred flush callbacks. Each handler enqueues at
  // most one scheduler-owned entry.
  using FlushList = util::BatchFlushScheduler::FlushList;

  // Redirects the deferred (end-of-loop) flush from the EventBase to a
  // caller-owned list.
  //
  // By default the batcher schedules its deferred flush on the EventBase, which
  // runs it at the end of the current loop iteration. When a flush list is
  // provided, the batcher enqueues onto that list instead of self-scheduling,
  // handing the caller control over when the buffered writes are flushed (the
  // caller drains the list). Frame and byte threshold flushes use the same
  // deferred path and are redirected to the list; the interval timer flush is
  // unaffected.
  //
  // The list must outlive this handler. Pass nullptr to restore EventBase
  // scheduling.
  void setFlushList(FlushList* flushList) noexcept {
    bool shouldScheduleDeferredFlush = needsDeferredFlush();
    scheduler_.setFlushList(flushList);
    if (shouldScheduleDeferredFlush) {
      scheduleDeferredFlush();
    }
  }

  // ===========================================================================
  // Accessors (for testing)
  // ===========================================================================

  size_t pendingBytes() const noexcept { return totalBytesBuffered_; }
  size_t pendingFrames() const noexcept { return bufferedWritesCount_; }
  bool empty() const noexcept { return bufferedWritesQueue_.empty(); }
  bool isBackpressured() const noexcept {
    if constexpr (Backpressure::kBackpressureEnabled) {
      return this->backpressured_;
    } else {
      return false;
    }
  }
  Tracker& tracker() noexcept { return tracker_; }

 private:
  // ===========================================================================
  // Internals
  // ===========================================================================

  void scheduleFlush() noexcept {
    scheduler_.scheduleTimeout(config_.batchingInterval);
  }

  // Enqueues the deferred flush onto the caller-owned list when one is set,
  // otherwise self-schedules on the EventBase.
  void scheduleDeferredFlush() noexcept { scheduler_.scheduleDeferredFlush(); }

  bool needsDeferredFlush() const noexcept {
    if (bufferedWritesQueue_.empty() || isBackpressured()) {
      return false;
    }
    return config_.batchingInterval == std::chrono::milliseconds::zero() ||
        scheduler_.hasScheduledDeferredFlush();
  }

  bool shouldEarlyFlush() const noexcept {
    if (config_.batchingInterval == std::chrono::milliseconds::zero()) {
      return false;
    }
    return bufferedWritesCount_ >= config_.batchingSize ||
        (config_.batchingByteSize != 0 &&
         totalBytesBuffered_ >= config_.batchingByteSize);
  }

  void earlyFlush() noexcept { scheduler_.scheduleEarlyFlush(); }

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
    tracker_.onFlush();

    auto result =
        ctx.fireWrite(channel_pipeline::TypeErasedBox(std::move(batchToSend)));

    if constexpr (Backpressure::kBackpressureEnabled) {
      if (result == channel_pipeline::Result::Backpressure) {
        this->backpressured_ = true;
        ctx.awaitWriteReady();
      }
    }

    return result;
  }

  void clearPendingState() noexcept {
    scheduler_.cancelAll();
    bufferedWritesQueue_.move(); // discard
    bufferedWritesCount_ = 0;
    totalBytesBuffered_ = 0;
    if constexpr (Backpressure::kBackpressureEnabled) {
      this->backpressured_ = false;
    }
  }

  IntervalBatchingHandlerConfig config_;
  util::BatchFlushScheduler scheduler_;

  // When set, deferred flushes enqueue here instead of on the EventBase.
  // Non-owning; the list outlives this handler (reset in handlerRemoved).
  FlushList* flushList_{nullptr};

  folly::IOBufQueue bufferedWritesQueue_{folly::IOBufQueue::cacheChainLength()};
  size_t bufferedWritesCount_{0};
  size_t totalBytesBuffered_{0};

  // Per-write tracker mixin; NoOp by default.
  [[no_unique_address]] Tracker tracker_{};
};

// Default specialization preserves the existing class name for callers that
// don't opt into per-write tracking.
using IntervalBatchingFrameHandler =
    IntervalBatchingFrameHandlerT<NoOpWriteCompletionTracker>;

// Batches identically, but declines to participate in write backpressure:
// no writeReadyHook_, no saturation flag, no awaitWriteReady. Selected by
// FastThriftServerConfig::enableBackpressure.
using IntervalBatchingFrameHandlerNoBackpressure =
    IntervalBatchingFrameHandlerT<
        NoOpWriteCompletionTracker,
        BackpressureDisabled>;

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        IntervalBatchingFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "IntervalBatchingFrameHandler must satisfy OutboundHandler concept");

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        IntervalBatchingFrameHandlerNoBackpressure,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "IntervalBatchingFrameHandlerNoBackpressure must satisfy OutboundHandler "
    "concept");

// The zero-cost claim. makeHandlerNode registers a handler for write-ready
// notification only when it exposes writeReadyHook_, so its absence is what
// keeps the no-backpressure specialization out of the pipeline's
// writeReadyList_ entirely. Losing the hook must also shrink the handler.
static_assert(
    HasWriteReadyHook<IntervalBatchingFrameHandler>,
    "IntervalBatchingFrameHandler must expose writeReadyHook_ so the pipeline "
    "can drive onWriteReady");
static_assert(
    !HasWriteReadyHook<IntervalBatchingFrameHandlerNoBackpressure>,
    "IntervalBatchingFrameHandlerNoBackpressure must not expose "
    "writeReadyHook_, or the pipeline would register it anyway");
static_assert(
    sizeof(IntervalBatchingFrameHandlerNoBackpressure) <
        sizeof(IntervalBatchingFrameHandler),
    "disabling backpressure must remove per-handler state, not just branches");
} // namespace apache::thrift::fast_thrift::frame::write::handler
