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

#include <folly/io/IOBuf.h>

#include <memory>

namespace apache::thrift::fast_thrift::frame::write {

/**
 * Frame serialization functions for FastTransport.
 *
 * These functions serialize frame headers and payload into IOBuf chains
 * suitable for transmission. The design follows a data-oriented approach
 * with typed header structs for each frame type.
 *
 * Serialization strategy:
 * - Takes ownership of metadata and data buffers (move semantics)
 * - Allocates a buffer for the frame header
 * - Chains metadata and data buffers directly (zero-copy, no cloning)
 *
 * Usage with C++20 designated initializers:
 *   auto frame = serialize(
 *       RequestStreamHeader{.streamId = 42, .initialRequestN = 100},
 *       std::move(metadata),
 *       std::move(data));
 */

// ============================================================================
// Request Frame Serializers
// ============================================================================

/**
 * Serialize a REQUEST_RESPONSE frame.
 *
 * @param header Frame header with streamId and flags
 * @param metadata Optional metadata buffer (nullptr if none), ownership taken
 * @param data Optional data buffer (nullptr if none), ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const RequestResponseHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

/**
 * Serialize a REQUEST_FNF (fire-and-forget) frame.
 */
std::unique_ptr<folly::IOBuf> serialize(
    const RequestFnfHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

/**
 * Serialize a REQUEST_STREAM frame.
 *
 * @param header Frame header with streamId, initialRequestN, and flags
 * @param metadata Optional metadata buffer, ownership taken
 * @param data Optional data buffer, ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const RequestStreamHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

/**
 * Serialize a REQUEST_CHANNEL frame.
 */
std::unique_ptr<folly::IOBuf> serialize(
    const RequestChannelHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

// ============================================================================
// Flow Control Frame Serializers
// ============================================================================

/**
 * Serialize a REQUEST_N frame.
 *
 * @param header Frame header with streamId and requestN
 * @return Serialized frame (no payload)
 */
std::unique_ptr<folly::IOBuf> serialize(const RequestNHeader& header);

/**
 * Serialize a CANCEL frame.
 *
 * @param header Frame header with streamId
 * @return Serialized frame (no payload)
 */
std::unique_ptr<folly::IOBuf> serialize(const CancelHeader& header);

// ============================================================================
// Payload & Error Frame Serializers
// ============================================================================

/**
 * Serialize a PAYLOAD frame.
 *
 * @param header Frame header with streamId and flags (complete, next, follows)
 * @param metadata Optional metadata buffer, ownership taken
 * @param data Optional data buffer, ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const PayloadHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

/**
 * Serialize an ERROR frame.
 *
 * @param header Frame header with streamId and errorCode
 * @param metadata Optional metadata buffer, ownership taken
 * @param data Optional error message/data buffer, ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const ErrorHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

// ============================================================================
// Connection-Level Frame Serializers
// ============================================================================

/**
 * Serialize a KEEPALIVE frame.
 *
 * @param header Frame header with lastReceivedPosition and respond flag
 * @param data Optional data buffer, ownership taken
 * @return Serialized frame
 */
std::unique_ptr<folly::IOBuf> serialize(
    const KeepAliveHeader& header,
    std::unique_ptr<folly::IOBuf> data = nullptr);

/**
 * Serialize a SETUP frame.
 *
 * @param header Frame header with version, keepalive, lifetime settings
 * @param metadata Optional metadata buffer, ownership taken
 * @param data Optional data buffer, ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const SetupHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

/**
 * Serialize a METADATA_PUSH frame.
 *
 * @param header Frame header (no fields)
 * @param metadata Metadata buffer, ownership taken
 * @return Serialized frame
 */
std::unique_ptr<folly::IOBuf> serialize(
    const MetadataPushHeader& header, std::unique_ptr<folly::IOBuf> metadata);

// ============================================================================
// Extension Frame Serializer
// ============================================================================

/**
 * Serialize an EXT (extension) frame.
 *
 * @param header Frame header with streamId, extendedType, and ignore flag
 * @param metadata Optional metadata buffer, ownership taken
 * @param data Optional data buffer, ownership taken
 * @return Serialized frame as IOBuf chain
 */
std::unique_ptr<folly::IOBuf> serialize(
    const ExtHeader& header,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data);

} // namespace apache::thrift::fast_thrift::frame::write
