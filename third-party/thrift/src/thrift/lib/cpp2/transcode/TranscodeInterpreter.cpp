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

#include <thrift/lib/cpp2/transcode/TranscodeInterpreter.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/transcode/ReadHelpers.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

#include <folly/CppAttributes.h>
#include <folly/ScopeGuard.h>
#include <folly/lang/Assume.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <variant>
#include <vector>

namespace apache::thrift::transcode {

namespace {

namespace wire = apache::thrift::transcode::wire;

constexpr int64_t kMalformedFieldType = 1;
constexpr int64_t kUnsupportedProtocol = 90;

// Interim mapping from the numeric error codes the intrinsics latch onto the
// cursor to the TranscodeErrc taxonomy. kUnsupportedProtocol is the
// interpreter's own unsupported-protocol code.
// TODO(D4a): canonicalize intrinsic codes so this becomes a direct cast.
TranscodeErrc errcFromCursor(int64_t code) {
  switch (code) {
    case 0:
      return TranscodeErrc::Ok;
    case kUnsupportedProtocol:
      return TranscodeErrc::Unsupported;
    default:
      return TranscodeErrc::Malformed;
  }
}

struct MallocOutput {
  uint8_t* buffer = nullptr;
  size_t capacity = 0;
};

bool checkedAdd(size_t lhs, size_t rhs, size_t& out) {
  if (lhs > std::numeric_limits<size_t>::max() - rhs) {
    return false;
  }
  out = lhs + rhs;
  return true;
}

TranscodeStatus mallocExtend(
    const TranscodeExtendRequest* request,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<MallocOutput*>(userData);
  const size_t written =
      static_cast<size_t>(request->writePoint - request->segment.begin);
  size_t requested = 0;
  if (!checkedAdd(written, request->minWritable, requested)) {
    return TranscodeStatus::Error;
  }
  const size_t doubled =
      output.capacity > std::numeric_limits<size_t>::max() / 2
      ? requested
      : output.capacity * 2;
  const size_t newCapacity = std::max(doubled, requested);
  auto* next = static_cast<uint8_t*>(realloc(output.buffer, newCapacity));
  if (next == nullptr) {
    return TranscodeStatus::Error;
  }
  result->kind = next == request->segment.begin
      ? TranscodeExtendKind::InPlaceExtension
      : TranscodeExtendKind::RelocatedContiguous;
  result->segment = {next, next + newCapacity};
  output.buffer = next;
  output.capacity = newCapacity;
  return TranscodeStatus::Ok;
}

TranscodeStatus fixedOutputExtend(
    const TranscodeExtendRequest* /*request*/,
    TranscodeExtendResult* /*result*/,
    void* /*userData*/) {
  return TranscodeStatus::Error;
}

uint64_t doubleToBits(double d) {
  uint64_t bits;
  std::memcpy(&bits, &d, sizeof(bits));
  return bits;
}
uint32_t floatToBits(float f) {
  uint32_t bits;
  std::memcpy(&bits, &f, sizeof(bits));
  return bits;
}

struct Framing {
  uint8_t (*readHeader)(TranscodeCursor*, int16_t*, int16_t) = nullptr;
  void (*writeHeader)(TranscodeCursor*, uint8_t, int16_t, int16_t) = nullptr;
  void (*writeStop)(TranscodeCursor*) = nullptr;
  void (*skip)(TranscodeCursor*, uint8_t) = nullptr;
};

Framing framingFor(FieldProto p) {
  switch (p) {
    case FieldProto::Compact:
      return {
          &thrift_transcode_compact_read_field_header,
          &thrift_transcode_compact_write_field_header,
          &thrift_transcode_compact_write_stop,
          &thrift_transcode_compact_skip_field};
    case FieldProto::Binary:
      return {
          &thrift_transcode_binary_read_field_header,
          &thrift_transcode_binary_write_field_header,
          &thrift_transcode_binary_write_stop,
          &thrift_transcode_binary_skip_field};
    case FieldProto::Protobuf:
      return {
          &thrift_transcode_proto_read_field_header,
          &thrift_transcode_proto_write_field_header,
          &thrift_transcode_proto_write_stop,
          &thrift_transcode_proto_skip_field};
    case FieldProto::Unsupported:
      return {};
  }
  return {};
}

void execCommand(TranscodeCursor* c, const Command& cmd, uint8_t fieldTypeInfo);
void writeDefaultCommand(TranscodeCursor* c, const Command& cmd);
void writeSeqHeader(
    TranscodeCursor* c,
    ContainerFraming framing,
    uint32_t count,
    uint8_t elemType);
void writeMapHeader(
    TranscodeCursor* c,
    ContainerFraming framing,
    uint32_t count,
    uint8_t keyType,
    uint8_t valueType);

bool fieldTypeMatches(
    FieldProto readProto, const FieldEntry& field, uint8_t actualTypeInfo) {
  if (readProto == FieldProto::Compact &&
      field.readTypeInfo == wire::kCompactBooleanTrue) {
    return actualTypeInfo == wire::kCompactBooleanTrue ||
        actualTypeInfo == wire::kCompactBooleanFalse;
  }
  return actualTypeInfo == field.readTypeInfo;
}

bool setReadEndFromByteLength(TranscodeCursor& c, uint64_t byteLen) {
  const uint8_t* parentReadEnd = c.readEnd;
  if (c.readPos > parentReadEnd ||
      byteLen > static_cast<uint64_t>(parentReadEnd - c.readPos)) {
    detail::setError(&c, kMalformedContainerHeader);
    return false;
  }
  c.readEnd = c.readPos + static_cast<size_t>(byteLen);
  return true;
}

bool intFits(ValueKind kind, int64_t v) {
  switch (kind) {
    case ValueKind::I8:
      return v >= std::numeric_limits<int8_t>::min() &&
          v <= std::numeric_limits<int8_t>::max();
    case ValueKind::I16:
      return v >= std::numeric_limits<int16_t>::min() &&
          v <= std::numeric_limits<int16_t>::max();
    case ValueKind::I32:
    case ValueKind::Enum:
      return v >= std::numeric_limits<int32_t>::min() &&
          v <= std::numeric_limits<int32_t>::max();
    case ValueKind::Bool:
    case ValueKind::I64:
      return true;
    case ValueKind::F32:
    case ValueKind::F64:
    case ValueKind::Bytes:
      folly::assume_unreachable();
  }
  return false;
}

void validateInputConsumed(TranscodeCursor* c, const TranscodePlan& plan) {
  if (c->error != 0 || plan.sourceProtocol != WireProtocol::Json) {
    return;
  }
  thrift_transcode_json_skip_whitespace(c);
  if (c->readPos != c->readEnd) {
    detail::setError(c, 1);
  }
}

bool writeScalarInt(TranscodeCursor* c, const ScalarOp& op, int64_t v) {
  switch (op.writeFn) {
    case WriteFn::ZigzagVarint:
      thrift_transcode_write_zigzag_varint(c, v);
      break;
    case WriteFn::UnsignedVarint:
      thrift_transcode_write_unsigned_varint(c, static_cast<uint64_t>(v));
      break;
    case WriteFn::Fixed8:
      thrift_transcode_write_byte_checked(c, static_cast<uint8_t>(v));
      break;
    case WriteFn::Fixed16BE:
      thrift_transcode_write_fixed16_be_checked(c, static_cast<uint16_t>(v));
      break;
    case WriteFn::Fixed32BE:
      thrift_transcode_write_fixed32_be_checked(c, static_cast<uint32_t>(v));
      break;
    case WriteFn::Fixed64BE:
      thrift_transcode_write_fixed64_be_checked(c, static_cast<uint64_t>(v));
      break;
    case WriteFn::Fixed32LE:
      thrift_transcode_write_fixed32_le_checked(c, static_cast<uint32_t>(v));
      break;
    case WriteFn::Fixed64LE:
      thrift_transcode_write_fixed64_le_checked(c, static_cast<uint64_t>(v));
      break;
    case WriteFn::ByteAsBool:
      thrift_transcode_write_byte_checked(c, v ? 1 : 0);
      break;
    case WriteFn::VarintAsBool:
      thrift_transcode_write_unsigned_varint(c, v ? 1 : 0);
      break;
    case WriteFn::IntToDecimalText:
      thrift_transcode_format_decimal_int(c, v);
      break;
    case WriteFn::EnumNameOrDecimalText:
      if (op.enumNames != nullptr) {
        if (const auto* name = op.enumNames->nameFor(static_cast<int32_t>(v))) {
          thrift_transcode_format_escaped_string(
              c, reinterpret_cast<const uint8_t*>(name->data()), name->size());
          break;
        }
      }
      thrift_transcode_format_decimal_int(c, v);
      break;
    case WriteFn::BoolToKeyword:
      if (v != 0) {
        thrift_transcode_write_raw_bytes_checked(
            c, reinterpret_cast<const uint8_t*>("true"), 4);
      } else {
        thrift_transcode_write_raw_bytes_checked(
            c, reinterpret_cast<const uint8_t*>("false"), 5);
      }
      break;
    // Not integer writers, so an integer value never routes here:
    // CompactBoolInType is folded into the field header by the StructOp
    // executor; bytes/float and struct-memory writers are dispatched by
    // writeScalarBytes / writeScalarFloat and the struct path.
    case WriteFn::CompactBoolInType:
    case WriteFn::LengthPrefixedVarint:
    case WriteFn::LengthPrefixedI32:
    case WriteFn::WriteQuotedString:
    case WriteFn::WriteBase64String:
    case WriteFn::FloatToDecimalText:
    case WriteFn::StoreAtOffset:
    case WriteFn::CallTypeInfoSet:
    case WriteFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

bool writeScalarFloat(TranscodeCursor* c, WriteFn fn, double v) {
  switch (fn) {
    case WriteFn::Fixed64LE:
      thrift_transcode_write_fixed64_le_checked(c, doubleToBits(v));
      break;
    case WriteFn::Fixed64BE:
      thrift_transcode_write_fixed64_be_checked(c, doubleToBits(v));
      break;
    case WriteFn::Fixed32LE:
      thrift_transcode_write_fixed32_le_checked(
          c, floatToBits(static_cast<float>(v)));
      break;
    case WriteFn::Fixed32BE:
      thrift_transcode_write_fixed32_be_checked(
          c, floatToBits(static_cast<float>(v)));
      break;
    case WriteFn::FloatToDecimalText:
      thrift_transcode_format_decimal_float(c, v);
      break;
    case WriteFn::ZigzagVarint:
    case WriteFn::UnsignedVarint:
    case WriteFn::Fixed8:
    case WriteFn::Fixed16BE:
    case WriteFn::LengthPrefixedVarint:
    case WriteFn::LengthPrefixedI32:
    case WriteFn::CompactBoolInType:
    case WriteFn::ByteAsBool:
    case WriteFn::VarintAsBool:
    case WriteFn::IntToDecimalText:
    case WriteFn::EnumNameOrDecimalText:
    case WriteFn::WriteQuotedString:
    case WriteFn::WriteBase64String:
    case WriteFn::BoolToKeyword:
    case WriteFn::StoreAtOffset:
    case WriteFn::CallTypeInfoSet:
    case WriteFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

bool writeScalarBytes(
    TranscodeCursor* c, WriteFn fn, const uint8_t* data, size_t len) {
  switch (fn) {
    case WriteFn::LengthPrefixedVarint:
      thrift_transcode_write_varint_prefixed(c, data, len);
      break;
    case WriteFn::LengthPrefixedI32:
      thrift_transcode_write_i32_prefixed(c, data, len);
      break;
    case WriteFn::WriteQuotedString:
      thrift_transcode_format_escaped_string(c, data, len);
      break;
    case WriteFn::WriteBase64String:
      thrift_transcode_format_base64_string(c, data, len);
      break;
    case WriteFn::ZigzagVarint:
    case WriteFn::UnsignedVarint:
    case WriteFn::Fixed8:
    case WriteFn::Fixed16BE:
    case WriteFn::Fixed32BE:
    case WriteFn::Fixed64BE:
    case WriteFn::Fixed32LE:
    case WriteFn::Fixed64LE:
    case WriteFn::CompactBoolInType:
    case WriteFn::ByteAsBool:
    case WriteFn::VarintAsBool:
    case WriteFn::IntToDecimalText:
    case WriteFn::EnumNameOrDecimalText:
    case WriteFn::FloatToDecimalText:
    case WriteFn::BoolToKeyword:
    case WriteFn::StoreAtOffset:
    case WriteFn::CallTypeInfoSet:
    case WriteFn::Custom:
      folly::assume_unreachable();
  }
  return c->error == 0;
}

void execJsonBytesScalar(TranscodeCursor* c, const ScalarOp& op) {
  TranscodeJsonStringToken token{};
  if (!thrift_transcode_read_json_string_token(c, &token)) {
    return;
  }

  if (op.readFn == ReadFn::ParseQuotedString) {
    switch (op.writeFn) {
      case WriteFn::LengthPrefixedVarint:
        thrift_transcode_write_json_string_token_varint_prefixed(c, &token);
        return;
      case WriteFn::LengthPrefixedI32:
        thrift_transcode_write_json_string_token_i32_prefixed(c, &token);
        return;
      case WriteFn::WriteQuotedString:
        thrift_transcode_write_json_string_token_quoted(c, &token);
        return;
      case WriteFn::WriteBase64String: {
        std::string bytes =
            thrift_transcode_decode_json_string_token_to_string(c, token);
        if (c->error != 0) {
          return;
        }
        thrift_transcode_format_base64_string(
            c, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());
        return;
      }
      case WriteFn::ZigzagVarint:
      case WriteFn::UnsignedVarint:
      case WriteFn::Fixed8:
      case WriteFn::Fixed16BE:
      case WriteFn::Fixed32BE:
      case WriteFn::Fixed64BE:
      case WriteFn::Fixed32LE:
      case WriteFn::Fixed64LE:
      case WriteFn::CompactBoolInType:
      case WriteFn::ByteAsBool:
      case WriteFn::VarintAsBool:
      case WriteFn::IntToDecimalText:
      case WriteFn::EnumNameOrDecimalText:
      case WriteFn::FloatToDecimalText:
      case WriteFn::BoolToKeyword:
      case WriteFn::StoreAtOffset:
      case WriteFn::CallTypeInfoSet:
      case WriteFn::Custom:
        detail::setError(c, 90);
        return;
    }
  }

  switch (op.writeFn) {
    case WriteFn::LengthPrefixedVarint:
      thrift_transcode_write_json_base64_token_varint_prefixed(c, &token);
      return;
    case WriteFn::LengthPrefixedI32:
      thrift_transcode_write_json_base64_token_i32_prefixed(c, &token);
      return;
    case WriteFn::WriteBase64String:
      thrift_transcode_write_json_string_token_quoted(c, &token);
      return;
    case WriteFn::ZigzagVarint:
    case WriteFn::UnsignedVarint:
    case WriteFn::Fixed8:
    case WriteFn::Fixed16BE:
    case WriteFn::Fixed32BE:
    case WriteFn::Fixed64BE:
    case WriteFn::Fixed32LE:
    case WriteFn::Fixed64LE:
    case WriteFn::CompactBoolInType:
    case WriteFn::ByteAsBool:
    case WriteFn::VarintAsBool:
    case WriteFn::IntToDecimalText:
    case WriteFn::EnumNameOrDecimalText:
    case WriteFn::FloatToDecimalText:
    case WriteFn::WriteQuotedString:
    case WriteFn::BoolToKeyword:
    case WriteFn::StoreAtOffset:
    case WriteFn::CallTypeInfoSet:
    case WriteFn::Custom:
      detail::setError(c, 90);
      return;
  }
}

void writeScalarDefault(TranscodeCursor* c, const ScalarOp& op) {
  if (op.valueKind == ValueKind::F32 || op.valueKind == ValueKind::F64) {
    writeScalarFloat(c, op.writeFn, 0.0);
    return;
  }
  if (op.valueKind == ValueKind::Bytes) {
    writeScalarBytes(c, op.writeFn, reinterpret_cast<const uint8_t*>(""), 0);
    return;
  }
  writeScalarInt(c, op, 0);
}

void execScalar(TranscodeCursor* c, const ScalarOp& op, uint8_t fieldTypeInfo) {
  if (readFnIsJsonBytes(op.readFn)) {
    execJsonBytesScalar(c, op);
    return;
  }
  if (readFnIsBytes(op.readFn)) {
    const uint8_t* data = nullptr;
    size_t len = 0;
    if (!readScalarBytes(c, op.readFn, &data, &len)) {
      return;
    }
    if (data == nullptr && len != 0) {
      detail::setError(c, kMalformedFieldType);
      return;
    }
    const uint8_t empty = 0;
    if (!writeScalarBytes(
            c, op.writeFn, data == nullptr ? &empty : data, len)) {
      return;
    }
    return;
  }
  if (op.valueKind == ValueKind::F32 || op.valueKind == ValueKind::F64) {
    double v = 0;
    if (!readScalarFloat(c, op.readFn, &v)) {
      return;
    }
    if (!writeScalarFloat(c, op.writeFn, v)) {
      return;
    }
    return;
  }
  int64_t v = 0;
  if (!readScalarInt(c, op, fieldTypeInfo, &v)) {
    return;
  }
  if (!intFits(op.valueKind, v)) {
    detail::setError(c, 1);
    return;
  }
  // CoerceOp is a no-op for our int64 register model (WidenI32ToI64 etc. are
  // already represented as int64); float widening is handled in the F32/F64
  // path above.
  if (!writeScalarInt(c, op, v)) {
    return;
  }
}

void writeDefaultStruct(TranscodeCursor* c, const StructOp& op) {
  FieldProto wp = op.writeFieldProto;
  if (wp == FieldProto::Unsupported) {
    detail::setError(c, 90);
    return;
  }
  Framing wf = framingFor(wp);

  TranscodePatchPoint writeMark{};
  bool patchWrite = false;
  if (op.writeLengthDelimited) {
    writeMark = thrift_transcode_cursor_mark(c);
    thrift_transcode_cursor_skip(c, 5);
    patchWrite = true;
  }

  int16_t prevWrite = 0;
  for (const auto& field : op.fields) {
    if (c->error != 0) {
      return;
    }
    if (field.optional) {
      continue;
    }
    if (const auto* sc = std::get_if<ScalarOp>(field.command.get());
        sc != nullptr && sc->writeFn == WriteFn::CompactBoolInType) {
      thrift_transcode_compact_write_bool_field(c, 0, field.fieldId, prevWrite);
      prevWrite = field.fieldId;
      continue;
    }
    wf.writeHeader(c, field.writeTypeInfo, field.fieldId, prevWrite);
    prevWrite = field.fieldId;
    writeDefaultCommand(c, *field.command);
  }

  if (!op.writeLengthDelimited) {
    wf.writeStop(c);
  }

  if (patchWrite) {
    size_t bodyBytes =
        thrift_transcode_cursor_bytes_since_mark(c, writeMark) - 5;
    thrift_transcode_cursor_patch_varint(c, writeMark, bodyBytes, 5);
  }
}

void writeDefaultCommand(TranscodeCursor* c, const Command& cmd) {
  if (const auto* s = std::get_if<ScalarOp>(&cmd)) {
    writeScalarDefault(c, *s);
  } else if (const auto* sq = std::get_if<SeqOp>(&cmd)) {
    if (sq->writeLoopKind == LoopKind::ByBytes) {
      thrift_transcode_write_unsigned_varint(c, 0);
    } else {
      writeSeqHeader(c, sq->writeFraming, 0, sq->writeElemType);
    }
  } else if (const auto* m = std::get_if<MapOp>(&cmd)) {
    writeMapHeader(c, m->writeFraming, 0, m->writeKeyType, m->writeValueType);
  } else if (const auto* st = std::get_if<StructOp>(&cmd)) {
    writeDefaultStruct(c, *st);
  }
}

// ── Container framing (inline, mirrors KernelCodegen) ──

void writeSeqHeader(
    TranscodeCursor* c,
    ContainerFraming framing,
    uint32_t count,
    uint8_t elemType) {
  switch (framing) {
    case ContainerFraming::Compact:
      if (count <= 14) {
        thrift_transcode_write_byte_checked(
            c, static_cast<uint8_t>((count << 4) | elemType));
      } else {
        thrift_transcode_write_byte_checked(
            c, static_cast<uint8_t>(0xF0 | elemType));
        thrift_transcode_write_unsigned_varint(c, count);
      }
      break;
    case ContainerFraming::Binary:
      thrift_transcode_cursor_ensure_write(c, 5);
      if (c->error != 0) {
        return;
      }
      thrift_transcode_write_byte_unchecked(c, elemType);
      thrift_transcode_write_fixed32_be_unchecked(c, count);
      break;
    case ContainerFraming::Json:
    case ContainerFraming::None:
      break;
  }
}

void writeMapHeader(
    TranscodeCursor* c,
    ContainerFraming framing,
    uint32_t count,
    uint8_t keyType,
    uint8_t valueType) {
  switch (framing) {
    case ContainerFraming::Compact:
      if (count == 0) {
        thrift_transcode_write_byte_checked(c, 0);
      } else {
        thrift_transcode_write_unsigned_varint(c, count);
        thrift_transcode_write_byte_checked(
            c, static_cast<uint8_t>((keyType << 4) | (valueType & 0x0F)));
      }
      break;
    case ContainerFraming::Binary:
      thrift_transcode_cursor_ensure_write(c, 6);
      if (c->error != 0) {
        return;
      }
      thrift_transcode_write_byte_unchecked(c, keyType);
      thrift_transcode_write_byte_unchecked(c, valueType);
      thrift_transcode_write_fixed32_be_unchecked(c, count);
      break;
    case ContainerFraming::Json:
    case ContainerFraming::None:
      break;
  }
}

TranscodePatchPoint reserveNonEmptyMapHeader(
    TranscodeCursor* c, ContainerFraming framing) {
  TranscodePatchPoint writeMark = thrift_transcode_cursor_mark(c);
  switch (framing) {
    case ContainerFraming::Compact:
    case ContainerFraming::Binary:
      thrift_transcode_cursor_skip(c, 6);
      break;
    case ContainerFraming::Json:
    case ContainerFraming::None:
      break;
  }
  return writeMark;
}

void patchNonEmptyMapHeader(
    TranscodeCursor* c,
    TranscodePatchPoint writeMark,
    ContainerFraming framing,
    uint32_t count,
    uint8_t keyType,
    uint8_t valueType) {
  switch (framing) {
    case ContainerFraming::Compact:
      thrift_transcode_cursor_patch_varint(c, writeMark, count, 5);
      thrift_transcode_cursor_patch_byte(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 5),
          static_cast<uint8_t>((keyType << 4) | (valueType & 0x0F)));
      break;
    case ContainerFraming::Binary:
      thrift_transcode_cursor_patch_byte(c, writeMark, keyType);
      thrift_transcode_cursor_patch_byte(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 1),
          valueType);
      thrift_transcode_cursor_patch_i32_be(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 2),
          static_cast<int32_t>(count));
      break;
    case ContainerFraming::Json:
    case ContainerFraming::None:
      break;
  }
}

bool jsonMapUsesObjectForm(const MapOp& op) {
  const auto* key = std::get_if<ScalarOp>(op.key.get());
  return key != nullptr &&
      ((key->valueKind == ValueKind::Bytes &&
        key->readFn == ReadFn::ParseQuotedString) ||
       key->valueKind == ValueKind::Enum);
}

bool isUnion(const StructOp& op) {
  return op.schemaType != nullptr && op.schemaType->isUnion();
}

void writeJsonObjectMapKey(
    TranscodeCursor* c, const ScalarOp& keyOp, const std::string& key) {
  if (keyOp.valueKind == ValueKind::Enum) {
    int64_t enumValue = 0;
    const auto* enumNames =
        keyOp.enumNames != nullptr ? &keyOp.enumNames->values : nullptr;
    if (!thrift_transcode_parse_json_object_enum_key(
            key, enumNames, enumValue) ||
        !intFits(keyOp.valueKind, enumValue)) {
      detail::setError(c, 1);
      return;
    }
    writeScalarInt(c, keyOp, enumValue);
    return;
  }
  if (keyOp.valueKind != ValueKind::Bytes ||
      keyOp.readFn != ReadFn::ParseQuotedString) {
    detail::setError(c, 90);
    return;
  }
  writeScalarBytes(
      c,
      keyOp.writeFn,
      reinterpret_cast<const uint8_t*>(key.data()),
      key.size());
}

void execJsonObjectMap(TranscodeCursor* c, const MapOp& op) {
  const auto* keyOp = std::get_if<ScalarOp>(op.key.get());
  if (keyOp == nullptr) {
    detail::setError(c, 90);
    return;
  }

  thrift_transcode_json_skip_whitespace(c);
  if (!thrift_transcode_json_expect_byte(c, '{')) {
    return;
  }
  thrift_transcode_json_skip_whitespace(c);
  if (thrift_transcode_json_peek(c) == '}') {
    if (!thrift_transcode_json_expect_byte(c, '}')) {
      return;
    }
    writeMapHeader(c, op.writeFraming, 0, op.writeKeyType, op.writeValueType);
    return;
  }

  TranscodePatchPoint writeMark = reserveNonEmptyMapHeader(c, op.writeFraming);
  if (c->error) {
    return;
  }
  uint32_t count = 0;
  bool first = true;
  while (true) {
    thrift_transcode_json_skip_whitespace(c);
    if (thrift_transcode_json_peek(c) == '}') {
      break;
    }
    if (!first) {
      if (!thrift_transcode_json_expect_byte(c, ',')) {
        return;
      }
      thrift_transcode_json_skip_whitespace(c);
    }
    first = false;

    std::string key;
    if (!thrift_transcode_read_json_object_key(c, key)) {
      return;
    }

    thrift_transcode_json_skip_whitespace(c);
    if (!thrift_transcode_json_expect_byte(c, ':')) {
      return;
    }

    writeJsonObjectMapKey(c, *keyOp, key);
    if (c->error) {
      return;
    }
    execCommand(c, *op.value, 0);
    if (c->error) {
      return;
    }
    ++count;
  }
  if (!thrift_transcode_json_expect_byte(c, '}')) {
    return;
  }
  patchNonEmptyMapHeader(
      c, writeMark, op.writeFraming, count, op.writeKeyType, op.writeValueType);
}

void execJsonKeyValueArrayEntry(TranscodeCursor* c, const MapOp& op) {
  if (!thrift_transcode_json_expect_byte(c, '{')) {
    return;
  }

  const uint8_t* keyStart = nullptr;
  const uint8_t* keyEnd = nullptr;
  const uint8_t* valueStart = nullptr;
  const uint8_t* valueEnd = nullptr;

  bool first = true;
  while (true) {
    thrift_transcode_json_skip_whitespace(c);
    if (thrift_transcode_json_peek(c) == '}') {
      break;
    }
    if (!first) {
      if (!thrift_transcode_json_expect_byte(c, ',')) {
        return;
      }
      thrift_transcode_json_skip_whitespace(c);
    }
    first = false;

    std::string name;
    if (!thrift_transcode_read_json_object_key(c, name)) {
      return;
    }
    thrift_transcode_json_skip_whitespace(c);
    if (!thrift_transcode_json_expect_byte(c, ':')) {
      return;
    }

    const uint8_t* valueBegin = c->readPos;
    if (!thrift_transcode_skip_json_value(c)) {
      return;
    }
    const uint8_t* valueFinish = c->readPos;
    if (name == "key") {
      keyStart = valueBegin;
      keyEnd = valueFinish;
    } else if (name == "value") {
      valueStart = valueBegin;
      valueEnd = valueFinish;
    }
  }

  if (keyStart == nullptr || valueStart == nullptr) {
    detail::setError(c, 1);
    return;
  }

  if (!thrift_transcode_json_expect_byte(c, '}')) {
    return;
  }
  const uint8_t* entryEnd = c->readPos;

  c->readPos = keyStart;
  execCommand(c, *op.key, 0);
  if (c->error) {
    return;
  }
  if (c->readPos != keyEnd) {
    detail::setError(c, 1);
    return;
  }

  c->readPos = valueStart;
  execCommand(c, *op.value, 0);
  if (c->error) {
    return;
  }
  if (c->readPos != valueEnd) {
    detail::setError(c, 1);
    return;
  }
  c->readPos = entryEnd;
}

void execJsonKeyValueArrayMap(TranscodeCursor* c, const MapOp& op) {
  thrift_transcode_json_skip_whitespace(c);
  if (!thrift_transcode_json_expect_byte(c, '[')) {
    return;
  }
  thrift_transcode_json_skip_whitespace(c);
  if (thrift_transcode_json_peek(c) == ']') {
    if (!thrift_transcode_json_expect_byte(c, ']')) {
      return;
    }
    writeMapHeader(c, op.writeFraming, 0, op.writeKeyType, op.writeValueType);
    return;
  }

  TranscodePatchPoint writeMark = reserveNonEmptyMapHeader(c, op.writeFraming);
  if (c->error) {
    return;
  }
  uint32_t count = 0;
  bool first = true;
  while (true) {
    thrift_transcode_json_skip_whitespace(c);
    if (thrift_transcode_json_peek(c) == ']') {
      break;
    }
    if (!first) {
      if (!thrift_transcode_json_expect_byte(c, ',')) {
        return;
      }
      thrift_transcode_json_skip_whitespace(c);
    }
    first = false;

    execJsonKeyValueArrayEntry(c, op);
    if (c->error) {
      return;
    }
    ++count;
  }
  if (!thrift_transcode_json_expect_byte(c, ']')) {
    return;
  }
  patchNonEmptyMapHeader(
      c, writeMark, op.writeFraming, count, op.writeKeyType, op.writeValueType);
}

void execJsonMap(TranscodeCursor* c, const MapOp& op) {
  if (jsonMapUsesObjectForm(op)) {
    execJsonObjectMap(c, op);
  } else {
    execJsonKeyValueArrayMap(c, op);
  }
}

void execSeq(TranscodeCursor* c, const SeqOp& op) {
  if (op.readLoopKind == LoopKind::ByBytes) {
    // ByBytes (e.g. protobuf packed) read: the element count is not on the
    // wire. Read the byte-length, scope the read window, loop until it is
    // consumed, then back-patch the deferred container header once the count is
    // known — mirrors the JIT's emitSeqOp using the same extern "C" cursor
    // intrinsics. The patched Thrift header uses the fixed-width long form
    // (non-canonical but reader-compatible); see the side-effect note in
    // KernelCodegen.cpp's emitSeqOp.
    uint64_t byteLen = thrift_transcode_read_unsigned_varint(c);
    if (c->error) {
      return;
    }
    const uint8_t* savedReadEnd = c->readEnd;
    if (!setReadEndFromByteLength(*c, byteLen)) {
      return;
    }

    TranscodePatchPoint writeMark = thrift_transcode_cursor_mark(c);
    if (op.writeLoopKind == LoopKind::ByBytes) {
      thrift_transcode_cursor_skip(c, 5); // protobuf: 5-byte varint length
    } else if (op.writeFraming == ContainerFraming::Compact) {
      thrift_transcode_cursor_skip(c, 6); // escape byte + 5-byte varint count
    } else if (op.writeFraming == ContainerFraming::Binary) {
      thrift_transcode_cursor_skip(c, 5); // elem-type byte + i32 BE count
    }

    uint32_t count = 0;
    while (c->readPos < c->readEnd && !c->error) {
      execCommand(c, *op.element, 0);
      ++count;
    }
    c->readEnd = savedReadEnd;
    if (c->error) {
      return;
    }

    if (op.writeLoopKind == LoopKind::ByBytes) {
      size_t bodyBytes =
          thrift_transcode_cursor_bytes_since_mark(c, writeMark) - 5;
      thrift_transcode_cursor_patch_varint(c, writeMark, bodyBytes, 5);
    } else if (op.writeFraming == ContainerFraming::Compact) {
      thrift_transcode_cursor_patch_byte(
          c, writeMark, static_cast<uint8_t>(0xF0 | op.writeElemType));
      thrift_transcode_cursor_patch_varint(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 1),
          count,
          5);
    } else if (op.writeFraming == ContainerFraming::Binary) {
      thrift_transcode_cursor_patch_byte(c, writeMark, op.writeElemType);
      thrift_transcode_cursor_patch_i32_be(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 1),
          static_cast<int32_t>(count));
    }
    return;
  }

