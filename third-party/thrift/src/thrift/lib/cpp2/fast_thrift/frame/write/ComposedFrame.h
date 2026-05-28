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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <folly/io/IOBuf.h>

#include <glog/logging.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::frame {

/**
 * `ComposedFrame` is the outbound counterpart to `read::ParsedFrame`: one
 * flat struct that spans every RSocket frame type, discriminated by
 * `frameType`. Fields beyond `{frameType, streamId, metadata, data}` are
 * sparse — only specific frame types read them (`errorCode` for ERROR,
 * `requestN` for REQUEST_N, `setupKeepaliveTime` for SETUP, etc.).
 *
 * Why flat over the previous typed `Composed*Frame` + `ComposedFrameVariant`:
 *   - Direct field access on accessors (`streamId`, `frameType`, flag
 *     bits) instead of tag-dispatch fold expressions — matters on the
 *     fragmentation / serializer hot path.
 *   - No template instantiation per pipeline (frag/serializer take
 *     `ComposedFrame`, not `ComposedFrameVariant<...>`).
 *   - Mirrors `ParsedFrame`'s design on the inbound side.
 *   - Continuation fragments (PAYLOAD with new chunk) are minted by
 *     field assignment, not typed-struct placement-new.
 *
 * `serialize()` reconstructs the per-type `write::*Header` struct on the
 * stack and forwards to the matching `write::serialize` overload — the
 * temporary header gets elided by the inliner.
 *
 * Shared across client and server: the wire shape is identical regardless
 * of which side serializes.
 */
struct ComposedFrame {
  FrameType frameType{FrameType::RESERVED};
  uint32_t streamId{0};

  // Payload buffers — present on PAYLOAD, REQUEST_*, ERROR, SETUP,
  // METADATA_PUSH (metadata only), KEEPALIVE (data only), EXT.
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  std::unique_ptr<folly::IOBuf> data{nullptr};

  // Common flag bits. Each is meaningful only for specific frame types
  // per the RSocket spec; ignored by `serialize()` for types that don't
  // carry that flag.
  bool follows{false}; // PAYLOAD, REQUEST_*
  bool complete{false}; // PAYLOAD, REQUEST_CHANNEL
  bool next{false}; // PAYLOAD
  bool ignore{false}; // EXT
  bool lease{false}; // SETUP
  bool respond{false}; // KEEPALIVE

  // Per-type extra header fields (sparse). Naming kept aligned with the
  // `write::*Header` field names for clarity.
  uint32_t errorCode{0}; // ERROR
  uint32_t requestN{0}; // REQUEST_N
  uint32_t initialRequestN{0}; // REQUEST_STREAM, REQUEST_CHANNEL
  uint32_t extendedType{0}; // EXT
  uint64_t lastReceivedPosition{0}; // KEEPALIVE
  uint16_t majorVersion{1}; // SETUP
  uint16_t minorVersion{0}; // SETUP
  uint32_t keepaliveTime{0}; // SETUP
  uint32_t maxLifetime{0}; // SETUP

  /// True for the five fragmentable frame types (PAYLOAD + REQUEST_*).
  /// Matches legacy `serializeInFragmentsSlow` overload set.
  [[nodiscard]] bool canFragment() const noexcept {
    switch (frameType) {
      case FrameType::PAYLOAD:
      case FrameType::REQUEST_RESPONSE:
      case FrameType::REQUEST_FNF:
      case FrameType::REQUEST_STREAM:
      case FrameType::REQUEST_CHANNEL:
        return true;
      case FrameType::RESERVED:
      case FrameType::SETUP:
      case FrameType::LEASE:
      case FrameType::KEEPALIVE:
      case FrameType::REQUEST_N:
      case FrameType::CANCEL:
      case FrameType::ERROR:
      case FrameType::METADATA_PUSH:
      case FrameType::RESUME:
      case FrameType::RESUME_OK:
      case FrameType::EXT:
        return false;
      default:
        DCHECK(false) << "Unknown FrameType: "
                      << static_cast<uint8_t>(frameType);
        break;
    }
    return false;
  }

