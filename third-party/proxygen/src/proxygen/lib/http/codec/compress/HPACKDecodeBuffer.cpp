/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKDecodeBuffer.h>

#include <limits>
#include <memory>
#include <proxygen/lib/http/codec/compress/Huffman.h>

using folly::IOBuf;
using proxygen::HPACK::DecodeError;
using std::unique_ptr;

namespace proxygen {

void HPACKDecodeBuffer::EOB_LOG(std::string msg, DecodeError code) const {
  if (endOfBufferIsError_ || code != DecodeError::BUFFER_UNDERFLOW) {
    LOG(ERROR) << msg;
  } else {
    VLOG(4) << msg;
  }
}

bool HPACKDecodeBuffer::empty() {
  return remainingBytes_ == 0;
}

uint8_t HPACKDecodeBuffer::next() {
  CHECK_GT(remainingBytes_, 0);
  // in case we are the end of an IOBuf, peek() will move to the next one
  uint8_t byte = peek();
  cursor_.skip(1);
  remainingBytes_--;

  return byte;
}

uint8_t HPACKDecodeBuffer::peek() {
  CHECK_GT(remainingBytes_, 0);
  if (cursor_.length() == 0) {
    cursor_.peek();
  }
  return *cursor_.data();
}

DecodeError HPACKDecodeBuffer::decodeLiteral(folly::fbstring& literal) {
  return decodeLiteral(7, literal);
}

DecodeError HPACKDecodeBuffer::decodeLiteral(uint8_t nbit,
                                             folly::fbstring& literal) {
  literal.clear();
  if (remainingBytes_ == 0) {
    EOB_LOG("remainingBytes_ == 0");
    return DecodeError::BUFFER_UNDERFLOW;
  }
  auto byte = peek();
  uint8_t huffmanCheck = uint8_t(1 << nbit);
  bool huffman = byte & huffmanCheck;
  // extract the size
  uint64_t size;
  DecodeError result = decodeInteger(nbit, size);
  if (result != DecodeError::NONE) {
    EOB_LOG("Could not decode literal size", result);
    return result;
  }
  if (size > remainingBytes_) {
    EOB_LOG(folly::to<std::string>(
        "size(", size, ") > remainingBytes_(", remainingBytes_, ")"));
    return DecodeError::BUFFER_UNDERFLOW;
  }
  if (size > maxLiteralSize_) {
    LOG(ERROR) << "Literal too large, size=" << size;
    return DecodeError::LITERAL_TOO_LARGE;
  }
  const uint8_t* data;
  unique_ptr<IOBuf> tmpbuf;
  // handle the case where the buffer spans multiple buffers
  if (cursor_.length() >= size) {
    data = cursor_.data();
    cursor_.skip(size);
  } else {
    // temporary buffer to pull the chunks together
    tmpbuf = IOBuf::create(size);
    // pull() will move the cursor
    cursor_.pull(tmpbuf->writableData(), size);
    data = tmpbuf->data();
  }
  if (huffman) {
    static auto& huffmanTree = huffman::huffTree();
    huffmanTree.decode(data, size, literal);
  } else {
    literal.append((const char*)data, size);
  }
  remainingBytes_ -= size;
  return DecodeError::NONE;
}

DecodeError HPACKDecodeBuffer::decodeInteger(uint64_t& integer) {
  return decodeInteger(8, integer);
}

DecodeError HPACKDecodeBuffer::decodeInteger(uint8_t nbit, uint64_t& integer) {
  if (remainingBytes_ == 0) {
    EOB_LOG("remainingBytes_ == 0");
    return DecodeError::BUFFER_UNDERFLOW;
  }
  uint8_t byte = next();
  uint8_t mask = HPACK::NBIT_MASKS[nbit];
  // remove the first (8 - nbit) bits
  byte = byte & mask;
  integer = byte;
  if (byte != mask) {
    // the value fit in one byte
    return DecodeError::NONE;
  }
  uint64_t f = 1;
  uint32_t fexp = 0;
  do {
    if (remainingBytes_ == 0) {
      EOB_LOG("remainingBytes_ == 0");
      return DecodeError::BUFFER_UNDERFLOW;
    }
    byte = next();
    if (fexp > 64) {
      // overflow in factorizer, f > 2^64
      LOG(ERROR) << "overflow fexp=" << fexp;
      return DecodeError::INTEGER_OVERFLOW;
    }
    uint64_t add = (byte & 127) * f;
    if (std::numeric_limits<uint64_t>::max() - integer <= add) {
      // overflow detected - we disallow uint64_t max.
      LOG(ERROR) << "overflow integer=" << integer << " add=" << add;
      return DecodeError::INTEGER_OVERFLOW;
    }
    integer += add;
    f = f << 7;
    fexp += 7;
  } while (byte & 128);
  return DecodeError::NONE;
}
namespace HPACK {
std::ostream& operator<<(std::ostream& os, DecodeError err) {
  return os << static_cast<uint32_t>(err);
}
} // namespace HPACK
} // namespace proxygen