  if (op.readFraming == ContainerFraming::Json) {
    // JSON array source: '[' elem (',' elem)* ']'. The element count isn't on
    // the wire, so reuse the ByBytes machinery — reserve the target header,
    // count while looping, then back-patch — with the same reserve sizes and
    // patch calls as the packed-read arm above.
    thrift_transcode_json_skip_whitespace(c);
    thrift_transcode_json_expect_byte(
        c, '['); // untrusted input: validate, unlike the JIT
    if (c->error) {
      return;
    }

    TranscodePatchPoint writeMark = thrift_transcode_cursor_mark(c);
    if (op.writeFraming == ContainerFraming::Compact) {
      thrift_transcode_cursor_skip(c, 6); // escape byte + 5-byte varint count
    } else if (op.writeFraming == ContainerFraming::Binary) {
      thrift_transcode_cursor_skip(c, 5); // elem-type byte + i32 BE count
    }

    uint32_t count = 0;
    bool first = true;
    while (true) {
      if (c->error) {
        return;
      }
      thrift_transcode_json_skip_whitespace(c);
      if (thrift_transcode_json_peek(c) == ']') {
        break;
      }
      if (!first) {
        thrift_transcode_json_expect_byte(c, ',');
        if (c->error) {
          return;
        }
      }
      first = false;
      execCommand(c, *op.element, 0);
      ++count;
    }

    thrift_transcode_json_expect_byte(c, ']');
    if (c->error) {
      return;
    }

    if (op.writeFraming == ContainerFraming::Compact) {
      thrift_transcode_cursor_patch_byte(
          c, writeMark, static_cast<uint8_t>(0xF0 | op.writeElemType));
      thrift_transcode_cursor_patch_varint(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 1),
          count,
          5);
    } else if (op.writeFraming == ContainerFraming::Binary) {
      thrift_transcode_cursor_patch_byte(c, writeMark, op.writeElemType);
      thrift_transcode_cursor_patch_i32_be(
          c,
          thrift_transcode_cursor_offset_patch_point(writeMark, 1),
          static_cast<int32_t>(count));
    }
    return;
  }

