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

} // namespace apache::thrift::fast_thrift::channel_pipeline
