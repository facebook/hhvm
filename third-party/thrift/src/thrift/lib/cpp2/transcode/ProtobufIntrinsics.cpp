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

#include <thrift/lib/cpp2/transcode/ProtobufIntrinsics.h>

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>

#include <limits>

using apache::thrift::transcode::setCursorError;

namespace {

constexpr int64_t kMalformedProtobuf = 1;
constexpr uint8_t PB_WIRE_VARINT = 0;
constexpr uint8_t PB_WIRE_64BIT = 1;
constexpr uint8_t PB_WIRE_LENGTH_DELIMITED = 2;
constexpr uint8_t PB_WIRE_32BIT = 5;

bool isSupportedWireType(uint8_t wireType) {
  switch (wireType) {
    case PB_WIRE_VARINT:
    case PB_WIRE_64BIT:
    case PB_WIRE_LENGTH_DELIMITED:
    case PB_WIRE_32BIT:
      return true;
    default:
      return false;
  }
}

} // namespace

extern "C" {

uint8_t thrift_transcode_proto_read_field_header(
    TranscodeCursor* cursor, int16_t* fieldId, int16_t /*prevFieldId*/) {
  if (cursor == nullptr) {
    return 0;
  }
  if (fieldId == nullptr) {
    setCursorError(cursor, kMalformedProtobuf);
    return 0;
  }
  if (cursor->error != 0) {
    *fieldId = 0;
    return 0;
  }
  if (cursor->readPos >= cursor->readEnd) {
    *fieldId = 0;
    return 0; // end of message
  }
  uint64_t tag = thrift_transcode_read_unsigned_varint(cursor);
  if (cursor->error != 0) {
    *fieldId = 0;
    return 0;
  }
  const uint8_t wireType = static_cast<uint8_t>(tag & 0x07);
  const uint64_t fieldNumber = tag >> 3;
  if (fieldNumber == 0 ||
      fieldNumber >
          static_cast<uint64_t>(std::numeric_limits<int16_t>::max()) ||
      !isSupportedWireType(wireType)) {
    *fieldId = 0;
    setCursorError(cursor, kMalformedProtobuf);
    return 0;
  }
  *fieldId = static_cast<int16_t>(fieldNumber);
  // Offset wire type by 1 so 0 is reserved for "end of message".
  // The codegen only checks for 0 = stop, the actual wire type value is not
  // used for dispatch (field ID is used instead).
  return static_cast<uint8_t>(wireType + 1);
}

void thrift_transcode_proto_write_field_header(
    TranscodeCursor* cursor,
    uint8_t typeInfo,
    int16_t fieldId,
    int16_t /*prevFieldId*/) {
  if (cursor == nullptr) {
    return;
  }
  if (cursor->error != 0) {
    return;
  }
  if (typeInfo == 0 || fieldId <= 0) {
    setCursorError(cursor, kMalformedProtobuf);
    return;
  }
  // typeInfo is wire_type + 1 (0 reserved for stop). Subtract 1 to get real
  // wire type for the tag.
  uint8_t wireType = typeInfo - 1;
  if (!isSupportedWireType(wireType)) {
    setCursorError(cursor, kMalformedProtobuf);
    return;
  }
  thrift_transcode_write_unsigned_varint(
      cursor, (static_cast<uint64_t>(fieldId) << 3) | wireType);
}

void thrift_transcode_proto_write_stop(TranscodeCursor* /*cursor*/) {
  // Protobuf messages don't have stop markers — no-op
}

void thrift_transcode_proto_skip_field(
    TranscodeCursor* cursor, uint8_t typeInfo) {
  if (cursor == nullptr) {
    return;
  }
  if (cursor->error != 0) {
    return;
  }
  if (typeInfo == 0) {
    setCursorError(cursor, kMalformedProtobuf);
    return;
  }
  // typeInfo is offset by 1 (0 = stop). Subtract 1 to get real wire type.
  uint8_t wireType = typeInfo - 1;
  switch (wireType) {
    case PB_WIRE_VARINT:
      thrift_transcode_read_unsigned_varint(cursor);
      break;
    case PB_WIRE_64BIT:
      thrift_transcode_read_fixed64_le_checked(cursor);
      break;
    case PB_WIRE_LENGTH_DELIMITED: {
      size_t len = 0;
      thrift_transcode_read_varint_prefixed(cursor, &len);
      break;
    }
    case PB_WIRE_32BIT:
      thrift_transcode_read_fixed32_le_checked(cursor);
      break;
    default:
      setCursorError(cursor, kMalformedProtobuf);
      break;
  }
}

} // extern "C"
