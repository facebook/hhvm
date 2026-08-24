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
 * the tracker's hooks at four points:
 *   - onWrite()           — per outbound frame entering the current batch.
 *   - onFlush()           — when the current batch is handed off downstream
 *                           (the batch boundary).
 *   - onDiscard()         — when the batcher abandons its buffered writes
 *                           instead of flushing them, so the tracker's counts
 *                           don't outlive the frames they stand for.
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
  { tracker.onDiscard() } noexcept;
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
  static constexpr apache::thrift::fast_thrift::channel_pipeline::
      Subscriptions<>
          kSubscribedEvents{};

  void onWrite() noexcept {}
  void onFlush() noexcept {}
  void onDiscard() noexcept {}

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
 * Whether a pipeline's event enum defines FlushWrites — the request to push
 * buffered outbound bytes downstream immediately, ahead of a teardown.
 */
template <typename E>
concept HasFlushWritesEvent = requires { E::FlushWrites; };

namespace detail {
template <auto... A, auto... B>
constexpr apache::thrift::fast_thrift::channel_pipeline::
    Subscriptions<A..., B...>
    concatSubscriptions(
        apache::thrift::fast_thrift::channel_pipeline::Subscriptions<A...>,
        apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
            B...>) noexcept {
  return {};
}
} // namespace detail

/**
 * A batching handler's subscription list: whatever its tracker subscribes to,
 * plus FlushWrites when the pipeline's event enum defines it. A pipeline whose
 * enum has neither gets an empty list and no event wiring at all.
 */
template <typename Tracker, typename Ev>
constexpr auto makeBatcherSubscriptions() noexcept {
  if constexpr (HasFlushWritesEvent<Ev>) {
    return detail::concatSubscriptions(
        Tracker::kSubscribedEvents,
        apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
            Ev::FlushWrites>{});
  } else {
    return Tracker::kSubscribedEvents;
  }
}

/**
 * Event factory contract for WriteCompletionTrackerT. Pins the member types the
 * tracker reads off and the exact `(status, count, bytes, quiesced)` argument
 * order and `pair<EventId, TypeErasedBox>` result of the batch factory method,
 * so a mismatched factory is rejected at the point of instantiation rather than
 * deep inside the tracker body.
 */
template <typename T>
concept BatchWriteCompleteEventFactory = requires(
    typename T::TransportWriteCompleteEventType transportEvent,
    size_t frameCount,
    bool quiesced) {
  typename T::EventId;
  typename T::TransportWriteCompleteEventType;
  {
    T::makeBatchWriteComplete(
        transportEvent.status, frameCount, transportEvent.bytes, quiesced)
  } noexcept -> std::same_as<std::pair<
      typename T::EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>;
};

/**
 * Concrete tracker — counts outbound frames per batch and, on each raw
 * TransportWriteComplete from transport, pops the front batch's frame count
 * and fires a BatchWriteComplete (enriched with frameCount and whether egress
 * has gone idle) upstream via
 * `EventFactory::makeBatchWriteComplete(status, count, bytes, quiesced)`.
 *
 * Batch-level is as far as this tracker's knowledge goes. Turning a batch
 * completion into whatever the pipeline's upper layers want — per-frame, or
 * per-connection — belongs to the handler above, which is the first one that
 * knows what the batch was made of.
 *
 * The tracker is the only place that can decide quiescence: it owns both the
 * queue of batches handed to the socket and the count of frames still buffered
 * for the next flush.
 *
 * Quiescence is an edge carried on a completion, not a state to be polled, and
 * the edge is not guaranteed to arrive: only a completion that pops a batch
 * reports it, and a connection torn down with batches still outstanding stops
 * receiving completions altogether. A consumer that releases a resource on
 * quiescence must release it on teardown too.
 *
 * Templated on the pipeline's event factory (see
 * BatchWriteCompleteEventFactory). The factory must expose:
 *   - `using EventId = ...;` with `TransportWriteComplete` and
 *     `BatchWriteComplete` values.
 *   - `using TransportWriteCompleteEventType = ...;` — the message carried by
 *     the TransportWriteComplete event, with `status` and `bytes` fields.
 *   - `static TypeErasedBox makeBatchWriteComplete(status, count, bytes,
 * quiesced) noexcept;`
 *
 * EB-thread only — no synchronization. The batch-count FIFO stays in
 * lockstep with the transport's writeSuccess/writeErr FIFO ordering
 * (per AsyncSocket's structural write-queue guarantee).
 */
template <BatchWriteCompleteEventFactory EventFactory>
class WriteCompletionTrackerT {
 public:
  // The tracker subscribes to the raw transport event and re-fires the
  // enriched one. Sourced from the factory so the tracker stays agnostic of
  // the concrete protocol enum. Subscribing only to TransportWriteComplete
  // means its own BatchWriteComplete re-fires are never routed back to it.
  using EventId = typename EventFactory::EventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
      EventId::TransportWriteComplete>
      kSubscribedEvents{};

  void onWrite() noexcept { ++framesInCurrentBatch_; }

  void onFlush() noexcept {
    if (framesInCurrentBatch_ == 0) {
      return;
    }
    batchFrameCounts_.push_back(framesInCurrentBatch_);
    framesInCurrentBatch_ = 0;
  }

  // The batcher threw away what it had buffered, so the counts standing for
  // those frames have to go too — otherwise the partial batch is charged to
  // whatever flushes next and quiescence can never be reached again. The
  // batcher only discards on teardown, which is also the point past which no
  // completion arrives for the batches already handed to the socket.
  void onDiscard() noexcept {
    framesInCurrentBatch_ = 0;
    batchFrameCounts_.clear();
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
    // Egress has gone idle when this completion leaves nothing else handed to
    // the socket and nothing buffered awaiting a flush. Both halves are load
    // bearing: with backpressure disabled the batcher keeps flushing without
    // waiting for completions, so an empty FIFO on its own can still be
    // followed immediately by a batch that is only buffered.
    const bool quiesced =
        batchFrameCounts_.empty() && framesInCurrentBatch_ == 0;
    auto [eventId, eventMsg] = EventFactory::makeBatchWriteComplete(
        evt.status, count, evt.bytes, quiesced);
    ctx.fireEvent(eventId, std::move(eventMsg));
  }

 private:
  size_t framesInCurrentBatch_{0};
  std::deque<size_t> batchFrameCounts_;
};

} // namespace apache::thrift::fast_thrift::frame::write::handler
