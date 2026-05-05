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

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kDataHeadroomBytes = 128;

/**
 * Serialize the data payload of a Thrift request.
 *
 * Caller provides serialize/size callbacks for the struct payload. The
 * adapter is responsible for building the request metadata separately;
 * this helper only produces the raw data bytes.
 *
 * @param serializeFn (ProtocolWriter&) -> void — serializes the payload
 * @param sizeFn      (ProtocolWriter&) -> uint32_t — returns serialized size
 */
template <typename ProtocolWriter, typename SerializeFn, typename SizeFn>
folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
serializeRequest(SerializeFn&& serializeFn, SizeFn&& sizeFn) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  try {
    ProtocolWriter writer;
    uint32_t serSize = sizeFn(writer);

    auto dataBuf = folly::IOBuf::create(kDataHeadroomBytes + serSize);
    dataBuf->advance(kDataHeadroomBytes);
    queue.append(std::move(dataBuf));
    writer.setOutput(&queue);
    serializeFn(writer);
  } catch (...) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            "Failed to serialize request data"));
  }

  return queue.move();
}

} // namespace apache::thrift::fast_thrift::thrift
