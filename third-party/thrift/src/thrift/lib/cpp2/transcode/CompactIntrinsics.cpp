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

#include <thrift/lib/cpp2/transcode/CompactIntrinsics.h>

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace wire = apache::thrift::transcode::wire;
using apache::thrift::transcode::cursorCanRead;
using apache::thrift::transcode::setCursorError;

namespace {

constexpr int64_t kMalformedCompact = 1;
constexpr size_t kMaxCompactSkipDepth = 64;

bool skipBytes(TranscodeCursor* cursor, size_t len) {
  if (!cursorCanRead(cursor, len)) {
    return false;
  }
  cursor->readPos += len;
  return true;
}

bool readCount(TranscodeCursor* cursor, int32_t* count) {
  uint64_t raw = thrift_transcode_read_unsigned_varint(cursor);
  if (cursor->error != 0) {
    return false;
  }
  if (raw > static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
    setCursorError(cursor, kMalformedCompact);
    return false;
  }
  *count = static_cast<int32_t>(raw);
  return true;
}

bool setFieldId(
    TranscodeCursor* cursor, int16_t* fieldId, int64_t decodedFieldId) {
  if (decodedFieldId < std::numeric_limits<int16_t>::min() ||
      decodedFieldId > std::numeric_limits<int16_t>::max()) {
    *fieldId = 0;
    setCursorError(cursor, kMalformedCompact);
    return false;
  }
  *fieldId = static_cast<int16_t>(decodedFieldId);
  return true;
}

bool readCollectionHeader(
    TranscodeCursor* cursor, uint8_t* elemType, int32_t* count) {
  uint8_t byte = thrift_transcode_read_byte_checked(cursor);
  if (cursor->error != 0) {
    return false;
  }

  *elemType = byte & 0x0F;
  *count = (byte >> 4) & 0x0F;
  if (*count == 0x0F) {
    return readCount(cursor, count);
  }
  return true;
}

bool readMapHeader(
    TranscodeCursor* cursor,
    uint8_t* keyType,
    uint8_t* valType,
    int32_t* count) {
  if (!readCount(cursor, count)) {
    return false;
  }
  if (*count == 0) {
    *keyType = 0;
    *valType = 0;
    return true;
  }

  uint8_t byte = thrift_transcode_read_byte_checked(cursor);
  if (cursor->error != 0) {
    return false;
  }
  *keyType = byte >> 4;
  *valType = byte & 0x0F;
  return true;
}

void skipCompactValue(
    TranscodeCursor* cursor,
    uint8_t ttype,
    size_t depth,
    bool boolValueInFieldHeader) {
  if (cursor->error != 0) {
    return;
  }

  switch (ttype) {
    case wire::kCompactBooleanTrue:
    case wire::kCompactBooleanFalse:
      if (!boolValueInFieldHeader) {
        skipBytes(cursor, 1);
      }
      return;
    case wire::kCompactByte:
      skipBytes(cursor, 1);
      return;
    case wire::kCompactI16:
    case wire::kCompactI32:
    case wire::kCompactI64:
      thrift_transcode_read_unsigned_varint(cursor);
      return;
    case wire::kCompactDouble:
      skipBytes(cursor, 8);
      return;
    case wire::kCompactFloat:
      skipBytes(cursor, 4);
      return;
    case wire::kCompactBinary: {
      size_t len = 0;
      thrift_transcode_read_varint_prefixed(cursor, &len);
      return;
    }
    case wire::kCompactStruct: {
      if (depth >= kMaxCompactSkipDepth) {
        setCursorError(cursor, kMalformedCompact);
        return;
      }
      int16_t prevFieldId = 0;
      while (cursor->error == 0) {
        int16_t fieldId = 0;
        uint8_t fieldType = thrift_transcode_compact_read_field_header(
            cursor, &fieldId, prevFieldId);
        if (cursor->error != 0) {
          return;
        }
        if (fieldType == wire::kCompactStop) {
          return;
        }
        prevFieldId = fieldId;
        skipCompactValue(
            cursor, fieldType, depth + 1, /*boolValueInFieldHeader=*/true);
      }
      return;
    }
    case wire::kCompactList:
    case wire::kCompactSet: {
      if (depth >= kMaxCompactSkipDepth) {
        setCursorError(cursor, kMalformedCompact);
        return;
      }
      uint8_t elemType = 0;
      int32_t count = 0;
      if (!readCollectionHeader(cursor, &elemType, &count)) {
        return;
      }
      for (int32_t i = 0; i < count && cursor->error == 0; ++i) {
        skipCompactValue(
            cursor, elemType, depth + 1, /*boolValueInFieldHeader=*/false);
      }
      return;
    }
    case wire::kCompactMap: {
      if (depth >= kMaxCompactSkipDepth) {
        setCursorError(cursor, kMalformedCompact);
        return;
      }
      uint8_t keyType = 0;
      uint8_t valType = 0;
      int32_t count = 0;
      if (!readMapHeader(cursor, &keyType, &valType, &count)) {
        return;
      }
      for (int32_t i = 0; i < count && cursor->error == 0; ++i) {
        skipCompactValue(
            cursor, keyType, depth + 1, /*boolValueInFieldHeader=*/false);
        if (cursor->error != 0) {
          return;
        }
        skipCompactValue(
            cursor, valType, depth + 1, /*boolValueInFieldHeader=*/false);
      }
      return;
    }
    default:
      setCursorError(cursor, kMalformedCompact);
      return;
  }
}

} // namespace

