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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

// Reserved headroom for downstream frame header serialization
// (stream ID + type + flags + metadata length = ~12 bytes, rounded up)
inline constexpr size_t kMetadataHeadroomBytes = 16;

/**
 * Serialize a populated RequestRpcMetadata into an IOBuf.
 *
 * Pre-calculates serialized size to allocate an exactly-sized buffer.
 * Reserves headroom so the downstream frame writer can prepend headers
 * without a separate allocation.
 *
 * Called from `ThriftRequestResponsePayload::toRocketFrame()` on the
 * client outbound path, and from the client-side metadata builders in
 * `thrift/client/RequestMetadata.h`.
 */
inline std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
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

} // namespace apache::thrift::fast_thrift::thrift