  // The interpreter baseline supports ByCount framing (Compact/Binary).
  uint32_t count = readSeqCount(c, op.readFraming, op.readElemType);
  if (c->error != 0) {
    return;
  }
  writeSeqHeader(c, op.writeFraming, count, op.writeElemType);
  if (c->error != 0) {
    return;
  }
  for (uint32_t i = 0; i < count; ++i) {
    if (c->error) {
      break;
    }
    execCommand(c, *op.element, 0);
  }
}

void execMap(TranscodeCursor* c, const MapOp& op) {
  if (op.readFraming == ContainerFraming::Json) {
    execJsonMap(c, op);
    return;
  }
  uint32_t count =
      readMapCount(c, op.readFraming, op.readKeyType, op.readValueType);
  if (c->error != 0) {
    return;
  }
  writeMapHeader(c, op.writeFraming, count, op.writeKeyType, op.writeValueType);
  if (c->error != 0) {
    return;
  }
  for (uint32_t i = 0; i < count; ++i) {
    if (c->error) {
      break;
    }
    execCommand(c, *op.key, 0);
    if (c->error) {
      break;
    }
    execCommand(c, *op.value, 0);
  }
}

const FieldEntry* FOLLY_NULLABLE findFieldByName(
    const StructOp& op, const TranscodeJsonStringToken& name, size_t* index) {
  for (size_t i = 0; i < op.fields.size(); ++i) {
    const auto& f = op.fields[i];
    if (thrift_transcode_json_string_token_equals(
            &name,
            reinterpret_cast<const uint8_t*>(f.fieldName.data()),
            f.fieldName.size())) {
      *index = i;
      return &f;
    }
  }
  return nullptr;
}

