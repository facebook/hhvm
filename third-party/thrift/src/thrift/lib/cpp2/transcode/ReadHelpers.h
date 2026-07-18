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

#pragma once

#include <thrift/lib/cpp2/transcode/Codec.h>
#include <thrift/lib/cpp2/transcode/Intrinsics.h>

#include <folly/CPortability.h>
#include <folly/lang/Assume.h>

#include <algorithm>
#include <cstring>
#include <limits>

namespace apache::thrift::transcode {

inline constexpr int64_t kMalformedContainerHeader = 1;

FOLLY_ALWAYS_INLINE bool checkedContainerCount(
    TranscodeCursor* c, uint64_t rawCount, uint32_t* count) {
  if (c->error != 0) {
    return false;
  }
  if (rawCount > static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
    setCursorError(c, kMalformedContainerHeader);
    return false;
  }
  *count = static_cast<uint32_t>(rawCount);
  return true;
}

FOLLY_ALWAYS_INLINE bool checkedContainerTypeInfo(
    TranscodeCursor* c, uint8_t actualTypeInfo, uint8_t expectedTypeInfo) {
  if (actualTypeInfo == expectedTypeInfo) {
    return true;
  }
  setCursorError(c, kMalformedContainerHeader);
  return false;
}

FOLLY_ALWAYS_INLINE bool readFnIsBytes(ReadFn fn) {
  return fn == ReadFn::LengthPrefixedVarint || fn == ReadFn::LengthPrefixedI32;
}

FOLLY_ALWAYS_INLINE bool readScalarInt(
    TranscodeCursor* c,
    const ScalarOp& op,
    uint8_t fieldTypeInfo,
    int64_t* value) {
  switch (op.readFn) {
    case ReadFn::ZigzagVarint:
      *value = thrift_transcode_read_zigzag_varint(c);
      break;
    case ReadFn::UnsignedVarint:
      *value = static_cast<int64_t>(thrift_transcode_read_unsigned_varint(c));
      break;
    case ReadFn::Fixed8:
      *value = static_cast<int8_t>(thrift_transcode_read_byte_checked(c));
      break;
    case ReadFn::Fixed16BE:
      *value =
          static_cast<int16_t>(thrift_transcode_read_fixed16_be_checked(c));
      break;
    case ReadFn::Fixed32BE:
      *value =
          static_cast<int32_t>(thrift_transcode_read_fixed32_be_checked(c));
      break;
    case ReadFn::Fixed64BE:
      *value =
          static_cast<int64_t>(thrift_transcode_read_fixed64_be_checked(c));
      break;
    case ReadFn::Fixed32LE:
      *value =
          static_cast<int32_t>(thrift_transcode_read_fixed32_le_checked(c));
      break;
    case ReadFn::Fixed64LE:
      *value =
          static_cast<int64_t>(thrift_transcode_read_fixed64_le_checked(c));
      break;
    case ReadFn::CompactBoolFromType:
      *value = fieldTypeInfo == 1 ? 1 : 0;
      break;
    case ReadFn::ByteAsBool:
      *value = thrift_transcode_read_byte_checked(c) != 0 ? 1 : 0;
      break;
    case ReadFn::VarintAsBool:
      *value = thrift_transcode_read_unsigned_varint(c) != 0 ? 1 : 0;
      break;
    case ReadFn::ParseDecimalInt:
      *value = thrift_transcode_parse_decimal_int(c);
      break;
    case ReadFn::ParseBoolKeyword:
      *value = thrift_transcode_parse_bool_keyword(c);
      break;
    case ReadFn::LengthPrefixedVarint:
    case ReadFn::LengthPrefixedI32:
    case ReadFn::ParseEnumNameOrDecimalText:
    case ReadFn::ParseDecimalFloat:
    case ReadFn::ParseQuotedString:
    case ReadFn::ParseBase64String:
    case ReadFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

FOLLY_ALWAYS_INLINE bool readScalarFloat(
    TranscodeCursor* c, ReadFn fn, double* value) {
  auto b2d = [](uint64_t bits) {
    double d;
    std::memcpy(&d, &bits, sizeof(d));
    return d;
  };
  auto b2f = [](uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
  };
  switch (fn) {
    case ReadFn::Fixed64LE:
      *value = b2d(thrift_transcode_read_fixed64_le_checked(c));
      break;
    case ReadFn::Fixed64BE:
      *value = b2d(thrift_transcode_read_fixed64_be_checked(c));
      break;
    case ReadFn::Fixed32LE:
      *value = b2f(thrift_transcode_read_fixed32_le_checked(c));
      break;
    case ReadFn::Fixed32BE:
      *value = b2f(thrift_transcode_read_fixed32_be_checked(c));
      break;
    case ReadFn::ParseDecimalFloat:
      *value = thrift_transcode_parse_decimal_float(c);
      break;
    case ReadFn::ZigzagVarint:
    case ReadFn::UnsignedVarint:
    case ReadFn::Fixed8:
    case ReadFn::Fixed16BE:
    case ReadFn::LengthPrefixedVarint:
    case ReadFn::LengthPrefixedI32:
    case ReadFn::CompactBoolFromType:
    case ReadFn::ByteAsBool:
    case ReadFn::VarintAsBool:
    case ReadFn::ParseDecimalInt:
    case ReadFn::ParseEnumNameOrDecimalText:
    case ReadFn::ParseQuotedString:
    case ReadFn::ParseBase64String:
    case ReadFn::ParseBoolKeyword:
    case ReadFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

FOLLY_ALWAYS_INLINE bool readScalarBytes(
    TranscodeCursor* c, ReadFn fn, const uint8_t** data, size_t* len) {
  switch (fn) {
    case ReadFn::LengthPrefixedVarint:
      *data = thrift_transcode_read_varint_prefixed(c, len);
      break;
    case ReadFn::LengthPrefixedI32:
      *data = thrift_transcode_read_i32_prefixed(c, len);
      break;
    case ReadFn::ZigzagVarint:
    case ReadFn::UnsignedVarint:
    case ReadFn::Fixed8:
    case ReadFn::Fixed16BE:
    case ReadFn::Fixed32BE:
    case ReadFn::Fixed64BE:
    case ReadFn::Fixed32LE:
    case ReadFn::Fixed64LE:
    case ReadFn::CompactBoolFromType:
    case ReadFn::ByteAsBool:
    case ReadFn::VarintAsBool:
    case ReadFn::ParseDecimalInt:
    case ReadFn::ParseEnumNameOrDecimalText:
    case ReadFn::ParseDecimalFloat:
    case ReadFn::ParseQuotedString:
    case ReadFn::ParseBase64String:
    case ReadFn::ParseBoolKeyword:
    case ReadFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

FOLLY_ALWAYS_INLINE uint32_t readSeqCount(
    TranscodeCursor* c, ContainerFraming framing, uint8_t expectedElemType) {
  switch (framing) {
    case ContainerFraming::Compact: {
      uint8_t b = thrift_transcode_read_byte_checked(c);
      if (c->error != 0) {
        return 0;
      }
      if (!checkedContainerTypeInfo(
              c, static_cast<uint8_t>(b & 0x0F), expectedElemType)) {
        return 0;
      }
      uint32_t count = static_cast<uint32_t>(b >> 4);
      if (count == 15) {
        uint64_t rawCount = thrift_transcode_read_unsigned_varint(c);
        if (!checkedContainerCount(c, rawCount, &count)) {
          return 0;
        }
      }
      return count;
    }
    case ContainerFraming::Binary: {
      if (!cursorCanRead(c, 5)) {
        return 0;
      }
      uint8_t actualElemType = thrift_transcode_read_byte_unchecked(c);
      if (!checkedContainerTypeInfo(c, actualElemType, expectedElemType)) {
        return 0;
      }
      uint32_t count = 0;
      checkedContainerCount(
          c, thrift_transcode_read_fixed32_be_unchecked(c), &count);
      return count;
    }
    case ContainerFraming::Json:
    case ContainerFraming::None:
      folly::assume_unreachable();
  }
}

FOLLY_ALWAYS_INLINE uint32_t readMapCount(
    TranscodeCursor* c,
    ContainerFraming framing,
    uint8_t expectedKeyType,
    uint8_t expectedValueType) {
  switch (framing) {
    case ContainerFraming::Compact: {
      uint32_t count = 0;
      uint64_t rawCount = thrift_transcode_read_unsigned_varint(c);
      if (!checkedContainerCount(c, rawCount, &count)) {
        return 0;
      }
      if (count > 0) {
        uint8_t typeInfo = thrift_transcode_read_byte_checked(c);
        if (c->error != 0) {
          return 0;
        }
        if (!checkedContainerTypeInfo(
                c, static_cast<uint8_t>(typeInfo >> 4), expectedKeyType)) {
          return 0;
        }
        if (!checkedContainerTypeInfo(
                c, static_cast<uint8_t>(typeInfo & 0x0F), expectedValueType)) {
          return 0;
        }
      }
      return count;
    }
    case ContainerFraming::Binary: {
      if (!cursorCanRead(c, 6)) {
        return 0;
      }
      uint8_t actualKeyType = thrift_transcode_read_byte_unchecked(c);
      uint8_t actualValueType = thrift_transcode_read_byte_unchecked(c);
      if (!checkedContainerTypeInfo(c, actualKeyType, expectedKeyType)) {
        return 0;
      }
      if (!checkedContainerTypeInfo(c, actualValueType, expectedValueType)) {
        return 0;
      }
      uint32_t count = 0;
      checkedContainerCount(
          c, thrift_transcode_read_fixed32_be_unchecked(c), &count);
      return count;
    }
    case ContainerFraming::Json:
    case ContainerFraming::None:
      folly::assume_unreachable();
  }
}

inline const FieldEntry* findField(const StructOp& op, int16_t fieldId) {
  auto it = std::lower_bound(
      op.fields.begin(),
      op.fields.end(),
      fieldId,
      [](const FieldEntry& field, int16_t id) { return field.fieldId < id; });
  if (it == op.fields.end() || it->fieldId != fieldId) {
    return nullptr;
  }
  return &*it;
}

} // namespace apache::thrift::transcode
