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

#include <cstddef>
#include <cstdint>
#include <optional>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerEventType — the event enum for thrift-server pipelines.
 *
 * Used as the channel pipeline's EventEnum template parameter: each value
 * names a distinct user event that a pipeline handler or endpoint may
 * subscribe to via `kSubscribedEvents`. `fireEvent(type, ...)` reaches only
 * the subscribers of `type`. Most events are payload-less — the type is the
 * whole signal, so emitters fire an empty `TypeErasedBox` — except
 * `WriteComplete`, which carries a `ThriftServerWriteCompleteEvent`.
 *
 * The trailing `Count` sentinel gives the pipeline the number of event types
 * at compile time and must remain the last enumerator.
 */
enum class ThriftServerEventType : std::uint32_t {
  // Connection close requested. The pipeline-resident
  // ThriftServerConnectionCloseHandler picks this up and runs the terminal
  // state machine (drain → reap → LOG(FATAL) on stuck callbacks). Emitted by
  // the tail adapter's close(), and by any handler refusing the connection.
  // Carries a ThriftServerCloseConnectionEvent naming what to tell the client.
  CloseConnection,
  // Emitted inbound by ThriftServerConnectionCloseHandler when the
  // connection has finished settling — all in-flight handler
  // callbacks have either returned (graceful drain or reap) or the
  // reap deadline fired (LOG(FATAL) handled inside the close handler
  // before this is emitted, so consumers can assume in-flight == 0).
  // Tail adapter fires its user closeCallback in response.
  ConnectionClosed,
  // Per-batch write completion relayed up from the rocket pipeline by
  // ThriftServerTransportAdapter. Carries a ThriftServerWriteCompleteEvent.
  WriteComplete,
  // The SETUP response is on the wire and the connection can carry requests.
  // Relayed up from the rocket pipeline's setup handler by
  // ThriftServerTransportAdapter, after the answer has been written and before
  // any request can arrive. Carries a ThriftServerSetupCompleteEvent*, whose
  // reject slot a subscriber fills to refuse a connection the server has
  // already answered.
  SetupComplete,
  // Sentinel: number of event types. Keep last.
  Count,
};

/**
 * Payload for ThriftServerEventType::CloseConnection — what the client is told
 * on the way out. Empty means an orderly shutdown, answered with
 * CONNECTION_CLOSE; a rejection means the connection is refused, answered with
 * that error code and reason.
 */
struct ThriftServerCloseConnectionEvent {
  std::optional<SetupRejection> rejection;
};

/**
 * Payload for ThriftServerEventType::WriteComplete — one outbound rocket frame
 * reached the socket, relayed up from the rocket pipeline. See
 * RocketWriteCompleteEvent; relayed through unchanged.
 */
struct ThriftServerWriteCompleteEvent {
  uint32_t streamId;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
};

/**
 * Payload for ThriftServerEventType::SetupComplete. Fired by pointer so a
 * subscriber can fill the out-slot in place; the emitter reads it back once
 * dispatch returns.
 *
 * Refusing here is a different act from refusing during the setup exchange:
 * the client has already been answered, so it sees an error frame following
 * the SETUP response rather than instead of it.
 */
struct ThriftServerSetupCompleteEvent {
  std::optional<SetupRejection> reject;
};

} // namespace apache::thrift::fast_thrift::thrift
