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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <memory>

namespace apache::thrift::fast_thrift::thrift {

// ============================================================================
// Client <-> Pipeline Interface
// ============================================================================
//
// These message types define the interface between ThriftClientChannel
// and the pipeline. The channel sends ThriftRequestMessage and receives
// ThriftResponseMessage.
//
//   Channel  ─────ThriftRequestMessage─────>  Pipeline
//   Channel  <────ThriftResponseMessage─────  Pipeline

/**
 * ThriftRequestPayload - Request payload data.
 *
 * Contains all the data fields for a request. This is separate from
 * ThriftRequestMessage so handlers can work with just the payload.
 *
 * The metadata field contains pre-serialized metadata (IOBuf) produced
 * by serializeRequestMetadata() in the channel.
 *
 * Fields ordered by alignment (largest to smallest) to minimize padding.
 */
#pragma pack(push, 1)
struct ThriftRequestPayload {
  // 8-byte aligned fields
  std::unique_ptr<folly::IOBuf> metadata;
  std::unique_ptr<folly::IOBuf> data;

  // 4-byte aligned fields
  uint32_t initialRequestN{0};

  apache::thrift::RpcKind rpcKind{};

  // 1-byte aligned fields
  bool complete{false};
};
#pragma pack(pop)

/**
 * ThriftRequestMessage - Outbound request from channel to pipeline.
 *
 * Wraps ThriftRequestPayload with a request handle for correlation.
 *
 * Fields ordered by alignment (largest to smallest) to minimize padding.
 */
#pragma pack(push, 1)
struct ThriftRequestMessage {
  ThriftRequestPayload payload;
  uint32_t requestHandle{apache::thrift::fast_thrift::rocket::kNoRequestHandle};
};
#pragma pack(pop)

// ============================================================================
// Response Message
// ============================================================================

/**
 * ThriftResponseMessage - Inbound response from pipeline to channel.
 *
 * Contains a ParsedFrame with the raw response data. The channel
 * deserializes metadata and processes the frame directly.
 */
#pragma pack(push, 1)
struct ThriftResponseMessage {
  apache::thrift::fast_thrift::frame::read::ParsedFrame frame;

  // 4-byte aligned fields
  uint32_t requestHandle{apache::thrift::fast_thrift::rocket::kNoRequestHandle};

  // 1-byte aligned fields
  apache::thrift::fast_thrift::frame::FrameType requestFrameType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
