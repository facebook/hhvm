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

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

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
  apache::thrift::fast_thrift::rocket::TypeErasedPtr requestContext;
};
#pragma pack(pop)

// ============================================================================
// Response Message
// ============================================================================

/**
 * ThriftResponseError — in-process per-request failure carried inbound.
 *
 * Used as the error alternative on `ThriftResponseMessage::payload` so a
 * single request can be failed (callback resolved with `ew`) without
 * fabricating a wire-format ERROR frame and without escalating to a
 * connection-fatal `fireException`. Mirrors `rocket::RocketResponseError`
 * one layer up: the rocket adapter translates the rocket error variant
 * into this thrift error variant before fireRead.
 *
 * The connection itself remains healthy; only this one request's pending
 * callback is failed.
 */
struct ThriftResponseError {
  folly::exception_wrapper ew;
};

/**
 * ThriftResponseMessage - Inbound response from pipeline to channel.
 *
 * `payload` is a CompactVariant of either a parsed wire frame (the normal
 * response path) or a `ThriftResponseError` (in-process per-request
 * failure). The channel inspects which alternative is present and either
 * decodes the frame or fails the pending callback with the wrapped
 * exception. In both cases the channel stays Open.
 */
#pragma pack(push, 1)
struct ThriftResponseMessage {
  apache::thrift::fast_thrift::CompactVariant<
      apache::thrift::fast_thrift::frame::read::ParsedFrame,
      ThriftResponseError>
      payload;
  apache::thrift::fast_thrift::rocket::TypeErasedPtr requestContext;
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
