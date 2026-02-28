/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <memory>
#include <proxygen/lib/http/codec/compress/HPACKDecodeBuffer.h>
#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>

using namespace folly::io;
using namespace folly;
using namespace proxygen;
using namespace std;
using proxygen::HPACK::DecodeError;

namespace {
const uint32_t kMaxLiteralSize = 1 << 17;
}

class HPACKBufferTests : public testing::Test {
 public:
  /*
   * most of the tests need a tiny slice of data, so we're setting up
   * a queue with one IOBuf of 512 bytes in it
   */
  HPACKBufferTests() : encoder_(512), decoder_(cursor_, 0, kMaxLiteralSize) {
  }

 protected:
  // extracts the data from the encoder and setup pointers to it
  void releaseData() {
    releaseData(encoder_);
  }

  void releaseData(HPACKEncodeBuffer& encoder) {
    buf_ = encoder.release();
    if (buf_) {
      data_ = buf_->data();
    }
  }

  void resetDecoder() {
    resetDecoder(buf_.get());
  }

  void resetDecoder(IOBuf* buf) {
    cursor_.reset(buf);
    decoder_.reset(cursor_);
  }

  Cursor cursor_{nullptr};
  HPACKEncodeBuffer encoder_;
  HPACKDecodeBuffer decoder_;
  std::unique_ptr<IOBuf> buf_{nullptr};
  const uint8_t* data_{nullptr};
};

TEST_F(HPACKBufferTests, EncodeInteger) {
  // all these fit in one byte
  EXPECT_EQ(encoder_.encodeInteger(7, 192, 6), 1);
  // this one fits perfectly, but needs an additional 0 byte
  EXPECT_EQ(encoder_.encodeInteger(7, 192, 3), 2);
  EXPECT_EQ(encoder_.encodeInteger(255, 0, 8), 2);
  releaseData();
  EXPECT_EQ(buf_->length(), 5);
  EXPECT_EQ(data_[0], 199); // 11000111
  EXPECT_EQ(data_[1], 199); // 11000111
  EXPECT_EQ(data_[2], 0);
  EXPECT_EQ(data_[3], 255); // 11111111
  EXPECT_EQ(data_[4], 0);

  // multiple byte span
  EXPECT_EQ(encoder_.encodeInteger(7, 192, 2), 2);
  releaseData();
  EXPECT_EQ(buf_->length(), 2);
  EXPECT_EQ(data_[0], 195); // 11000011
  EXPECT_EQ(data_[1], 4);   // 00000100

  // the one from the spec - 1337
  EXPECT_EQ(encoder_.encodeInteger(1337, 0, 5), 3);
  releaseData();
  EXPECT_EQ(buf_->length(), 3);
  EXPECT_EQ(data_[0], 31);  // 00011111
  EXPECT_EQ(data_[1], 154); // 10011010
  EXPECT_EQ(data_[2], 10);  // 00001010
}

TEST_F(HPACKBufferTests, EncodePlainLiteral) {
  string literal("accept-encoding");
  EXPECT_EQ(encoder_.encodeLiteral(literal), 16);
  releaseData();
  EXPECT_EQ(buf_->length(), 16);
  EXPECT_EQ(data_[0], 15);
  for (size_t i = 0; i < literal.size(); i++) {
    EXPECT_EQ(data_[i + 1], literal[i]);
  }
}

TEST_F(HPACKBufferTests, EncodePlainLiteralN) {
  string literal("accept-encodin"); // len=14
  // length must fit in 4 bits, with room for 3 bit instruction
  EXPECT_EQ(encoder_.encodeLiteral(0xE0, 4, literal), 15);
  releaseData();
  EXPECT_EQ(buf_->length(), 15);
  EXPECT_EQ(data_[0], 0xE0 | 14);
  for (size_t i = 0; i < literal.size(); i++) {
    EXPECT_EQ(data_[i + 1], literal[i]);
  }
}

TEST_F(HPACKBufferTests, EncodeHuffmanLiteral) {
  string accept("accept-encoding");
  HPACKEncodeBuffer encoder(512, true);
  uint32_t size = encoder.encodeLiteral(accept);
  EXPECT_EQ(size, 12);
  releaseData(encoder);
  EXPECT_EQ(buf_->length(), 12);
  EXPECT_EQ(data_[0], 0x80 | 11); // 128(huffman bit) | 11(length)
  EXPECT_EQ(data_[11], 0x7f);
}

