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

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace apache::thrift::type_system {
class TypeRef;
} // namespace apache::thrift::type_system

namespace apache::thrift::transcode {

enum class WireProtocol : uint8_t {
  Unknown,
  ThriftCompact,
  ThriftBinary,
  ProtobufBinary,
  Json,
};

// ─────────────────────────────────────────────────────────────────────────
// Command-tree model for transcoding
// ─────────────────────────────────────────────────────────────────────────
//
// A transcoding plan is a tree of Commands:
//   Command = ScalarOp | SeqOp | MapOp | StructOp
//
// The kernel compiler walks the tree and emits LLVM IR recursively.
// The root can be any command — a plan for list<i32> is just a SeqOp.
//
// Scalars use typed enum intrinsics (ReadFn/WriteFn) for compile-time
// checked codegen. Container framing uses string intrinsic names since
// they have uniform calling conventions.

// ─────────────────────────────────────────────────────────────────────────
// Value types
// ─────────────────────────────────────────────────────────────────────────

/**
 * The logical value type flowing through the pipeline.
 * Determines the LLVM register type and the calling convention for Custom
 * intrinsics:
 *   Bool/I8/I16/I32/I64/Enum: int64_t fn(TranscodeCursor*)
 *   F32/F64:                  double fn(TranscodeCursor*)
 *   Bytes:                    const uint8_t* fn(TranscodeCursor*, size_t*)
 */
enum class ValueKind : uint8_t {
  Bool,
  I8,
  I16,
  I32,
  I64,
  Enum,
  F32,
  F64,
  Bytes,
};

// ─────────────────────────────────────────────────────────────────────────
// Scalar intrinsics (enum + Custom fallback)
// ─────────────────────────────────────────────────────────────────────────

/**
 * How to read a scalar value from the input.
 * Each variant maps to a specific extern "C" intrinsic function.
 */
enum class ReadFn : uint8_t {
  // Varint encodings
  ZigzagVarint, // Thrift Compact: zigzag → signed int
  UnsignedVarint, // Protobuf: unsigned varint → int

  // Fixed-width big-endian (Thrift Binary protocol)
  Fixed8,
  Fixed16BE,
  Fixed32BE,
  Fixed64BE,

  // Fixed-width little-endian (Protobuf fixed32/64)
  Fixed32LE,
  Fixed64LE,

  // Length-prefixed bytes
  LengthPrefixedVarint, // varint len + N bytes (Compact, Protobuf)
  LengthPrefixedI32, // i32 len + N bytes (Binary protocol)

  // Bool
  CompactBoolFromType, // Thrift Compact: encoded in field type byte
  ByteAsBool, // single byte 0/1
  VarintAsBool, // varint 0/1 (Protobuf)

  // Text protocol parsing (fused extract+decode)
  ParseDecimalInt, // decimal text → integer
  ParseEnumNameOrDecimalText, // quoted enum name or decimal integer
  ParseDecimalFloat, // decimal text → f64
  ParseQuotedString, // quoted string with escape handling
  ParseBase64String, // quoted base64 string → bytes
  ParseBoolKeyword, // "true"/"false" keyword

  // Escape hatch
  Custom, // use ScalarOp::customReadFn
};

/**
 * How to write a scalar value to the output.
 */
enum class WriteFn : uint8_t {
  // Varint encodings
  ZigzagVarint,
  UnsignedVarint,

  // Fixed-width big-endian
  Fixed8,
  Fixed16BE,
  Fixed32BE,
  Fixed64BE,

  // Fixed-width little-endian
  Fixed32LE,
  Fixed64LE,

  // Length-prefixed bytes
  LengthPrefixedVarint,
  LengthPrefixedI32,

  // Bool
  CompactBoolInType, // Thrift Compact: encode in field type byte
  ByteAsBool,
  VarintAsBool,

  // Text protocol encoding
  IntToDecimalText,
  EnumNameOrDecimalText,
  FloatToDecimalText,
  WriteQuotedString,
  WriteBase64String,
  BoolToKeyword,

  // Struct memory target
  StoreAtOffset, // direct memory store
  CallTypeInfoSet, // call TypeInfo::set function pointer

