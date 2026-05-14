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
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/StreamPayloadMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadConcept.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Response-side payloads emitted on an established exchange. Three of the
 * four (InitialResponse, StreamInitialResponse, Error) are unambiguously
 * server→client. `ThriftStreamPayload` lives here because its wire shape
 * matches the response side, but it is bidirectional — see its docblock.
 *
 * Each struct provides `toRocketFrame() &&` returning the matching
 * `frame::Composed*Frame`. The `RocketFrame` typedef on each struct names
 * the return type for variant-level dispatch (see ThriftPayloadVariant).
 */

/**
 * ThriftInitialResponsePayload — terminal initial response for RR / Sink.
 * The whole reply is one PAYLOAD frame: typed `ResponseRpcMetadata` plus
 * `data`, with both `complete` and `next` always true (terminal + carries
 * data). The metadata is serialized inline by `toRocketFrame()` so the
 * channel never has to pre-serialize.
 *
 * For Stream / Bidi initial chunks (which may not be terminal), use
 * `ThriftStreamInitialResponsePayload` instead.
 *
 * `toRocketFrame()` may throw on serializer/allocator failure; the transport
 * adapter catches and surfaces such failures.
 */
struct ThriftInitialResponsePayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedPayloadFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata{nullptr};
  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol metadataProtocol) && {
    DCHECK(metadata != nullptr) << "metadata must be set before serializing";
    return {
        .data = std::move(data),
        .metadata = serializeResponseMetadata(*metadata, metadataProtocol),
        // RR / Sink invariants: terminal + carries data.
        .header = {.streamId = streamId, .complete = true, .next = true},
    };
  }

  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadata()
      const noexcept {
    return metadata.get();
  }
};

/**
 * ThriftStreamInitialResponsePayload — first response chunk for Stream /
 * Bidi. Carries the typed `ResponseRpcMetadata` (so the receiver can
 * classify success vs error before the first data chunk arrives) plus
 * caller-controlled `complete` / `next` flags — the chunk may or may not
 * be terminal, may or may not carry data.
 *
 * Subsequent chunks (no metadata) use `ThriftStreamPayload`.
 *
 * `toRocketFrame()` may throw on serializer/allocator failure; the transport
 * adapter catches and surfaces such failures.
 */
struct ThriftStreamInitialResponsePayload {
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

  const apache::thrift::ResponseRpcMetadata* getResponseRpcMetadata()
      const noexcept {
    return metadata.get();
  }
};

/**
 * ThriftStreamPayload — continuing chunk on an established stream. Used
 * for Stream / Bidi response chunks (server→client) AND Sink / Bidi
 * request chunks (client→server) — same wire shape, direction is
 * contextual via the streamId's existing routing state.
 *
 * `metadata` is optional and uses `StreamPayloadMetadata` (distinct from
 * `RequestRpcMetadata` / `ResponseRpcMetadata` — carries per-chunk
 * concerns: compression algorithm, payload-kind variant for mid-stream
 * declared exceptions, FDs, checksum, app-controlled key/value). Pure
 * data passthroughs leave it null. `complete` / `next` flags match
 * RSocket PAYLOAD semantics.
 *
 * `toRocketFrame()` may throw on serializer/allocator failure when
 * `metadata` is set; the transport adapter catches and surfaces such
 * failures.
 */
struct ThriftStreamPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedPayloadFrame;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<apache::thrift::StreamPayloadMetadata> metadata{nullptr};
  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};
  bool complete{false};
  bool next{true};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol metadataProtocol) && {
    return {
        .data = std::move(data),
        .metadata = metadata
            ? serializeStreamPayloadMetadata(*metadata, metadataProtocol)
            : nullptr,
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

} // namespace apache::thrift::fast_thrift::thrift
