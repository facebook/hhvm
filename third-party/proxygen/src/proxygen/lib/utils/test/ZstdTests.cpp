/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/compression/Compression.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <proxygen/lib/utils/ZstdStreamCompressor.h>
#include <proxygen/lib/utils/ZstdStreamDecompressor.h>

using namespace folly;
using namespace proxygen;
using namespace std;
using namespace testing;

namespace {

class ZstdTests : public Test {};

std::unique_ptr<folly::IOBuf> makeBuf(uint32_t size) {
  auto out = folly::IOBuf::create(size);
  out->append(size);
  // fill with random junk
  folly::io::RWPrivateCursor cursor(out.get());
  while (cursor.length() >= 8) {
    cursor.write<uint64_t>(folly::Random::rand64());
  }
  while (cursor.length()) {
    cursor.write<uint8_t>((uint8_t)folly::Random::rand32());
  }
  return out;
}

void verify(std::unique_ptr<IOBuf> original,
            std::unique_ptr<IOBuf> compressed,
            bool reuseBuf) {
  auto zd = std::make_unique<ZstdStreamDecompressor>(reuseBuf);

  auto decompressed = zd->decompress(compressed.get());
  ASSERT_FALSE(zd->hasError()) << "Decompression error.";
  ASSERT_TRUE(zd->finished());

  IOBufEqualTo eq;
  ASSERT_TRUE(eq(original, decompressed) ||
              (original->empty() && decompressed == nullptr));
}

void verifyPieces(std::unique_ptr<IOBuf> original,
                  std::vector<std::unique_ptr<IOBuf>> compressed,
                  bool reuseBuf) {
  auto zd = std::make_unique<ZstdStreamDecompressor>(reuseBuf);

  auto decompressed = folly::IOBuf::create(0);

  for (const auto& piece : compressed) {
    auto dpiece = zd->decompress(piece.get());
    ASSERT_FALSE(zd->hasError()) << "Decompression error.";
    if (dpiece != nullptr) {
      decompressed->prev()->appendChain(std::move(dpiece));
    }
  }

  ASSERT_TRUE(zd->finished());

  IOBufEqualTo eq;
  ASSERT_TRUE(eq(original, decompressed));
}

std::unique_ptr<folly::IOBuf> compress(
    const std::unique_ptr<folly::IOBuf>& in) {
  auto codec = std::make_unique<ZstdStreamCompressor>(
      folly::io::COMPRESSION_LEVEL_DEFAULT);
  auto out = codec->compress(in.get());
  return out;
}

void compressThenDecompress(unique_ptr<IOBuf> buf, bool reuseBuf) {
  auto compressed = compress(buf);
  verify(std::move(buf), std::move(compressed), reuseBuf);
}

void compressThenDecompressPieces(
    std::vector<std::unique_ptr<folly::IOBuf>> input_pieces, bool reuseBuf) {
  auto codec = folly::io::getStreamCodec(folly::io::CodecType::ZSTD);

  std::vector<std::unique_ptr<folly::IOBuf>> compressed_pieces;

  size_t i = 0;
  for (const auto& piece : input_pieces) {
    const auto end = ++i == input_pieces.size();
    const auto op = end ? folly::io::StreamCodec::FlushOp::END
                        : folly::io::StreamCodec::FlushOp::FLUSH;
    auto irange = piece->coalesce();
    if (!end && irange.size() == 0) {
      continue;
    }
    bool done;
    do {
      auto cpiece = folly::IOBuf::create(ZSTD_compressBound(irange.size()));
      folly::MutableByteRange crange(cpiece->writableData(),
                                     cpiece->tailroom());
      done = codec->compressStream(irange, crange, op);
      cpiece->append(crange.begin() - cpiece->tail());
      compressed_pieces.push_back(std::move(cpiece));
    } while (irange.size() || !done);
  }

  // assembles from back to front
  auto input = folly::IOBuf::create(0);
  while (!input_pieces.empty()) {
    input->appendChain(std::move(input_pieces.back()));
    input_pieces.pop_back();
  }

  verifyPieces(std::move(input), std::move(compressed_pieces), reuseBuf);
}

void compressDecompressPiecesProxygenCodec(
    std::vector<std::unique_ptr<folly::IOBuf>> input_pieces,
    bool independent,
    bool reuseBuf) {
  auto codec = std::make_unique<ZstdStreamCompressor>(
      folly::io::COMPRESSION_LEVEL_DEFAULT, independent);

  std::vector<std::unique_ptr<folly::IOBuf>> compressed_pieces;

  size_t i = 0;
  for (const auto& piece : input_pieces) {
    const auto end = ++i == input_pieces.size();
    compressed_pieces.push_back(codec->compress(piece.get(), end));
  }

  // assembles from back to front
  auto input = folly::IOBuf::create(0);
  while (!input_pieces.empty()) {
    input->appendChain(std::move(input_pieces.back()));
    input_pieces.pop_back();
  }

  verifyPieces(std::move(input), std::move(compressed_pieces), reuseBuf);
}

} // anonymous namespace

// Try many different sizes because we've hit truncation problems before
TEST_F(ZstdTests, CompressDecompress1M) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE(
        { compressThenDecompress(makeBuf(8 * 128 * 1024), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompress2000) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE(
        { compressThenDecompress(makeBuf(2000), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompress1024) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE(
        { compressThenDecompress(makeBuf(1024), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompress500) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE(
        { compressThenDecompress(makeBuf(500), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompress50) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE({ compressThenDecompress(makeBuf(50), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompressEmpty) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE({ compressThenDecompress(makeBuf(0), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompressChain) {
  for (bool reuseBuf : {false, true}) {
    ASSERT_NO_FATAL_FAILURE({
      auto buf = makeBuf(4);
      buf->prependChain(makeBuf(38));
      buf->prependChain(makeBuf(12));
      buf->prependChain(makeBuf(0));
      compressThenDecompress(std::move(buf), reuseBuf);
    });
  }
}

TEST_F(ZstdTests, CompressDecompressStreaming) {
  for (bool reuseBuf : {false, true}) {
    std::vector<std::unique_ptr<folly::IOBuf>> input_pieces;
    input_pieces.push_back(makeBuf(38));
    input_pieces.push_back(makeBuf(12));
    input_pieces.push_back(makeBuf(4096));
    input_pieces.push_back(makeBuf(0));

    ASSERT_NO_FATAL_FAILURE(
        { compressThenDecompressPieces(std::move(input_pieces), reuseBuf); });
  }
}

TEST_F(ZstdTests, CompressDecompressStreamingProxygen) {
  for (bool reuseBuf : {false, true}) {
    std::vector<std::unique_ptr<folly::IOBuf>> input_pieces;
    input_pieces.push_back(makeBuf(38));
    input_pieces.push_back(makeBuf(12));
    input_pieces.push_back(makeBuf(4096));
    input_pieces.push_back(makeBuf(0));

    compressDecompressPiecesProxygenCodec(
        std::move(input_pieces), false, reuseBuf);
  }
}

TEST_F(ZstdTests, CompressDecompressStreamingProxygenIndependent) {
  for (bool reuseBuf : {false, true}) {
    std::vector<std::unique_ptr<folly::IOBuf>> input_pieces;
    input_pieces.push_back(makeBuf(38));
    input_pieces.push_back(makeBuf(12));
    input_pieces.push_back(makeBuf(4096));
    input_pieces.push_back(makeBuf(0));

    compressDecompressPiecesProxygenCodec(
        std::move(input_pieces), true, reuseBuf);
  }
}