  // Escape hatch
  Custom, // use ScalarOp::customWriteFn
};

/**
 * Value coercion between read and write.
 */
enum class CoerceOp : uint8_t {
  None,
  WidenSignedInt,
  WidenF32ToF64,
};

// ─────────────────────────────────────────────────────────────────────────
// Command variants
// ─────────────────────────────────────────────────────────────────────────

struct ScalarOp;
struct SeqOp;
struct MapOp;
struct StructOp;

/**
 * A Command describes how to transcode one value (scalar or composite).
 * The kernel compiler emits LLVM IR by walking the command tree.
 */
using Command = std::variant<ScalarOp, SeqOp, MapOp, StructOp>;

/**
 * Snapshot of an enum's value<->name mapping, taken at codec-build time (the
 * op tree does not retain a runtime TypeRef, and an EnumNode is invalidated if
 * its TypeSystem is destroyed). Shared via shared_ptr across all fields of the
 * same enum. Numeric protocols still carry the value as their normal integer
 * encoding; label-capable text protocols use this table for name/value
 * conversion. Enums are small, so a linear scan matches the runtime EnumNode
 * lookup.
 */
struct EnumNames {
  std::vector<std::pair<int32_t, std::string>> values; // (i32, name)

  const std::string* nameFor(int32_t v) const {
    for (const auto& [val, name] : values) {
      if (val == v) {
        return &name;
      }
    }
    return nullptr;
  }
  bool valueFor(std::string_view name, int32_t& out) const {
    for (const auto& [val, n] : values) {
      if (n == name) {
        out = val;
        return true;
      }
    }
    return false;
  }
};

/**
 * Read one scalar, coerce, write it.
 */
struct ScalarOp {
  ValueKind valueKind{};
  ReadFn readFn{};
  WriteFn writeFn{};
  CoerceOp coerce = CoerceOp::None;

  // Only used when readFn/writeFn == Custom
  std::string customReadFn;
  std::string customWriteFn;

  // For StoreAtOffset/CallTypeInfoSet (struct memory target)
  ptrdiff_t memberOffset = 0;
  ptrdiff_t issetOffset = 0;
  const void* typeInfoSetFn = nullptr;

  // Non-null iff valueKind == Enum. Numeric read/write functions ignore it;
  // label-capable text functions use it for name/value conversion.
  std::shared_ptr<const EnumNames> enumNames;
};

/**
 * Protocol-specific container framing kind.
 * The codegen emits inline IR for each variant — no intrinsic calls.
 */
enum class ContainerFraming : uint8_t {
  // Compact: 1 byte (count << 4 | elemType), or 0xF0|elemType + varint count
  Compact,
  // Binary: elemType:i8 + count:i32BE (list/set) or keyType:i8 + valType:i8 +
  // count:i32BE (map)
  Binary,
  // JSON: '[' / ']' with comma separators (seq) or '{' / '}' (map)
  Json,
  // No framing — used when the source protocol doesn't have containers
  // (protobuf repeated fields are handled at the StructOp level)
  None,
};

/**
 * How the element loop terminates.
 */
enum class LoopKind : uint8_t {
  // Loop N times (count known upfront from container header).
  // Used by Compact, Binary, and JSON (after counting).
  ByCount,
  // Loop until a byte budget is exhausted (read position reaches a limit).
  // Used by protobuf packed repeated fields: varint_length bytes of elements.
  ByBytes,
};

/**
 * Transcode a sequence (list or set): read header → loop → element command.
 * Container framing is emitted inline by the codegen.
 */
struct SeqOp {
  ContainerFraming readFraming{};
  ContainerFraming writeFraming{};

  // How the read loop terminates.
  LoopKind readLoopKind = LoopKind::ByCount;
  // How the write loop terminates (always ByCount for non-protobuf targets).
  LoopKind writeLoopKind = LoopKind::ByCount;

  // Compact/Binary protocol: element type for framing headers.
  uint8_t readElemType = 0;
  uint8_t writeElemType = 0;

