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

#include <array>
#include <cstdint>

namespace apache::thrift::fast_thrift::frame {

// ============================================================================
// RSocket Frame Wire Format
// ============================================================================
//
// All frames follow this basic structure:
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         Stream ID                             |
//  +-----------+-+-+-+-+-+-+-+-+-+-+-------------------------------+
//  |Frame Type |I|M|  Flags (10)   |
//  +-----------+-+-+-+-+-+-+-+-+-+-+-------------------------------+
//  |                    Frame-specific header...                   |
//  +---------------------------------------------------------------+
//  |                    Payload (metadata + data)...               |
//  +---------------------------------------------------------------+
//
// Frame Header (6 bytes minimum):
//   - Stream ID:      4 bytes, big-endian (0 for connection-level frames)
//   - Type+Flags:     2 bytes, big-endian
//     - Upper 6 bits: Frame Type (0x00-0x3F)
//     - Lower 10 bits: Flags
//
// Type+Flags Field Layout (16 bits):
//   Bit:  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
//         [  Frame Type  ] [I][M][      Flags (bits 0-7)      ]
//
// ============================================================================
// Flag Bit Positions (see detail::k*Bit constants in FrameType.h)
// ============================================================================
//
// Common Flags (applicable to multiple frame types):
//   Bit 8 (M): Metadata present in payload      (detail::kMetadataBit)
//   Bit 7 (F): Follows - more fragments coming  (detail::kFollowsBit)
//   Bit 6 (C): Complete - stream is complete    (detail::kCompleteBit)
//   Bit 5 (N): Next - next payload in stream    (detail::kNextBit)
//
// Context-Specific Flags (same bit positions, frame-type-dependent meaning):
//   Bit 9 (I): Ignore - EXT frame only          (detail::kIgnoreBit)
//   Bit 7 (R): Respond - KEEPALIVE only         (detail::kRespondBit)
//   Bit 7 (T): Resume Token - SETUP only        (detail::kResumeTokenBit)
//   Bit 6 (L): Lease - SETUP only               (detail::kLeaseBit)
//
// Flag Applicability by Frame Type:
//   +------------------+---+---+---+---+---+---+
//   | Frame Type       | M | F | C | N | I | R |
//   +------------------+---+---+---+---+---+---+
//   | REQUEST_RESPONSE | * | * |   |   |   |   |
//   | REQUEST_FNF      | * | * |   |   |   |   |
//   | REQUEST_STREAM   | * | * |   |   |   |   |
//   | REQUEST_CHANNEL  | * | * | * |   |   |   |
//   | PAYLOAD          | * | * | * | * |   |   |
//   | KEEPALIVE        |   |   |   |   |   | * |
//   | SETUP            | * | T |   | L |   |   |  (T=ResumeToken, L=Lease)
//   | EXT              | * |   |   |   | * |   |
//   | ERROR            | * |   |   |   |   |   |
//   | CANCEL           |   |   |   |   |   |   |  (no flags)
//   | REQUEST_N        |   |   |   |   |   |   |  (no flags)
//   | METADATA_PUSH    |   |   |   |   |   |   |  (entire payload is metadata)
//   +------------------+---+---+---+---+---+---+
//
// API Design:
//   - Write side: Header structs have semantic bool fields (e.g., .follows,
//   .complete)
//   - Read side: Views/FrameMetadata provide bool accessors (e.g.,
//   hasFollows())
//   - Internal: Flag encoding uses uint16_t + detail::k*Bit constants
//
// ============================================================================

/**
 * FrameDescriptor - Flyweight pattern for frame type metadata.
 *
 * This struct contains compile-time, immutable metadata about each frame type.
 * All frames of the same type share a single descriptor instance, eliminating
 * per-frame storage overhead for type information.
 *
 * Design rationale:
 * - constexpr: All descriptors are compile-time constants
 * - Zero runtime cost: No per-frame allocation for type metadata
 * - Single source of truth: Header sizes defined once, used everywhere
 */
struct FrameDescriptor {
  FrameType type;
  uint8_t headerSize; // Size of frame header (including streamId + typeFlags)
  const char* name; // Human-readable name for debugging
  bool hasStreamId; // True if frame has a meaningful stream ID
  bool isRequestFrame; // True for REQUEST_RESPONSE, REQUEST_FNF, etc.
  bool isStreamZeroOnly; // True if frame must have stream ID = 0
};

// Frame header size constants (in bytes)
// Base header size (kBaseHeaderSize) is defined in FrameType.h
// Frame-specific sizes add to the base header
namespace detail {
constexpr uint8_t kRequestNHeaderSize =
    kBaseHeaderSize + 4; // base + requestN (4)
constexpr uint8_t kErrorHeaderSize =
    kBaseHeaderSize + 4; // base + errorCode (4)
constexpr uint8_t kExtHeaderSize =
    kBaseHeaderSize + 4; // base + extFrameType (4)
constexpr uint8_t kKeepAliveHeaderSize =
    kBaseHeaderSize + 8; // base + lastPosition (8)
constexpr uint8_t kSetupHeaderSize = kBaseHeaderSize +
    14; // base + version(4) + keepalive(4) + lifetime(4) + resumeTokenLen(2)
} // namespace detail

/**
 * Compile-time array of all frame descriptors.
 *
 * Indexed by FrameType value for O(1) lookup.
 * Note: Not all indices are valid frame types (gaps in the enum).
 */
// clang-format off
inline constexpr std::array<FrameDescriptor, 64> kFrameDescriptors = {{
  // 0x00 RESERVED
  {FrameType::RESERVED, 0, "RESERVED", false, false, false},
  // 0x01 SETUP
  {FrameType::SETUP, detail::kSetupHeaderSize, "SETUP", false, false, true},
  // 0x02 LEASE
  {FrameType::LEASE, kBaseHeaderSize, "LEASE", false, false, true},
  // 0x03 KEEPALIVE
  {FrameType::KEEPALIVE, detail::kKeepAliveHeaderSize, "KEEPALIVE", false, false, true},
  // 0x04 REQUEST_RESPONSE
  {FrameType::REQUEST_RESPONSE, kBaseHeaderSize, "REQUEST_RESPONSE", true, true, false},
  // 0x05 REQUEST_FNF
  {FrameType::REQUEST_FNF, kBaseHeaderSize, "REQUEST_FNF", true, true, false},
  // 0x06 REQUEST_STREAM
  {FrameType::REQUEST_STREAM, detail::kRequestNHeaderSize, "REQUEST_STREAM", true, true, false},
  // 0x07 REQUEST_CHANNEL
  {FrameType::REQUEST_CHANNEL, detail::kRequestNHeaderSize, "REQUEST_CHANNEL", true, true, false},
  // 0x08 REQUEST_N
  {FrameType::REQUEST_N, detail::kRequestNHeaderSize, "REQUEST_N", true, false, false},
  // 0x09 CANCEL
  {FrameType::CANCEL, kBaseHeaderSize, "CANCEL", true, false, false},
  // 0x0A PAYLOAD
  {FrameType::PAYLOAD, kBaseHeaderSize, "PAYLOAD", true, false, false},
  // 0x0B ERROR
  {FrameType::ERROR, detail::kErrorHeaderSize, "ERROR", true, false, false},
  // 0x0C METADATA_PUSH
  {FrameType::METADATA_PUSH, kBaseHeaderSize, "METADATA_PUSH", false, false, true},
  // 0x0D RESUME
  {FrameType::RESUME, kBaseHeaderSize, "RESUME", false, false, true},
  // 0x0E RESUME_OK
  {FrameType::RESUME_OK, kBaseHeaderSize, "RESUME_OK", false, false, true},
  // Fill gaps with RESERVED entries (0x0F - 0x3E)
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x0F
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x10
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x11
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x12
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x13
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x14
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x15
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x16
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x17
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x18
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x19
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1A
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1B
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1C
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1D
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1E
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x1F
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x20
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x21
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x22
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x23
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x24
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x25
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x26
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x27
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x28
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x29
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2A
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2B
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2C
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2D
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2E
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x2F
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x30
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x31
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x32
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x33
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x34
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x35
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x36
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x37
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x38
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x39
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x3A
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x3B
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x3C
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x3D
  {FrameType::RESERVED, 0, "RESERVED", false, false, false}, // 0x3E
  // 0x3F EXT
  {FrameType::EXT, detail::kExtHeaderSize, "EXT", true, false, false},
}};
// clang-format on

/**
 * Get the descriptor for a frame type.
 *
 * O(1) lookup via array indexing.
 *
 * @param type The frame type to look up
 * @return Reference to the frame descriptor (never null, returns RESERVED for
 * unknown types)
 */
constexpr const FrameDescriptor& getDescriptor(FrameType type) noexcept {
  auto index = static_cast<uint8_t>(type);
  if (index < kFrameDescriptors.size()) {
    return kFrameDescriptors[index];
  }
  return kFrameDescriptors[0]; // Return RESERVED for out-of-bounds
}

/**
 * Check if a frame type is valid (not RESERVED).
 */
constexpr bool isValidFrameType(FrameType type) noexcept {
  return getDescriptor(type).type != FrameType::RESERVED;
}

} // namespace apache::thrift::fast_thrift::frame
