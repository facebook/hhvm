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

#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <folly/io/Cursor.h>

namespace apache::thrift::fast_thrift::frame::read {

/**
 * Frame header field offsets (in bytes from frame start).
 *
 * RSocket frame layout:
 *   [0-3]   Stream ID (4 bytes, big-endian)
 *   [4-5]   Type (6 bits) + Flags (10 bits)
 *   [6+]    Frame-specific fields (varies by type)
 */
namespace detail {
constexpr size_t kStreamIdOffset = 0;
constexpr size_t kTypeAndFlagsOffset = 4;
constexpr size_t kFrameSpecificOffset = 6; // Start of frame-specific fields
} // namespace detail

/**
 * FrameView - Base view providing common accessors.
 *
 * This is a lightweight, non-owning view over a ParsedFrame.
 * It provides convenient access to the frame's metadata and payload.
 *
 * Design rationale:
 * - No storage: Just holds a reference to ParsedFrame
 * - No virtual functions: Zero overhead abstraction
 * - Cursor-based: Handles IOBuf chains transparently
 */
class FrameView {
 public:
  explicit FrameView(const ParsedFrame& frame) noexcept : frame_(frame) {}

  // Cached metadata accessors (no parsing needed)
  uint32_t streamId() const noexcept { return frame_.streamId(); }
  FrameType type() const noexcept { return frame_.type(); }
  const char* typeName() const noexcept { return frame_.typeName(); }

  // Flag accessors
  bool hasMetadata() const noexcept { return frame_.hasMetadata(); }
  bool hasFollows() const noexcept { return frame_.hasFollows(); }
  bool isComplete() const noexcept { return frame_.isComplete(); }
  bool hasNext() const noexcept { return frame_.hasNext(); }

  // Size accessors
  uint32_t payloadSize() const noexcept { return frame_.payloadSize(); }
  uint16_t metadataSize() const noexcept { return frame_.metadataSize(); }
  uint32_t dataSize() const noexcept { return frame_.dataSize(); }

  // Cursor accessors (creates cursor on demand)
  folly::io::Cursor payloadCursor() const { return frame_.payloadCursor(); }
  folly::io::Cursor metadataCursor() const { return frame_.metadataCursor(); }
  folly::io::Cursor dataCursor() const { return frame_.dataCursor(); }
  folly::io::Cursor frameCursor() const { return frame_.frameCursor(); }

  // Access underlying frame
  const ParsedFrame& frame() const noexcept { return frame_; }

 protected:
  const ParsedFrame& frame_;
};

/**
 * RequestStreamView - View for REQUEST_STREAM frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID
 *   [4-5]   Type + Flags (M=metadata, F=follows)
 *   [6-9]   Initial Request N (uint32_t, big-endian)
 *   [10+]   Payload (metadata size if M flag, then metadata, then data)
 */
class RequestStreamView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Read the initial request N value.
   *
   * This creates a cursor and reads from the frame buffer on each call.
   * For hot paths, consider caching the result.
   */
  uint32_t initialRequestN() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint32_t>();
  }
};

/**
 * RequestChannelView - View for REQUEST_CHANNEL frames.
 *
 * Same layout as REQUEST_STREAM.
 */
using RequestChannelView = RequestStreamView;

/**
 * RequestNView - View for REQUEST_N frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID
 *   [4-5]   Type + Flags
 *   [6-9]   Request N (uint32_t, big-endian)
 */
class RequestNView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Read the request N value.
   */
  uint32_t requestN() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint32_t>();
  }
};

/**
 * ErrorView - View for ERROR frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID
 *   [4-5]   Type + Flags
 *   [6-9]   Error Code (uint32_t, big-endian)
 *   [10+]   Payload (error data)
 */
class ErrorView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Read the error code.
   */
  uint32_t errorCode() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint32_t>();
  }
};

/**
 * KeepAliveView - View for KEEPALIVE frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID (always 0)
 *   [4-5]   Type + Flags (R=respond)
 *   [6-13]  Last Received Position (uint64_t, big-endian)
 *   [14+]   Data (optional)
 */
class KeepAliveView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Check if the respond flag is set.
   *
   * When true, the receiver should respond with a KEEPALIVE frame.
   */
  bool shouldRespond() const noexcept {
    return frame_.metadata.shouldRespond();
  }

  /**
   * Read the last received position.
   *
   * This is used for resumption support.
   */
  uint64_t lastReceivedPosition() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint64_t>();
  }
};

