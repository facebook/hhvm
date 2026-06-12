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
#include <deque>
#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::frame::write::handler {

/**
 * Composable tracker mixin for batching frame handlers. The batcher invokes
 * the tracker's hooks at three points:
 *   - onWrite()           — per outbound frame entering the current batch.
 *   - onFlush()           — when the current batch is handed off downstream
 *                           (the batch boundary).
 *   - onEvent(ctx, box)   — when the pipeline's per-pipeline event arrives
 *                           via the batcher's onEvent. The tracker subscribes
 *                           only to the raw transport-fired
 *                           TransportWriteComplete event, so its own enriched
 *                           re-fires are never routed back to it.
 *
 * `onEvent` is a member template parameterized on the pipeline's Context
 * type and consumes a `TypeErasedBox` directly, so the tracker — not the
 * batcher — owns the per-pipeline event type.
 */
template <typename T>
concept WriteCompletionTracker = requires(T tracker) {
  { tracker.onWrite() } noexcept;
  { tracker.onFlush() } noexcept;
};

/**
 * Default tracker — fully elided in batchers whose pipeline composition does
 * not opt into per-write fan-out. All hooks are inline no-ops; the compiler
 * removes the calls.
 */
struct NoOpWriteCompletionTracker {
  // Events disabled: NoEvent + an empty subscription list. The batcher forwards
  // these uniformly, so with this tracker it subscribes to nothing and the
  // whole event path compiles out.
  using EventId = apache::thrift::fast_thrift::channel_pipeline::NoEvent;
  static constexpr std::array<EventId, 0> kSubscribedEvents{};

  void onWrite() noexcept {}
  void onFlush() noexcept {}

  template <typename Context>
  void onEvent(
      Context& /*ctx*/,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
      /*box*/) noexcept {}
};

static_assert(
    WriteCompletionTracker<NoOpWriteCompletionTracker>,
    "NoOpWriteCompletionTracker must satisfy WriteCompletionTracker concept");

/**
 * Concrete tracker — counts outbound frames per batch and, on each raw
 * TransportWriteComplete from transport, pops the front batch's frame count
 * and fires a RocketWriteComplete (enriched with frameCount) upstream
 * via `EventFactory::makeRocketWriteComplete(status, count, bytes)`.
 *
 * Templated on the pipeline's event factory; the factory must expose:
 *   - `using EventId = ...;` with `TransportWriteComplete` and
 *     `RocketWriteComplete` values.
 *   - `using TransportWriteCompleteEventType = ...;` — the message carried by
 *     the TransportWriteComplete event, with `status` and `bytes` fields.
 *   - `static TypeErasedBox makeRocketWriteComplete(status, count, bytes)
 * noexcept;`
 *
 * EB-thread only — no synchronization. The batch-count FIFO stays in
 * lockstep with the transport's writeSuccess/writeErr FIFO ordering
 * (per AsyncSocket's structural write-queue guarantee).
 */
template <typename EventFactory>
class WriteCompletionTrackerT {
 public:
  // The tracker subscribes to the raw transport event and re-fires the
  // enriched one. Sourced from the factory so the tracker stays agnostic of
  // the concrete protocol enum. Subscribing only to TransportWriteComplete
  // means its own RocketWriteComplete re-fires are never routed back to it.
  using EventId = typename EventFactory::EventId;
  static constexpr std::array<EventId, 1> kSubscribedEvents{
      EventId::TransportWriteComplete};

  void onWrite() noexcept { ++framesInCurrentBatch_; }

  void onFlush() noexcept {
    if (framesInCurrentBatch_ == 0) {
      return;
    }
    batchFrameCounts_.push_back(framesInCurrentBatch_);
    framesInCurrentBatch_ = 0;
  }

  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    using TransportEvent =
        typename EventFactory::TransportWriteCompleteEventType;
    auto& evt = box.template get<TransportEvent>();
    if (batchFrameCounts_.empty()) {
      // Defensive: writeSuccess without a prior flush shouldn't happen.
      return;
    }
    auto count = batchFrameCounts_.front();
    batchFrameCounts_.pop_front();
    auto [eventId, eventMsg] =
        EventFactory::makeRocketWriteComplete(evt.status, count, evt.bytes);
    ctx.fireEvent(eventId, std::move(eventMsg));
  }

 private:
  size_t framesInCurrentBatch_{0};
  std::deque<size_t> batchFrameCounts_;
};

} // namespace apache::thrift::fast_thrift::frame::write::handler
