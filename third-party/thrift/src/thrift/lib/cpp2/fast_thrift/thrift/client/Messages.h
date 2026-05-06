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
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h>

#include <cstdint>

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
 * ThriftRequestMessage - Outbound request from channel to pipeline.
 *
 * `payload` is a typed variant of per-RPC-kind payload structs from
 * `thrift/common/ThriftPayload.h`. Today only `ThriftRequestResponsePayload`
 * is in the variant — the only RpcKind wired end-to-end through the
 * client pipeline. As FNF / Stream / Sink / Bidi handlers come online,
 * their payload alternatives join the variant.
 */
#pragma pack(push, 1)
struct ThriftRequestMessage {
  ThriftPayloadVariant<ThriftRequestResponsePayload> payload;
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
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