  /// True if this frame terminates the sender's half of the stream
  /// (CANCEL, ERROR, REQUEST_FNF, or PAYLOAD/REQUEST_CHANNEL with
  /// `complete = true`).
  [[nodiscard]] bool isComplete() const noexcept {
    switch (frameType) {
      case FrameType::CANCEL:
      case FrameType::ERROR:
      case FrameType::REQUEST_FNF:
        return true;
      case FrameType::PAYLOAD:
      case FrameType::REQUEST_CHANNEL:
        return complete;
      case FrameType::RESERVED:
      case FrameType::SETUP:
      case FrameType::LEASE:
      case FrameType::KEEPALIVE:
      case FrameType::REQUEST_RESPONSE:
      case FrameType::REQUEST_STREAM:
      case FrameType::REQUEST_N:
      case FrameType::METADATA_PUSH:
      case FrameType::RESUME:
      case FrameType::RESUME_OK:
      case FrameType::EXT:
        return false;
      default:
        DCHECK(false) << "Unknown FrameType: "
                      << static_cast<uint8_t>(frameType);
        break;
    }
    return false;
  }

  /// Produce wire bytes. Consumes the frame. Returns nullptr for
  /// unsupported frame types (LEASE, RESUME, RESUME_OK, RESERVED).
  std::unique_ptr<folly::IOBuf> serialize() && {
    switch (frameType) {
      case FrameType::REQUEST_RESPONSE:
        return write::serialize(
            write::RequestResponseHeader{
                .streamId = streamId, .follows = follows},
            std::move(metadata),
            std::move(data));
      case FrameType::REQUEST_FNF:
        return write::serialize(
            write::RequestFnfHeader{.streamId = streamId, .follows = follows},
            std::move(metadata),
            std::move(data));
      case FrameType::REQUEST_STREAM:
        return write::serialize(
            write::RequestStreamHeader{
                .streamId = streamId,
                .initialRequestN = initialRequestN,
                .follows = follows},
            std::move(metadata),
            std::move(data));
      case FrameType::REQUEST_CHANNEL:
        return write::serialize(
            write::RequestChannelHeader{
                .streamId = streamId,
                .initialRequestN = initialRequestN,
                .follows = follows,
                .complete = complete},
            std::move(metadata),
            std::move(data));
      case FrameType::REQUEST_N:
        return write::serialize(
            write::RequestNHeader{.streamId = streamId, .requestN = requestN});
      case FrameType::CANCEL:
        return write::serialize(write::CancelHeader{.streamId = streamId});
      case FrameType::PAYLOAD:
        return write::serialize(
            write::PayloadHeader{
                .streamId = streamId,
                .follows = follows,
                .complete = complete,
                .next = next},
            std::move(metadata),
            std::move(data));
      case FrameType::ERROR:
        return write::serialize(
            write::ErrorHeader{.streamId = streamId, .errorCode = errorCode},
            std::move(metadata),
            std::move(data));
      case FrameType::KEEPALIVE:
        return write::serialize(
            write::KeepAliveHeader{
                .lastReceivedPosition = lastReceivedPosition,
                .respond = respond},
            std::move(data));
      case FrameType::SETUP:
        return write::serialize(
            write::SetupHeader{
                .majorVersion = majorVersion,
                .minorVersion = minorVersion,
                .keepaliveTime = keepaliveTime,
                .maxLifetime = maxLifetime,
                .lease = lease},
            std::move(metadata),
            std::move(data));
      case FrameType::METADATA_PUSH:
        return write::serialize(
            write::MetadataPushHeader{}, std::move(metadata));
      case FrameType::EXT:
        return write::serialize(
            write::ExtHeader{
                .streamId = streamId,
                .extendedType = extendedType,
                .ignore = ignore},
            std::move(metadata),
            std::move(data));
      case FrameType::RESERVED:
      case FrameType::LEASE:
      case FrameType::RESUME:
      case FrameType::RESUME_OK:
        return nullptr;
      default:
        DCHECK(false) << "Unknown FrameType: "
                      << static_cast<uint8_t>(frameType);
        break;
    }
    return nullptr;
  }
};

} // namespace apache::thrift::fast_thrift::frame
