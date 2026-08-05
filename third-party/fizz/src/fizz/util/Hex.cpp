/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/util/Hex.h>

#include <array>
#include <cstdint>

namespace fizz {

namespace {
constexpr char kHexValues[] = "0123456789abcdef";

// Maps a hex character to its 0-15 value, or 16 if it is not a hex digit
// (mirrors folly::detail::hexTable; the 0x10 bit signals an invalid digit).
constexpr std::array<uint8_t, 256> makeHexTable() {
  std::array<uint8_t, 256> table{};
  for (auto& entry : table) {
    entry = 16;
  }
  for (int i = 0; i < 10; ++i) {
    table[static_cast<size_t>('0' + i)] = static_cast<uint8_t>(i);
  }
  for (int i = 0; i < 6; ++i) {
    table[static_cast<size_t>('a' + i)] = static_cast<uint8_t>(10 + i);
    table[static_cast<size_t>('A' + i)] = static_cast<uint8_t>(10 + i);
  }
  return table;
}

constexpr std::array<uint8_t, 256> kHexTable = makeHexTable();
} // namespace

std::string hexlify(folly::ByteRange input) {
  std::string ret(input.size() * 2, '\0');
  size_t j = 0;
  for (size_t i = 0; i < input.size(); ++i) {
    ret[j++] = kHexValues[(input[i] >> 4) & 0xf];
    ret[j++] = kHexValues[input[i] & 0xf];
  }
  return ret;
}

Status unhexlify(std::string& ret, Error& err, folly::StringPiece input) {
  if (input.size() % 2 != 0) {
    return err.error("unhexlify: input has odd length");
  }
  ret.resize(input.size() / 2);
  size_t j = 0;
  for (size_t i = 0; i < input.size(); i += 2) {
    uint8_t highBits = kHexTable[static_cast<uint8_t>(input[i])];
    uint8_t lowBits = kHexTable[static_cast<uint8_t>(input[i + 1])];
    if ((highBits | lowBits) & 0x10) {
      return err.error("unhexlify: input contains non-hex character");
    }
    ret[j++] = static_cast<char>((highBits << 4) + lowBits);
  }
  return Status::Success;
}

} // namespace fizz
