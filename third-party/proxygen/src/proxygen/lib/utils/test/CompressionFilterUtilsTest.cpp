/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/utils/CompressionFilterUtils.h>

using namespace proxygen;

namespace {

class DetermineCompressionTypeTest : public ::testing::Test {
 protected:
  using CodecType = CompressionFilterUtils::CodecType;

  CodecType determineType(const std::string& acceptEncoding,
                          bool enableZstd = true,
                          bool enableGzip = true) {
    return CompressionFilterUtils::determineCompressionType(
        acceptEncoding, enableZstd, enableGzip);
  }
};

TEST_F(DetermineCompressionTypeTest, Zstd) {
  EXPECT_EQ(determineType("zstd"), CodecType::ZSTD);
}

TEST_F(DetermineCompressionTypeTest, ZstdWithLevel) {
  EXPECT_EQ(determineType("zstd-8"), CodecType::ZSTD);
}

TEST_F(DetermineCompressionTypeTest, Gzip) {
  EXPECT_EQ(determineType("gzip"), CodecType::ZLIB);
}

TEST_F(DetermineCompressionTypeTest, GzipWithQuality) {
  EXPECT_EQ(determineType("gzip; q=5.0"), CodecType::ZLIB);
}

TEST_F(DetermineCompressionTypeTest, Empty) {
  EXPECT_EQ(determineType(""), CodecType::NO_COMPRESSION);
}

TEST_F(DetermineCompressionTypeTest, UnsupportedEncoding) {
  EXPECT_EQ(determineType("br"), CodecType::NO_COMPRESSION);
}

TEST_F(DetermineCompressionTypeTest, ZstdDisabled) {
  EXPECT_EQ(determineType("zstd", false, true), CodecType::NO_COMPRESSION);
}

TEST_F(DetermineCompressionTypeTest, GzipDisabled) {
  EXPECT_EQ(determineType("gzip", true, false), CodecType::NO_COMPRESSION);
}

TEST_F(DetermineCompressionTypeTest, MultipleEncodingsZstdFirst) {
  EXPECT_EQ(determineType("zstd, gzip"), CodecType::ZSTD);
}

TEST_F(DetermineCompressionTypeTest, MultipleEncodingsGzipFirst) {
  EXPECT_EQ(determineType("gzip, zstd"), CodecType::ZLIB);
}

TEST_F(DetermineCompressionTypeTest, ZstdDisabledFallbackToGzip) {
  EXPECT_EQ(determineType("zstd, gzip", false, true), CodecType::ZLIB);
}

class DetermineCompressionInfoTest : public ::testing::Test {
 protected:
  using CodecType = CompressionFilterUtils::CodecType;

  CompressionFilterUtils::CompressionInfo determineInfo(
      const std::string& acceptEncoding,
      bool enableZstd = true,
      bool enableGzip = true) {
    return CompressionFilterUtils::determineCompressionInfo(
        acceptEncoding, enableZstd, enableGzip);
  }
};

TEST_F(DetermineCompressionInfoTest, ZstdWithLevel) {
  auto info = determineInfo("zstd-8");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  ASSERT_TRUE(info.level.has_value());
  EXPECT_EQ(info.level.value(), 8);
}

TEST_F(DetermineCompressionInfoTest, ZstdWithoutLevel) {
  auto info = determineInfo("zstd");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  EXPECT_FALSE(info.level.has_value());
}

TEST_F(DetermineCompressionInfoTest, ZstdWithLevelAndQvalue) {
  auto info = determineInfo("zstd-5; q=1.0");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  ASSERT_TRUE(info.level.has_value());
  EXPECT_EQ(info.level.value(), 5);
}

TEST_F(DetermineCompressionInfoTest, ZstdWithInvalidLevel) {
  auto info = determineInfo("zstd-abc");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  EXPECT_FALSE(info.level.has_value());
}

TEST_F(DetermineCompressionInfoTest, ZstdWithOutOfRangeLevel) {
  auto info = determineInfo("zstd-30");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  EXPECT_FALSE(info.level.has_value());
}

TEST_F(DetermineCompressionInfoTest, ZstdWithZeroLevel) {
  auto info = determineInfo("zstd-0");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  EXPECT_FALSE(info.level.has_value());
}

TEST_F(DetermineCompressionInfoTest, ZstdWithMaxValidLevel) {
  auto info = determineInfo("zstd-22");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  ASSERT_TRUE(info.level.has_value());
  EXPECT_EQ(info.level.value(), 22);
}

TEST_F(DetermineCompressionInfoTest, ZstdWithMinValidLevel) {
  auto info = determineInfo("zstd--5");
  EXPECT_EQ(info.codecType, CodecType::ZSTD);
  ASSERT_TRUE(info.level.has_value());
  EXPECT_EQ(info.level.value(), -5);
}

TEST_F(DetermineCompressionInfoTest, GzipNoLevel) {
  auto info = determineInfo("gzip");
  EXPECT_EQ(info.codecType, CodecType::ZLIB);
  EXPECT_FALSE(info.level.has_value());
}

} // namespace
