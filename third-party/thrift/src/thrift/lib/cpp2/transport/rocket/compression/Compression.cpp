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

#include <thrift/lib/cpp2/transport/rocket/compression/Compression.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionAlgorithmSelector.h>

namespace apache::thrift::rocket {
namespace detail {

template <typename Metadata>
void setCompressionCodec(
    CompressionConfig compressionConfig,
    Metadata& metadata,
    size_t payloadSize) {
  if (const auto& codecConfig = compressionConfig.codecConfig()) {
    if (payloadSize >
        static_cast<size_t>(
            compressionConfig.compressionSizeLimit().value_or(0))) {
      metadata.compression() =
          CompressionAlgorithmSelector::fromCodecConfig(*codecConfig);
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
  data = compressBuffer(std::move(data), compression);
}
} // namespace detail

std::unique_ptr<folly::IOBuf> compressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer,
    CompressionAlgorithm compressionAlgorithm) {
  auto [codecType, level] =
      CompressionAlgorithmSelector::toCodecTypeAndLevel(compressionAlgorithm);

  return folly::io::getCodec(codecType, level)->compress(buffer.get());
}

std::unique_ptr<folly::IOBuf> uncompressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer,
    CompressionAlgorithm compressionAlgorithm) {
  auto [codecType, level] =
      CompressionAlgorithmSelector::toCodecTypeAndLevel(compressionAlgorithm);
  try {
    return folly::io::getCodec(codecType, level)->uncompress(buffer.get());
  } catch (const std::exception& e) {
    throw TApplicationException(
        TApplicationException::INVALID_TRANSFORM,
        fmt::format("decompression failure: {}", e.what()));
  }
}
} // namespace apache::thrift::rocket