TEST_F(HPACKBufferTests, EncodeHuffmanLiteralN) {
  string accept("accept-encoding");
  HPACKEncodeBuffer encoder(512, true);
  uint32_t size = encoder.encodeLiteral(0xE0, 4, accept);
  EXPECT_EQ(size, 12);
  releaseData(encoder);
  EXPECT_EQ(buf_->length(), 12);
  EXPECT_EQ(data_[0], 0xE0 | 0x10 | 11); // instruction | huffman | length
  EXPECT_EQ(data_[11], 0x7f);
}

TEST_F(HPACKBufferTests, DecodeSingleByte) {
  buf_ = IOBuf::create(512);
  uint8_t* wdata = buf_->writableData();
  buf_->append(1);
  // 6-bit prefix
  *wdata = 67;
  resetDecoder();
  uint64_t integer;
  CHECK_EQ(decoder_.decodeInteger(7, integer), DecodeError::NONE);
  CHECK_EQ(integer, 67);

  resetDecoder();

  CHECK_EQ(decoder_.decodeInteger(6, integer), DecodeError::NONE);
  CHECK_EQ(integer, 3);

  // set a bit in the prefix - it should not affect the decoded value
  *wdata = 195; // 195 = 128 + 67
  resetDecoder();
  CHECK_EQ(decoder_.decodeInteger(7, integer), DecodeError::NONE);
  CHECK_EQ(integer, 67);

  // 8-bit prefix - the entire byte
  resetDecoder();
  CHECK_EQ(decoder_.decodeInteger(8, integer), DecodeError::NONE);
  CHECK_EQ(integer, 195);
}

TEST_F(HPACKBufferTests, DecodeMultiByte) {
  buf_ = IOBuf::create(512);
  uint8_t* wdata = buf_->writableData();
  // edge case - max value in a 2-bit space
  buf_->append(2);
  wdata[0] = 67;
  wdata[1] = 0;
  resetDecoder();
  uint64_t integer;
  CHECK_EQ(decoder_.decodeInteger(2, integer), DecodeError::NONE);
  CHECK_EQ(integer, 3);
  CHECK_EQ(decoder_.cursor().length(), 0);
  // edge case - encode 130 = 127 + 3 on 2-bit prefix
  wdata[0] = 3;
  wdata[1] = 127;
  resetDecoder();
  CHECK_EQ(decoder_.decodeInteger(2, integer), DecodeError::NONE);
  CHECK_EQ(integer, 130);
  CHECK_EQ(decoder_.cursor().length(), 0);
  // edge case - encode 131 = 128 + 3
  buf_->append(1);
  wdata[0] = 3;
  wdata[1] = 128;
  wdata[2] = 1;
  resetDecoder();
  CHECK_EQ(decoder_.decodeInteger(2, integer), DecodeError::NONE);
  CHECK_EQ(integer, 131);
  CHECK_EQ(decoder_.cursor().length(), 0);
  // encode the value from the RFC example - 1337
  wdata[0] = 31;
  wdata[1] = 154;
  wdata[2] = 10;
  resetDecoder();
  CHECK_EQ(decoder_.decodeInteger(5, integer), DecodeError::NONE);
  CHECK_EQ(integer, 1337);
  CHECK_EQ(decoder_.cursor().length(), 0);
}

TEST_F(HPACKBufferTests, DecodeIntegerError) {
  buf_ = IOBuf::create(128);
  resetDecoder();
  // empty buffer
  uint64_t integer;
  CHECK_EQ(decoder_.decodeInteger(5, integer), DecodeError::BUFFER_UNDERFLOW);

  // incomplete buffer
  buf_->append(2);
  uint8_t* wdata = buf_->writableData();
  wdata[0] = 31;
  wdata[1] = 154;
  // wdata[2] = 10 missing
  CHECK_EQ(decoder_.decodeInteger(5, integer), DecodeError::BUFFER_UNDERFLOW);
}

TEST_F(HPACKBufferTests, DecodeLiteralError) {
  buf_ = IOBuf::create(128);

  uint8_t* wdata = buf_->writableData();
  buf_->append(3);
  resetDecoder();
  wdata[0] = 255; // size
  wdata[1] = 'a';
  wdata[2] = 'b';
  folly::fbstring literal;
  CHECK_EQ(decoder_.decodeLiteral(literal), DecodeError::BUFFER_UNDERFLOW);

  resetDecoder();
  // error decoding the size of the literal
  wdata[0] = 0xFF;
  wdata[1] = 0x80;
  wdata[2] = 0x80;
  EXPECT_EQ(decoder_.decodeLiteral(literal), DecodeError::BUFFER_UNDERFLOW);
}

