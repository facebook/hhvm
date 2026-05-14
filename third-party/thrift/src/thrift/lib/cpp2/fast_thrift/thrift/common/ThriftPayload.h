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

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ResponseMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Per-pattern Thrift payload structs. There are two organizing principles
 * in this file:
 *
 *   1. **Per-RpcKind initial-request payloads** — one per Thrift RpcKind.
 *      These model the *first frame* a client sends to open an exchange:
 *      RequestResponse, Fnf, Stream, Sink, Bidi. Each carries the typed
 *      `RequestRpcMetadata` struct (universal across all RPC kinds — the
 *      `kind` field disambiguates Sink vs Bidi, which share the wire frame)
 *      plus the request data, plus any per-pattern flow-control fields.
 *
 *   2. **Per-stream payloads** — emitted by either side on an established
 *      stream. Modeled per Thrift App-level operation (respond, error,
 *      cancel, request-more), not per RpcKind. Same shape regardless of
 *      direction — both client and server can emit any of these.
 *
 * Each struct provides `toRocketFrame() &&` returning the matching
 * `frame::Composed*Frame`. The `RocketFrame` typedef on each struct names
 * the return type for variant-level dispatch (see ThriftPayloadVariant).
 *
 * Initial-request payloads serialize their metadata struct inline in
 * `toRocketFrame()` and may throw on serializer/allocator failure; the
 * transport adapter catches and delivers the error inbound. Per-stream
 * payloads' `toRocketFrame()` is `noexcept`.
 *
 * Construction sites pick the alternative; the variant + `toRocketFrame()`
 * makes the Thrift→Rocket translation a single fold-expression dispatch
 * with no runtime switch in the transport adapter.
 */

// ============================================================================
// Per-RpcKind Initial Request Payloads
// ============================================================================

struct ThriftRequestResponsePayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata{nullptr};

  // Serializes metadata as part of frame composition. Throws on
  // serializer/allocator failure; the transport adapter catches and
  // delivers the error inbound as a per-request `ThriftClientResponseError`.
  // The metadata protocol param is unused today (request metadata is always
  // Binary on the client outbound path until SETUP-time negotiation lands
  // on the client transport adapter). Kept for variant-uniform dispatch.
  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeRequestMetadata(*metadata),
        .header =
            {.streamId = apache::thrift::fast_thrift::rocket::kInvalidStreamId},
    };
  }
};

struct ThriftRequestFnfPayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedRequestFnfFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata{nullptr};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeRequestMetadata(*metadata),
        .header =
            {.streamId = apache::thrift::fast_thrift::rocket::kInvalidStreamId},
    };
  }
};

struct ThriftRequestStreamPayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE;
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedRequestStreamFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata{nullptr};
  uint32_t initialRequestN{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeRequestMetadata(*metadata),
        .header =
            {.streamId = apache::thrift::fast_thrift::rocket::kInvalidStreamId,
             .initialRequestN = initialRequestN},
    };
  }
};

struct ThriftRequestSinkPayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::SINK;
  // SINK and BIDI both map to REQUEST_CHANNEL on the wire. The Thrift layer
  // distinguishes them via metadata (RequestRpcMetadata.kind) on the initial
  // request; the rocket layer sees them as identical channel openings.
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedRequestChannelFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata{nullptr};
  uint32_t initialRequestN{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeRequestMetadata(*metadata),
        .header =
            {.streamId = apache::thrift::fast_thrift::rocket::kInvalidStreamId,
             .initialRequestN = initialRequestN},
    };
  }
};

struct ThriftRequestBidiPayload {
  static constexpr apache::thrift::RpcKind kRpcKind =
      apache::thrift::RpcKind::BIDIRECTIONAL_STREAM;
  // See ThriftRequestSinkPayload for the SINK/BIDI → REQUEST_CHANNEL note.
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedRequestChannelFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata{nullptr};
  uint32_t initialRequestN{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeRequestMetadata(*metadata),
        .header =
            {.streamId = apache::thrift::fast_thrift::rocket::kInvalidStreamId,
             .initialRequestN = initialRequestN},
    };
  }
};

// ============================================================================
// Per-Stream Payloads (bidirectional)
// ============================================================================

/**
 * ThriftFirstResponsePayload — App's "first response" operation: the initial
 * (and, for RR / Sink, only) PAYLOAD frame on a server-initiated reply.
 * Carries the typed `ResponseRpcMetadata` struct; the metadata is serialized
 * inline by `toRocketFrame()` so the channel never has to pre-serialize.
 *
 * `complete` and `next` correspond to the RSocket PAYLOAD flags:
 *   - complete = stream is done after this frame
 *   - next     = the data field carries data (vs control-only payloads)
 *
 * Semantics by RpcKind (per-message rpcKind tag controls dispatch):
 *   - RR / Sink: rocket / Thrift adapter enforces complete=next=true.
 *     App-set values are DCHECKed; the wire frame is always terminal+data.
 *   - Stream / Bidi: caller controls per chunk for the initial frame; later
 *     stream chunks (without metadata) will use a separate payload type.
 *
 * `toRocketFrame()` may throw on serializer/allocator failure; the transport
 * adapter catches and surfaces such failures.
 */
struct ThriftFirstResponsePayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedPayloadFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata{nullptr};
  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};
  bool complete{true};
  bool next{true};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol metadataProtocol) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeResponseMetadata(*metadata, metadataProtocol),
        .header = {.streamId = streamId, .complete = complete, .next = next},
    };
  }
};

/**
 * ThriftErrorPayload — App's "error" operation. Always terminal.
 *
 * `errorCode` is rocket-level (REJECTED, INVALID, APPLICATION_ERROR, …).
 * Applies to any RpcKind.
 */
struct ThriftErrorPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedErrorFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};
  uint32_t errorCode{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {
        .data = std::move(data),
        .metadata = std::move(metadata),
        .header = {.streamId = streamId, .errorCode = errorCode},
    };
  }
};

/**
 * ThriftCancelPayload — App's "cancel this stream" operation.
 *
 * Header-only on the wire — no data, no metadata, no fields. The streamId
 * comes from the enclosing message wrapper.
 */
struct ThriftCancelPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedCancelFrame;

  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {.header = {.streamId = streamId}};
  }
};

/**
 * ThriftRequestNPayload — App's "request more" operation (flow control).
 *
 * Tells the peer to send N more payloads on this stream.
 */
struct ThriftRequestNPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedRequestNFrame;

  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};
  uint32_t requestN{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {.header = {.streamId = streamId, .requestN = requestN}};
  }
};

/**
 * ThriftMetadataPushPayload — connection-level metadata push.
 *
 * Wire frame is rocket-spec METADATA_PUSH (streamId=0, no data, metadata
 * only) but the metadata payload is thrift-specific (server version
 * negotiation, setup signaling, etc.) — so the typed payload lives in the
 * thrift layer and is interpreted by `ThriftClientMetadataPushHandler`.
 *
 * Bidirectional: server sends after SETUP to advertise capabilities;
 * client may send to push runtime metadata.
 */
struct ThriftMetadataPushPayload {
  using RocketFrame =
      apache::thrift::fast_thrift::frame::ComposedMetadataPushFrame;

  std::unique_ptr<folly::IOBuf> metadata{nullptr};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {.metadata = std::move(metadata), .header = {}};
  }
};

} // namespace apache::thrift::fast_thrift::thrift
