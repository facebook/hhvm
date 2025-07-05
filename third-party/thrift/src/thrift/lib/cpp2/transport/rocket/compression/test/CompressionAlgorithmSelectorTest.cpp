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

#include <gtest/gtest.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionAlgorithmSelector.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::rocket;

// Test helpers.

class CodecConfigSelector {
 public:
  static CodecConfig fromConfig(const ZlibCompressionCodecConfig& zlibConfig) {
    CodecConfig codecConfig;
    codecConfig.zlibConfig() = zlibConfig;
    return codecConfig;
  }
  static CodecConfig fromConfig(const ZstdCompressionCodecConfig& zstdConfig) {
    CodecConfig codecConfig;
    codecConfig.zstdConfig() = zstdConfig;
    return codecConfig;
  }
  static CodecConfig fromConfig(const Lz4CompressionCodecConfig& lz4Config) {
    CodecConfig codecConfig;
    codecConfig.lz4Config() = lz4Config;
    return codecConfig;
  }

  template <typename Config>
  static CodecConfig fromLevelPreset(
      const typename std::remove_reference<
          decltype(Config().levelPreset().value())>::type& levelPreset) {
    Config config;
    config.levelPreset() = levelPreset;
    return fromConfig(config);
  }
};

template <typename Config>
void testFromCodecConfigWithoutLevelPresetSucceeds(
    const CompressionAlgorithm expectCompressionAlgorithm) {
  auto codecConfig = CodecConfigSelector::fromConfig(Config());

  auto compressionAlgorithm =
      CompressionAlgorithmSelector::fromCodecConfig(codecConfig);

  EXPECT_EQ(compressionAlgorithm, expectCompressionAlgorithm);
}

TEST(CompressionTest, fromCodecConfigWithoutLevelPresetSucceeds) {
  testFromCodecConfigWithoutLevelPresetSucceeds<ZlibCompressionCodecConfig>(
      CompressionAlgorithm::ZLIB /* expectCompressionAlgorithm */);
  testFromCodecConfigWithoutLevelPresetSucceeds<ZstdCompressionCodecConfig>(
      CompressionAlgorithm::ZSTD /* expectCompressionAlgorithm */);
  testFromCodecConfigWithoutLevelPresetSucceeds<Lz4CompressionCodecConfig>(
      CompressionAlgorithm::LZ4 /* expectCompressionAlgorithm */);
}

template <typename Config>
void testFromCodecConfigWithLevelPresetSucceeds(
    const typename std::remove_reference<
        decltype(Config().levelPreset().value())>::type& levelPreset,
    const CompressionAlgorithm expectCompressionAlgorithm) {
  auto codecConfig = CodecConfigSelector::fromLevelPreset<Config>(levelPreset);

  auto compressionAlgorithm =
      CompressionAlgorithmSelector::fromCodecConfig(codecConfig);

  EXPECT_EQ(compressionAlgorithm, expectCompressionAlgorithm);
}

TEST(
    CompressionAlgorithmSelectorTest,
    zlibFromCodecConfigWithLevelPresetSucceeds) {
  for (auto [levelPreset, expectCompressionAlgorithm] : std::initializer_list<
           std::pair<ZlibCompressionLevelPreset, CompressionAlgorithm>>{
           {ZlibCompressionLevelPreset::DEFAULT, CompressionAlgorithm::ZLIB},
           {ZlibCompressionLevelPreset::MORE, CompressionAlgorithm::ZLIB_MORE},
           {ZlibCompressionLevelPreset::LESS, CompressionAlgorithm::ZLIB_LESS},
       }) {
    testFromCodecConfigWithLevelPresetSucceeds<ZlibCompressionCodecConfig>(
        levelPreset, expectCompressionAlgorithm);
  }
}

TEST(
    CompressionAlgorithmSelectorTest,
    zstdFromCodecConfigWithLevelPresetSucceeds) {
  for (auto [levelPreset, expectCompressionAlgorithm] : std::initializer_list<
           std::pair<ZstdCompressionLevelPreset, CompressionAlgorithm>>{
           {ZstdCompressionLevelPreset::DEFAULT, CompressionAlgorithm::ZSTD},
           {ZstdCompressionLevelPreset::MORE, CompressionAlgorithm::ZSTD_MORE},
           {ZstdCompressionLevelPreset::LESS, CompressionAlgorithm::ZSTD_LESS},
       }) {
    testFromCodecConfigWithLevelPresetSucceeds<ZstdCompressionCodecConfig>(
        levelPreset, expectCompressionAlgorithm);
  }
}

TEST(
    CompressionAlgorithmSelectorTest,
    lz4FromCodecConfigWithLevelPresetSucceeds) {
  for (auto [levelPreset, expectCompressionAlgorithm] : std::initializer_list<
           std::pair<Lz4CompressionLevelPreset, CompressionAlgorithm>>{
           {Lz4CompressionLevelPreset::DEFAULT, CompressionAlgorithm::LZ4},
           {Lz4CompressionLevelPreset::MORE, CompressionAlgorithm::LZ4_MORE},
           {Lz4CompressionLevelPreset::LESS, CompressionAlgorithm::LZ4_LESS},
       }) {
    testFromCodecConfigWithLevelPresetSucceeds<Lz4CompressionCodecConfig>(
        levelPreset, expectCompressionAlgorithm);
  }
}

TEST(
    CompressionAlgorithmSelectorTest,
    toCodecTypeAndLevelWithInvalidCompressionAlgorithmThrows) {
  auto compressionAlgorithm = static_cast<CompressionAlgorithm>(-1);

  EXPECT_THROW(
      CompressionAlgorithmSelector::toCodecTypeAndLevel(compressionAlgorithm),
      TApplicationException);
}

TEST(
    CompressionAlgorithmSelectorTest,
    fromTTransformMapsTTransformOntoCompressionAlgorithm) {
  for (const auto& expectTTransform : TEnumTraits<TTransform>::values) {
    auto tTransform = CompressionAlgorithmSelector::toTTransform(
        CompressionAlgorithmSelector::fromTTransform(expectTTransform));

    EXPECT_EQ(tTransform, expectTTransform);
  }
}

TEST(
    CompressionAlgorithmSelectorTest,
    toTTransformMapsCompressionAlgorithmOntoTTransform) {
  for (const auto& expectCompressionAlgorithm :
       TEnumTraits<CompressionAlgorithm>::values) {
    auto compressionAlgorithm = CompressionAlgorithmSelector::fromTTransform(
        CompressionAlgorithmSelector::toTTransform(expectCompressionAlgorithm));

    EXPECT_EQ(compressionAlgorithm, expectCompressionAlgorithm);
  }
}
