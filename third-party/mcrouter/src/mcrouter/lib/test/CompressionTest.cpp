/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/Format.h>
#include <folly/Random.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/Compression.h"

namespace facebook {
namespace memcache {
namespace test {

namespace {

std::unique_ptr<folly::IOBuf> getAsciiDictionary() {
  static const char dic[] =
      "VALUE key 0 10\r\n"
      "0123456789\r\n"
      "END\r\n"
      "CLIENT_ERROR malformed request\r\n"
      "VALUE anotherkey 0 12\r\n"
      "anothervalue\r\n"
      "END\r\n"
      "END\r\n"
      "VALUE test.aap.j 0 50\r\n"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n"
      "END\r\n"
      "END\r\n";
  return folly::IOBuf::wrapBuffer(
      reinterpret_cast<const uint8_t*>(dic), sizeof(dic));
}

std::unique_ptr<folly::IOBuf> getAsciiReply() {
  static const char reply[] =
      "VALUE test.aap.f 0 12\r\n"
      "thisisavalue\r\n"
      "END\r\n";
  return folly::IOBuf::wrapBuffer(
      reinterpret_cast<const uint8_t*>(reply), sizeof(reply));
}

std::unique_ptr<folly::IOBuf> getRandomLargeReply() {
  static const char alphabet[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  uint32_t size = folly::Random::rand32(1024, 32 * 1024);
  std::string reply;
  reply.reserve(size);
  reply.append(folly::sformat("VALUE test.aap.f 0 {}\r\n", size));
  for (size_t i = 0; i < size; ++i) {
    reply.push_back(alphabet[folly::Random::rand32(0, sizeof(alphabet))]);
  }
  reply.append("\r\nEND\r\n");

  return folly::IOBuf::copyBuffer(std::move(reply));
}

std::unique_ptr<folly::IOBuf> buildChain(
    const folly::IOBuf& data,
    size_t chainLength) {
  assert(chainLength > 1);
  assert(!data.isChained());
  assert(data.length() >= chainLength);

  size_t partSize = data.length() / chainLength;
  size_t lastPartSize = data.length() - (partSize * (chainLength - 1));
  size_t bufSize = std::max(partSize, lastPartSize);
  auto head = folly::IOBuf::create(bufSize);

  auto cur = head.get();
  size_t i;
  for (i = 0; i < (chainLength - 1); ++i) {
    std::memcpy(cur->writableTail(), data.data() + (partSize * i), partSize);
    cur->append(partSize);
    cur->appendChain(folly::IOBuf::create(bufSize));
    cur = cur->next();
  }
  std::memcpy(cur->writableTail(), data.data() + (partSize * i), lastPartSize);
  cur->append(lastPartSize);

  assert(head->isChained());
  assert(chainLength == head->countChainElements());
  assert(data.length() == head->computeChainDataLength());

  return head;
}

void testCompressAndUncompress(
    CompressionCodec* compressor,
    folly::IOBuf& data) {
  ASSERT_GT(data.computeChainDataLength(), 0);

  auto compressedData = compressor->compress(data);
  EXPECT_GT(compressedData->computeChainDataLength(), 0);

  auto uncompressedData =
      compressor->uncompress(*compressedData, data.length());
  EXPECT_EQ(
      data.computeChainDataLength(),
      uncompressedData->computeChainDataLength());
  EXPECT_EQ(data.coalesce(), uncompressedData->coalesce());
}

void testCompressTwice(CompressionCodec* compressor, folly::IOBuf& data) {
  ASSERT_GT(data.computeChainDataLength(), 0);

  auto compressedData = compressor->compress(data);
  EXPECT_GT(compressedData->computeChainDataLength(), 0);

  auto compressedData2 = compressor->compress(data);
  EXPECT_EQ(
      compressedData->computeChainDataLength(),
      compressedData2->computeChainDataLength());
  EXPECT_EQ(compressedData->coalesce(), compressedData2->coalesce());
}

void testCompressChained(
    CompressionCodec* compressor,
    folly::IOBuf& data,
    size_t chainLength) {
  ASSERT_GT(data.computeChainDataLength(), 0);

  auto compressedData = compressor->compress(data);
  EXPECT_GT(compressedData->computeChainDataLength(), 0);

  auto chainedData = buildChain(data, chainLength);
  auto compressedChainedData = compressor->compress(*chainedData);
  EXPECT_EQ(
      compressedData->computeChainDataLength(),
      compressedChainedData->computeChainDataLength());
  EXPECT_EQ(compressedData->coalesce(), compressedChainedData->coalesce());
}

void testUncompressChained(
    CompressionCodec* compressor,
    folly::IOBuf& data,
    size_t chainLength) {
  ASSERT_GT(chainLength, 1);
  ASSERT_FALSE(data.isChained());
  ASSERT_GE(data.length(), chainLength);

  auto compressedData = compressor->compress(data);
  EXPECT_GT(compressedData->computeChainDataLength(), 0);

  auto compressedChainedData = buildChain(*compressedData, chainLength);

  auto uncompressedData =
      compressor->uncompress(*compressedChainedData, data.length());
  EXPECT_EQ(
      data.computeChainDataLength(),
      uncompressedData->computeChainDataLength());
  EXPECT_EQ(data.coalesce(), uncompressedData->coalesce());
}

} // anonymous namespace

TEST(NoCompressionCodec, compressAndUncompress) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::NO_COMPRESSION, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(NoCompressionCodec, compressTwice) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::NO_COMPRESSION, getAsciiDictionary(), 1);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(NoCompressionCodec, compressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::NO_COMPRESSION, getAsciiDictionary(), 1);
  auto data = getAsciiReply();
  for (size_t i = 2; i < data->length(); ++i) {
    testCompressChained(compressor.get(), *data, i);
  }
}

TEST(NoCompressionCodec, uncompressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::NO_COMPRESSION, getAsciiDictionary(), 1);
  testUncompressChained(compressor.get(), *getAsciiReply(), 3);
}

#if FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)
TEST(Lz4CompressionCodec, compressAndUncompress) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(Lz4CompressionCodec, compressAndUncompressWithFilters) {
  constexpr int64_t kTypeId = -1;
  FilteringOptions filters;
  filters.typeId = kTypeId;
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1, filters);
  EXPECT_EQ(compressor->filteringOptions().isEnabled, true);
  EXPECT_EQ(compressor->filteringOptions().typeId, kTypeId);
  EXPECT_EQ(compressor->filteringOptions().minCompressionThreshold, 0);
  EXPECT_EQ(
      compressor->filteringOptions().maxCompressionThreshold,
      std::numeric_limits<uint32_t>::max());
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(Lz4CompressionCodec, compressAndUncompress_largeValues) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getRandomLargeReply());
}

