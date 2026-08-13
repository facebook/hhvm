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

#include <memory>
#include <string_view>
#include <utility>

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift {

// Outbound messages for the connection setup exchange.
//
// Deliberately separate from ResponsePayloads.h: the helpers there reach
// ResponseError.h and, through it, GeneratedCodeHelper. The setup path needs
// none of that, and the handlers on it are cheaper to compile without it.

// Stream-0 ERROR refusing the connection setup. `reason` reaches the client in
// the frame body, so a refusal says why rather than arriving bare.
inline ThriftServerResponseMessage makeSetupRejectionMessage(
    apache::thrift::fast_thrift::frame::ErrorCode errorCode,
    std::string_view reason) {
  return ThriftServerResponseMessage{
      .payload = ThriftSetupRejectionPayload{
          .reason = folly::IOBuf::copyBuffer(reason),
          .errorCode = static_cast<uint32_t>(errorCode)}};
}

// The SETUP response, on its way out as a connection-level METADATA_PUSH. Still
// structured: handlers on the write path stamp the fields they own before it is
// serialized at the edge.
inline ThriftServerResponseMessage makeSetupResponseMessage(
    std::unique_ptr<apache::thrift::SetupResponse> response) {
  return ThriftServerResponseMessage{
      .payload = ThriftSetupResponsePayload{.response = std::move(response)}};
}

} // namespace apache::thrift::fast_thrift::thrift
