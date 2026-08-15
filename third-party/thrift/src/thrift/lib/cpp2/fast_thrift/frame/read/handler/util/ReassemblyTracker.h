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

#include <chrono>
#include <concepts>
#include <cstdint>
#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

namespace apache::thrift::fast_thrift::frame::read::handler {

/**
 * Composable tracker mixin for FrameDefragmentationHandlerT.
 *
 * The handler invokes `onFirstFragment(ctx, streamId)` at exactly one point:
 * when a stream's first fragment starts an accumulation. Nothing is reported
 * for a frame that arrives whole, because such a frame is not reassembled and
 * the handler forwards it untouched.
 *
 * That single hook is the only thing the frame layer can say about reassembly
 * timing without acquiring an opinion about what the timing is for. What the
 * arrival of a first fragment *means* belongs to the layer above.
 *
 * A tracker only ever fires; it never subscribes. `ctx.fireEvent` is a template
 * over any event enum, so a tracker needs no `EventId` typedef and no
 * `kSubscribedEvents` — and the handler carrying it needs neither either.
 */
template <typename T>
concept ReassemblyTracker = requires(
    T tracker,
    apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl& ctx,
    uint32_t streamId) {
  { tracker.onFirstFragment(ctx, streamId) } noexcept;
};

/**
 * Default tracker — the hook is an inline no-op and the compiler removes the
 * call. Empty, so `[[no_unique_address]]` collapses it to zero bytes.
 */
struct NoOpReassemblyTracker {
  template <typename Context>
  void onFirstFragment(Context& /*ctx*/, uint32_t /*streamId*/) noexcept {}
};

static_assert(
    ReassemblyTracker<NoOpReassemblyTracker>,
    "NoOpReassemblyTracker must satisfy ReassemblyTracker concept");

/**
 * Event factory contract for FirstFragmentTrackerT. Pins the exact
 * `(streamId, arrivalTime)` argument order and `pair<EventId, TypeErasedBox>`
 * result, so a mismatched factory is rejected at the point of instantiation
 * rather than deep inside the tracker body.
 */
template <typename T>
concept FirstFragmentEventFactory = requires(
    uint32_t streamId, std::chrono::steady_clock::time_point arrivalTime) {
  typename T::EventId;
  {
    T::makeFirstResponseFrame(streamId, arrivalTime)
  } noexcept -> std::same_as<std::pair<
      typename T::EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>;
};

/**
 * Concrete tracker — timestamps a stream's first fragment and fires the
 * protocol layer's event so a subscriber above the defragmentation handler can
 * learn when reassembly started.
 *
 * This exists because reassembly destroys the information. Above the
 * defragmentation handler only the assembled frame is ever observed, so a
 * subscriber there cannot distinguish "the response arrived now" from "the
 * response finished arriving now" — the two differ by exactly the reassembly
 * window this tracker reports.
 *
 * EB-thread only — no synchronization.
 */
template <FirstFragmentEventFactory EventFactory>
class FirstFragmentTrackerT {
 public:
  template <typename Context>
  void onFirstFragment(Context& ctx, uint32_t streamId) noexcept {
    auto [eventId, eventMsg] = EventFactory::makeFirstResponseFrame(
        streamId, std::chrono::steady_clock::now());
    ctx.fireEvent(eventId, std::move(eventMsg));
  }
};

} // namespace apache::thrift::fast_thrift::frame::read::handler
