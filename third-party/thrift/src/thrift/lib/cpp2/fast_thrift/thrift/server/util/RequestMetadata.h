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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Deserialize RequestRpcMetadata from a ParsedFrame's metadata section
 * using Binary protocol. Returns an error on deserialization failure.
 *
 * Server-side mirror of client's deserializeResponseMetadata.
 */
inline folly::exception_wrapper deserializeRequestMetadata(
    const frame::read::ParsedFrame& frame,
    apache::thrift::RequestRpcMetadata& metadata) noexcept {
  try {
    if (frame.hasMetadata() && frame.metadataSize() > 0) {
      apache::thrift::BinaryProtocolReader reader;
      reader.setInput(frame.metadataCursor());
      metadata.read(&reader);
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        ("Failed to deserialize request metadata: " +
         folly::exceptionStr(std::current_exception()))
            .toStdString());
  }
  return folly::exception_wrapper{};
}

} // namespace apache::thrift::fast_thrift::thrift
