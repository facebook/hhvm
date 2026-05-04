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
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::frame {

/**
 * Per-frame "composed frame" structs — one per RSocket frame type. Each
 * composes the matching `write::*Header` (which owns all wire fields,
 * including streamId) with the optional metadata/data buffers, plus:
 *
 *   - `kFrameType` — compile-time `FrameType` tag identifying this frame.
 *   - `streamId()` — the stream this frame belongs to (0 for
 *     connection-level frames per RSocket spec).
 *   - `complete()` — does this frame terminate the sender's half of the
 *     stream? `true` for ERROR / CANCEL / REQUEST_FNF (terminal by frame
 *     type), forwards `header.complete` for PAYLOAD / REQUEST_CHANNEL,
 *     `false` for everything else.
 *   - `serialize() &&` — produce wire bytes by consuming the frame.
 *     Forwards directly to the matching `write::serialize(header, ...)`
 *     worker; with inlining + LTO this collapses to a single call.
 *
 * Together these satisfy `ComposedFrameConcept` (see
 * ComposedFrameVariant.h), which lets composed frames be held in a typed
 * `ComposedFrameVariant<...>` whose 4-API surface (streamId / complete /
 * frameType / serialize) is dispatched inline with no `std::visit` at the
 * call site.
 *
 * Shared across client and server because the wire shape is identical
 * regardless of which side serializes it.
 */

// ============================================================================
// Request Frames
// ============================================================================

struct ComposedRequestResponseFrame {
  static constexpr FrameType kFrameType = FrameType::REQUEST_RESPONSE;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::RequestResponseHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

struct ComposedRequestFnfFrame {
  static constexpr FrameType kFrameType = FrameType::REQUEST_FNF;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::RequestFnfHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return true; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

struct ComposedRequestStreamFrame {
  static constexpr FrameType kFrameType = FrameType::REQUEST_STREAM;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::RequestStreamHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

struct ComposedRequestChannelFrame {
  static constexpr FrameType kFrameType = FrameType::REQUEST_CHANNEL;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::RequestChannelHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return header.complete; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

// ============================================================================
// Flow Control Frames (header only — no metadata / data)
// ============================================================================

struct ComposedRequestNFrame {
  static constexpr FrameType kFrameType = FrameType::REQUEST_N;

  write::RequestNHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header);
  }
};

struct ComposedCancelFrame {
  static constexpr FrameType kFrameType = FrameType::CANCEL;

  write::CancelHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return true; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header);
  }
};

// ============================================================================
// Payload & Error Frames
// ============================================================================

struct ComposedPayloadFrame {
  static constexpr FrameType kFrameType = FrameType::PAYLOAD;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::PayloadHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return header.complete; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

struct ComposedErrorFrame {
  static constexpr FrameType kFrameType = FrameType::ERROR;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::ErrorHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return true; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

// ============================================================================
// Connection-Level Frames (streamId is always 0 per RSocket spec)
// ============================================================================

struct ComposedKeepAliveFrame {
  static constexpr FrameType kFrameType = FrameType::KEEPALIVE;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  write::KeepAliveHeader header{};

  uint32_t streamId() const noexcept { return 0; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(data));
  }
};

struct ComposedSetupFrame {
  static constexpr FrameType kFrameType = FrameType::SETUP;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::SetupHeader header{};

  uint32_t streamId() const noexcept { return 0; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

struct ComposedMetadataPushFrame {
  static constexpr FrameType kFrameType = FrameType::METADATA_PUSH;

  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::MetadataPushHeader header{};

  uint32_t streamId() const noexcept { return 0; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata));
  }
};

// ============================================================================
// Extension Frame
// ============================================================================

struct ComposedExtFrame {
  static constexpr FrameType kFrameType = FrameType::EXT;

  std::unique_ptr<folly::IOBuf> data{nullptr};
  std::unique_ptr<folly::IOBuf> metadata{nullptr};
  write::ExtHeader header{};

  uint32_t streamId() const noexcept { return header.streamId; }
  bool complete() const noexcept { return false; }
  std::unique_ptr<folly::IOBuf> serialize() && {
    return write::serialize(header, std::move(metadata), std::move(data));
  }
};

} // namespace apache::thrift::fast_thrift::frame