TEST_F(HPACKBufferTests, DecodeLiteralMultiBuffer) {
  auto buf1 = IOBuf::create(128);
  auto buf2 = IOBuf::create(128);
  // encode the size
  // buf2 will not be entirely filled, to keep space for encoding the size
  // without overflowing
  uint32_t size = buf1->capacity() + buf2->capacity() - 10;
  releaseData();
  uint32_t sizeLen = encoder_.encodeInteger(size, 0, 7);
  releaseData();
  // copy the encoding of the size at the beginning
  memcpy(buf1->writableData(), buf_->data(), sizeLen);
  for (size_t i = sizeLen; i < buf1->capacity(); i++) {
    buf1->writableData()[i] = 'x';
  }
  buf1->append(buf1->capacity());
  for (size_t i = 0; i < buf2->capacity() - 10 + sizeLen; i++) {
    buf2->writableData()[i] = 'y';
  }
  buf2->append(buf2->capacity() - 10 + sizeLen);
  buf1->appendChain(std::move(buf2));
  // decode
  resetDecoder(buf1.get());
  folly::fbstring literal;
  EXPECT_EQ(decoder_.decodeLiteral(literal), DecodeError::NONE);
  EXPECT_EQ(literal.size(), size);
  EXPECT_EQ(literal[0], 'x');
  EXPECT_EQ(literal[literal.size() - 1], 'y');
}

TEST_F(HPACKBufferTests, DecodeHuffmanLiteralMultiBuffer) {
  // "gzip" fits perfectly in a 3 bytes block
  std::array<uint8_t, 3> gzip{0x9b, 0xd9, 0xab};
  auto buf1 = IOBuf::create(128);
  auto buf2 = IOBuf::create(128);
  // total size
  uint32_t size = buf1->capacity() + buf2->capacity() - 10;
  // it needs to fit a multiple of 3 blocks
  size -= (size % 3);
  // just in case we have some bytes left encoded
  releaseData();
  uint32_t sizeLen = encoder_.encodeInteger(size, 128, 7);
  // extract the encoded size
  releaseData();
  memcpy(buf1->writableData(), buf_->data(), sizeLen);
  // huffman index
  uint32_t hi = 0;
  for (size_t i = sizeLen; i < buf1->capacity(); i++) {
    buf1->writableData()[i] = gzip[hi];
    hi = (hi + 1) % 3;
  }
  buf1->append(buf1->capacity());
  for (size_t i = 0; i < buf2->capacity() - 10 + sizeLen; i++) {
    buf2->writableData()[i] = gzip[hi];
    hi = (hi + 1) % 3;
  }
  buf2->append(buf2->capacity() - 10 + sizeLen);
  buf1->appendChain(std::move(buf2));
  // decode
  resetDecoder(buf1.get());
  folly::fbstring literal;
  EXPECT_EQ(decoder_.decodeLiteral(literal), DecodeError::NONE);
  EXPECT_EQ(literal.size(), 4 * (size / 3));
  EXPECT_EQ(literal.find("gzip"), 0);
  EXPECT_EQ(literal.rfind("gzip"), literal.size() - 4);
}

TEST_F(HPACKBufferTests, DecodeHuffmanLiteralN) {
  // "gzip" fits perfectly in a 3 bytes block
  std::array<uint8_t, 3> gzip{0x9b, 0xd9, 0xab};
  uint32_t size = 3;
  // just in case we have some bytes left encoded
  releaseData();
  encoder_.encodeInteger(size, 0x80 | 0x10, 4);
  releaseData();
  memcpy(buf_->writableData() + 1, gzip.data(), size);
  buf_->append(size);
  // decode
  resetDecoder(buf_.get());
  folly::fbstring literal;
  EXPECT_EQ(decoder_.decodeLiteral(4, literal), DecodeError::NONE);
  EXPECT_EQ(literal.size(), 4 * (size / 3));
  EXPECT_EQ(literal.find("gzip"), 0);
  EXPECT_EQ(literal.rfind("gzip"), literal.size() - 4);
}

TEST_F(HPACKBufferTests, DecodePlainLiteral) {
  buf_ = IOBuf::create(512);
  std::string gzip("gzip");
  folly::fbstring literal;
  uint8_t* wdata = buf_->writableData();

  buf_->append(1 + gzip.size());
  wdata[0] = gzip.size();
  memcpy(wdata + 1, gzip.c_str(), gzip.size());

  resetDecoder();
  decoder_.decodeLiteral(literal);
  CHECK_EQ(literal, gzip);
}

TEST_F(HPACKBufferTests, DecodePlainLiteralN) {
  buf_ = IOBuf::create(512);
  std::string gzip("gzip");
  folly::fbstring literal;
  uint8_t* wdata = buf_->writableData();

  buf_->append(1 + gzip.size());
  wdata[0] = 0xE0 | gzip.size(); // add a 3 bit instruction
  memcpy(wdata + 1, gzip.c_str(), gzip.size());

  resetDecoder();
  decoder_.decodeLiteral(4, literal);
  CHECK_EQ(literal, gzip);
}

