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
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kServerPushMetadataHeadroomBytes = 16;

// Serializes ServerPushMetadata for a connection-level METADATA_PUSH frame.
// Always Compact: METADATA_PUSH bodies are Compact-encoded regardless of the
// per-connection RpcMetadata protocol negotiated via the SETUP MIME type, and
// clients decode this payload as Compact unconditionally.
inline std::unique_ptr<folly::IOBuf> serializeServerPushMetadata(
    const apache::thrift::ServerPushMetadata& metadata) {
  apache::thrift::CompactProtocolWriter writer;
  auto serializedSize = metadata.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  auto buf =
      folly::IOBuf::create(kServerPushMetadataHeadroomBytes + serializedSize);
  buf->advance(kServerPushMetadataHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

} // namespace apache::thrift::fast_thrift::thrift
