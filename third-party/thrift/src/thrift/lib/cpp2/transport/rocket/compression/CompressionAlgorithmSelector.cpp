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

template <typename TEnum>
[[noreturn]] void throwUnknownTEnumTApplicationException(const TEnum& tEnum) {
  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown {} enum value: {}",
          TEnumTraits<std::decay_t<decltype(tEnum)>>::typeName(),
          folly::to_underlying(tEnum)));
}

CompressionAlgorithm fromZlibConfig(
    const ZlibCompressionCodecConfig& zlibConfig) {
  const auto& levelPreset =
      zlibConfig.levelPreset().value_or(ZlibCompressionLevelPreset::DEFAULT);
  switch (levelPreset) {
    case ZlibCompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::ZLIB;
    case ZlibCompressionLevelPreset::LESS:
      return CompressionAlgorithm::ZLIB_LESS;
    case ZlibCompressionLevelPreset::MORE:
      return CompressionAlgorithm::ZLIB_MORE;
  };

  throwUnknownTEnumTApplicationException(levelPreset);
}

CompressionAlgorithm fromZstdConfig(
    const ZstdCompressionCodecConfig& zstdConfig) {
  const auto& levelPreset =
      zstdConfig.levelPreset().value_or(ZstdCompressionLevelPreset::DEFAULT);
  switch (levelPreset) {
    case ZstdCompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::ZSTD;
    case ZstdCompressionLevelPreset::LESS:
      return CompressionAlgorithm::ZSTD_LESS;
    case ZstdCompressionLevelPreset::MORE:
      return CompressionAlgorithm::ZSTD_MORE;
  };

  throwUnknownTEnumTApplicationException(levelPreset);
}

CompressionAlgorithm fromLz4Config(const Lz4CompressionCodecConfig& lz4Config) {
  const auto& levelPreset =
      lz4Config.levelPreset().value_or(ZstdCompressionLevelPreset::DEFAULT);
  switch (levelPreset) {
    case Lz4CompressionLevelPreset::DEFAULT:
      return CompressionAlgorithm::LZ4;
    case Lz4CompressionLevelPreset::LESS:
      return CompressionAlgorithm::LZ4_LESS;
    case Lz4CompressionLevelPreset::MORE:
      return CompressionAlgorithm::LZ4_MORE;
  };

  throwUnknownTEnumTApplicationException(levelPreset);
}

/* static */ CompressionAlgorithm CompressionAlgorithmSelector::fromCodecConfig(
    const CodecConfig& codecConfig) {
  const auto& codecConfigType = codecConfig.getType();
  switch (codecConfigType) {
    case CodecConfig::Type::zlibConfig:
      return fromZlibConfig(*codecConfig.zlibConfig());
    case CodecConfig::Type::zstdConfig:
      return fromZstdConfig(*codecConfig.zstdConfig());
    case CodecConfig::Type::lz4Config:
      return fromLz4Config(*codecConfig.lz4Config());
    case CodecConfig::Type::customConfig:
      return CompressionAlgorithm::CUSTOM;
    case CodecConfig::Type::__EMPTY__:
      return CompressionAlgorithm::NONE;
  };

  throw TApplicationException(
      TApplicationException::PROTOCOL_ERROR,
      fmt::format(
          "Unknown CodecConfig::Type enum value: {}",
          folly::to_underlying(codecConfigType)));
}

/* static */ CodecConfig CompressionAlgorithmSelector::toCodecConfig(
    const CompressionAlgorithm& compressionAlgorithm) {
  CodecConfig codecConfig;
  switch (compressionAlgorithm) {
    case CompressionAlgorithm::ZLIB:
      codecConfig.set_zlibConfig();
      return codecConfig;
    case CompressionAlgorithm::ZSTD:
      codecConfig.set_zstdConfig();
      return codecConfig;
    case CompressionAlgorithm::LZ4:
      codecConfig.set_lz4Config();
      return codecConfig;
    case CompressionAlgorithm::CUSTOM:
      codecConfig.set_customConfig();
      return codecConfig;
    case CompressionAlgorithm::ZLIB_LESS:
      codecConfig.set_zlibConfig();
      codecConfig.zlibConfig()->levelPreset() =
          ZlibCompressionLevelPreset::LESS;
      return codecConfig;
    case CompressionAlgorithm::ZSTD_LESS:
      codecConfig.set_zstdConfig();
      codecConfig.zstdConfig()->levelPreset() =
          ZstdCompressionLevelPreset::LESS;
      return codecConfig;
    case CompressionAlgorithm::LZ4_LESS:
      codecConfig.set_lz4Config();
      codecConfig.lz4Config()->levelPreset() = Lz4CompressionLevelPreset::LESS;
      return codecConfig;
    case CompressionAlgorithm::ZLIB_MORE:
      codecConfig.set_zlibConfig();
      codecConfig.zlibConfig()->levelPreset() =
          ZlibCompressionLevelPreset::MORE;
      return codecConfig;
    case CompressionAlgorithm::ZSTD_MORE:
      codecConfig.set_zstdConfig();
      codecConfig.zstdConfig()->levelPreset() =
          ZstdCompressionLevelPreset::MORE;
      return codecConfig;
    case CompressionAlgorithm::LZ4_MORE:
      codecConfig.set_lz4Config();
      codecConfig.lz4Config()->levelPreset() = Lz4CompressionLevelPreset::MORE;
      return codecConfig;
    case CompressionAlgorithm::NONE:
      codecConfig.set_customConfig();
      return codecConfig;
  };

  throwUnknownTEnumTApplicationException(compressionAlgorithm);
}

