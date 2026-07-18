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

#include <thrift/lib/cpp2/transcode/ReadHelpers.h>
#include <thrift/lib/cpp2/transcode/WireType.h>

#include <folly/ScopeGuard.h>
#include <folly/lang/Assume.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <variant>

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

bool mallocExtend(
    const TranscodeExtendRequest* request,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<MallocOutput*>(userData);
  const size_t written =
      static_cast<size_t>(request->writePoint - request->segment.begin);
  size_t requested = 0;
  if (!checkedAdd(written, request->minWritable, requested)) {
    return false;
  }
  const size_t doubled =
      output.capacity > std::numeric_limits<size_t>::max() / 2
      ? requested
      : output.capacity * 2;
  const size_t newCapacity = std::max(doubled, requested);
  auto* next = static_cast<uint8_t*>(realloc(output.buffer, newCapacity));
  if (next == nullptr) {
    return false;
  }
  result->kind = next == request->segment.begin
      ? TranscodeExtendKind::InPlaceExtension
      : TranscodeExtendKind::RelocatedContiguous;
  result->segment = {next, next + newCapacity};
  output.buffer = next;
  output.capacity = newCapacity;
  return true;
}

bool fixedOutputExtend(
    const TranscodeExtendRequest* /*request*/,
    TranscodeExtendResult* /*result*/,
    void* /*userData*/) {
  return false;
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
    case WriteFn::EnumNameOrDecimalText:
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

void execScalar(TranscodeCursor* c, const ScalarOp& op, uint8_t fieldTypeInfo) {
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

void execStruct(TranscodeCursor* c, const StructOp& op) {
  FieldProto rp = op.readFieldProto;
  if (rp == FieldProto::Unsupported) {
    // JSON / Custom source struct framing is not implemented in the baseline.
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

  if (cursor.error != 0) {
    return folly::makeUnexpected(
        TranscodeError{
            errcFromCursor(cursor.error), "interpreter returned error"});
  }
  return thrift_transcode_cursor_bytes_written(&cursor);
}

} // namespace apache::thrift::transcode