TEST(Lz4CompressionCodec, compressTwice) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(Lz4CompressionCodec, compressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1);
  auto data = getAsciiReply();
  for (size_t i = 2; i < data->length(); ++i) {
    testCompressChained(compressor.get(), *data, i);
  }
}

TEST(Lz4CompressionCodec, uncompressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4, getAsciiDictionary(), 1);
  testUncompressChained(compressor.get(), *getAsciiReply(), 3);
}
#endif // FOLLY_HAVE_LIBLZ4 && !defined(DISABLE_COMPRESSION)

#if FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)
TEST(ZstdCompressionCodec, compressAndUncompress) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(ZstdCompressionCodec, compressAndUncompressWithCompressionLevel) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      {} /* filtering options */,
      5 /* compression level */);
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(ZstdCompressionCodec, compressAndUncompress_largeValues) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getRandomLargeReply());
}

TEST(
    ZstdCompressionCodec,
    compressAndUncompressWithCompressionLevel_largeValues) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      {} /* filtering options */,
      5 /* compression level */);
  testCompressAndUncompress(compressor.get(), *getRandomLargeReply());
}

TEST(ZstdCompressionCodec, compressTwiceWith) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD, getAsciiDictionary(), 1);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(ZstdCompressionCodec, compressTwiceWithCompressionLevel) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      {} /* filtering options */,
      5 /* compression level */);

  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(ZstdCompressionCodec, compressTwiceWithCompressionLevelAndFilters) {
  FilteringOptions filters;
  filters.isEnabled = false;
  filters.minCompressionThreshold = 64;
  filters.maxCompressionThreshold = 1024;
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      filters /* filtering options */,
      5 /* compression level */);
  EXPECT_EQ(compressor->filteringOptions().isEnabled, false);
  EXPECT_EQ(compressor->filteringOptions().typeId, 0);
  EXPECT_EQ(compressor->filteringOptions().minCompressionThreshold, 64);
  EXPECT_EQ(compressor->filteringOptions().maxCompressionThreshold, 1024);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(ZstdCompressionCodec, compressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD, getAsciiDictionary(), 1);
  auto data = getAsciiReply();
  for (size_t i = 2; i < data->length(); ++i) {
    testCompressChained(compressor.get(), *data, i);
  }
}

