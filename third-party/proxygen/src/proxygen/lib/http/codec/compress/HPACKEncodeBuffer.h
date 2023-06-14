/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/FBString.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/Huffman.h>

namespace proxygen {

class HPACKEncodeBuffer {

 public:
  HPACKEncodeBuffer(uint32_t growthSize, bool huffmanEnabled);

  explicit HPACKEncodeBuffer(uint32_t growthSize);

  ~HPACKEncodeBuffer() {
  }

  void setHuffmanLimits(std::pair<uint32_t, uint32_t> limits) {
    if (huffMax_ >= huffMin_) {
      huffMin_ = limits.first;
      huffMax_ = limits.second;
    }
  }

  /**
   * transfer ownership of the underlying IOBuf's
   */
  std::unique_ptr<folly::IOBuf> release() {
    return bufQueuePtr_->move();
  }

  /**
   * Add headroom at the beginning of the IOBufQueue
   * Meant to be called before encoding anything.
   */
  void addHeadroom(uint32_t bytes);

  /**
   * Encode the integer value using variable-length layout and the given
   * instruction using an nbit prefix.  Per the spec, prefix is the portion
   * of value that fits in one byte.
   * The instruction is given as 1-byte value (not need for shifting) used only
   * for the first byte. It starts from MSB.
   *
   * For example for integer=3, instruction=0x80, nbit=6:
   *
   * MSB           LSB
   * X X 0 0 0 0 1 1 (value)
   * 1 0 X X X X X X (instruction)
   * 1 0 0 0 0 0 1 1 (encoded value)
   *
   * @return how many bytes were used to encode the value
   */
  uint32_t encodeInteger(uint64_t value, uint8_t instruction, uint8_t nbit);

  uint32_t encodeInteger(uint64_t value, const HPACK::Instruction& instruction);

  uint32_t encodeInteger(uint64_t value);

  /**
   * encodes a string, either header name or header value
   *
   * @return bytes used for encoding
   */
  uint32_t encodeLiteral(folly::StringPiece literal);

  /**
   * encodes a string, either header name or header value QPACK style, where
   * literal length has an nbit prefix.
   *
   * @return bytes used for encoding
   */
  uint32_t encodeLiteral(uint8_t instruction,
                         uint8_t nbit,
                         folly::StringPiece literal);

  /**
   * encodes a string using huffman encoding
   */
  uint32_t encodeHuffman(folly::StringPiece literal);

  /**
   * encodes a string using huffman encoding QPACK style, where
   * literal length has an nbit prefix.
   */
  uint32_t encodeHuffman(uint8_t instruction,
                         uint8_t nbit,
                         folly::StringPiece literal);

  /**
   * prints the content of an IOBuf in binary format. Useful for debugging.
   */
  std::string toBin();

  void setWriteBuf(folly::IOBufQueue* writeBuf) {
    if (writeBuf) {
      bufQueuePtr_ = writeBuf;
    } else {
      bufQueuePtr_ = &bufQueue_;
    }
    buf_.reset(bufQueuePtr_, growthSize_);
  }

 private:
  /**
   * append one byte at the end of buffer ensuring we always have enough space
   */
  void append(uint8_t byte);

  folly::IOBufQueue bufQueue_;
  folly::IOBufQueue* bufQueuePtr_{&bufQueue_};
  folly::io::QueueAppender buf_;
  uint32_t growthSize_;
  uint32_t huffMin_{0};
  uint32_t huffMax_{std::numeric_limits<uint32_t>::max()};
};

} // namespace proxygen
