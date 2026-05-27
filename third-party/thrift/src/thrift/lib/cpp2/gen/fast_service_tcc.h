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
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

template <typename ProtocolReader, typename Pargs>
inline void deserializeRequest(folly::IOBuf& data, Pargs& pargs) {
  ProtocolReader reader;
  folly::io::Cursor cursor(&data);
  reader.setInput(cursor);
  apache::thrift::detail::deserializeRequestBodySimple(&reader, &pargs);
}

inline void sendDeserializationError(
    ThriftServerAppAdapter* adapter,
    uint32_t streamId,
    const folly::exception_wrapper& ew) noexcept {
  adapter->writeResponse(makeAppErrorMessage(
      streamId,
      "TApplicationException",
      ew.what().toStdString(),
      apache::thrift::ErrorBlame::SERVER));
}

} // namespace apache::thrift::fast_thrift::thrift
