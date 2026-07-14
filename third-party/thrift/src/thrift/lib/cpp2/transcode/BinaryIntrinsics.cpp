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

#include <thrift/lib/cpp2/transcode/BinaryIntrinsics.h>

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace wire = apache::thrift::transcode::wire;
using apache::thrift::transcode::cursorCanRead;
using apache::thrift::transcode::setCursorError;

namespace {

constexpr int64_t kMalformedBinary = 1;
constexpr size_t kMaxBinarySkipDepth = 64;

bool skipBytes(TranscodeCursor* cursor, size_t len) {
  if (!cursorCanRead(cursor, len)) {
    return false;
  }
  cursor->readPos += len;
  return true;
}

bool readNonNegativeI32(TranscodeCursor* cursor, int32_t* value) {
  uint32_t raw = thrift_transcode_read_fixed32_be_checked(cursor);
  if (cursor->error != 0) {
    return false;
  }
  if (raw > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
    setCursorError(cursor, kMalformedBinary);
    return false;
  }
  *value = static_cast<int32_t>(raw);
  return true;
}

void skipBinaryField(TranscodeCursor* cursor, uint8_t ttype, size_t depth) {
  if (cursor->error != 0) {
    return;
  }

  switch (ttype) {
    case wire::kBinaryBool:
    case wire::kBinaryByte:
      skipBytes(cursor, 1);
      return;
    case wire::kBinaryI16:
      skipBytes(cursor, 2);
      return;
    case wire::kBinaryI32:
    case wire::kBinaryFloat:
      skipBytes(cursor, 4);
      return;
    case wire::kBinaryI64:
    case wire::kBinaryDouble:
      skipBytes(cursor, 8);
      return;
    case wire::kBinaryString: {
      int32_t len = 0;
      if (!readNonNegativeI32(cursor, &len)) {
        return;
      }
      skipBytes(cursor, static_cast<size_t>(len));
      return;
    }
    case wire::kBinaryStruct: {
      if (depth >= kMaxBinarySkipDepth) {
        setCursorError(cursor, kMalformedBinary);
        return;
      }
      while (cursor->error == 0) {
        int16_t fid = 0;
        uint8_t ft = thrift_transcode_binary_read_field_header(cursor, &fid, 0);
        if (cursor->error != 0) {
          return;
        }
        if (ft == wire::kBinaryStop) {
          return;
        }
        skipBinaryField(cursor, ft, depth + 1);
      }
      return;
    }
    case wire::kBinaryList:
    case wire::kBinarySet: {
      if (!cursorCanRead(cursor, 5)) {
        setCursorError(cursor, kMalformedBinary);
        return;
      }
      uint8_t elemType = *cursor->readPos++;
      int32_t count = 0;
      if (!readNonNegativeI32(cursor, &count)) {
        return;
      }
      for (int32_t i = 0; i < count && cursor->error == 0; ++i) {
        skipBinaryField(cursor, elemType, depth);
      }
      return;
    }
    case wire::kBinaryMap: {
      if (!cursorCanRead(cursor, 6)) {
        setCursorError(cursor, kMalformedBinary);
        return;
      }
      uint8_t keyType = *cursor->readPos++;
      uint8_t valType = *cursor->readPos++;
      int32_t count = 0;
      if (!readNonNegativeI32(cursor, &count)) {
        return;
      }
      for (int32_t i = 0; i < count && cursor->error == 0; ++i) {
        skipBinaryField(cursor, keyType, depth);
        if (cursor->error != 0) {
          return;
        }
        skipBinaryField(cursor, valType, depth);
      }
      return;
    }
    default:
      setCursorError(cursor, kMalformedBinary);
      return;
  }
}

} // namespace

extern "C" {

uint8_t thrift_transcode_binary_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t /*prevFieldId*/) {
  if (!cursorCanRead(cursor, 1)) {
    *fieldId = 0;
    return 0;
  }
  uint8_t ttype = *cursor->readPos++;
  if (ttype == wire::kBinaryStop) {
    *fieldId = 0;
    return 0;
  }
  if (!cursorCanRead(cursor, 2)) {
    setCursorError(cursor, kMalformedBinary);
    *fieldId = 0;
    return 0;
  }
  *fieldId =
      static_cast<int16_t>(thrift_transcode_read_fixed16_be_unchecked(cursor));
  return ttype;
}

void thrift_transcode_binary_write_field_header(
    TranscodeCursor* cursor,
    uint8_t ttype,
    int16_t fieldId,
    int16_t /*prevFieldId*/) {
  thrift_transcode_cursor_ensure_write(cursor, 3);
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_write_byte_unchecked(cursor, ttype);
  thrift_transcode_write_fixed16_be_unchecked(
      cursor, static_cast<uint16_t>(fieldId));
}

void thrift_transcode_binary_write_stop(TranscodeCursor* cursor) {
  thrift_transcode_write_byte_checked(cursor, wire::kBinaryStop);
}

void thrift_transcode_binary_skip_field(
    TranscodeCursor* cursor, uint8_t ttype) {
  skipBinaryField(cursor, ttype, 0);
}

} // extern "C"
