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

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <cstdint>
#include <limits>
#include <memory>

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

// Payload
/**
 * RocketFramePayload - Unserialized frame payload.
 */
#pragma pack(push, 1)
struct RocketFramePayload {
  std::unique_ptr<folly::IOBuf> metadata;
  std::unique_ptr<folly::IOBuf> data;

  uint32_t streamId{kInvalidStreamId};
  uint32_t initialRequestN{0};

  bool follows{false};
  bool complete{false};
  bool next{false};
};
#pragma pack(pop)

// ============================================================================
// Rocket Message Types
// ============================================================================

/**
 * RocketRequestMessage - Outbound request message.
 *
 * The frame field is a variant that holds either:
 * - RocketFramePayload: unserialized frame data (metadata, data, flags)
 * - std::unique_ptr<folly::IOBuf>: serialized frame buffer
 *
 * Handlers work with the payload variant, then serialization converts it
 * to the IOBuf variant before transmission.
 */
#pragma pack(push, 1)
struct RocketRequestMessage {
  /// Frame data - either unserialized payload or serialized buffer
  apache::thrift::fast_thrift::
      CompactVariant<RocketFramePayload, std::unique_ptr<folly::IOBuf>>
          frame;

  /// Opaque handle for correlating responses with requests.
  uint32_t requestHandle{kNoRequestHandle};

  /// Frame type (REQUEST_RESPONSE, REQUEST_STREAM, etc.)
  apache::thrift::fast_thrift::frame::FrameType frameType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};
#pragma pack(pop)

/**
 * RocketResponseMessage - Inbound response message.
 *
 * Contains the parsed response frame along with the original request's
 * frame type and handle for proper dispatching.
 */
#pragma pack(push, 1)
struct RocketResponseMessage {
  /// The parsed response frame
  apache::thrift::fast_thrift::frame::read::ParsedFrame frame;

  /// Opaque handle for correlating responses with requests.
  uint32_t requestHandle{kNoRequestHandle};

  /// The original request's frame type (REQUEST_RESPONSE, REQUEST_STREAM, etc.)
  apache::thrift::fast_thrift::frame::FrameType requestFrameType;
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::rocket
