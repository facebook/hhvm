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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <array>
#include <memory>

namespace apache::thrift::fast_thrift::frame::read {

namespace detail {

inline size_t readFrameOrMetadataSize(folly::io::Cursor& cursor) {
  std::array<uint8_t, frame::kMetadataLengthSize> bytes;
  cursor.pull(bytes.data(), bytes.size());
  return (static_cast<size_t>(bytes[0]) << 16) |
      (static_cast<size_t>(bytes[1]) << 8) | static_cast<size_t>(bytes[2]);
}

/**
 * Read the frame type and flags from a cursor.
 *
 * Reads 2 bytes (big-endian uint16_t) and extracts:
 * - Upper 6 bits: frame type
 * - Lower 10 bits: flags
 *
 * @param cursor Cursor positioned at the typeAndFlags field
 * @return Pair of (frameType raw value, flags as uint16_t)
 */
inline std::pair<uint8_t, uint16_t> readFrameTypeAndFlags(
    folly::io::Cursor& cursor) {
  uint16_t typeAndFlags = cursor.readBE<uint16_t>();
  uint8_t frameTypeRaw = static_cast<uint8_t>(
      typeAndFlags >> ::apache::thrift::fast_thrift::frame::detail::kFlagsBits);
  uint16_t flags =
      typeAndFlags & ::apache::thrift::fast_thrift::frame::detail::kFlagsMask;
  return {frameTypeRaw, flags};
}

} // namespace detail

/**
 * Parse a complete frame into a ParsedFrame.
 *
 * This is the primary entry point for frame parsing. It reads the frame header
 * once and caches all commonly-accessed fields in FrameMetadata. The buffer
 * is moved into the ParsedFrame for subsequent payload access via cursors.
 *
 * The parsing strategy is "parse once, use everywhere":
 * - Common header fields (streamId, type, flags) are cached in FrameMetadata
 * - Frame-specific fields (initialRequestN, errorCode, etc.) are read on-demand
 *   via typed views (RequestStreamView, ErrorView, etc.)
 * - Payload data is accessed via cursors, handling IOBuf chains transparently
 *
 * Frame layout (RSocket protocol):
 *   [streamId: 4 bytes][typeAndFlags: 2 bytes][frame-specific...][payload...]
 *
 * @param buffer Unique pointer to the frame buffer (ownership transferred)
 * @return ParsedFrame with cached metadata and buffer ownership
 *
 * @pre buffer != nullptr
 * @pre buffer contains a complete, valid RSocket frame
 *
 * Example usage:
 *   auto frame = parseFrame(std::move(buffer));
 *   if (frame.type() == FrameType::REQUEST_STREAM) {
 *     RequestStreamView view(frame);
 *     auto requestN = view.initialRequestN();
 *     // ...
 *   }
 */
inline ParsedFrame parseFrame(std::unique_ptr<folly::IOBuf> buffer) {
  folly::io::Cursor cursor(buffer.get());

  // Read common header fields (6 bytes minimum)
  // Layout: [streamId: 4][typeAndFlags: 2]
  uint32_t streamId = cursor.readBE<uint32_t>();
  auto [frameTypeRaw, flags] = detail::readFrameTypeAndFlags(cursor);
  FrameType frameType = static_cast<FrameType>(frameTypeRaw);

  // Look up the frame descriptor (O(1) array lookup)
  const FrameDescriptor& desc = getDescriptor(frameType);

  // Skip frame-specific header fields
  // (we've already read streamId + typeAndFlags = kBaseHeaderSize bytes)
  size_t extraHeaderBytes = desc.headerSize - kBaseHeaderSize;
  cursor.skip(extraHeaderBytes);

  // Parse metadata size if present (3-byte big-endian)
  uint16_t metadataSize = 0;
  if (flags & ::apache::thrift::fast_thrift::frame::detail::kMetadataBit) {
    metadataSize =
        static_cast<uint16_t>(detail::readFrameOrMetadataSize(cursor));
  }

  // Calculate payload offset and size
  // Note: getCurrentPosition() returns the position in the current IOBuf,
  // which for typical single-buffer frames equals the total bytes read.
  // For chained IOBufs with header in first buffer, this still works
  // correctly.
  auto payloadOffset = static_cast<uint32_t>(cursor.getCurrentPosition());
  auto totalSize = static_cast<uint32_t>(buffer->computeChainDataLength());
  uint32_t payloadSize = totalSize - payloadOffset;

  return ParsedFrame{
      .metadata =
          FrameMetadata{
              .descriptor = &desc,
              .streamId = streamId,
              .flags_ = flags,
              .metadataSize = metadataSize,
              .payloadOffset = payloadOffset,
              .payloadSize = payloadSize,
              .reserved_ = 0,
          },
      .buffer = std::move(buffer),
  };
}

/**
 * Try to parse a frame, returning an empty ParsedFrame on failure.
 *
 * This is a defensive variant that checks for null buffers and insufficient
 * data before parsing. Use this when the buffer validity is uncertain.
 *
 * @param buffer Unique pointer to the frame buffer (may be null)
 * @return ParsedFrame if successful, empty ParsedFrame if buffer is null or
 * too small
 */
inline ParsedFrame tryParseFrame(std::unique_ptr<folly::IOBuf> buffer) {
  if (!buffer || buffer->computeChainDataLength() < 6) {
    return ParsedFrame{};
  }
  return parseFrame(std::move(buffer));
}

} // namespace apache::thrift::fast_thrift::frame::read