  // Command applied to each element
  std::unique_ptr<Command> element;
};

/**
 * Transcode a map: read count → loop → (key, value) commands.
 *
 * Protobuf maps encode each entry as a length-delimited submessage with
 * field 1 = key and field 2 = value. The entryIsSubmessage flags tell the
 * codegen to wrap/unwrap entries with submessage framing.
 */
struct MapOp {
  ContainerFraming readFraming{};
  ContainerFraming writeFraming{};

  // Compact/Binary: key and value types for framing headers.
  uint8_t readKeyType = 0;
  uint8_t readValueType = 0;
  uint8_t writeKeyType = 0;
  uint8_t writeValueType = 0;

  // Protobuf: each map entry is a length-delimited submessage
  // with field 1 = key and field 2 = value.
  bool readEntryIsSubmessage = false;
  bool writeEntryIsSubmessage = false;

  // Protobuf: wire types for key/value field tags within submessage
  uint8_t readKeyWireType = 0; // protobuf wire type for key
  uint8_t readValueWireType = 0; // protobuf wire type for value
  uint8_t writeKeyWireType = 0;
  uint8_t writeValueWireType = 0;

  std::unique_ptr<Command> key;
  std::unique_ptr<Command> value;
};

/**
 * A single field entry within a StructOp.
 *
 * For protobuf repeated fields: isRepeated=true means the same fieldId may
 * appear multiple times in the source wire data. The codegen generates a
 * state machine that accumulates elements and writes them as a container
 * on the target side. The command describes how to process ONE element.
 */
struct FieldEntry {
  int16_t fieldId{};
  std::string fieldName; // for name-based framing (JSON)
  uint8_t readTypeInfo = 0; // expected source type byte (for ttype validation)
  uint8_t writeTypeInfo = 0; // protocol-specific type byte for write header
  bool isRepeated = false; // protobuf: same fieldId appears multiple times
  bool optional = false;
  bool required = false; // schema presence == UNQUALIFIED (for dynamic->wire)
  bool hasCustomDefault = false;
  std::unique_ptr<Command> command;
};

/**
 * How a struct protocol identifies fields on the wire.
 */
enum class FieldIdent : uint8_t {
  ById, // field header carries numeric ID (Compact, Binary, Protobuf)
  ByName, // field identity is a string key (JSON)
};

enum class FieldProto : uint8_t {
  Compact,
  Binary,
  Protobuf,
  Unsupported,
};

/**
 * Transcode a struct: read field headers → dispatch by ID/name → per-field
 * commands.
 */
struct StructOp {
  FieldIdent fieldIdent{};

  // Write-side field identity. The read side uses `fieldIdent`; the two differ
  // for cross-protocol transcodes to/from JSON (e.g. Compact `ById` read →
  // JSON `ByName` write), where the target's framing must be selected
  // independently of the source's.
  FieldIdent writeFieldIdent = FieldIdent::ById;
  FieldProto readFieldProto = FieldProto::Unsupported;
  FieldProto writeFieldProto = FieldProto::Unsupported;

  // Framing intrinsics
  std::string readFieldHeader; // returns (fieldId, type) or (name)
  std::string writeFieldHeader; // writes field header
  std::string readEnd; // detect end (STOP byte, end of message)
  std::string writeEnd; // write end marker

  // Skip intrinsic for unknown fields
  std::string skipField;

  // Protobuf: nested structs are length-delimited blocks.
  // When true, the codegen wraps read/write with length framing:
  //   Read: varint length → scoped readEnd → fields → restore readEnd
  //   Write: mark → skip(5) → fields → patch varint length
  bool readLengthDelimited = false;
  bool writeLengthDelimited = false;
  std::shared_ptr<const type_system::TypeRef> schemaType;

  // Per-field commands, sorted by fieldId
  std::vector<FieldEntry> fields;

  // For struct memory target: isset management
  using SetIssetFn = void (*)(void*, ptrdiff_t, bool);
  using GetBasePtrFn = const void* (*)(const void*);
  SetIssetFn setIssetFn = nullptr;
  GetBasePtrFn getBasePtrFn = nullptr;
};

/**
 * A Codec is a named Command tree describing how a protocol encodes a type.
 * Produced by codec factories from schemas.
 */
struct Codec {
  std::string name;
  WireProtocol protocol = WireProtocol::Unknown;
  Command root;
};

} // namespace apache::thrift::transcode
