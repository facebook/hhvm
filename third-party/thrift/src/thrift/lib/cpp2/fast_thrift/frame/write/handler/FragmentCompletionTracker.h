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

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>

#include <glog/logging.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::frame::write::handler {

/**
 * Composable tracker mixin for the fragmentation handler. The handler invokes
 * the tracker at two points:
 *   - onFragment(streamId, isLastFragment) — per fragment written downstream
 *     in doFlush. `isLastFragment` is true when this fragment completes the
 *     original frame (either unfragmented fast-path or final SRPT chunk).
 *   - onFlush()                           — when a doFlush completes
 *     successfully (all queued fragments drained).
 *
 * The concrete tracker maintains a per-fragment FIFO and subscribes to
 * BatchWriteComplete; on each batch completion it pops `frameCount` records
 * and fires FrameWriteComplete for every completed original frame.
 */
template <typename T>
concept FragmentCompletionTracker =
    requires(T tracker, uint32_t streamId, bool isLast) {
      { tracker.onFragment(streamId, isLast) } noexcept;
      { tracker.onFlush() } noexcept;
    };

/**
 * Default tracker — fully elided when the pipeline does not opt into
 * per-fragment write completion. All hooks are inline no-ops.
 */
struct NoOpFragmentCompletionTracker {
  using EventId = apache::thrift::fast_thrift::channel_pipeline::NoEvent;
  static constexpr std::array<EventId, 0> kSubscribedEvents{};

  void onFragment(uint32_t, bool) noexcept {}
  void onFlush() noexcept {}

  template <typename Context>
  void onEvent(
      Context& /*ctx*/,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
      /*box*/) noexcept {}
};

static_assert(
    FragmentCompletionTracker<NoOpFragmentCompletionTracker>,
    "NoOpFragmentCompletionTracker must satisfy FragmentCompletionTracker concept");

/**
 * Event factory contract for FragmentCompletionTrackerT. Pins the member types
 * the tracker reads off and the exact `(status, streamId)` argument order and
 * `pair<EventId, TypeErasedBox>` result of the per-frame factory method, so a
 * mismatched factory is rejected at the point of instantiation rather than deep
 * inside the tracker body.
 */
template <typename T>
concept FrameWriteCompleteEventFactory = requires(
    typename T::BatchWriteCompleteEventType batchEvent, uint32_t streamId) {
  typename T::EventId;
  typename T::BatchWriteCompleteEventType;
  {
    T::makeFrameWriteComplete(batchEvent.status, streamId)
  } noexcept -> std::same_as<std::pair<
      typename T::EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>;
};

/**
 * Concrete tracker — records {streamId, isLastFragment} per fragment written
 * downstream. On each BatchWriteComplete (from the downstream batch tracker),
 * pops `frameCount` fragment records and fires FrameWriteComplete{streamId}
 * for every record where isLastFragment is true.
 *
 * Templated on the pipeline's event factory (see
 * FrameWriteCompleteEventFactory). The factory must expose:
 *   - `using EventId = ...;` with `BatchWriteComplete` and
 *     `FrameWriteComplete` values.
 *   - `using BatchWriteCompleteEventType = ...;` — the message carried by
 *     the BatchWriteComplete event, with `status`, `frameCount`, and `bytes`.
 *   - `static makeFrameWriteComplete(status, streamId) noexcept;`
 *
 * EB-thread only — no synchronization.
 */
template <FrameWriteCompleteEventFactory EventFactory>
class FragmentCompletionTrackerT {
 public:
  using EventId = typename EventFactory::EventId;
  static constexpr std::array<EventId, 1> kSubscribedEvents{
      EventId::BatchWriteComplete};

  void onFragment(uint32_t streamId, bool isLastFragment) noexcept {
    fragments_.push_back({streamId, isLastFragment});
  }

  void onFlush() noexcept {}

  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    using BatchEvent = typename EventFactory::BatchWriteCompleteEventType;
    auto& evt = box.template get<BatchEvent>();
    size_t remaining = evt.frameCount;
    while (remaining > 0 && !fragments_.empty()) {
      auto rec = fragments_.front();
      fragments_.pop_front();
      --remaining;
      if (rec.isLastFragment) {
        auto [eventId, eventMsg] =
            EventFactory::makeFrameWriteComplete(evt.status, rec.streamId);
        ctx.fireEvent(eventId, std::move(eventMsg));
      }
    }
    // frameCount must be matched by exactly that many tracked fragments. A
    // leftover count means the FIFO held fewer records than the batch reported,
    // so the tracker ran out mid-pop and silently dropped the completions for
    // this batch's remaining frames (FIFO/batch drift).
    DCHECK_EQ(remaining, 0u)
        << "FragmentCompletionTracker: BatchWriteComplete frameCount exceeds "
           "tracked fragments";
  }

 private:
  struct FragmentRecord {
    uint32_t streamId;
    bool isLastFragment;
  };
  std::deque<FragmentRecord> fragments_;
};

} // namespace apache::thrift::fast_thrift::frame::write::handler