// JSON object source → binary target. Mirrors KernelCodegen's emitJsonStructOp:
// read `{`, loop over `"name": value` pairs writing the matched field through
// the target's binary framing (skipping unknown keys), read `}`, and finish the
// target framing. A JSON source only ever targets a binary protocol here (JSON
// is a leaf endpoint), so the write side is always header-intrinsic framing.
void execJsonStruct(TranscodeCursor* c, const StructOp& op) {
  FieldProto wp = op.writeFieldProto;
  if (wp == FieldProto::Unsupported) {
    detail::setError(c, 90); // interpreter: unsupported protocol
    return;
  }
  Framing wf = framingFor(wp);

  TranscodePatchPoint writeMark{};
  bool patchWrite = false;
  if (op.writeLengthDelimited) {
    writeMark = thrift_transcode_cursor_mark(c);
    thrift_transcode_cursor_skip(c, 5);
    patchWrite = true;
  }

  thrift_transcode_json_skip_whitespace(c);
  if (!thrift_transcode_json_expect_byte(c, '{')) {
    return;
  }

  int16_t prevWrite = 0;
  bool first = true;
  bool unionMemberSeen = false;
  std::vector<uint8_t> fieldsWithInput(op.fields.size(), 0);
  while (true) {
    if (c->error) {
      return;
    }
    thrift_transcode_json_skip_whitespace(c);
    if (thrift_transcode_json_peek(c) == '}') {
      break;
    }
    if (!first) {
      if (!thrift_transcode_json_expect_byte(c, ',')) {
        return;
      }
      thrift_transcode_json_skip_whitespace(c);
    }
    first = false;

    TranscodeJsonStringToken name{};
    if (!thrift_transcode_read_json_string_token(c, &name)) {
      return;
    }
    thrift_transcode_json_skip_whitespace(c);
    if (!thrift_transcode_json_expect_byte(c, ':')) {
      return;
    }
    thrift_transcode_json_skip_whitespace(c);

    size_t fieldIndex = 0;
    const FieldEntry* fe = findFieldByName(op, name, &fieldIndex);
    if (fe == nullptr) {
      if (!thrift_transcode_skip_json_value(c)) {
        return;
      }
      continue;
    }

    if (fieldsWithInput.at(fieldIndex) != 0) {
      detail::setError(c, 1);
      return;
    }
    fieldsWithInput.at(fieldIndex) = 1;

    if (thrift_transcode_json_consume_null(c)) {
      if (!fe->optional) {
        detail::setError(c, 1);
      }
      continue;
    }

    if (isUnion(op)) {
      if (unionMemberSeen) {
        detail::setError(c, 1);
        return;
      }
      unionMemberSeen = true;
    }

    // Deferred Compact bool: the value is encoded in the field header type
    // byte, so it must be read before the header is written. Same handling as
    // the binary execStruct path.
    if (const auto* sc = std::get_if<ScalarOp>(fe->command.get());
        sc != nullptr && sc->writeFn == WriteFn::CompactBoolInType) {
      int64_t boolVal = 0;
      if (!readScalarInt(c, *sc, 0, &boolVal)) {
        return;
      }
      thrift_transcode_compact_write_bool_field(
          c, boolVal ? 1 : 0, fe->fieldId, prevWrite);
      prevWrite = fe->fieldId;
      continue;
    }

    wf.writeHeader(c, fe->writeTypeInfo, fe->fieldId, prevWrite);
    prevWrite = fe->fieldId;
    execCommand(c, *fe->command, 0);
  }

  for (size_t i = 0; i < op.fields.size(); ++i) {
    if (c->error != 0) {
      return;
    }
    const auto& field = op.fields[i];
    if (field.optional || fieldsWithInput.at(i) != 0) {
      continue;
    }
    if (const auto* sc = std::get_if<ScalarOp>(field.command.get());
        sc != nullptr && sc->writeFn == WriteFn::CompactBoolInType) {
      thrift_transcode_compact_write_bool_field(c, 0, field.fieldId, prevWrite);
      prevWrite = field.fieldId;
      continue;
    }
    wf.writeHeader(c, field.writeTypeInfo, field.fieldId, prevWrite);
    prevWrite = field.fieldId;
    writeDefaultCommand(c, *field.command);
  }

  if (!thrift_transcode_json_expect_byte(c, '}')) {
    return;
  }
  if (isUnion(op) && !unionMemberSeen) {
    detail::setError(c, 1);
    return;
  }
  if (!op.writeLengthDelimited) {
    wf.writeStop(c);
  }
  if (c->error != 0) {
    return;
  }

  if (patchWrite) {
    size_t bodyBytes =
        thrift_transcode_cursor_bytes_since_mark(c, writeMark) - 5;
    thrift_transcode_cursor_patch_varint(c, writeMark, bodyBytes, 5);
  }
}

