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

#include <folly/Utility.h>
#include <folly/compression/Compression.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionAlgorithmSelector.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

CompressionAlgorithm fromZlibConfig(
    const ZlibCompressionCodecConfig& zlibConfig) {
  switch (
      zlibConfig.levelPreset().value_or(ZlibCompressionLevelPreset::DEFAULT)) {
    case ZlibCompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::ZLIB;
    case ZlibCompressionLevelPreset::LESS:
      return CompressionAlgorithm::ZLIB_LESS;
    case ZlibCompressionLevelPreset::MORE:
      return CompressionAlgorithm::ZLIB_MORE;
  };

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown {} enum value: {}",
          TEnumTraits<ZlibCompressionLevelPreset>::typeName(),
          folly::to_underlying(zlibConfig.levelPreset().value())));
}

CompressionAlgorithm fromZstdConfig(
    const ZstdCompressionCodecConfig& zstdConfig) {
  switch (
      zstdConfig.levelPreset().value_or(ZstdCompressionLevelPreset::DEFAULT)) {
    case ZstdCompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::ZSTD;
    case ZstdCompressionLevelPreset::LESS:
      return CompressionAlgorithm::ZSTD_LESS;
    case ZstdCompressionLevelPreset::MORE:
      return CompressionAlgorithm::ZSTD_MORE;
  };

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown {} enum value: {}",
          TEnumTraits<ZstdCompressionLevelPreset>::typeName(),
          folly::to_underlying(zstdConfig.levelPreset().value())));
}

CompressionAlgorithm fromLz4Config(const Lz4CompressionCodecConfig& lz4Config) {
  switch (
      lz4Config.levelPreset().value_or(Lz4CompressionLevelPreset::DEFAULT)) {
    case Lz4CompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::LZ4;
    case Lz4CompressionLevelPreset::LESS:
      return CompressionAlgorithm::LZ4_LESS;
    case Lz4CompressionLevelPreset::MORE:
      return CompressionAlgorithm::LZ4_MORE;
  };

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown {} enum value: {}",
          TEnumTraits<Lz4CompressionLevelPreset>::typeName(),
          folly::to_underlying(lz4Config.levelPreset().value())));
}

CompressionAlgorithm CompressionAlgorithmSelector::fromCodecConfig(
    const CodecConfig& codecConfig) {
  switch (codecConfig.getType()) {
    case CodecConfig::Type::zlibConfig:
      return fromZlibConfig(*codecConfig.zlibConfig_ref());
    case CodecConfig::Type::zstdConfig:
      return fromZstdConfig(*codecConfig.zstdConfig_ref());
    case CodecConfig::Type::lz4Config:
      return fromLz4Config(*codecConfig.lz4Config_ref());
    case CodecConfig::Type::customConfig:
      return CompressionAlgorithm::CUSTOM;
    case CodecConfig::Type::__EMPTY__:
      return CompressionAlgorithm::NONE;
  };

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown CodecConfig::Type enum value: {}",
          folly::to_underlying(codecConfig.getType())));
}

std::pair<folly::io::CodecType, int>
CompressionAlgorithmSelector::toCodecTypeAndLevel(
    const CompressionAlgorithm& compressionAlgorithm) {
  // clang-format off
  switch (compressionAlgorithm) {
    case CompressionAlgorithm::ZLIB:
      return {
          folly::io::CodecType::ZLIB,
          folly::io::COMPRESSION_LEVEL_DEFAULT};
    case CompressionAlgorithm::ZSTD:
      return {
          folly::io::CodecType::ZSTD,
          folly::io::COMPRESSION_LEVEL_DEFAULT};
    case CompressionAlgorithm::LZ4:
      return {
          folly::io::CodecType::LZ4_VARINT_SIZE,
          folly::io::COMPRESSION_LEVEL_DEFAULT};

    case CompressionAlgorithm::CUSTOM:
      // TODO: use custom compression implementation
      return {
          folly::io::CodecType::NO_COMPRESSION,
          folly::io::COMPRESSION_LEVEL_DEFAULT};

    case CompressionAlgorithm::ZLIB_LESS:
      return {
          folly::io::CodecType::ZLIB,
          folly::io::COMPRESSION_LEVEL_FASTEST};
    case CompressionAlgorithm::ZSTD_LESS:
      // ZSTD is special in that it also uses negative (faster) levels. Due to
      // implementation details, folly segmented these negative ZSTD levels into
      // a "ZSTD_FAST" codec type where positive levels are translated to
      // negative levels for the underlying base codec. This magic "7" (-7 for
      // the underlying base codec) was determined to to be sane by an internal
      // customer.
      return {
          folly::io::CodecType::ZSTD_FAST,
          7};
    case CompressionAlgorithm::LZ4_LESS:
      return {
          folly::io::CodecType::LZ4_VARINT_SIZE,
          folly::io::COMPRESSION_LEVEL_FASTEST};

    case CompressionAlgorithm::ZLIB_MORE:
      return {
          folly::io::CodecType::ZLIB,
          folly::io::COMPRESSION_LEVEL_BEST};
    case CompressionAlgorithm::ZSTD_MORE:
      return {
          folly::io::CodecType::ZSTD,
          folly::io::COMPRESSION_LEVEL_BEST};
    case CompressionAlgorithm::LZ4_MORE:
      return {
          folly::io::CodecType::LZ4_VARINT_SIZE,
          folly::io::COMPRESSION_LEVEL_BEST};

    case CompressionAlgorithm::NONE:
      return {
          folly::io::CodecType::NO_COMPRESSION,
          folly::io::COMPRESSION_LEVEL_DEFAULT};
  };
  // clang-format on

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown {} enum value: {}",
          TEnumTraits<CompressionAlgorithm>::typeName(),
          folly::to_underlying(compressionAlgorithm)));
}
} // namespace apache::thrift::rocket
