/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/aead/IOBufUtil.h>
#include <folly/io/IOBuf.h>

using namespace folly;

namespace fizz {
namespace test {

TEST(IOBufUtilTest, TrimBytes) {
  auto buf = IOBuf::copyBuffer("hello");
  buf->prependChain(IOBuf::copyBuffer("world"));
  buf->prependChain(IOBuf::copyBuffer("I"));
  buf->prependChain(IOBuf::create(0));
  buf->prependChain(IOBuf::copyBuffer("speak"));

  auto expected = IOBuf::copyBuffer("Ispeak");
  auto bufExpected = IOBuf::copyBuffer("helloworld");
  auto bufLen = buf->computeChainDataLength();
  constexpr size_t trimLen = 6;
  std::array<uint8_t, trimLen> trimData;
  folly::MutableByteRange trim(trimData);
  trimBytes(*buf, trim);

  IOBufEqualTo eq;
  EXPECT_EQ(expected->coalesce(), trim.castToConst());
  EXPECT_EQ(bufLen - trimLen, buf->computeChainDataLength());
  EXPECT_TRUE(eq(bufExpected, buf));
}

TEST(IOBufUtilTest, TransformBufferInplace) {
  auto buf = IOBuf::copyBuffer("hello");
  buf->prependChain(IOBuf::copyBuffer("world"));
  buf->prependChain(IOBuf::copyBuffer("I"));
  buf->prependChain(IOBuf::create(0));
  buf->prependChain(IOBuf::copyBuffer("speak"));

  auto expected = IOBuf::create(buf->computeChainDataLength());
  expected->append(buf->computeChainDataLength());
  memset(expected->writableData(), 'w', expected->length());

  transformBuffer(
      *buf, *buf, [](uint8_t* out, const uint8_t* /*in*/, size_t len) {
        memset(out, 'w', len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(expected, buf));
}

std::unique_ptr<IOBuf> createBuf(size_t len) {
  auto res = IOBuf::create(len);
  res->append(len);
  return res;
}

TEST(IOBufUtilTest, TransformBufferWithFragmentedBuffer) {
  auto buf = IOBuf::copyBuffer("hello");
  buf->prependChain(IOBuf::copyBuffer("world"));
  buf->prependChain(IOBuf::copyBuffer("I"));
  buf->prependChain(IOBuf::create(0));
  buf->prependChain(IOBuf::copyBuffer("speak"));

  // needs to add up to 16
  auto fragmented = createBuf(3);
  auto fragment1 = createBuf(6);
  auto fragment2 = createBuf(0);
  auto fragment3 = createBuf(2);
  auto fragment4 = createBuf(4);
  auto fragment5 = createBuf(1);

  fragmented->prependChain(std::move(fragment1));
  fragmented->prependChain(std::move(fragment2));
  fragmented->prependChain(std::move(fragment3));
  fragmented->prependChain(std::move(fragment4));
  fragmented->prependChain(std::move(fragment5));

  transformBuffer(
      *buf, *fragmented, [](uint8_t* out, const uint8_t* in, size_t len) {
        memcpy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, fragmented));
}

struct BlockWriter {
  size_t copy(uint8_t* out, const uint8_t* in, size_t len) {
    if (len + internalOffset < 8) {
      // still not enough for a block
      memcpy(block + internalOffset, in, len);
      internalOffset += len;
      // nothing copied to out
      return 0;
    }
    size_t outOffset = 0;
    // need to copy in block chunks
    auto numToWrite = 8 * ((internalOffset + len) / 8);
    if (internalOffset > 0) {
      memcpy(out, block, internalOffset);
      outOffset += internalOffset;
      numToWrite -= internalOffset;
      internalOffset = 0;
    }

    // copy the rest
    memcpy(out + outOffset, in, numToWrite);
    // check if we need to internal buffer anything left
    if (len > numToWrite) {
      auto numToBuf = len - numToWrite;
      DCHECK(numToBuf < 8);
      memcpy(block, in + numToWrite, numToBuf);
      internalOffset = numToBuf;
    }
    return numToWrite + outOffset;
  }

  size_t internalOffset = 0;
  uint8_t block[8] = {0};
};

TEST(IOBufUtilTest, TransformBufferBlocks) {
  // 2 blocks of size 8
  auto buf = IOBuf::copyBuffer("0000111122223333");
  auto output = createBuf(16);

  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

TEST(IOBufUtilTest, TransformBufferBlocksSplit) {
  // 1 block of size 8
  auto buf = IOBuf::copyBuffer("0000");
  buf->prependChain(IOBuf::copyBuffer("1111"));
  auto output = createBuf(4);
  auto fragment1 = createBuf(4);
  output->prependChain(std::move(fragment1));

  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

TEST(IOBufUtilTest, TransformBufferBlocksInputFragmented) {
  // do 3 blocks
  auto buf = IOBuf::copyBuffer("00");
  buf->prependChain(IOBuf::copyBuffer("00"));
  buf->prependChain(IOBuf::copyBuffer("1"));
  buf->prependChain(IOBuf::create(0));
  buf->prependChain(IOBuf::copyBuffer("11122223"));
  buf->prependChain(IOBuf::copyBuffer("33344"));
  buf->prependChain(IOBuf::copyBuffer("44"));
  buf->prependChain(IOBuf::copyBuffer("5555"));

  auto output = IOBuf::create(24);
  output->append(24);

  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

TEST(IOBufUtilTest, TransformBufferBlocksOutputFragmented) {
  // 3 blocks of 8
  auto buf = IOBuf::copyBuffer("fizzbuzzfizzbuzzfizzbuzz");

  // needs to add up to 24
  auto output = createBuf(3);
  output->prependChain(createBuf(6));
  output->prependChain(createBuf(0));
  output->prependChain(createBuf(2));
  output->prependChain(createBuf(4));
  output->prependChain(createBuf(1));
  output->prependChain(createBuf(1));
  output->prependChain(createBuf(7));

  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

TEST(IOBufUtilTest, TransformBufferBlocksInputFragmented2) {
  auto buf = IOBuf::copyBuffer("1111111122222222");
  auto output = createBuf(10);
  output->prependChain(createBuf(6));
  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

TEST(IOBufUtilTest, TransformBufferBlocksFragmented) {
  auto buf = IOBuf::copyBuffer("00");
  buf->prependChain(IOBuf::copyBuffer("00"));
  buf->prependChain(IOBuf::copyBuffer("1"));
  buf->prependChain(IOBuf::create(0));
  buf->prependChain(IOBuf::copyBuffer("11122223"));
  buf->prependChain(IOBuf::copyBuffer("33344"));
  buf->prependChain(IOBuf::copyBuffer("44"));
  buf->prependChain(IOBuf::copyBuffer("5555"));

  // needs to add up to 24
  auto output = createBuf(3);
  output->prependChain(createBuf(6));
  output->prependChain(createBuf(0));
  output->prependChain(createBuf(2));
  output->prependChain(createBuf(4));
  output->prependChain(createBuf(1));
  output->prependChain(createBuf(1));
  output->prependChain(createBuf(7));

  BlockWriter writer;
  transformBufferBlocks<8>(
      *buf, *output, [&writer](uint8_t* out, const uint8_t* in, size_t len) {
        return writer.copy(out, in, len);
      });
  IOBufEqualTo eq;
  EXPECT_TRUE(eq(buf, output));
}

} // namespace test
} // namespace fizz
