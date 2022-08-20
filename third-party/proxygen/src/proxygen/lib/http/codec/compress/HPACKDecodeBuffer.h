/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/FBString.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <proxygen/lib/http/codec/compress/Huffman.h>

namespace proxygen {

class HPACKDecodeBuffer {
 public:
  explicit HPACKDecodeBuffer(folly::io::Cursor& cursorVal,
                             uint32_t totalBytes,
                             uint32_t maxLiteralSize,
                             bool endOfBufferIsError = true)
      : cursor_(cursorVal),
        totalBytes_(totalBytes),
        remainingBytes_(totalBytes),
        maxLiteralSize_(maxLiteralSize),
        endOfBufferIsError_(endOfBufferIsError) {
  }

  ~HPACKDecodeBuffer() {
  }

  void reset(folly::io::Cursor& cursorVal) {
    reset(cursorVal, folly::to<uint32_t>(cursorVal.totalLength()));
  }

  void reset(folly::io::Cursor& cursorVal, uint32_t totalBytes) {
    cursor_ = cursorVal;
    totalBytes_ = totalBytes;
    remainingBytes_ = totalBytes;
  }

  uint32_t consumedBytes() const {
    return totalBytes_ - remainingBytes_;
  }

  const folly::io::Cursor& cursor() const {
    return cursor_;
  }

  /**
   * @returns true if there are no more bytes to decode. Calling this method
   * might move the cursor from the current IOBuf to the next one
   */
  bool empty();

  /**
   * extracts one byte from the buffer and advances the cursor
   */
  uint8_t next();

  /**
   * just peeks at the next available byte without moving the cursor
   */
  uint8_t peek();

  /**
   * decode an integer from the current position, given a nbit prefix.
   * Ignores 8 - nbit bits in the first byte of the buffer.
   */
  HPACK::DecodeError decodeInteger(uint8_t nbit, uint64_t& integer);

  /**
   * As above but with no prefix
   */
  HPACK::DecodeError decodeInteger(uint64_t& integer);

  /**
   * decode a literal starting from the current position
   */
  HPACK::DecodeError decodeLiteral(folly::fbstring& literal);

  HPACK::DecodeError decodeLiteral(uint8_t nbit, folly::fbstring& literal);

 private:
  void EOB_LOG(
      std::string msg,
      HPACK::DecodeError code = HPACK::DecodeError::BUFFER_UNDERFLOW) const;

  folly::io::Cursor& cursor_;
  uint32_t totalBytes_;
  uint32_t remainingBytes_;
  uint32_t maxLiteralSize_{std::numeric_limits<uint32_t>::max()};
  bool endOfBufferIsError_{true};
};

} // namespace proxygen
