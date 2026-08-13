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
#include <memory>
#include <optional>
#include <string>

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/SetupParameters.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * RocketServerEventId — the rocket-server pipeline's event enum (the
 * `EventEnum` the pipeline is built with). Each value is a distinct pipeline
 * event with its own message type; subscribers select the events they consume
 * via `kSubscribedEvents` and the id alone identifies the message.
 */
enum class RocketServerEventId : std::uint32_t {
  // Raw socket-level write-completion fired by TransportHandlerT (via
  // RocketServerEventFactory::make) once per writev. Carries
  // TransportWriteCompleteEvent.
  TransportWriteComplete,
  // Enriched per-rocket-batch completion fired by WriteCompletionTrackerT (via
  // makeRocketWriteComplete) after popping one entry from its frame-count FIFO.
  // Carries RocketWriteCompleteEvent.
  RocketWriteComplete,
  // Fired by RocketServerSetupFrameHandler once the client's SETUP frame has
  // passed RSocket validation, to ask the layer above what to answer with.
  // Carries RocketSetupEvent*.
  SetupReceived,
  // Fired by the same handler once that answer is on the write path and the
  // connection is ready to carry requests. Carries RocketSetupCompleteEvent*.
  SetupComplete,
  Count,
};

/**
 * Message for RocketServerEventId::TransportWriteComplete — the outcome of one
 * socket-level writev.
 */
struct TransportWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t bytes;
};

/**
 * Message for RocketServerEventId::RocketWriteComplete — the completion of one
 * rocket-frame batch. `frameCount` is the number of rocket frames in that batch
 * (> 0); consumers that need per-request attribution pop `frameCount` entries
 * from their own outbound FIFO per event.
 */
struct RocketWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

/**
 * A refusal of the setup exchange: the frame-layer code to close with, and a
 * human-readable reason for it. The reason reaches the client in the ERROR
 * frame's body, so whoever refuses can say why — the rocket layer never has to
 * understand it, it only carries it.
 */
struct SetupRejection {
  apache::thrift::fast_thrift::frame::ErrorCode code;
  std::string reason;
};

/**
 * Message for RocketServerEventId::SetupReceived — a validated SETUP frame and
 * the slots for the answer to it.
 *
 * This event is a question, not a notification: the emitter reads the response
 * slots back once dispatch returns, so a subscriber fills them in place rather
 * than replying through some other channel. Exactly one of `metadataPush` and
 * `reject` should be set; leaving both empty accepts the connection without
 * pushing anything back, which is what an unsubscribed pipeline does.
 *
 * The event stays thrift-free: only opaque bytes and a frame-layer error code
 * cross the rocket boundary, so interpreting the setup metadata remains the
 * upper layer's business.
 */
struct RocketSetupEvent {
  // In: parameters read off the frame, and the client's opaque setup metadata
  // (null when the frame carried none).
  const SetupParameters* params{nullptr};
  std::unique_ptr<folly::IOBuf> metadata;
  // Out: the response to push back to the client, or the refusal to close with.
  std::unique_ptr<folly::IOBuf> metadataPush;
  std::optional<SetupRejection> reject;
};

/**
 * Message for RocketServerEventId::SetupComplete — the second decision point,
 * once the SETUP response is on the write path and before any request can be
 * dispatched. `reject` is an out-slot: setting it refuses a connection that
 * has already been answered, which the client sees as an error frame following
 * the setup response.
 */
struct RocketSetupCompleteEvent {
  std::optional<SetupRejection> reject;
};

} // namespace apache::thrift::fast_thrift::rocket::server
