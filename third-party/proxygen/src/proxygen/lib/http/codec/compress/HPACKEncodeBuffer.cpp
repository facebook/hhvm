/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKEncodeBuffer.h>

#include <memory>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/Logging.h>
#include <proxygen/lib/utils/Logging.h>

using folly::IOBuf;
using std::string;
using std::unique_ptr;

namespace proxygen {

HPACKEncodeBuffer::HPACKEncodeBuffer(uint32_t growthSize, bool huffmanEnabled)
    : buf_(&bufQueue_, growthSize),
      growthSize_(growthSize),
      huffMin_(huffmanEnabled ? 0 : std::numeric_limits<uint32_t>::max()),
      huffMax_(huffmanEnabled ? std::numeric_limits<uint32_t>::max() : 0) {
}

HPACKEncodeBuffer::HPACKEncodeBuffer(uint32_t growthSize)
    : buf_(&bufQueue_, growthSize), growthSize_(growthSize), huffMax_(0) {
}

void HPACKEncodeBuffer::addHeadroom(uint32_t headroom) {
  // we expect that this function is called before any encoding happens
  CHECK(bufQueuePtr_->front() == nullptr);
  // create a custom IOBuf and add it to the queue
  unique_ptr<IOBuf> buf = IOBuf::create(std::max(headroom, growthSize_));
  buf->advance(headroom);
  bufQueuePtr_->append(std::move(buf));
}

void HPACKEncodeBuffer::append(uint8_t byte) {
  buf_.push(&byte, 1);
}

uint32_t HPACKEncodeBuffer::encodeInteger(uint64_t value) {
  return encodeInteger(value, 0, 8);
}

uint32_t HPACKEncodeBuffer::encodeInteger(
    uint64_t value, const HPACK::Instruction& instruction) {
  return encodeInteger(value, instruction.code, instruction.prefixLength);
}

uint32_t HPACKEncodeBuffer::encodeInteger(uint64_t value,
                                          uint8_t instruction,
                                          uint8_t nbit) {
  CHECK(nbit > 0 && nbit <= 8);
  uint32_t count = 0;
  uint8_t mask = HPACK::NBIT_MASKS[nbit];
  // The instruction should not extend into mask
  DCHECK_EQ(instruction & mask, 0);

  // write the first byte
  uint8_t byte = instruction;
  if (value < mask) {
    // fits in the first byte
    byte |= value;
    append(byte);
    return 1;
  }

  byte |= mask;
  value -= mask;
  ++count;
  append(byte);
  // variable length encoding
  while (value >= 128) {
    byte = 128 | (127 & value);
    append(byte);
    value = value >> 7;
    ++count;
  }
  // last byte, which should always fit on 1 byte
  append(value);
  ++count;
  return count;
}

uint32_t HPACKEncodeBuffer::encodeHuffman(folly::StringPiece literal) {
  return encodeHuffman(0, 7, literal);
}

/*
 * Huffman encode the literal and serialize, with an optional leading
 * instruction.  The instruction can be at most 8 - 1 - nbit bits long.  nbit
 * bits of the first byte will contain the prefix of the encoded literal's
 * length.  For HPACK instruction/nbit should always be 0/7.
 *
 * The encoded output looks like this
 *
 * | instruction | 1 | Length... | Huffman Coded Literal |
 */
uint32_t HPACKEncodeBuffer::encodeHuffman(uint8_t instruction,
                                          uint8_t nbit,
                                          folly::StringPiece literal) {
  static const auto& huffmanTree = huffman::huffTree();
  uint32_t size = huffmanTree.getEncodeSize(literal);
  // add the length
  DCHECK_LE(nbit, 7);
  uint8_t huffmanOn = uint8_t(1 << nbit);
  DCHECK_EQ(instruction & huffmanOn, 0);
  uint32_t count = encodeInteger(size, instruction | huffmanOn, nbit);
  // ensure we have enough bytes before performing the encoding
  count += huffmanTree.encode(literal, buf_);
  return count;
}

uint32_t HPACKEncodeBuffer::encodeLiteral(folly::StringPiece literal) {
  return encodeLiteral(0, 7, literal);
}

uint32_t HPACKEncodeBuffer::encodeLiteral(uint8_t instruction,
                                          uint8_t nbit,
                                          folly::StringPiece literal) {
  if (literal.size() >= huffMin_ && literal.size() <= huffMax_) {
    return encodeHuffman(instruction, nbit, literal);
  }
  // otherwise use simple layout
  uint32_t count = encodeInteger(literal.size(), instruction, nbit);
  // copy the entire string
  buf_.push((uint8_t*)literal.data(), literal.size());
  count += literal.size();
  return count;
}

string HPACKEncodeBuffer::toBin() {
  return IOBufPrinter::printBin(bufQueuePtr_->front());
}

} // namespace proxygen