/**
 * SetupView - View for SETUP frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID (always 0)
 *   [4-5]   Type + Flags (M=metadata, R=resume, L=lease)
 *   [6-7]   Major Version (uint16_t, big-endian)
 *   [8-9]   Minor Version (uint16_t, big-endian)
 *   [10-13] Time Between KEEPALIVE Frames (uint32_t, big-endian)
 *   [14-17] Max Lifetime (uint32_t, big-endian)
 *   [18-19] Token Length (if R flag) + Resume Identification Token (variable)
 *   [+1]    MIME Length + Metadata Encoding MIME Type (variable)
 *   [+1]    MIME Length + Data Encoding MIME Type (variable)
 *   [+N]    Setup Payload (metadata + data)
 *
 * Note: The base header size in FrameDescriptor is 20 bytes, which accounts
 * for the fixed portion up to and including Max Lifetime.
 */
class SetupView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Check if the lease flag is set.
   *
   * When true, the client requests lease semantics from the server.
   */
  bool hasLease() const noexcept { return frame_.metadata.hasLease(); }

  /**
   * Check if the resume token flag is set.
   *
   * When true, a resume identification token is present in the frame.
   */
  bool hasResumeToken() const noexcept {
    return frame_.metadata.hasResumeToken();
  }

  /**
   * Read the major version.
   *
   * RSocket protocol major version (typically 1).
   */
  uint16_t majorVersion() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint16_t>();
  }

  /**
   * Read the minor version.
   *
   * RSocket protocol minor version (typically 0).
   */
  uint16_t minorVersion() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset + 2);
    return cursor.readBE<uint16_t>();
  }

  /**
   * Read the keepalive time in milliseconds.
   *
   * Time between KEEPALIVE frames that the client will send.
   * A value of 0 means keepalives are disabled.
   */
  uint32_t keepaliveTime() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset + 4);
    return cursor.readBE<uint32_t>();
  }

  /**
   * Read the max lifetime in milliseconds.
   *
   * Maximum time a client will allow a connection to be idle.
   */
  uint32_t maxLifetime() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset + 8);
    return cursor.readBE<uint32_t>();
  }
};

/**
 * ExtView - View for EXT (extension) frames.
 *
 * Frame layout:
 *   [0-3]   Stream ID
 *   [4-5]   Type + Flags (I=ignore)
 *   [6-9]   Extended Type (uint32_t, big-endian)
 *   [10+]   Payload
 */
class ExtView : public FrameView {
 public:
  using FrameView::FrameView;

  /**
   * Read the extended frame type.
   */
  uint32_t extendedType() const {
    auto cursor = frameCursor();
    cursor.skip(detail::kFrameSpecificOffset);
    return cursor.readBE<uint32_t>();
  }

  /**
   * Check if the ignore flag is set.
   *
   * When true, unknown extension types should be ignored rather than
   * causing an error.
   */
  bool shouldIgnore() const noexcept { return frame_.metadata.shouldIgnore(); }
};

// Aliases for simple frames that don't have extra fields beyond the base
// These frames only have streamId, typeAndFlags, and payload

/**
 * RequestResponseView - View for REQUEST_RESPONSE frames.
 *
 * No extra header fields beyond base (6 bytes header).
 */
using RequestResponseView = FrameView;

/**
 * RequestFnfView - View for REQUEST_FNF (fire-and-forget) frames.
 *
 * No extra header fields beyond base (6 bytes header).
 */
using RequestFnfView = FrameView;

/**
 * PayloadView - View for PAYLOAD frames.
 *
 * No extra header fields beyond base (6 bytes header).
 * Uses flags: M=metadata, F=follows, C=complete, N=next
 */
using PayloadView = FrameView;

/**
 * CancelView - View for CANCEL frames.
 *
 * No extra header fields beyond base (6 bytes header).
 */
using CancelView = FrameView;

/**
 * MetadataPushView - View for METADATA_PUSH frames.
 *
 * No extra header fields beyond base (6 bytes header).
 * Stream ID is always 0.
 */
using MetadataPushView = FrameView;

/**
 * Helper function to create a typed view from a ParsedFrame.
 *
 * Usage:
 *   auto frame = parseFrame(std::move(buffer));
 *   if (frame.type() == FrameType::REQUEST_STREAM) {
 *     auto view = asView<RequestStreamView>(frame);
 *     auto n = view.initialRequestN();
 *   }
 */
template <typename ViewT>
ViewT asView(const ParsedFrame& frame) noexcept {
  return ViewT{frame};
}

} // namespace apache::thrift::fast_thrift::frame::read
