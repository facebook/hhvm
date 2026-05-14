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

#include <algorithm>
#include <cstddef>
#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/RequestMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

template <typename ProtocolWriter>
inline std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  ProtocolWriter writer;
  size_t serSize = metadata.serializedSizeZC(&writer);

  constexpr size_t kMinAllocBytes = 1024;
  auto buf = folly::IOBuf::create(
      std::max(kMetadataHeadroomBytes + serSize, kMinAllocBytes));
  buf->advance(kMetadataHeadroomBytes);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

template <typename ProtocolReader>
inline folly::exception_wrapper deserializeResponseMetadata(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame,
    apache::thrift::ResponseRpcMetadata& metadata) noexcept {
  try {
    if (frame.hasMetadata() && frame.metadataSize() > 0) {
      ProtocolReader reader;
      reader.setInput(frame.metadataCursor());
      metadata.read(&reader);
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        "Failed to deserialize response metadata");
  }
  return folly::exception_wrapper{};
}

} // namespace detail

/**
 * Serialize a populated ResponseRpcMetadata into an IOBuf using the
 * given metadata protocol (Binary or Compact).
 *
 * Pre-calculates serialized size to allocate an exactly-sized buffer.
 * Reserves headroom so a downstream frame writer can prepend headers
 * without a separate allocation.
 */
inline std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata,
    rocket::server::MetadataProtocol metadataProtocol) {
  if (metadataProtocol == rocket::server::MetadataProtocol::COMPACT) {
    return detail::serializeResponseMetadata<
        apache::thrift::CompactProtocolWriter>(metadata);
  }
  return detail::serializeResponseMetadata<
      apache::thrift::BinaryProtocolWriter>(metadata);
}

/**
 * Deserialize ResponseRpcMetadata from a ParsedFrame's metadata section
 * using Binary protocol. Returns an error on deserialization failure.
 *
 * Inverse of `serializeResponseMetadata`. Used by `fromRocketFrame` on the
 * client inbound path.
 *
 * TODO: thread MetadataProtocol through this call once the client wires
 * the SETUP-negotiated protocol into the transport adapter (matching the
 * server outbound path). The templated detail::deserializeResponseMetadata
 * above is ready for that switch.
 */
inline folly::exception_wrapper deserializeResponseMetadata(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame,
    apache::thrift::ResponseRpcMetadata& metadata) noexcept {
  return detail::deserializeResponseMetadata<
      apache::thrift::BinaryProtocolReader>(frame, metadata);
}

} // namespace apache::thrift::fast_thrift::thrift
