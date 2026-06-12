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
 * `EventEnum` the pipeline is built with). Each value is a distinct pipeline
 * event with its own message type, or no payload for pure signals; subscribers
 * select the events they consume via `kSubscribedEvents` and the id alone
 * identifies the message.
 */
enum class RocketClientEventId : std::uint32_t {
  // Raw socket-level write-completion fired by TransportHandlerT (via
  // RocketClientEventFactory::make) once per writev. Carries
  // TransportWriteCompleteEvent.
  TransportWriteComplete,
  // Enriched per-rocket-batch completion fired by WriteCompletionTrackerT (via
  // makeRocketWriteComplete) after popping one entry from its frame-count FIFO.
  // Carries RocketWriteCompleteEvent.
  RocketWriteComplete,
  // The server sent RSocket ERROR(CONNECTION_CLOSE), the graceful-drain signal.
  // Fired by RocketClientConnectionErrorHandler; the RocketClientAppAdapter
  // (pipeline tail) relays it to the upper (thrift) layer via onClose. The id
  // is the whole signal — it carries no payload. Unlike an exception, this
  // never fails in-flight work.
  ConnectionClose,
  Count,
};

/**
 * Message for RocketClientEventId::TransportWriteComplete — the outcome of one
 * socket-level writev.
 */
struct TransportWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t bytes;
};

/**
 * Message for RocketClientEventId::RocketWriteComplete — the completion of one
 * rocket-frame batch. `frameCount` is the number of rocket frames in that batch
 * (> 0); consumers that need per-request attribution pop `frameCount` entries
 * from their own outbound FIFO per event.
 */
struct RocketWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

} // namespace apache::thrift::fast_thrift::rocket::client
