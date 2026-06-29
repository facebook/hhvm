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
  // One per flushed batch. Carries BatchWriteCompleteEvent. Consumed by the
  // WriteCompletionRouter (or FragmentCompletionTracker when fragmentation is
  // enabled), which fans it out into per-frame FrameWriteComplete events.
  BatchWriteComplete,
  // Per-frame write completion — one per original outbound frame, carrying the
  // streamId so StreamStateHandler can look up the request context. Fired by
  // WriteCompletionRouter (simple 1:1 fan-out from BatchWriteComplete) or by
  // FragmentCompletionTracker (fragment reassembly). Carries
  // FrameWriteCompleteEvent.
  FrameWriteComplete,
  // Per-request write completion fired by StreamStateHandler after resolving
  // FrameWriteComplete.streamId to the request's context pointer. Carries
  // RocketWriteCompleteEvent.
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
 * Message for RocketClientEventId::BatchWriteComplete — the completion of one
 * rocket-frame batch. `frameCount` is the number of rocket frames in that batch
 * (> 0). Consumed by the WriteCompletionRouter, which pops `frameCount` entries
 * from its outbound FIFO and fans them out as per-write RocketWriteComplete
 * events.
 */
struct BatchWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

/**
 * Message for RocketClientEventId::FrameWriteComplete — the completion of one
 * original outbound frame (or one reassembled request when fragmentation is
 * active). Carries the streamId so StreamStateHandler can look up the request
 * context from its per-stream slot map.
 */
struct FrameWriteCompleteEvent {
  uint32_t streamId;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
};

/**
 * Message for RocketClientEventId::RocketWriteComplete — the completion of one
 * individual write. `requestContext` is a NON-OWNING borrow of that request's
 * context (the same TypeErasedPtr the rocket layer carries opaquely); it is
 * only valid for the duration of the onEvent call — subscribers must not retain
 * it. The owning layer (which knows the concrete context type) static_casts it.
 */
struct RocketWriteCompleteEvent {
  void* requestContext;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
};

} // namespace apache::thrift::fast_thrift::rocket::client
