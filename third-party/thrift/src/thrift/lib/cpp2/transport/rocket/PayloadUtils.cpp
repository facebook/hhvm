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
  DCHECK(
      !metadata.otherMetadata() ||
      !metadata.otherMetadata()->contains("__UNSAFE_FDS_FOR_REQUEST__"))
      << "Not implemented: attaching FDs via "
      << "otherMetadata[__UNSAFE_FDS_FOR_REQUEST__] on this code path";

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

template <typename Metadata>
void setCompressionCodec(
    CompressionConfig compressionConfig,
    Metadata& metadata,
    size_t payloadSize) {
  if (auto codecRef = compressionConfig.codecConfig_ref()) {
    if (payloadSize >
        static_cast<size_t>(
            compressionConfig.compressionSizeLimit_ref().value_or(0))) {
      switch (codecRef->getType()) {
        case CodecConfig::Type::zlibConfig:
          metadata.compression_ref() = CompressionAlgorithm::ZLIB;
          break;
        case CodecConfig::Type::zstdConfig:
          metadata.compression_ref() = CompressionAlgorithm::ZSTD;
          break;
        default:
          break;
      }
    }
  }
}

template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    RequestRpcMetadata& metadata,
    size_t payloadSize);
template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    ResponseRpcMetadata& metadata,
    size_t payloadSize);
template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    StreamPayloadMetadata& metadata,
    size_t payloadSize);

void compressPayload(
    std::unique_ptr<folly::IOBuf>& data, CompressionAlgorithm compression) {
  folly::io::CodecType codec;
  switch (compression) {
    case CompressionAlgorithm::ZSTD:
      codec = folly::io::CodecType::ZSTD;
      break;
    case CompressionAlgorithm::ZLIB:
      codec = folly::io::CodecType::ZLIB;
      break;
    case CompressionAlgorithm::NONE:
      codec = folly::io::CodecType::NO_COMPRESSION;
      break;
  }
  data = folly::io::getCodec(codec)->compress(data.get());
}

folly::Expected<std::unique_ptr<folly::IOBuf>, std::string> uncompressPayload(
    CompressionAlgorithm compression, std::unique_ptr<folly::IOBuf> data) {
  folly::io::CodecType codec;
  switch (compression) {
    case CompressionAlgorithm::ZSTD:
      codec = folly::io::CodecType::ZSTD;
      break;
    case CompressionAlgorithm::ZLIB:
      codec = folly::io::CodecType::ZLIB;
      break;
    case CompressionAlgorithm::NONE:
      codec = folly::io::CodecType::NO_COMPRESSION;
      break;
  }

  try {
    return folly::io::getCodec(codec)->uncompress(data.get());
  } catch (const std::exception& e) {
    return folly::makeUnexpected(std::string(e.what()));
  }
}
} // namespace detail
std::unique_ptr<folly::IOBuf> uncompressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer, CompressionAlgorithm compression) {
  auto result = detail::uncompressPayload(compression, std::move(buffer));
  if (!result) {
    folly::throw_exception<TApplicationException>(
        TApplicationException::INVALID_TRANSFORM,
        fmt::format("decompression failure: {}", std::move(result.error())));
  }
  return std::move(result.value());
}
} // namespace rocket
} // namespace thrift
} // namespace apache
