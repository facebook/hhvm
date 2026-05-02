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
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>

#include <folly/ExceptionWrapper.h>

#include <cstdint>
#include <limits>

namespace apache::thrift::fast_thrift::rocket {

// ============================================================================
// Constants
// ============================================================================

/// Sentinel value for requests that don't need response correlation
/// (e.g., setup frames, fire-and-forget requests).
constexpr uint32_t kNoRequestHandle = 0;

/// Sentinel value for unassigned stream IDs.
/// StreamStateHandler assigns a valid streamId on outbound requests.
constexpr uint32_t kInvalidStreamId = std::numeric_limits<uint32_t>::max();

// ============================================================================
// Rocket Message Types
// ============================================================================

/**
 * RocketRequestMessage - Outbound request message.
 *
 * The `frame` field is a typed variant of per-frame payload structs from
 * `frame::ComposedFrame.h`. Handlers carry the typed payload all the way
 * down to the codec; the codec is the single point that serializes the
 * held payload into wire bytes via `frame.serialize()`. The 3-API surface
 * (streamId / frameType / serialize) dispatches inline with no
 * `std::visit` at any call site.
 *
 * `streamType` is the originating REQUEST_* frame type that establishes
 * the stream (REQUEST_RESPONSE, REQUEST_STREAM, REQUEST_CHANNEL,
 * REQUEST_FNF). The application sets it; StreamStateHandler stores it
 * keyed by streamId so it can be stamped on inbound responses.
 *
 * As STREAM/CHANNEL/FNF patterns get wired up, their corresponding
 * `Composed*Frame` types will join the variant.
 */
struct RocketRequestMessage {
  /// Per-frame payload (header + buffers).
  apache::thrift::fast_thrift::frame::ComposedFrameVariant<
      apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame,
      apache::thrift::fast_thrift::frame::ComposedSetupFrame>
      frame;

  /// Opaque handle for correlating responses with requests.
  uint32_t requestHandle{kNoRequestHandle};

  /// The originating REQUEST_* frame type that establishes the stream.
  /// Distinct from `frame.frameType()` — the latter is the current frame's
  /// type, which differs from streamType for SETUP and for follow-up frames
  /// like REQUEST_N / CANCEL on a stream.
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

/**
 * RocketResponseError - In-process per-request failure carried inbound.
 *
 * Used as the error alternative on `RocketResponseMessage::payload` so a
 * single request can be failed (callback resolved with `ew`) without
 * fabricating a wire-format ERROR frame and without escalating to a
 * connection-fatal `fireException`. Produced by handlers when an in-
 * process condition (e.g. outbound serialize threw) breaks one request;
 * the connection itself remains healthy.
 *
 * `streamId` carries the affected stream so StreamStateHandler can
 * complete its per-stream cleanup the same way it does for terminal
 * wire frames.
 */
struct RocketResponseError {
  folly::exception_wrapper ew;
  uint32_t streamId{kInvalidStreamId};
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
 */
struct RocketResponseMessage {
  apache::thrift::fast_thrift::CompactVariant<
      apache::thrift::fast_thrift::frame::read::ParsedFrame,
      RocketResponseError>
      payload;

  /// Opaque handle for correlating responses with requests.
  uint32_t requestHandle{kNoRequestHandle};

  /// The originating REQUEST_* frame type for this response's stream.
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

} // namespace apache::thrift::fast_thrift::rocket