extern "C" {

uint8_t thrift_transcode_compact_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t prevFieldId) {
  uint8_t byte = thrift_transcode_read_byte_checked(cursor);
  if (cursor->error != 0) {
    *fieldId = 0;
    return 0;
  }
  if (byte == wire::kCompactStop) {
    *fieldId = 0;
    return 0;
  }

  uint8_t ttype = byte & 0x0F;
  int16_t delta = (byte >> 4) & 0x0F;

  if (delta != 0) {
    if (ttype == wire::kCompactStop) {
      setCursorError(cursor, kMalformedCompact);
      *fieldId = 0;
      return 0;
    }
    if (!setFieldId(
            cursor, fieldId, static_cast<int32_t>(prevFieldId) + delta)) {
      return 0;
    }
  } else {
    int64_t decodedFieldId = thrift_transcode_read_zigzag_varint(cursor);
    if (cursor->error != 0 || !setFieldId(cursor, fieldId, decodedFieldId)) {
      *fieldId = 0;
      return 0;
    }
  }

  return ttype;
}

void thrift_transcode_compact_write_field_header(
    TranscodeCursor* cursor,
    uint8_t ttype,
    int16_t fieldId,
    int16_t prevFieldId) {
  int32_t delta =
      static_cast<int32_t>(fieldId) - static_cast<int32_t>(prevFieldId);
  if (delta > 0 && delta <= 15) {
    thrift_transcode_write_byte_checked(
        cursor, static_cast<uint8_t>((delta << 4) | ttype));
  } else {
    thrift_transcode_write_byte_checked(cursor, ttype);
    thrift_transcode_write_zigzag_varint(cursor, fieldId);
  }
}

void thrift_transcode_compact_write_stop(TranscodeCursor* cursor) {
  thrift_transcode_write_byte_checked(cursor, wire::kCompactStop);
}

void thrift_transcode_compact_write_bool_field(
    TranscodeCursor* cursor,
    uint8_t boolValue,
    int16_t fieldId,
    int16_t prevFieldId) {
  uint8_t ttype =
      boolValue ? wire::kCompactBooleanTrue : wire::kCompactBooleanFalse;
  thrift_transcode_compact_write_field_header(
      cursor, ttype, fieldId, prevFieldId);
}

void thrift_transcode_compact_skip_field(
    TranscodeCursor* cursor, uint8_t ttype) {
  skipCompactValue(cursor, ttype, 0, /*boolValueInFieldHeader=*/true);
}

} // extern "C"
