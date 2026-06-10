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

#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * RocketClientEventId — the rocket-client pipeline's event enum (the
 * `EventEnum` the pipeline is built with). Subscribers register per id, so the
 * id selects which subscribers wake; the boxed payload is always a
 * RocketClientEvent, further discriminated by RocketClientEvent::kind.
 */
enum class RocketClientEventId : std::uint32_t {
  WriteComplete,
  ConnectionClose,
  Count,
};

/**
 * RocketClientEvent — the single boxed payload the rocket-client pipeline
 * carries on every event id. The flavor is discriminated by `kind`:
 *
 *   - BatchWriteComplete: the raw socket-level write-completion fired by
 *     TransportHandlerT (via RocketClientEventFactory::make) once per
 *     writev. `frameCount` is meaningless (0).
 *
 *   - RocketWriteComplete: enriched per-rocket-batch completion fired by
 *     WriteCompletionTrackerT (via makeRocketWriteComplete) after popping
 *     one entry from its frame-count FIFO. `frameCount` is the number of
 *     rocket frames in that batch (> 0). Consumers that need per-request
 *     attribution subscribe to this kind and pop `frameCount` entries from
 *     their own outbound FIFO per event.
 *
 *   - ConnectionClose: the server sent RSocket ERROR(CONNECTION_CLOSE), the
 *     graceful-drain signal. Fired under RocketClientEventId::ConnectionClose
 *     by RocketClientConnectionErrorHandler; the RocketClientAppAdapter
 *     (pipeline tail) relays it to the upper (thrift) layer via onClose. The
 *     kind is the whole signal — status/frameCount/bytes are unused. Unlike an
 *     exception, this never fails in-flight work.
 *
 * Handlers that implement onEvent must discriminate on `kind` and ignore
 * kinds they don't consume.
 */
struct RocketClientEvent {
  enum class Kind : uint8_t {
    BatchWriteComplete,
    RocketWriteComplete,
    ConnectionClose,
  };

  Kind kind;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

} // namespace apache::thrift::fast_thrift::rocket::client
