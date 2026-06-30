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
#include <cstdint>
#include <type_traits>

#include <folly/IntrusiveList.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

// Forward declarations — EventHook references these only through pointers.
class TypeErasedBox;
namespace detail {
class ContextImpl;
} // namespace detail

/**
 * NoEvent is the default event enum and disables the event subsystem.
 *
 * A pipeline parameterized with NoEvent allocates no event lists, links no
 * hooks, and compiles out all event wiring — it pays nothing for events.
 */
enum class NoEvent : std::uint32_t { Count = 0 };

/**
 * EventEnum constrains the type that identifies a pipeline's user events.
 *
 * It must be a `uint32_t`-backed enum class whose last value is `Count`, the
 * sentinel that gives the pipeline its number of distinct event types at
 * compile time. NoEvent satisfies this and means "events disabled".
 */
template <typename E>
concept EventEnum = std::is_enum_v<E> &&
    std::same_as<std::underlying_type_t<E>, std::uint32_t> &&
    requires { E::Count; };

/**
 * Whether events are enabled for a given event enum. False only for NoEvent.
 */
template <typename E>
inline constexpr bool kEventsEnabled = !std::same_as<E, NoEvent>;

/**
 * Number of distinct event types — i.e. the value of the `Count` sentinel.
 */
template <typename E>
inline constexpr std::uint32_t kEventCount =
    static_cast<std::uint32_t>(E::Count);

/**
 * LayerEvent — an EventEnum that also exposes `Base`, the first (anchor) id of
 * its range. Anchored layer enums set `Base` to the layer below's `Count` (or 0
 * for the bottom layer) so all layers tile one contiguous id space. Required
 * only to participate in the kLayersTile compile-time check.
 */
template <typename E>
concept LayerEvent = EventEnum<E> && requires { E::Base; };

/**
 * Base id for a layer anchored directly above `Prev`: `Prev`'s `Count`. The
 * bottom layer uses 0. Lets a layer enum anchor its `Base` without a
 * hand-written cast: `enum class FrameEvent { Base =
 * kLayerBaseAfter<Transport>,
 * ... }`.
 */
template <EventEnum Prev>
inline constexpr std::uint32_t kLayerBaseAfter =
    static_cast<std::uint32_t>(Prev::Count);

/**
 * Whether the given layer enums, in order, tile [0, last::Count) with no gaps
 * and no overlaps: the first layer's `Base` is 0 and each subsequent layer's
 * `Base` equals the previous layer's `Count`. A mis-anchored, reordered, or
 * forgotten-anchor layer makes this false, so `static_assert(kLayersTile<...>)`
 * at the pipeline-assembly site turns a silent id collision into a compile
 * error.
 */
template <LayerEvent... Layers>
inline constexpr bool kLayersTile = [] {
  constexpr auto n = sizeof...(Layers);
  if constexpr (n == 0) {
    return true;
  } else {
    constexpr std::array<std::uint32_t, n> bases = {
        static_cast<std::uint32_t>(Layers::Base)...};
    constexpr std::array<std::uint32_t, n> counts = {
        static_cast<std::uint32_t>(Layers::Count)...};
    if (bases[0] != 0) {
      return false;
    }
    for (std::uint32_t i = 1; i < n; ++i) {
      // No gap and no overlap: each layer starts exactly where the last ended.
      if (bases[i] != counts[i - 1]) {
        return false;
      }
    }
    return true;
  }
}();

/**
 * EventHook is the per-(subscriber, event-type) registration record.
 *
 * A subscriber owns one EventHook for each event type it subscribes to, and
 * each hook is linked into the intrusive list dedicated to that event type.
 * `fireEvent(ev)` walks only the list for `ev`, so a subscriber is invoked
 * exactly for the events it registered for — no broadcast, no filtering.
 *
 * The hook is self-contained, carrying everything needed to dispatch, so
 * internal handlers and head/tail endpoints share the same per-event lists:
 *   - fn:     type-erased onEvent thunk for the subscriber
 *   - target: the subscriber instance (handler or endpoint)
 *   - ctx:    the subscriber's context; nullptr for endpoints
 */
struct EventHook {
  using DispatchFn = void (*)(
      void* target,
      detail::ContextImpl* ctx,
      std::uint32_t ev,
      const TypeErasedBox& eventMessage) noexcept;

  folly::IntrusiveListHook hook;
  DispatchFn fn{nullptr};
  void* target{nullptr};
  detail::ContextImpl* ctx{nullptr};
};

/**
 * List of subscribers for a single event type. The pipeline holds one per
 * event type and walks the relevant one on `fireEvent`.
 */
using EventList = folly::IntrusiveList<EventHook, &EventHook::hook>;

/**
 * A single subscription: the global event id plus the typed dispatch thunk that
 * casts the id back to the subscriber's layer enum and invokes its onEvent. The
 * pipeline links one EventHook per subscription.
 */
struct EventSubscription {
  std::uint32_t id{0};
  EventHook::DispatchFn thunk{nullptr};
};

/**
 * Subscriptions<Evs...> is the set of events a handler or endpoint subscribes
 * to, declared as its static `kSubscribedEvents`. The values may come from
 * several layer enums — a handler listens to its own layer and any lower layer
 * — so each carries its own enum type and onEvent is dispatched typed per
 * layer. Layer enums share one anchored id space, so each value is its global
 * id.
 */
template <auto... Evs>
struct Subscriptions {};

} // namespace apache::thrift::fast_thrift::channel_pipeline