TEST_F(HPACKBufferTests, IntegerEncodeDecode) {
  HPACKEncodeBuffer encoder(512);
  // first encode
  uint32_t value = 145333;
  encoder.encodeInteger(value, 128, 5);
  releaseData(encoder);
  resetDecoder();
  EXPECT_EQ(decoder_.cursor().length(), 4);
  // now decode
  uint64_t integer;
  EXPECT_EQ(decoder_.decodeInteger(5, integer), DecodeError::NONE);
  EXPECT_EQ(integer, value);
  EXPECT_EQ(decoder_.cursor().length(), 0);

  // corner case
  value = 63;
  encoder.encodeInteger(value, 64, 6);
  releaseData(encoder);
  resetDecoder();
  EXPECT_EQ(decoder_.decodeInteger(6, integer), DecodeError::NONE);
  EXPECT_EQ(integer, value);
}

/**
 * really large integers
 */
TEST_F(HPACKBufferTests, IntegerOverflow) {
  uint64_t integer;
  buf_ = IOBuf::create(128);
  uint8_t* wdata = buf_->writableData();

  // enough headroom for both cases
  buf_->append(12);
  // overflow the accumulated value
  wdata[0] = 0xFF;
  wdata[1] = 0xFF;
  wdata[2] = 0xFF;
  wdata[3] = 0xFF;
  wdata[4] = 0xFF;
  wdata[5] = 0xFF;
  wdata[6] = 0xFF;
  wdata[7] = 0xFF;
  wdata[8] = 0xFF;
  wdata[9] = 0xFF;
  wdata[10] = 0x0F;
  resetDecoder();
  EXPECT_EQ(decoder_.decodeInteger(8, integer), DecodeError::INTEGER_OVERFLOW);

  // overflow the factorizer
  wdata[0] = 0xFF;
  wdata[1] = 0x80;
  wdata[2] = 0x80;
  wdata[3] = 0x80;
  wdata[4] = 0x80;
  wdata[5] = 0x80;
  wdata[6] = 0x80;
  wdata[7] = 0x80;
  wdata[8] = 0x80;
  wdata[9] = 0x80;
  wdata[10] = 0x80;
  wdata[11] = 0x01;
  resetDecoder();
  EXPECT_EQ(decoder_.decodeInteger(8, integer), DecodeError::INTEGER_OVERFLOW);
}

/**
 * test that we're able to decode the max integer
 */
TEST_F(HPACKBufferTests, IntegerMax) {
  uint64_t hpackMax = std::numeric_limits<uint64_t>::max() - 1;
  releaseData();
  // encoding with all the bit prefixes
  for (uint8_t bitprefix = 1; bitprefix <= 8; bitprefix++) {
    encoder_.encodeInteger(hpackMax, 0, bitprefix);
    // take the encoded data and shove it in the decoder
    releaseData();
    resetDecoder();
    uint64_t integer = 0;
    EXPECT_EQ(decoder_.decodeInteger(bitprefix, integer), DecodeError::NONE);
    EXPECT_EQ(integer, hpackMax);
  }
}

/**
 * making sure we're calling peek() before deferencing the first byte
 * to figure out if it's a huffman encoding or not
 */
TEST_F(HPACKBufferTests, EmptyIobufLiteral) {
  // construct an IOBuf chain made of 1 empty chain and a literal
  unique_ptr<IOBuf> first = IOBuf::create(128);
  // set a trap by setting first byte to 128, which signals Huffman encoding
  first->writableData()[0] = 0x80;

  HPACKEncodeBuffer encoder(128); // no huffman
  folly::fbstring literal("randomheadervalue");
  encoder.encodeLiteral(literal);
  first->appendChain(encoder.release());

  uint32_t size = first->next()->length();
  Cursor cursor(first.get());
  HPACKDecodeBuffer decoder(cursor, size, kMaxLiteralSize);
  folly::fbstring decoded;
  decoder.decodeLiteral(decoded);

  EXPECT_EQ(literal, decoded);
}

/**
 * the that we enforce a limit on the literal size
 */
TEST_F(HPACKBufferTests, LargeLiteralError) {
  uint32_t largeSize = 10 + kMaxLiteralSize;
  // encode a large string
  string largeLiteral;
  largeLiteral.append(largeSize, 'x');
  EXPECT_TRUE(encoder_.encodeLiteral(largeLiteral));
  releaseData();
  resetDecoder();
  folly::fbstring decoded = "";
  EXPECT_EQ(decoder_.decodeLiteral(decoded), DecodeError::LITERAL_TOO_LARGE);
  EXPECT_EQ(decoded.size(), 0);
}
