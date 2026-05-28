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

#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Control payloads — flow-control and connection-level signals that don't
 * carry application data. Cancel and RequestN are per-stream control;
 * MetadataPush is connection-level (streamId=0). All `toRocketFrame()`
 * implementations are `noexcept` — no serialization, just header
 * construction.
 *
 * Each struct provides `toRocketFrame() &&` returning a flat
 * `frame::ComposedFrame`. The `RocketFrame` typedef on each struct names
 * the return type for variant-level dispatch (see ThriftPayloadVariant).
 */

/**
 * ThriftCancelPayload — App's "cancel this stream" operation.
 *
 * Header-only on the wire — no data, no metadata, no fields. The streamId
 * comes from the enclosing message wrapper.
 */
struct ThriftCancelPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {
        .frameType = apache::thrift::fast_thrift::frame::FrameType::CANCEL,
        .streamId = streamId,
        .metadata = nullptr,
        .data = nullptr,
    };
  }
};

/**
 * ThriftRequestNPayload — App's "request more" operation (flow control).
 *
 * Tells the peer to send N more payloads on this stream.
 */
struct ThriftRequestNPayload {
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  uint32_t streamId{apache::thrift::fast_thrift::rocket::kInvalidStreamId};
  uint32_t requestN{0};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {
        .frameType = apache::thrift::fast_thrift::frame::FrameType::REQUEST_N,
        .streamId = streamId,
        .metadata = nullptr,
        .data = nullptr,
        .requestN = requestN,
    };
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
  using RocketFrame = apache::thrift::fast_thrift::frame::ComposedFrame;

  std::unique_ptr<folly::IOBuf> metadata{nullptr};

  RocketFrame toRocketFrame(
      rocket::server::MetadataProtocol /*metadataProtocol*/) && noexcept {
    return {
        .frameType =
            apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH,
        .streamId = 0,
        .metadata = std::move(metadata),
        .data = nullptr,
    };
  }
};

} // namespace apache::thrift::fast_thrift::thrift
