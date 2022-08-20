/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CompressionTestUtil.h"

#include <limits>
#include <memory>

#include <folly/Random.h>

#include "mcrouter/lib/Compression.h"
#include "mcrouter/lib/CompressionCodecManager.h"

namespace facebook {
namespace memcache {
namespace test {

std::string createBinaryData(size_t size) {
  std::string dic;
  dic.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    dic.push_back(static_cast<char>(
        folly::Random::rand32(0, std::numeric_limits<char>::max() + 1)));
  }
  return dic;
}

std::unordered_map<uint32_t, CodecConfigPtr> testCodecConfigs() {
  std::unordered_map<uint32_t, CodecConfigPtr> codecConfigs;
  codecConfigs.emplace(
      1,
      std::make_unique<CodecConfig>(
          1, /* id */
          CompressionCodecType::LZ4,
          createBinaryData(1024),
          FilteringOptions(
              1025, /* minCompressionThreshold */
              std::numeric_limits<uint32_t>::max(), /* maxCompressionThreshold*/
              0, /* typeId */
              true /* isEnabled */)));
  codecConfigs.emplace(
      2,
      std::make_unique<CodecConfig>(
          2, /* id */
          CompressionCodecType::ZSTD,
          createBinaryData(1024),
          FilteringOptions(
              64, /* minCompressionThreshold */
              1024, /* maxCompressionThreshold*/
              0, /* typeId */
              true /* isEnabled */
              ),
          5 /* compressionLevel*/));
  codecConfigs.emplace(
      3,
      std::make_unique<CodecConfig>(
          3, /* id */
          CompressionCodecType::ZSTD,
          createBinaryData(1024),
          FilteringOptions(
              1025, /* minCompressionThreshold */
              std::numeric_limits<uint32_t>::max(), /* maxCompressionThreshold*/
              1, /* typeId */
              false /* isEnabled */)));
  codecConfigs.emplace(
      4,
      std::make_unique<CodecConfig>(
          4, /* id */
          CompressionCodecType::LZ4,
          createBinaryData(1024),
          FilteringOptions(
              1025, /* minCompressionThreshold */
              std::numeric_limits<uint32_t>::max(), /* maxCompressionThreshold*/
              0, /* typeId */
              true /* isEnabled */)));
  codecConfigs.emplace(
      5,
      std::make_unique<CodecConfig>(
          5, /* id */
          CompressionCodecType::LZ4Immutable,
          createBinaryData(1024),
          FilteringOptions(
              64, /* minCompressionThreshold */
              1024, /* maxCompressionThreshold*/
              0, /* typeId */
              false /* isEnabled */)));
  codecConfigs.emplace(
      6,
      std::make_unique<CodecConfig>(
          6, /* id */
          CompressionCodecType::LZ4Immutable,
          createBinaryData(1024),
          FilteringOptions(
              64, /* minCompressionThreshold */
              1024, /* maxCompressionThreshold*/
              2 /* typeId */,
              true /* isEnabled */)));
  codecConfigs.emplace(
      7,
      std::make_unique<CodecConfig>(
          7, /* id */
          CompressionCodecType::LZ4Immutable,
          createBinaryData(1024),
          FilteringOptions(
              64, /* minCompressionThreshold */
              1024, /* maxCompressionThreshold*/
              2, /* typeId */
              false /* isEnabled */)));
  codecConfigs.emplace(
      8,
      std::make_unique<CodecConfig>(
          8, /* id */
          CompressionCodecType::LZ4,
          createBinaryData(1024),
          FilteringOptions(
              1025, /* minCompressionThreshold */
              std::numeric_limits<uint32_t>::max(), /* maxCompressionThreshold*/
              1, /* typeId */
              true /* isEnabled */)));
  codecConfigs.emplace(
      9,
      std::make_unique<CodecConfig>(
          9, /* id */
          CompressionCodecType::ZSTD,
          createBinaryData(1024),
          FilteringOptions(
              1025, /* minCompressionThreshold */
              std::numeric_limits<uint32_t>::max(), /* maxCompressionThreshold*/
              2, /* typeId */
              true /* isEnabled */)));
  codecConfigs.emplace(
      10,
      std::make_unique<CodecConfig>(
          10, /* id */
          CompressionCodecType::ZSTD,
          createBinaryData(1024),
          FilteringOptions(
              64, /* minCompressionThreshold */
              1024, /* maxCompressionThreshold*/
              2, /* typeId */
              true /* isEnabled */)));

  return codecConfigs;
}

} // namespace test
} // namespace memcache
} // namespace facebook
