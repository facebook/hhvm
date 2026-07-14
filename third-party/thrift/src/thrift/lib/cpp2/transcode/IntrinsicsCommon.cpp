/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>

#include <folly/lang/Bits.h>

#include <cstring>
#include <limits>

using apache::thrift::transcode::cursorCanRead;
using apache::thrift::transcode::setCursorError;

namespace {

constexpr int64_t kReadOverrun = 1;
constexpr int64_t kVarintTooLong = 3;
constexpr int64_t kLengthOverflow = 4;

size_t unsignedVarintSize(uint64_t value) {
  size_t size = 1;
  while (value >= 0x80) {
    value >>= 7;
    ++size;
  }
  return size;
}

bool wireLengthToSize(TranscodeCursor* cursor, uint64_t wireLen, size_t* len) {
  if constexpr (
      std::numeric_limits<size_t>::digits <
      std::numeric_limits<uint64_t>::digits) {
    if (wireLen > static_cast<uint64_t>(std::numeric_limits<size_t>::max())) {
      setCursorError(cursor, kLengthOverflow);
      *len = 0;
      return false;
    }
  }
  *len = static_cast<size_t>(wireLen);
  return true;
}

bool sizeToWireLength(TranscodeCursor* cursor, size_t len, uint64_t* wireLen) {
  if constexpr (
      std::numeric_limits<size_t>::digits >
      std::numeric_limits<uint64_t>::digits) {
    if (len > static_cast<size_t>(std::numeric_limits<uint64_t>::max())) {
      setCursorError(cursor, kLengthOverflow);
      *wireLen = 0;
      return false;
    }
  }
  *wireLen = static_cast<uint64_t>(len);
  return true;
}

} // namespace

extern "C" {

uint64_t thrift_transcode_read_unsigned_varint(TranscodeCursor* cursor) {
  if (cursor->error != 0) {
    return 0;
  }
  uint64_t result = 0;
  int shift = 0;
  while (cursor->readPos < cursor->readEnd) {
    uint8_t b = *cursor->readPos++;
    if (shift == 63 && (b & 0xFE) != 0) {
      setCursorError(cursor, kVarintTooLong);
      return 0;
    }
    result |= static_cast<uint64_t>(b & 0x7F) << shift;
    if ((b & 0x80) == 0) {
      return result;
    }
    shift += 7;
    if (shift > 63) {
      setCursorError(cursor, kVarintTooLong);
      return 0;
    }
  }
  setCursorError(cursor, kReadOverrun);
  return 0;
}

int64_t thrift_transcode_read_zigzag_varint(TranscodeCursor* cursor) {
  uint64_t n = thrift_transcode_read_unsigned_varint(cursor);
  return static_cast<int64_t>((n >> 1) ^ -(n & 1));
}

void thrift_transcode_write_unsigned_varint(
    TranscodeCursor* cursor, uint64_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, unsignedVarintSize(value));
  if (cursor->error != 0) {
    return;
  }
  while (value >= 0x80) {
    *cursor->writePos++ = static_cast<uint8_t>((value & 0x7F) | 0x80);
    value >>= 7;
  }
  *cursor->writePos++ = static_cast<uint8_t>(value);
}

void thrift_transcode_write_zigzag_varint(
    TranscodeCursor* cursor, int64_t value) {
  uint64_t n = (static_cast<uint64_t>(value) << 1) ^
      (static_cast<uint64_t>(value >> 63));
  thrift_transcode_write_unsigned_varint(cursor, n);
}

uint8_t thrift_transcode_read_byte(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 1)) {
    return 0;
  }
  return *cursor->readPos++;
}

void thrift_transcode_write_byte(TranscodeCursor* cursor, uint8_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 1);
  if (cursor->error != 0) {
    return;
  }
  *cursor->writePos++ = value;
}

uint32_t thrift_transcode_read_fixed32_le(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 4)) {
    return 0;
  }
  uint32_t val;
  memcpy(&val, cursor->readPos, 4);
  cursor->readPos += 4;
  return folly::Endian::little(val);
}

uint64_t thrift_transcode_read_fixed64_le(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 8)) {
    return 0;
  }
  uint64_t val;
  memcpy(&val, cursor->readPos, 8);
  cursor->readPos += 8;
  return folly::Endian::little(val);
}

void thrift_transcode_write_fixed32_le(
    TranscodeCursor* cursor, uint32_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 4);
  if (cursor->error != 0) {
    return;
  }
  value = folly::Endian::little(value);
  memcpy(cursor->writePos, &value, 4);
  cursor->writePos += 4;
}

