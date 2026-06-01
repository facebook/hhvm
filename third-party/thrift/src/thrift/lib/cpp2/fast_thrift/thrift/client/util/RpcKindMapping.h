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
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

constexpr apache::thrift::fast_thrift::frame::FrameType rpcKindToFrameType(
    apache::thrift::RpcKind kind) noexcept {
  switch (kind) {
    case apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      return apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    case apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      return apache::thrift::fast_thrift::frame::FrameType::REQUEST_FNF;
    case apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
      return apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM;
    case apache::thrift::RpcKind::SINK:
    case apache::thrift::RpcKind::BIDIRECTIONAL_STREAM:
    default:
      return apache::thrift::fast_thrift::frame::FrameType::RESERVED;
  }
}

} // namespace apache::thrift::fast_thrift::thrift