TEST(ZstdCompressionCodec, compressChainedWithCompressionLevel) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      {} /* filtering options */,
      5 /* compression level */);

  auto data = getAsciiReply();
  for (size_t i = 2; i < data->length(); ++i) {
    testCompressChained(compressor.get(), *data, i);
  }
}

TEST(ZstdCompressionCodec, uncompressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD, getAsciiDictionary(), 1);
  testUncompressChained(compressor.get(), *getAsciiReply(), 3);
}

TEST(ZstdCompressionCodec, uncompressChainedWithCompressionLevel) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::ZSTD,
      getAsciiDictionary(),
      1 /* id */,
      {} /* filtering options */,
      5 /* compression level */);

  testUncompressChained(compressor.get(), *getAsciiReply(), 3);
}
#endif // FOLLY_HAVE_LIBZSTD && !defined(DISABLE_COMPRESSION)

TEST(Lz4ImmutableCompressionCodec, compressAndUncompress) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getAsciiReply());
}

TEST(Lz4ImmutableCompressionCodec, compressAndUncompress_largeValues) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable, getAsciiDictionary(), 1);
  testCompressAndUncompress(compressor.get(), *getRandomLargeReply());
}

TEST(Lz4ImmutableCompressionCodec, compressTwiceWith) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable, getAsciiDictionary(), 1);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(
    Lz4ImmutableCompressionCodec,
    compressTwiceWithCompressionLevelAndFilters) {
  FilteringOptions filters;
  filters.isEnabled = false;
  filters.minCompressionThreshold = 64;
  filters.maxCompressionThreshold = 1024;
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable,
      getAsciiDictionary(),
      1 /* id */,
      filters /* filtering options */,
      5 /* compression level */);
  EXPECT_EQ(compressor->filteringOptions().isEnabled, false);
  EXPECT_EQ(compressor->filteringOptions().typeId, 0);
  EXPECT_EQ(compressor->filteringOptions().minCompressionThreshold, 64);
  EXPECT_EQ(compressor->filteringOptions().maxCompressionThreshold, 1024);
  testCompressTwice(compressor.get(), *getAsciiReply());
}

TEST(Lz4ImmutableCompressionCodec, compressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable, getAsciiDictionary(), 1);
  auto data = getAsciiReply();
  for (size_t i = 2; i < data->length(); ++i) {
    testCompressChained(compressor.get(), *data, i);
  }
}

TEST(Lz4ImmutableCompressionCodec, uncompressChained) {
  auto compressor = createCompressionCodec(
      CompressionCodecType::LZ4Immutable, getAsciiDictionary(), 1);
  testUncompressChained(compressor.get(), *getAsciiReply(), 3);
}
} // namespace test
} // namespace memcache
} // namespace facebook
