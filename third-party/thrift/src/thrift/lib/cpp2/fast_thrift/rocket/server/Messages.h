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
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::rocket::server {

// ============================================================================
// Rocket Server Message Types
// ============================================================================

/**
 * RocketRequestMessage - Inbound request from client, delivered to App.
 *
 * Contains the client-assigned streamId and the parsed request frame.
 * App uses streamId to correlate responses back to this request.
 * Use FrameView to check frame type (REQUEST_RESPONSE, REQUEST_STREAM,
 * CANCEL, etc.).
 *
 * If error is set, the connection failed and frame may be empty.
 * App layer should check error first before processing frame.
 */
#pragma pack(push, 1)
struct RocketRequestMessage {
  apache::thrift::fast_thrift::frame::read::ParsedFrame frame; // 40B
  folly::exception_wrapper error; // 8B
  uint32_t streamId{0}; // 4B
};
#pragma pack(pop)

/**
 * RocketResponseMessage - Outbound response from App to StreamHandler.
 *
 * App sends this with the streamId received from RocketRequestMessage.
 * Set complete=true for terminal responses (last PAYLOAD, ERROR).
 * Set complete=false for intermediate streaming responses.
 */
#pragma pack(push, 1)
struct RocketResponseMessage {
  std::unique_ptr<folly::IOBuf> payload; // 8B
  std::unique_ptr<folly::IOBuf> metadata; // 8B
  uint32_t streamId{0}; // 4B
  uint32_t errorCode{0}; // 4B
  bool complete{true}; // 1B
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::rocket::server
