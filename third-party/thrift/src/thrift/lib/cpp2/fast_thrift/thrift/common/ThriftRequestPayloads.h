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
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadConcept.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Per-RpcKind initial-request payloads — one per Thrift RpcKind. These
 * model the *first frame* a client sends to open an exchange:
 * RequestResponse, Fnf, Stream, Sink, Bidi. Each carries the typed
 * `RequestRpcMetadata` struct (universal across all RPC kinds — the
 * `kind` field disambiguates Sink vs Bidi, which share the wire frame)
 * plus the request data, plus any per-pattern flow-control fields.
 *
 * Each struct provides `toRocketFrame() &&` returning the matching
 * `frame::ComposedRequest*Frame`. The `RocketFrame` typedef on each
 * struct names the return type for variant-level dispatch (see
 * ThriftPayloadVariant).
 *
 * `toRocketFrame()` serializes the metadata struct inline and may throw
 * on serializer/allocator failure; the transport adapter catches and
 * delivers the error inbound.
 */

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

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return metadata.get();
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

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return metadata.get();
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

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return metadata.get();
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

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return metadata.get();
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

  const apache::thrift::RequestRpcMetadata* getRequestRpcMetadata()
      const noexcept {
    return metadata.get();
  }
};

} // namespace apache::thrift::fast_thrift::thrift
