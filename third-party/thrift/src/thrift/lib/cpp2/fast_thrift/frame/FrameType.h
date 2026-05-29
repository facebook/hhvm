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

#include <cstddef>
#include <cstdint>

namespace apache::thrift::fast_thrift::frame {

/**
 * RSocket frame types.
 *
 * These match the RSocket protocol specification used by Thrift Rocket.
 */
enum class FrameType : uint8_t {
  RESERVED = 0x00,
  SETUP = 0x01,
  LEASE = 0x02,
  KEEPALIVE = 0x03,
  REQUEST_RESPONSE = 0x04,
  REQUEST_FNF = 0x05,
  REQUEST_STREAM = 0x06,
  REQUEST_CHANNEL = 0x07,
  REQUEST_N = 0x08,
  CANCEL = 0x09,
  PAYLOAD = 0x0A,
  ERROR = 0x0B,
  METADATA_PUSH = 0x0C,
  RESUME = 0x0D,
  RESUME_OK = 0x0E,
  EXT = 0x3F,
};

// ============================================================================
// Stream ID Constants
// ============================================================================

// Connection-level frames use stream ID 0 (not associated with any stream)
constexpr uint32_t kConnectionStreamId = 0;

// ============================================================================
// Frame Header Layout Constants (RSocket Protocol)
// ============================================================================
// These constants define the wire format for frame headers.
// Used by both reading (FrameParser) and writing (FrameWriter) code.

// Individual field sizes
constexpr size_t kStreamIdSize = 4;
constexpr size_t kTypeAndFlagsSize = 2;

// Base header: streamId (4 bytes) + typeAndFlags (2 bytes)
constexpr size_t kBaseHeaderSize = kStreamIdSize + kTypeAndFlagsSize;

// Metadata/frame length field (3-byte big-endian per RSocket spec)
constexpr size_t kMetadataLengthSize = 3;

// ============================================================================
// Internal Flag Constants (Protocol Encoding Details)
// ============================================================================
// These constants are used internally for encoding/decoding frame flags.
// They should not be exposed in public APIs - use semantic bool fields
// in Header structs (write) and bool accessors in Views (read) instead.

namespace detail {

constexpr uint8_t kFlagsBits = 10;
constexpr uint16_t kFlagsMask = (1 << kFlagsBits) - 1;

// Common flags (valid for multiple frame types)
constexpr uint16_t kMetadataBit = 1 << 8; // Metadata present in payload
constexpr uint16_t kFollowsBit = 1 << 7; // More fragments follow
constexpr uint16_t kCompleteBit = 1 << 6; // Stream is complete
constexpr uint16_t kNextBit = 1 << 5; // Next payload in stream

// Context-specific flags (same bit positions, different meanings)
constexpr uint16_t kRespondBit = 1 << 7; // KEEPALIVE: should respond
constexpr uint16_t kIgnoreBit = 1 << 9; // EXT: ignore unknown type
constexpr uint16_t kLeaseBit = 1 << 6; // SETUP: client requests lease
constexpr uint16_t kResumeTokenBit = 1 << 7; // SETUP: resume token present

} // namespace detail

// ============================================================================
// Frame Classification Utility Functions
// ============================================================================

/**
 * Check if the given frame type represents a terminal frame.
 *
 * Terminal frames end the stream:
 * - ERROR
 * - CANCEL
 *
 * @param type The frame type to check
 * @return true if this frame would terminate the rsocket stream
 */
constexpr bool isTerminalFrameType(FrameType type) noexcept {
  return (type == FrameType::ERROR) || (type == FrameType::CANCEL);
}

} // namespace apache::thrift::fast_thrift::frame
