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
  // Enriched per-batch completion fired by WriteCompletionTrackerT (via
  // makeBatchWriteComplete) after popping one entry from its frame-count FIFO.
  // One per flushed batch. Carries BatchWriteCompleteEvent. Consumed by the
  // fragmentation handler's tracker, which is the first handler above the
  // batcher that knows what the batch was made of.
  BatchWriteComplete,
  // Per-frame write completion — one per original outbound frame, carrying the
  // streamId. Fired by FragmentCompletionTracker, which reassembles the batch
  // into the frames it was made of. Carries FrameWriteCompleteEvent.
  FrameWriteComplete,
  // The rocket layer's view of the completion. Carries
  // RocketWriteCompleteEvent.
  RocketWriteComplete,
  // Fired by RocketServerSetupFrameHandler once the client's SETUP frame has
  // passed RSocket validation, to ask the layer above what to answer with.
  // Carries RocketSetupEvent*.
  SetupReceived,
  // Fired by the same handler once that answer is on the write path and the
  // connection is ready to carry requests. Carries RocketSetupCompleteEvent*.
  SetupComplete,
  // Fired before the connection is torn down: buffered outbound frames must
  // reach the socket now, while the transport still accepts writes. Carries no
  // message — the id is the whole signal. Subscribers must not close or
  // deactivate in response; the teardown follows once dispatch returns.
  FlushWrites,
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
 * Message for RocketServerEventId::BatchWriteComplete — the completion of one
 * batch as the batcher saw it. `frameCount` is the number of rocket frames in
 * that batch (> 0).
 */
struct BatchWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

/**
 * Message for RocketServerEventId::FrameWriteComplete — the completion of one
 * original outbound frame, whether it went out whole or in fragments. The
 * streamId is the one the fragmentation handler recorded at write time: below
 * it the frame is serialized bytes and the stream is no longer recoverable.
 */
struct FrameWriteCompleteEvent {
  uint32_t streamId;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
};

/**
 * Message for RocketServerEventId::RocketWriteComplete — one outbound rocket
 * frame reached the socket. The streamId identifies it; unlike the client there
 * is no request context to resolve against, because the server's is carried on
 * the request message rather than held in a per-stream table.
 */
struct RocketWriteCompleteEvent {
  uint32_t streamId;
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
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
