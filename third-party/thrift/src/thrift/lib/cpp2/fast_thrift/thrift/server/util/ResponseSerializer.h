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
#include <folly/io/IOBufQueue.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kServerDataHeadroomBytes = 128;

// Serialize a presult into a single IOBuf with the standard server data
// headroom reserved at the front. Codegen calls this with a presult
// write/size pair; the resulting IOBuf is handed directly to
// ThriftServerAppAdapter::writeResponse alongside a metadata IOBuf.
//
// Mirrors GeneratedAsyncProcessorBase::serializeResponse from legacy
// (fbcode/thrift/lib/cpp2/async/processor/GeneratedAsyncProcessorBase.h),
// without the ContextStack/header-transform tail.
template <typename ProtocolWriter, typename SerializeFn, typename SizeFn>
std::unique_ptr<folly::IOBuf> serializeResponse(
    SerializeFn&& serializeFn, SizeFn&& sizeFn) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

  ProtocolWriter writer;
  uint32_t serSize = sizeFn(writer);

  auto dataBuf = folly::IOBuf::create(kServerDataHeadroomBytes + serSize);
  dataBuf->advance(kServerDataHeadroomBytes);
  queue.append(std::move(dataBuf));
  writer.setOutput(&queue);
  serializeFn(writer);

  return queue.move();
}

} // namespace apache::thrift::fast_thrift::thrift