void execStruct(TranscodeCursor* c, const StructOp& op) {
  // JSON source structs are name-keyed (built with FieldIdent::ByName and an
  // empty readFieldHeader). Route them to the dedicated JSON object reader.
  if (op.fieldIdent == FieldIdent::ByName) {
    execJsonStruct(c, op);
    return;
  }

  FieldProto rp = op.readFieldProto;
  if (rp == FieldProto::Unsupported) {
    // Custom source struct framing is not implemented in the baseline.
    if (c->error == 0) {
      c->error = kUnsupportedProtocol;
    }
    return;
  }
  Framing rf = framingFor(rp);

  // The write side is either a binary protocol (header intrinsics) or JSON
  // (inline object framing: '{' "name": value , ... '}'). JSON write framing is
  // selected by writeFieldIdent so a Compact(ById) read can target
  // JSON(ByName).
  const bool jsonWrite = op.writeFieldIdent == FieldIdent::ByName;
  Framing wf{};
  if (!jsonWrite) {
    FieldProto wp = op.writeFieldProto;
    if (wp == FieldProto::Unsupported) {
      if (c->error == 0) {
        c->error = kUnsupportedProtocol;
      }
      return;
    }
    wf = framingFor(wp);
  }

  // Protobuf nested structs are length-delimited blocks.
  const uint8_t* savedReadEnd = nullptr;
  TranscodePatchPoint writeMark{};
  bool patchWrite = false;
  if (op.readLengthDelimited) {
    uint64_t len = thrift_transcode_read_unsigned_varint(c);
    if (c->error != 0) {
      return;
    }
    savedReadEnd = c->readEnd;
    if (!setReadEndFromByteLength(*c, len)) {
      return;
    }
  }
  if (jsonWrite) {
    thrift_transcode_write_byte_checked(c, '{');
  } else if (op.writeLengthDelimited) {
    writeMark = thrift_transcode_cursor_mark(c);
    thrift_transcode_cursor_skip(c, 5); // reserve varint length prefix
    patchWrite = true;
  }

  int16_t prevRead = 0;
  int16_t prevWrite = 0;
  bool wroteJsonField = false;
  while (true) {
    if (c->error) {
      break;
    }
    int16_t fieldId = 0;
    uint8_t ttype = rf.readHeader(c, &fieldId, prevRead);
    if (ttype == 0) {
      break; // STOP / end of message
    }
    prevRead = fieldId;

    const FieldEntry* fe = findField(op, fieldId);
    if (fe == nullptr) {
      rf.skip(c, ttype);
      continue;
    }
    if (!fieldTypeMatches(rp, *fe, ttype)) {
      detail::setError(c, kMalformedFieldType);
      break;
    }

    if (jsonWrite) {
      // Emit `"name": value`, with a separating comma between entries.
      if (isUnion(op) && wroteJsonField) {
        detail::setError(c, 1);
        return;
      }
      if (wroteJsonField) {
        thrift_transcode_write_byte_checked(c, ',');
      }
      wroteJsonField = true;
      thrift_transcode_format_escaped_string(
          c,
          reinterpret_cast<const uint8_t*>(fe->fieldName.data()),
          fe->fieldName.size());
      thrift_transcode_write_byte_checked(c, ':');
      execCommand(c, *fe->command, ttype);
      continue;
    }

    // Deferred Compact bool: the value lives in the field header type byte, so
    // we must know it before writing the header.
    if (const auto* sc = std::get_if<ScalarOp>(fe->command.get());
        sc != nullptr && sc->writeFn == WriteFn::CompactBoolInType) {
      int64_t boolVal = 0;
      if (!readScalarInt(c, *sc, ttype, &boolVal)) {
        return;
      }
      thrift_transcode_compact_write_bool_field(
          c, boolVal ? 1 : 0, fieldId, prevWrite);
      if (c->error != 0) {
        break;
      }
      prevWrite = fieldId;
      continue;
    }

    wf.writeHeader(c, fe->writeTypeInfo, fieldId, prevWrite);
    if (c->error != 0) {
      break;
    }
    prevWrite = fieldId;
    execCommand(c, *fe->command, ttype);
  }

  if (op.readLengthDelimited && savedReadEnd != nullptr) {
    c->readEnd = savedReadEnd;
  }
  if (c->error != 0) {
    return;
  }

  if (jsonWrite) {
    if (isUnion(op) && !wroteJsonField) {
      detail::setError(c, 1);
      return;
    }
    thrift_transcode_write_byte_checked(c, '}');
  } else if (!op.writeLengthDelimited) {
    wf.writeStop(c);
  }
  if (c->error != 0) {
    return;
  }

  if (patchWrite) {
    size_t bodyBytes =
        thrift_transcode_cursor_bytes_since_mark(c, writeMark) - 5;
    thrift_transcode_cursor_patch_varint(c, writeMark, bodyBytes, 5);
  }
}

