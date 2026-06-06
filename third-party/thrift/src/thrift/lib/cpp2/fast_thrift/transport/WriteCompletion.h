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
#include <cstdint>
#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::transport {

/**
 * Outcome of a single socket-level writeChain on TransportHandler.
 * Carried in the per-write event a WriteCompleteEventFactory produces.
 */
enum class WriteCompletionStatus : uint8_t {
  Success,
  Error,
};

namespace detail {
// True only for `std::pair<E, TypeErasedBox>` where `E` is a pipeline event
// enum — the exact shape `make` must return so its result feeds straight into
// the pipeline's typed `fireEvent(eventId, eventMessage)`.
template <typename T>
inline constexpr bool isWriteCompleteEventResult = false;
template <apache::thrift::fast_thrift::channel_pipeline::EventEnum E>
inline constexpr bool isWriteCompleteEventResult<std::pair<
    E,
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>> = true;
} // namespace detail

/**
 * Concept for the per-pipeline factory that constructs the write-completion
 * event TransportHandler fires from writeSuccess / writeErr. The factory's
 * `make` is invoked with the per-write outcome and (on error) the partially
 * written byte count; it returns the event id paired with the boxed event the
 * pipeline carries. The id is a value of the pipeline's shared event enum (the
 * transport has no enum of its own), so the pair forwards straight into the
 * typed `fireEvent`.
 */
template <typename T>
concept WriteCompleteEventFactory =
    requires(WriteCompletionStatus status, size_t bytes) {
      { T::make(status, bytes) } noexcept;
    } &&
    detail::isWriteCompleteEventResult<decltype(T::make(
        WriteCompletionStatus{}, size_t{}))>;

/**
 * Compile-time-elided default factory. TransportHandler gates its fireEvent
 * via `std::is_same_v<Factory, NoOpWriteCompleteEventFactory>`; for pipelines
 * that don't consume the event (tests, bare TCP), the entire fireEvent call
 * is removed by `if constexpr`. The returned pair is never fired — it exists
 * only to satisfy the concept.
 */
struct NoOpWriteCompleteEventFactory {
  static std::pair<
      apache::thrift::fast_thrift::channel_pipeline::NoEvent,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
  make(WriteCompletionStatus, size_t) noexcept {
    return {};
  }
};

static_assert(
    WriteCompleteEventFactory<NoOpWriteCompleteEventFactory>,
    "NoOpWriteCompleteEventFactory must satisfy WriteCompleteEventFactory concept");

} // namespace apache::thrift::fast_thrift::transport
