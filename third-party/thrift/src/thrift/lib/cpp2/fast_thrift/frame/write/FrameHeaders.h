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

#include <cstdint>

namespace apache::thrift::fast_thrift::frame::write {

// ============================================================================
// Request Frames
// ============================================================================

/**
 * Header for REQUEST_RESPONSE frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       RequestResponseHeader{.streamId = 1, .follows = true},
 *       metadata.get(),
 *       data.get());
 */
struct RequestResponseHeader {
  uint32_t streamId;
  bool follows{false}; // More fragments follow
};

/**
 * Header for REQUEST_FNF (fire-and-forget) frames.
 */
struct RequestFnfHeader {
  uint32_t streamId;
  bool follows{false};
};

/**
 * Header for REQUEST_STREAM frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       RequestStreamHeader{.streamId = 1, .initialRequestN = 100},
 *       metadata.get(),
 *       data.get());
 */
struct RequestStreamHeader {
  uint32_t streamId;
  uint32_t initialRequestN;
  bool follows{false};
};

/**
 * Header for REQUEST_CHANNEL frames.
 */
struct RequestChannelHeader {
  uint32_t streamId;
  uint32_t initialRequestN;
  bool follows{false};
  bool complete{false}; // Initial payload is complete
};

// ============================================================================
// Flow Control Frames
// ============================================================================

/**
 * Header for REQUEST_N frames.
 *
 * Usage:
 *   auto frame = serialize(RequestNHeader{.streamId = 1, .requestN = 10});
 */
struct RequestNHeader {
  uint32_t streamId;
  uint32_t requestN;
};

/**
 * Header for CANCEL frames.
 *
 * Usage:
 *   auto frame = serialize(CancelHeader{.streamId = 1});
 */
struct CancelHeader {
  uint32_t streamId;
};

// ============================================================================
// Payload & Error Frames
// ============================================================================

/**
 * Header for PAYLOAD frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       PayloadHeader{.streamId = 1, .complete = true, .next = true},
 *       metadata.get(),
 *       data.get());
 */
struct PayloadHeader {
  uint32_t streamId;
  bool follows{false};
  bool complete{false};
  bool next{false};
};

/**
 * Header for ERROR frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       ErrorHeader{.streamId = 1, .errorCode = 0x00000201},
 *       nullptr,
 *       errorMessage.get());
 */
struct ErrorHeader {
  uint32_t streamId;
  uint32_t errorCode;
};

// ============================================================================
// Connection-Level Frames
// ============================================================================

/**
 * Header for KEEPALIVE frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       KeepAliveHeader{.lastReceivedPosition = 12345, .respond = true});
 */
struct KeepAliveHeader {
  uint64_t lastReceivedPosition{0};
  bool respond{false};
};

/**
 * Header for SETUP frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       SetupHeader{
 *           .majorVersion = 1,
 *           .minorVersion = 0,
 *           .keepaliveTime = 30000,
 *           .maxLifetime = 60000},
 *       metadata.get(),
 *       data.get());
 */
struct SetupHeader {
  uint16_t majorVersion{1};
  uint16_t minorVersion{0};
  uint32_t keepaliveTime{0};
  uint32_t maxLifetime{0};
  bool lease{false};
  // Note: Resume token handling TBD
};

/**
 * Header for METADATA_PUSH frames.
 *
 * Usage:
 *   auto frame = serialize(MetadataPushHeader{}, metadata.get());
 */
struct MetadataPushHeader {
  // No fields - metadata-only frame
};

// ============================================================================
// Extension Frame
// ============================================================================

/**
 * Header for EXT (extension) frames.
 *
 * Usage:
 *   auto frame = serialize(
 *       ExtHeader{.streamId = 1, .extendedType = 0x01, .ignore = true},
 *       metadata.get(),
 *       data.get());
 */
struct ExtHeader {
  uint32_t streamId;
  uint32_t extendedType;
  bool ignore{false};
};

} // namespace apache::thrift::fast_thrift::frame::write