void execCommand(
    TranscodeCursor* c, const Command& cmd, uint8_t fieldTypeInfo) {
  if (c->error != 0) {
    return;
  }
  if (const auto* s = std::get_if<ScalarOp>(&cmd)) {
    execScalar(c, *s, fieldTypeInfo);
  } else if (const auto* sq = std::get_if<SeqOp>(&cmd)) {
    execSeq(c, *sq);
  } else if (const auto* m = std::get_if<MapOp>(&cmd)) {
    execMap(c, *m);
  } else if (const auto* st = std::get_if<StructOp>(&cmd)) {
    execStruct(c, *st);
  } else {
    detail::setError(c, kUnsupportedProtocol);
  }
}

} // namespace

TranscodeInterpreter::TranscodeInterpreter(TranscodePlan plan)
    : plan_(std::move(plan)) {}

folly::Expected<std::unique_ptr<folly::IOBuf>, TranscodeError>
TranscodeInterpreter::transcode(const folly::IOBuf& input) const {
  // TODO @sadroeck - Support chained input buffers without coalescing
  // Note: this is part of the "streaming" input support.
  auto coalesced = input.isChained()
      ? input.cloneCoalescedAsValue()
      : folly::IOBuf::wrapBufferAsValue(input.data(), input.length());

  size_t capacity = 0;
  if (!checkedAdd(coalesced.length(), coalesced.length(), capacity) ||
      !checkedAdd(capacity, 64, capacity)) {
    return folly::makeUnexpected(
        TranscodeError{TranscodeErrc::Oom, "interpreter: output too large"});
  }
  MallocOutput output;
  output.capacity = capacity;
  output.buffer = static_cast<uint8_t*>(malloc(capacity));
  if (output.buffer == nullptr) {
    return folly::makeUnexpected(
        TranscodeError{TranscodeErrc::Oom, "interpreter: oom"});
  }
  auto bufferGuard = folly::makeGuard([&] { free(output.buffer); });

  TranscodeCursor cursor{};
  TranscodeByteRange inputRange{
      coalesced.data(), coalesced.data() + coalesced.length()};
  thrift_transcode_cursor_init(
      &cursor,
      inputRange,
      {output.buffer, output.buffer + output.capacity},
      mallocExtend,
      nullptr,
      &output);

  execCommand(&cursor, plan_.root, 0);
  validateInputConsumed(&cursor, plan_);

  if (cursor.error != 0) {
    return folly::makeUnexpected(
        TranscodeError{
            errcFromCursor(cursor.error), "interpreter returned error"});
  }

  size_t len = thrift_transcode_cursor_bytes_written(&cursor);
  bufferGuard.dismiss();
  return folly::IOBuf::takeOwnership(
      output.buffer, len, [](void* p, void*) { free(p); });
}

folly::Expected<size_t, TranscodeError> TranscodeInterpreter::transcodeInto(
    const folly::IOBuf& input, uint8_t* output, size_t outputCapacity) const {
  auto coalesced = input.isChained()
      ? input.cloneCoalescedAsValue()
      : folly::IOBuf::wrapBufferAsValue(input.data(), input.length());

  TranscodeCursor cursor{};
  TranscodeByteRange inputRange{
      coalesced.data(), coalesced.data() + coalesced.length()};
  thrift_transcode_cursor_init(
      &cursor,
      inputRange,
      {output, output + outputCapacity},
      fixedOutputExtend,
      nullptr,
      nullptr);

  execCommand(&cursor, plan_.root, 0);
  validateInputConsumed(&cursor, plan_);

  if (cursor.error != 0) {
    return folly::makeUnexpected(
        TranscodeError{
            errcFromCursor(cursor.error), "interpreter returned error"});
  }
  return thrift_transcode_cursor_bytes_written(&cursor);
}

} // namespace apache::thrift::transcode
