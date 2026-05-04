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

#include <cstdint>

namespace apache::thrift::fast_thrift::frame::read {

/**
 * FrameMetadata - Parsed common frame header fields.
 *
 * This struct holds the parsed header information that is common to all frames.
 * It is designed to fit within 32 bytes to enable inline storage in
 * TypeErasedBox for zero-allocation message passing through the pipeline.
 *
 * Design rationale:
 * - 32 bytes: Fits in TypeErasedBox inline storage (no heap allocation)
 * - Parse once: Header is parsed once and cached here
 * - Flyweight: Uses pointer to shared FrameDescriptor instead of copying type
 * info
 * - Direct access: All fields are direct reads, no cursor creation needed
 *
 * Layout (32 bytes total):
 *   descriptor:     8 bytes (pointer to flyweight)
 *   streamId:       4 bytes
 *   flags_:         2 bytes (internal - use bool accessors)
 *   metadataSize:   2 bytes
 *   payloadOffset:  4 bytes
 *   payloadSize:    4 bytes
 *   reserved:       8 bytes (padding/future use)
 */
struct FrameMetadata {
  // Flyweight pointer to frame type descriptor (8 bytes)
  // Never null after parsing - points to element in kFrameDescriptors
  const FrameDescriptor* descriptor{nullptr};

  // Stream ID (4 bytes)
  // 0 for connection-level frames (SETUP, KEEPALIVE, METADATA_PUSH)
  // Non-zero for stream-level frames (REQUEST_*, PAYLOAD, CANCEL, etc.)
  uint32_t streamId{0};

  // Frame flags (2 bytes) - internal encoding detail
  // Use bool accessors below instead of accessing directly
  uint16_t flags_{0};

  // Metadata size in bytes (2 bytes)
  // 0 if no metadata present (hasMetadata() == false)
  // Otherwise, the size of the metadata portion of the payload
  uint16_t metadataSize{0};

  // Offset in buffer where payload starts (4 bytes)
  // This is the position after the frame header and metadata size field
  uint32_t payloadOffset{0};

  // Total payload size in bytes (4 bytes)
  // Includes both metadata and data portions
  uint32_t payloadSize{0};

  // Reserved for future use / padding (8 bytes)
  // Ensures struct is exactly 32 bytes for TypeErasedBox inline storage
  uint64_t reserved_{0};

  // Convenience accessors
  FrameType type() const noexcept {
    return descriptor ? descriptor->type : FrameType::RESERVED;
  }

  const char* typeName() const noexcept {
    return descriptor ? descriptor->name : "UNKNOWN";
  }

  // Flag accessors - provide semantic access to encoded flags
  bool hasMetadata() const noexcept { return flags_ & detail::kMetadataBit; }

  bool hasFollows() const noexcept { return flags_ & detail::kFollowsBit; }

  bool isComplete() const noexcept { return flags_ & detail::kCompleteBit; }

  bool hasNext() const noexcept { return flags_ & detail::kNextBit; }

  // Frame-type specific flag accessors
  bool shouldRespond() const noexcept { // KEEPALIVE
    return flags_ & detail::kRespondBit;
  }

  bool shouldIgnore() const noexcept { // EXT
    return flags_ & detail::kIgnoreBit;
  }

  bool hasLease() const noexcept { // SETUP
    return flags_ & detail::kLeaseBit;
  }

  bool hasResumeToken() const noexcept { // SETUP
    return flags_ & detail::kResumeTokenBit;
  }

  // Size of just the data portion (payload minus metadata)
  uint32_t dataSize() const noexcept {
    return payloadSize > metadataSize ? payloadSize - metadataSize : 0;
  }

  // Check if this is a request-initiating frame
  bool isRequestFrame() const noexcept {
    return descriptor && descriptor->isRequestFrame;
  }

  // Check if this is a connection-level frame (stream ID must be 0)
  bool isConnectionFrame() const noexcept {
    return descriptor && descriptor->isStreamZeroOnly;
  }
};

// Compile-time size verification. The precise byte count depends on the ABI
// (uint64_t alignment + sizeof(void*)): 32 bytes on 64-bit, 28 bytes on LP32
// x86_32, 32 bytes again on arm32 if uint64_t is 8-aligned. The design
// intent is just "fits comfortably in TypeErasedBox inline storage", so we
// assert the upper bound rather than an exact value.
static_assert(
    sizeof(FrameMetadata) <= 32,
    "FrameMetadata must fit within 32 bytes for TypeErasedBox inline storage");

} // namespace apache::thrift::fast_thrift::frame::read
