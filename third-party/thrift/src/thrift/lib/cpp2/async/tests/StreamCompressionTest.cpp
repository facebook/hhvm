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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/async/ServerStreamDetail.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::detail;

namespace {

// A compressible payload large enough to exceed typical size limits.
const std::string kLargePayloadStr =
    "Compression test payload that needs to be long enough to compress. "
    "Compression test payload that needs to be long enough to compress. "
    "Compression test payload that needs to be long enough to compress. "
    "Compression test payload that needs to be long enough to compress. "
    "Compression test payload that needs to be long enough to compress. "
    "Compression test payload that needs to be long enough to compress. ";

const std::string kSmallPayloadStr = "tiny";

CompressionConfig makeZstdConfig(int64_t sizeLimit = 0) {
  CompressionConfig config;
  CodecConfig codecConfig;
  codecConfig.zstdConfig() = ZstdCompressionCodecConfig();
  config.codecConfig() = std::move(codecConfig);
  if (sizeLimit > 0) {
    config.compressionSizeLimit() = sizeLimit;
  }
  return config;
}

} // namespace

// -- makeCompressionContext tests --

TEST(StreamCompressionTest, makeCompressionContext_ZstdSucceeds) {
  auto config = makeZstdConfig(/*sizeLimit=*/1024);
  auto ctx = makeCompressionContext(config);
  ASSERT_TRUE(ctx.has_value());
  EXPECT_EQ(ctx->algorithm, CompressionAlgorithm::ZSTD);
  EXPECT_NE(ctx->codec, nullptr);
  EXPECT_EQ(ctx->sizeLimit, 1024);
}

TEST(
    StreamCompressionTest, makeCompressionContext_NoCodecConfigReturnsNullopt) {
  CompressionConfig config;
  // No codecConfig set.
  auto ctx = makeCompressionContext(config);
  EXPECT_FALSE(ctx.has_value());
}

TEST(
    StreamCompressionTest,
    makeCompressionContext_CustomAlgorithmReturnsNullopt) {
  CompressionConfig config;
  CodecConfig codecConfig;
  codecConfig.customConfig() = CustomCompressionCodecConfig();
  config.codecConfig() = std::move(codecConfig);
  auto ctx = makeCompressionContext(config);
  EXPECT_FALSE(ctx.has_value());
}

// -- compressStreamItem tests --

TEST(StreamCompressionTest, CompressesPayloadAboveSizeLimit) {
  auto config = makeZstdConfig(/*sizeLimit=*/10);
  auto ctx = makeCompressionContext(config);
  ASSERT_TRUE(ctx.has_value());

  auto original = folly::IOBuf::copyBuffer(kLargePayloadStr);
  auto originalClone = original->clone();
  auto payloadSize = original->computeChainDataLength();

  StreamPayload sp(std::move(original), StreamPayloadMetadata());
  compressStreamItem(sp, *ctx, payloadSize);

  // Payload should be compressed (different from original).
  EXPECT_FALSE(folly::IOBufEqualTo()(sp.payload, originalClone));
  // Metadata should indicate compression.
  ASSERT_TRUE(sp.metadata.compression().has_value());
  EXPECT_EQ(*sp.metadata.compression(), CompressionAlgorithm::ZSTD);
}

TEST(StreamCompressionTest, DoesNotCompressPayloadBelowSizeLimit) {
  auto config = makeZstdConfig(/*sizeLimit=*/10000);
  auto ctx = makeCompressionContext(config);
  ASSERT_TRUE(ctx.has_value());

  auto original = folly::IOBuf::copyBuffer(kSmallPayloadStr);
  auto originalClone = original->clone();
  auto payloadSize = original->computeChainDataLength();

  StreamPayload sp(std::move(original), StreamPayloadMetadata());
  compressStreamItem(sp, *ctx, payloadSize);

  // Payload should be unchanged.
  EXPECT_TRUE(folly::IOBufEqualTo()(sp.payload, originalClone));
  // No compression metadata should be set.
  EXPECT_FALSE(sp.metadata.compression().has_value());
}

TEST(StreamCompressionTest, DoesNotCompressPayloadAtExactSizeLimit) {
  auto original = folly::IOBuf::copyBuffer(kLargePayloadStr);
  auto payloadSize = original->computeChainDataLength();
  auto originalClone = original->clone();

  // Size limit == payload size: should NOT compress (<=).
  auto config = makeZstdConfig(static_cast<int64_t>(payloadSize));
  auto ctx = makeCompressionContext(config);
  ASSERT_TRUE(ctx.has_value());

  StreamPayload sp(std::move(original), StreamPayloadMetadata());
  compressStreamItem(sp, *ctx, payloadSize);

  EXPECT_TRUE(folly::IOBufEqualTo()(sp.payload, originalClone));
  EXPECT_FALSE(sp.metadata.compression().has_value());
}

TEST(StreamCompressionTest, CompressedPayloadCanBeDecompressed) {
  auto config = makeZstdConfig(/*sizeLimit=*/0);
  auto ctx = makeCompressionContext(config);
  ASSERT_TRUE(ctx.has_value());

  auto original = folly::IOBuf::copyBuffer(kLargePayloadStr);
  auto originalClone = original->clone();
  auto payloadSize = original->computeChainDataLength();

  StreamPayload sp(std::move(original), StreamPayloadMetadata());
  compressStreamItem(sp, *ctx, payloadSize);

  // Verify round-trip: decompress and compare.
  auto decompressed = ctx->codec->uncompress(sp.payload.get());
  EXPECT_TRUE(folly::IOBufEqualTo()(decompressed, originalClone));
}
