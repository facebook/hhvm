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

#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>

#include <thrift/lib/cpp/TApplicationException.h>

#include <thrift/lib/cpp2/transport/rocket/compression/CompressionAlgorithmSelector.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace detail {

template <typename Metadata>
static void setCompressionCodec(
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
} // namespace detail

static std::unique_ptr<folly::IOBuf> compressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer,
    CompressionAlgorithm compressionAlgorithm) {
  auto [codecType, level] =
      CompressionAlgorithmSelector::toCodecTypeAndLevel(compressionAlgorithm);

  return folly::io::getCodec(codecType, level)->compress(buffer.get());
}

static std::unique_ptr<folly::IOBuf> uncompressBuffer(
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

CompressionAlgorithm CompressionManager::fromCodecConfig(
    const CodecConfig& codecConfig) {
  return CompressionAlgorithmSelector::fromCodecConfig(codecConfig);
}

std::pair<folly::io::CodecType, int> CompressionManager::toCodecTypeAndLevel(
    const CompressionAlgorithm& compressionAlgorithm) {
  return CompressionAlgorithmSelector::toCodecTypeAndLevel(
      compressionAlgorithm);
}

void CompressionManager::setCompressionCodec(
    CompressionConfig compressionConfig,
    RequestRpcMetadata& metadata,
    size_t payloadSize) {
  detail::setCompressionCodec(compressionConfig, metadata, payloadSize);
}

void CompressionManager::setCompressionCodec(
    CompressionConfig compressionConfig,
    ResponseRpcMetadata& metadata,
    size_t payloadSize) {
  detail::setCompressionCodec(compressionConfig, metadata, payloadSize);
}

void CompressionManager::setCompressionCodec(
    CompressionConfig compressionConfig,
    StreamPayloadMetadata& metadata,
    size_t payloadSize) {
  detail::setCompressionCodec(compressionConfig, metadata, payloadSize);
}

std::unique_ptr<folly::IOBuf> CompressionManager::compressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer,
    CompressionAlgorithm compressionAlgorithm) {
  return rocket::compressBuffer(std::move(buffer), compressionAlgorithm);
}

std::unique_ptr<folly::IOBuf> CompressionManager::uncompressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer,
    CompressionAlgorithm compressionAlgorithm) {
  return rocket::uncompressBuffer(std::move(buffer), compressionAlgorithm);
}

} // namespace rocket
} // namespace thrift
} // namespace apache