void thrift_transcode_write_fixed64_le(
    TranscodeCursor* cursor, uint64_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 8);
  if (cursor->error != 0) {
    return;
  }
  value = folly::Endian::little(value);
  memcpy(cursor->writePos, &value, 8);
  cursor->writePos += 8;
}

uint16_t thrift_transcode_read_fixed16_be(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 2)) {
    return 0;
  }
  uint16_t val;
  memcpy(&val, cursor->readPos, 2);
  cursor->readPos += 2;
  return folly::Endian::big(val);
}

uint32_t thrift_transcode_read_fixed32_be(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 4)) {
    return 0;
  }
  uint32_t val;
  memcpy(&val, cursor->readPos, 4);
  cursor->readPos += 4;
  return folly::Endian::big(val);
}

uint64_t thrift_transcode_read_fixed64_be(TranscodeCursor* cursor) {
  if (!cursorCanRead(cursor, 8)) {
    return 0;
  }
  uint64_t val;
  memcpy(&val, cursor->readPos, 8);
  cursor->readPos += 8;
  return folly::Endian::big(val);
}

void thrift_transcode_write_fixed16_be(
    TranscodeCursor* cursor, uint16_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 2);
  if (cursor->error != 0) {
    return;
  }
  value = folly::Endian::big(value);
  memcpy(cursor->writePos, &value, 2);
  cursor->writePos += 2;
}

void thrift_transcode_write_fixed32_be(
    TranscodeCursor* cursor, uint32_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 4);
  if (cursor->error != 0) {
    return;
  }
  value = folly::Endian::big(value);
  memcpy(cursor->writePos, &value, 4);
  cursor->writePos += 4;
}

void thrift_transcode_write_fixed64_be(
    TranscodeCursor* cursor, uint64_t value) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, 8);
  if (cursor->error != 0) {
    return;
  }
  value = folly::Endian::big(value);
  memcpy(cursor->writePos, &value, 8);
  cursor->writePos += 8;
}

const uint8_t* thrift_transcode_read_varint_prefixed(
    TranscodeCursor* cursor, size_t* len) {
  uint64_t wireLen = thrift_transcode_read_unsigned_varint(cursor);
  if (cursor->error != 0) {
    *len = 0;
    return cursor->readPos;
  }
  if (!wireLengthToSize(cursor, wireLen, len)) {
    return cursor->readPos;
  }
  if (!cursorCanRead(cursor, *len)) {
    *len = 0;
    return cursor->readPos;
  }
  const uint8_t* ptr = cursor->readPos;
  cursor->readPos += *len;
  return ptr;
}

const uint8_t* thrift_transcode_read_i32_prefixed(
    TranscodeCursor* cursor, size_t* len) {
  uint32_t rawLen = thrift_transcode_read_fixed32_be(cursor);
  if (cursor->error != 0) {
    *len = 0;
    return cursor->readPos;
  }
  if (rawLen > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
    setCursorError(cursor, kLengthOverflow);
    *len = 0;
    return cursor->readPos;
  }
  *len = static_cast<size_t>(rawLen);
  if (!cursorCanRead(cursor, *len)) {
    *len = 0;
    return cursor->readPos;
  }
  const uint8_t* ptr = cursor->readPos;
  cursor->readPos += *len;
  return ptr;
}

void thrift_transcode_write_varint_prefixed(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  if (cursor->error != 0) {
    return;
  }
  uint64_t wireLen = 0;
  if (!sizeToWireLength(cursor, len, &wireLen)) {
    return;
  }
  thrift_transcode_write_unsigned_varint(cursor, wireLen);
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, len);
  if (cursor->error != 0) {
    return;
  }
  memcpy(cursor->writePos, data, len);
  cursor->writePos += len;
}

void thrift_transcode_write_i32_prefixed(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  if (cursor->error != 0) {
    return;
  }
  if (len > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
    setCursorError(cursor, kLengthOverflow);
    return;
  }
  thrift_transcode_write_fixed32_be(cursor, static_cast<uint32_t>(len));
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, len);
  if (cursor->error != 0) {
    return;
  }
  memcpy(cursor->writePos, data, len);
  cursor->writePos += len;
}

void thrift_transcode_write_raw_bytes(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, len);
  if (cursor->error != 0) {
    return;
  }
  memcpy(cursor->writePos, data, len);
  cursor->writePos += len;
}

} // extern "C"