/* static */ std::pair<folly::compression::CodecType, int>
CompressionAlgorithmSelector::toCodecTypeAndLevel(
    const CompressionAlgorithm& compressionAlgorithm) {
  // clang-format off
  switch (compressionAlgorithm) {
    case CompressionAlgorithm::ZLIB:
      return {
          folly::compression::CodecType::ZLIB,
          folly::compression::COMPRESSION_LEVEL_DEFAULT};
    case CompressionAlgorithm::ZSTD:
      return {
          folly::compression::CodecType::ZSTD,
          folly::compression::COMPRESSION_LEVEL_DEFAULT};
    case CompressionAlgorithm::LZ4:
      return {
          folly::compression::CodecType::LZ4_VARINT_SIZE,
          folly::compression::COMPRESSION_LEVEL_DEFAULT};

    case CompressionAlgorithm::CUSTOM:
      // TODO: use custom compression implementation
      return {
          folly::compression::CodecType::NO_COMPRESSION,
          folly::compression::COMPRESSION_LEVEL_DEFAULT};

    case CompressionAlgorithm::ZLIB_LESS:
      return {
          folly::compression::CodecType::ZLIB,
          folly::compression::COMPRESSION_LEVEL_FASTEST};
    case CompressionAlgorithm::ZSTD_LESS:
      // ZSTD is special in that it also uses negative (faster) levels. Due to
      // implementation details, folly segmented these negative ZSTD levels into
      // a "ZSTD_FAST" codec type where positive levels are translated to
      // negative levels for the underlying base codec. This magic "7" (-7 for
      // the underlying base codec) was determined to to be sane by an internal
      // customer.
      return {
          folly::compression::CodecType::ZSTD_FAST,
          7};
    case CompressionAlgorithm::LZ4_LESS:
      return {
          folly::compression::CodecType::LZ4_VARINT_SIZE,
          folly::compression::COMPRESSION_LEVEL_FASTEST};

    case CompressionAlgorithm::ZLIB_MORE:
      return {
          folly::compression::CodecType::ZLIB,
          folly::compression::COMPRESSION_LEVEL_BEST};
    case CompressionAlgorithm::ZSTD_MORE:
      return {
          folly::compression::CodecType::ZSTD,
          folly::compression::COMPRESSION_LEVEL_BEST};
    case CompressionAlgorithm::LZ4_MORE:
      return {
          folly::compression::CodecType::LZ4_VARINT_SIZE,
          folly::compression::COMPRESSION_LEVEL_BEST};

    case CompressionAlgorithm::NONE:
      return {
          folly::compression::CodecType::NO_COMPRESSION,
          folly::compression::COMPRESSION_LEVEL_DEFAULT};
  };
  // clang-format on

  throwUnknownTEnumTApplicationException(compressionAlgorithm);
}

/* static */ CompressionAlgorithm CompressionAlgorithmSelector::fromTTransform(
    const TTransform& tTransform) {
  switch (tTransform) {
    case TTransform::NONE:
      return CompressionAlgorithm::NONE;
    case TTransform::ZLIB:
      return CompressionAlgorithm::ZLIB;
    case TTransform::ZSTD:
      return CompressionAlgorithm::ZSTD;
    case TTransform::LZ4:
      return CompressionAlgorithm::LZ4;
    case TTransform::CUSTOM:
      return CompressionAlgorithm::CUSTOM;
    case TTransform::ZLIB_LESS:
      return CompressionAlgorithm::ZLIB_LESS;
    case TTransform::ZSTD_LESS:
      return CompressionAlgorithm::ZSTD_LESS;
    case TTransform::LZ4_LESS:
      return CompressionAlgorithm::LZ4_LESS;
    case TTransform::ZLIB_MORE:
      return CompressionAlgorithm::ZLIB_MORE;
    case TTransform::ZSTD_MORE:
      return CompressionAlgorithm::ZSTD_MORE;
    case TTransform::LZ4_MORE:
      return CompressionAlgorithm::LZ4_MORE;
  };

  throwUnknownTEnumTApplicationException(tTransform);
}

/* static */ TTransform CompressionAlgorithmSelector::toTTransform(
    const CompressionAlgorithm& compressionAlgorithm) {
  switch (compressionAlgorithm) {
    case CompressionAlgorithm::NONE:
      return TTransform::NONE;
    case CompressionAlgorithm::ZLIB:
      return TTransform::ZLIB;
    case CompressionAlgorithm::ZSTD:
      return TTransform::ZSTD;
    case CompressionAlgorithm::LZ4:
      return TTransform::LZ4;
    case CompressionAlgorithm::CUSTOM:
      return TTransform::CUSTOM;
    case CompressionAlgorithm::ZLIB_LESS:
      return TTransform::ZLIB_LESS;
    case CompressionAlgorithm::ZSTD_LESS:
      return TTransform::ZSTD_LESS;
    case CompressionAlgorithm::LZ4_LESS:
      return TTransform::LZ4_LESS;
    case CompressionAlgorithm::ZLIB_MORE:
      return TTransform::ZLIB_MORE;
    case CompressionAlgorithm::ZSTD_MORE:
      return TTransform::ZSTD_MORE;
    case CompressionAlgorithm::LZ4_MORE:
      return TTransform::LZ4_MORE;
  };

  throwUnknownTEnumTApplicationException(compressionAlgorithm);
}
} // namespace apache::thrift::rocket
