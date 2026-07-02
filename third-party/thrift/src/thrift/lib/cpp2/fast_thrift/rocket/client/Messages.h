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

#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

#include <folly/ExceptionWrapper.h>

#include <chrono>
#include <cstdint>
#include <limits>

namespace apache::thrift::fast_thrift::rocket {

// ============================================================================
// Constants
// ============================================================================

/// Sentinel value for unassigned stream IDs.
/// StreamStateHandler assigns a valid streamId on outbound requests.
constexpr uint32_t kInvalidStreamId = std::numeric_limits<uint32_t>::max();

// ============================================================================
// Rocket Message Types
// ============================================================================

/**
 * RocketRequestMessage - Outbound request message.
 *
 * The `frame` field is a flat `ComposedFrame` that spans every RSocket
 * frame type via its `frameType` discriminator. Handlers carry it all
 * the way down to the codec; the codec is the single point that
 * serializes it into wire bytes via `frame.serialize()`.
 *
 * `streamType` is the originating REQUEST_* frame type that establishes
 * the stream (REQUEST_RESPONSE, REQUEST_STREAM, REQUEST_CHANNEL,
 * REQUEST_FNF). The application sets it; StreamStateHandler stores it
 * keyed by streamId so it can be stamped on inbound responses.
 */
struct RocketRequestMessage {
  apache::thrift::fast_thrift::frame::ComposedFrame frame;

  TypeErasedPtr requestContext;

  /// The originating REQUEST_* frame type that establishes the stream.
  /// Distinct from `frame.frameType` — the latter is the current frame's
  /// type, which differs from streamType for SETUP and for follow-up frames
  /// like REQUEST_N / CANCEL on a stream.
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

/**
 * RocketResponseError - In-process per-request failure carried inbound.
 *
 * Used as the error alternative on `RocketResponseMessage::payload` to
 * resolve a single request's pending callback with `ew` without
 * fabricating a wire-format ERROR frame. Two producers today:
 *   - downstream handlers, on conditions that fail just this request
 *     (e.g. outbound serialize threw) while the connection is healthy
 *   - StreamStateHandler itself, when the connection is going away
 *     (onException / onPipelineInactive), so each in-flight request's
 *     heap context is recovered and its callback resolves
 *
 * The variant is agnostic to whether the connection survives — it just
 * carries (streamId, ew) so StreamStateHandler can route + clean up the
 * same way it does for terminal wire frames.
 */
struct RocketResponseError {
  folly::exception_wrapper ew;
  uint32_t streamId{kInvalidStreamId};
};

/**
 * RocketStats - Transport-level per-request stats observable at the rocket
 * layer. Sizes are wire bytes (post-compression for data); latencies are
 * measured at the transport boundary. The thrift layer translates these into
 * an `apache::thrift::RpcTransportStats` at the pipeline bridge — rocket code
 * carries no thrift types.
 *
 * Zero on any field means "not measured" for that field on this response.
 */
struct RocketStats {
  uint32_t requestWireSizeBytes{0};
  uint32_t requestMetadataAndPayloadSizeBytes{0};
  uint32_t responseWireSizeBytes{0};
  uint32_t responseMetadataAndPayloadSizeBytes{0};

  std::chrono::nanoseconds requestWriteLatency{0};
  std::chrono::nanoseconds responseRoundTripLatency{0};
};

/**
 * RocketResponseMessage - Inbound response message.
 *
 * `payload` is a CompactVariant of the parsed wire frame and an in-process
 * `RocketResponseError`. The error alternative carries per-request
 * failures inbound through the same path as a normal response, so the
 * App's pending callback resolves; the connection is unaffected.
 *
 * `streamType` is the originating REQUEST_* frame type for the stream
 * this response belongs to. Stamped by StreamStateHandler from its per-
 * stream map; downstream per-pattern handlers (RequestResponse, Stream,
 * ...) use it as a stateless dispatch key.
 *
 * `stats` rides alongside the response, populated by RocketClientStatsHandler
 * as the response travels inbound. The bridge reads it when converting to the
 * thrift response message.
 */
struct RocketResponseMessage {
  apache::thrift::fast_thrift::CompactVariant<
      apache::thrift::fast_thrift::frame::read::ParsedFrame,
      RocketResponseError>
      payload;

  TypeErasedPtr requestContext;

  /// The originating REQUEST_* frame type for this response's stream.
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};

  RocketStats stats{};
};

} // namespace apache::thrift::fast_thrift::rocket
