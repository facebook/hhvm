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
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::thrift {

// ============================================================================
// Server <-> Pipeline Interface
// ============================================================================
//
//   Pipeline  ─────ThriftServerRequestMessage─────>  Handler
//   Pipeline  <────ThriftServerResponseMessage─────  Handler

/**
 * ThriftServerRequestMessage - Inbound request from pipeline to handler.
 */
#pragma pack(push, 1)
struct ThriftServerRequestMessage {
  frame::read::ParsedFrame frame;
  uint32_t streamId{0};
};
#pragma pack(pop)

/**
 * ThriftServerResponsePayload - Response payload data.
 */
#pragma pack(push, 1)
struct ThriftServerResponsePayload {
  std::unique_ptr<folly::IOBuf> data;
  std::unique_ptr<folly::IOBuf> metadata;
  bool complete{true};
};
#pragma pack(pop)

/**
 * ThriftServerResponseMessage - Outbound response from handler to pipeline.
 */
#pragma pack(push, 1)
struct ThriftServerResponseMessage {
  ThriftServerResponsePayload payload;
  uint32_t streamId{0};
  uint32_t errorCode{0};
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
