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

#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace detail {

template <class Metadata>
Payload makePayload(
    const Metadata& metadata, std::unique_ptr<folly::IOBuf> data) {
  CompactProtocolWriter writer;
  // Default is to leave some headroom for rsocket headers
  size_t serSize = metadata.serializedSizeZC(&writer);
  constexpr size_t kHeadroomBytes = 16;

  folly::IOBufQueue queue;

  // If possible, serialize metadata into the headeroom of data.
  if (data && !data->isChained() &&
      data->headroom() >= serSize + kHeadroomBytes && !data->isSharedOne()) {
    // Store previous state of the buffer pointers and rewind it.
    auto startBuffer = data->buffer();
    auto start = data->data();
    auto origLen = data->length();
    data->trimEnd(origLen);
    data->retreat(start - startBuffer);

    queue.append(std::move(data), false);
    writer.setOutput(&queue);
    auto metadataLen = metadata.write(&writer);

    // Move the new data to come right before the old data and restore the
    // old tail pointer.
    data = queue.move();
    data->advance(start - data->tail());
    data->append(origLen);

    return Payload::makeCombined(std::move(data), metadataLen);
  } else {
    constexpr size_t kMinAllocBytes = 1024;
    auto buf = folly::IOBuf::create(
        std::max(kHeadroomBytes + serSize, kMinAllocBytes));
    buf->advance(kHeadroomBytes);
    queue.append(std::move(buf));
    writer.setOutput(&queue);
    auto metadataLen = metadata.write(&writer);
    queue.append(std::move(data));
    return Payload::makeCombined(queue.move(), metadataLen);
  }
}

template Payload makePayload<>(
    const RequestRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload makePayload<>(
    const ResponseRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload makePayload<>(
    const StreamPayloadMetadata&, std::unique_ptr<folly::IOBuf> data);
} // namespace detail
} // namespace rocket
} // namespace thrift
} // namespace apache
