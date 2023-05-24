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

#include <folly/compression/Compression.h>
#include <thrift/lib/cpp/TApplicationException.h>

#include <thrift/lib/cpp2/transport/rocket/Compression.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace detail {
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
