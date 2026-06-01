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

#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameMetadata.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <memory>

namespace apache::thrift::fast_thrift::frame::read {

/**
 * ParsedFrame - Complete parsed frame with buffer ownership.
 *
 * This struct combines the parsed frame metadata (FrameMetadata) with ownership
 * of the underlying buffer. It provides convenient accessors for reading
 * frame payload data via cursors.
 *
 * Design rationale:
 * - FrameMetadata holds cached header fields (parsed once, used many times)
 * - Buffer ownership travels with the frame through the pipeline
 * - Cursor-based access handles IOBuf chains transparently
 * - Separate cursors for metadata vs data portions
 *
 * Usage:
 *   auto frame = parseFrame(std::move(buffer));
 *
 *   // Access cached metadata (no parsing, just field reads)
 *   auto streamId = frame.metadata.streamId;
 *   auto type = frame.type();
 *
 *   // Read payload via cursor (handles IOBuf chains)
 *   auto cursor = frame.dataCursor();
 *   auto value = cursor.readBE<uint32_t>();
 */
struct ParsedFrame {
  // Parsed header metadata (32 bytes)
  FrameMetadata metadata;

  // Ownership of the raw frame buffer
  // May be a chain of IOBufs for large frames
  std::unique_ptr<folly::IOBuf> buffer;

  // === Convenience accessors (delegate to metadata) ===

  FrameType type() const noexcept { return metadata.type(); }

  const char* typeName() const noexcept { return metadata.typeName(); }

  uint32_t streamId() const noexcept { return metadata.streamId; }

  /**
   * Check if this is a connection-level frame (streamId == 0).
   */
  bool isConnectionFrame() const noexcept {
    return metadata.streamId == kConnectionStreamId;
  }

  /**
   * Check if this is a terminal frame (ends the stream).
   */
  bool isTerminalFrame() const noexcept {
    return isTerminalFrameType(type()) ||
        (type() == FrameType::PAYLOAD && isComplete());
  }

  bool hasMetadata() const noexcept { return metadata.hasMetadata(); }

  bool hasFollows() const noexcept { return metadata.hasFollows(); }

  bool isComplete() const noexcept { return metadata.isComplete(); }

  bool hasNext() const noexcept { return metadata.hasNext(); }

  uint32_t payloadSize() const noexcept { return metadata.payloadSize; }

  uint16_t metadataSize() const noexcept { return metadata.metadataSize; }

  uint32_t dataSize() const noexcept { return metadata.dataSize(); }

  // === Cursor accessors ===

  /**
   * Get a cursor positioned at the start of the payload.
   *
   * The payload includes both metadata (if present) and data.
   * Use this when you need to read the raw payload bytes.
   */
  folly::io::Cursor payloadCursor() const {
    folly::io::Cursor cursor(buffer.get());
    cursor.skip(metadata.payloadOffset);
    return cursor;
  }

  /**
   * Get a cursor positioned at the start of the metadata.
   *
   * This is the same as payloadCursor() since metadata comes first.
   * Returns a cursor at payload start even if no metadata is present
   * (in which case metadataSize() == 0).
   */
  folly::io::Cursor metadataCursor() const { return payloadCursor(); }

  /**
   * Get a cursor positioned at the start of the data (after metadata).
   *
   * Skips past the metadata portion to position at the actual data.
   * If no metadata is present, this is the same as payloadCursor().
   */
  folly::io::Cursor dataCursor() const {
    auto cursor = payloadCursor();
    cursor.skip(metadata.metadataSize);
    return cursor;
  }

  /**
   * Get a cursor positioned at the start of the entire frame buffer.
   *
   * Use this when you need to read frame-specific header fields
   * that aren't cached in FrameMetadata (e.g., initialRequestN for
   * REQUEST_STREAM).
   */
  folly::io::Cursor frameCursor() const {
    return folly::io::Cursor(buffer.get());
  }

  /**
   * Move out the underlying buffer.
   *
   * This transfers ownership of the raw frame buffer out of the ParsedFrame.
   * After calling this, the ParsedFrame is left in an invalid state and should
   * not be used further.
   */
  std::unique_ptr<folly::IOBuf> getUnderlyingBuffer() && {
    return std::move(buffer);
  }

  /**
   * Extract just the data portion of the payload as an IOBuf (zero-copy).
   *
   * This moves ownership of the buffer and trims the header and metadata,
   * leaving only the data portion. Handles IOBuf chains correctly by
   * iterating through buffers when the trim spans multiple chain elements.
   *
   * After calling this, the ParsedFrame is left in an invalid state.
   */
  std::unique_ptr<folly::IOBuf> extractData() && {
    auto buf = std::move(buffer);

    // Total bytes to trim: header + metadata
    auto toTrim = metadata.payloadOffset + metadata.metadataSize;

    while (toTrim > 0 && buf) {
      if (buf->length() > toTrim) {
        // This buffer contains more data than we need to trim
        buf->trimStart(toTrim);
        toTrim = 0;
      } else {
        // This entire buffer should be removed, continue to next
        toTrim -= buf->length();
        buf = buf->pop();
      }
    }
    return buf;
  }

  // === Validation ===

  /**
   * Check if the frame is valid (has buffer and valid descriptor).
   */
  bool isValid() const noexcept {
    return buffer != nullptr && metadata.descriptor != nullptr &&
        metadata.descriptor->type != FrameType::RESERVED;
  }

  /**
   * Check if the buffer is empty.
   */
  bool empty() const noexcept { return buffer == nullptr; }

  explicit operator bool() const noexcept { return isValid(); }
};

} // namespace apache::thrift::fast_thrift::frame::read
