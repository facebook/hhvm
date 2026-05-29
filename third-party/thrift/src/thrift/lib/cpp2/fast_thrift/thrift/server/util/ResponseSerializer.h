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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kServerDataHeadroomBytes = 128;

template <typename ProtocolWriter, typename SerializeFn, typename SizeFn>
ThriftServerResponseMessage serializeResponse(
    SerializeFn&& serializeFn, SizeFn&& sizeFn, uint32_t streamId) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

  ProtocolWriter writer;
  uint32_t serSize = sizeFn(writer);

  auto dataBuf = folly::IOBuf::create(kServerDataHeadroomBytes + serSize);
  dataBuf->advance(kServerDataHeadroomBytes);
  queue.append(std::move(dataBuf));
  writer.setOutput(&queue);
  serializeFn(writer);

  return ThriftServerResponseMessage{
      .payload =
          ThriftServerResponsePayload{
              .data = queue.move(),
              .metadata = getDefaultSuccessMetadata(),
              .complete = true},
      .streamId = streamId};
}

template <typename ProtocolWriter, typename SerializeFn>
ThriftServerResponseMessage serializeResponse(
    SerializeFn&& serializeFn, uint32_t streamId) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

  auto dataBuf = folly::IOBuf::create(kServerDataHeadroomBytes + 256);
  dataBuf->advance(kServerDataHeadroomBytes);
  queue.append(std::move(dataBuf));
  ProtocolWriter writer;
  writer.setOutput(&queue);
  serializeFn(writer);

  return ThriftServerResponseMessage{
      .payload =
          ThriftServerResponsePayload{
              .data = queue.move(),
              .metadata = getDefaultSuccessMetadata(),
              .complete = true},
      .streamId = streamId};
}

inline ThriftServerResponseMessage buildErrorResponse(
    uint32_t streamId, std::string errorMessage) {
  return ThriftServerResponseMessage{
      .payload =
          ThriftServerResponsePayload{
              .data = nullptr,
              .metadata = makeErrorResponseMetadata(std::move(errorMessage)),
              .complete = true},
      .streamId = streamId};
}

} // namespace apache::thrift::fast_thrift::thrift
