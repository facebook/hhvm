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
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

/**
 * Typed event structs for the unified Thrift stream/sink metric logging layer.
 *
 * Each struct represents a single lifecycle event that can be dispatched to
 * IThriftServerCounters (real-time counters) or IThriftRequestLogging
 * (completion-time structured logging). The variants below group events
 * by stream vs. sink to allow type-safe dispatch in the log classes.
 */
namespace apache::thrift::detail {

// ---------------------------------------------------------------------------
// Stream events
// ---------------------------------------------------------------------------

struct StreamSubscribeEvent {
  std::optional<std::chrono::steady_clock::time_point> interactionCreationTime;
};

struct StreamNextEvent {
  uint64_t payloadBytes{0};
};

struct StreamNextSentEvent {};

struct StreamCreditEvent {
  uint32_t credits;
};

enum class StreamPauseReason {
  NO_CREDITS,
  EXPLICIT_PAUSE,
};

struct StreamPauseEvent {
  StreamPauseReason reason;
};

struct StreamResumeEvent {};

enum class StreamEndReason {
  COMPLETE,
  ERROR,
  CANCEL,
};

struct StreamCompleteEvent {
  StreamEndReason reason;
};

using StreamEvent = std::variant<
    StreamSubscribeEvent,
    StreamNextEvent,
    StreamNextSentEvent,
    StreamCreditEvent,
    StreamPauseEvent,
    StreamResumeEvent,
    StreamCompleteEvent>;

// ---------------------------------------------------------------------------
// Sink events
// ---------------------------------------------------------------------------

struct SinkSubscribeEvent {
  std::optional<std::chrono::steady_clock::time_point> interactionCreationTime;
};

struct SinkNextEvent {
  uint64_t payloadBytes{0};
};

struct SinkConsumedEvent {};

struct SinkCancelEvent {};

struct SinkCreditEvent {
  uint32_t credits;
};

enum class SinkEndReason {
  COMPLETE,
  ERROR,
  COMPLETE_WITH_ERROR,
};

struct SinkCompleteEvent {
  SinkEndReason reason;
};

using SinkEvent = std::variant<
    SinkSubscribeEvent,
    SinkNextEvent,
    SinkConsumedEvent,
    SinkCancelEvent,
    SinkCreditEvent,
    SinkCompleteEvent>;

// ---------------------------------------------------------------------------
// BiDi events
// ---------------------------------------------------------------------------

struct BiDiSubscribeEvent {};

struct BiDiSinkNextEvent {};

struct BiDiSinkCreditEvent {
  uint32_t credits;
};

struct BiDiStreamNextEvent {};

struct BiDiStreamCreditEvent {
  uint32_t credits;
};

struct BiDiStreamPauseEvent {
  StreamPauseReason reason;
};

enum class BiDiEndReason {
  COMPLETE,
  CANCEL,
  ERROR,
};

struct BiDiFinallyEvent {
  BiDiEndReason reason;
};

} // namespace apache::thrift::detail
