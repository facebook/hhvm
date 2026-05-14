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

#include <cstddef>
#include <memory>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kStreamPayloadMetadataHeadroomBytes = 16;

namespace detail {

template <typename Writer>
inline std::unique_ptr<folly::IOBuf> serializeStreamPayloadMetadataWithWriter(
    const apache::thrift::StreamPayloadMetadata& metadata) {
  Writer writer;
  auto serializedSize = metadata.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  auto buf = folly::IOBuf::create(
      kStreamPayloadMetadataHeadroomBytes + serializedSize);
  buf->advance(kStreamPayloadMetadataHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

} // namespace detail

/**
 * Serialize a populated StreamPayloadMetadata into an IOBuf using the
 * given protocol (Binary or Compact).
 *
 * Pre-calculates serialized size to allocate an exactly-sized buffer.
 * Reserves headroom so a downstream frame writer can prepend headers
 * without a separate allocation.
 */
inline std::unique_ptr<folly::IOBuf> serializeStreamPayloadMetadata(
    const apache::thrift::StreamPayloadMetadata& metadata,
    rocket::server::MetadataProtocol metadataProtocol) {
  if (metadataProtocol == rocket::server::MetadataProtocol::COMPACT) {
    return detail::serializeStreamPayloadMetadataWithWriter<
        apache::thrift::CompactProtocolWriter>(metadata);
  }
  return detail::serializeStreamPayloadMetadataWithWriter<
      apache::thrift::BinaryProtocolWriter>(metadata);
}

} // namespace apache::thrift::fast_thrift::thrift
