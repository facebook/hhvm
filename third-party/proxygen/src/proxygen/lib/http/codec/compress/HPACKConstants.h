/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iosfwd>
#include <stdint.h>

namespace proxygen::HPACK {

struct Instruction {
  uint8_t code;
  uint8_t prefixLength;
};

const Instruction INDEX_REF{.code = 0x80, .prefixLength = 7};
const Instruction LITERAL_INC_INDEX{.code = 0x40, .prefixLength = 6};
const Instruction LITERAL{.code = 0x00, .prefixLength = 4};
const Instruction LITERAL_NEV_INDEX{.code = 0x10, .prefixLength = 4};
const Instruction TABLE_SIZE_UPDATE{.code = 0x20, .prefixLength = 5};

// Encoder Stream
const Instruction Q_INSERT_NAME_REF{.code = 0x80, .prefixLength = 6};
const Instruction Q_INSERT_NO_NAME_REF{.code = 0x40, .prefixLength = 5};
const Instruction Q_TABLE_SIZE_UPDATE{.code = 0x20, .prefixLength = 5};
const Instruction Q_DUPLICATE{.code = 0x00, .prefixLength = 5};

// Decoder Stream
const Instruction Q_HEADER_ACK{.code = 0x80, .prefixLength = 7};
const Instruction Q_CANCEL_STREAM{.code = 0x40, .prefixLength = 6};
const Instruction Q_INSERT_COUNT_INC{.code = 0x00, .prefixLength = 6};

// Request/Push Streams

// Prefix
const uint8_t Q_DELTA_BASE_NEG = 0x80;
const uint8_t Q_DELTA_BASE_POS = 0x00;

const Instruction Q_DELTA_BASE{.code = 0x00, .prefixLength = 7};

// Instructions
const Instruction Q_INDEXED{.code = 0x80, .prefixLength = 6};
const Instruction Q_INDEXED_POST{.code = 0x10, .prefixLength = 4};
const Instruction Q_LITERAL_NAME_REF{.code = 0x40, .prefixLength = 4};
const Instruction Q_LITERAL_NAME_REF_POST{.code = 0x00, .prefixLength = 3};
const Instruction Q_LITERAL{.code = 0x20, .prefixLength = 3};

const uint8_t Q_INDEXED_STATIC = 0x40;
const uint8_t Q_INSERT_NAME_REF_STATIC = 0x40;
const uint8_t Q_LITERAL_STATIC = 0x10;

const uint32_t kDefaultBlocking = 100;

const uint32_t kTableSize = 4096;

const uint8_t NBIT_MASKS[9] = {
    0x00, // 00000000, unused
    0x01, // 00000001
    0x03, // 00000011
    0x07, // 00000111
    0x0F, // 00001111
    0x1F, // 00011111
    0x3F, // 00111111
    0x7F, // 01111111
    0xFF, // 11111111
};

enum LiteralEncoding : uint8_t { PLAIN = 0x00, HUFFMAN = 0x80 };

enum class DecodeError : uint8_t {
  NONE = 0,
  INVALID_INDEX = 1,
  INVALID_HUFFMAN_CODE = 2,
  INVALID_ENCODING = 3,
  INTEGER_OVERFLOW = 4,
  INVALID_TABLE_SIZE = 5,
  HEADERS_TOO_LARGE = 6,
  BUFFER_UNDERFLOW = 7,
  LITERAL_TOO_LARGE = 8,
  TIMEOUT = 9,
  ENCODER_STREAM_CLOSED = 10,
  BAD_SEQUENCE_NUMBER = 11,
  INVALID_ACK = 12,
  TOO_MANY_BLOCKING = 13,
  INSERT_TOO_LARGE = 14
};

std::ostream& operator<<(std::ostream& os, DecodeError err);
} // namespace proxygen::HPACK
