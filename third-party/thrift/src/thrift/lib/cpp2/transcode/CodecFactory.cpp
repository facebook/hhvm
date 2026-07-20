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

#include <thrift/lib/cpp2/transcode/CodecFactory.h>

#include <thrift/lib/cpp2/transcode/WireType.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>

#include <stdexcept>
#include <string_view>

namespace apache::thrift::transcode {

namespace {

ScalarOp makeScalar(ValueKind kind, ReadFn read, WriteFn write) {
  ScalarOp op;
  op.valueKind = kind;
  op.readFn = read;
  op.writeFn = write;
  return op;
}

// Snapshot an enum's value<->name mapping at build time; the op tree does not
// retain the runtime EnumNode.
std::shared_ptr<const EnumNames> makeEnumNames(
    const type_system::EnumNode& node) {
  auto table = std::make_shared<EnumNames>();
  for (const auto& v : node.values()) {
    table->values.emplace_back(v.i32, v.name);
  }
  return table;
}

[[noreturn]] void throwUnsupportedTypeRef(
    std::string_view context, const type_system::TypeRef& typeRef) {
  throw std::invalid_argument(
      std::string(context) + " does not support Thrift type kind " +
      std::string(type_system::TypeRef::kindName(typeRef.kind())));
}

const type_system::TypeRef& resolveOpaqueAlias(
    const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  if (typeRef.kind() == Kind::OPAQUE_ALIAS) {
    return resolveOpaqueAlias(typeRef.asOpaqueAlias().targetType());
  }
  return typeRef;
}

// Context in which a bool is being encoded. Affects Compact protocol encoding:
// struct fields encode bool in the ttype byte, container elements use a
// regular byte.
enum class BoolContext {
  StructField, // inside a struct/union field
  Container, // inside a list/set/map element
};

// ─────────────────────────────────────────────────────────────────────────
// Per-protocol scalar op factories
// ─────────────────────────────────────────────────────────────────────────

ScalarOp compactScalarOp(
    const type_system::TypeRef& typeRef, BoolContext boolCtx) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return compactScalarOp(typeRef.asOpaqueAlias().targetType(), boolCtx);
    case Kind::BOOL:
      if (boolCtx == BoolContext::StructField) {
        return makeScalar(
            ValueKind::Bool,
            ReadFn::CompactBoolFromType,
            WriteFn::CompactBoolInType);
      }
      return makeScalar(
          ValueKind::Bool, ReadFn::ByteAsBool, WriteFn::ByteAsBool);
    case Kind::BYTE:
      return makeScalar(ValueKind::I8, ReadFn::Fixed8, WriteFn::Fixed8);
    case Kind::I16:
      return makeScalar(
          ValueKind::I16, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    case Kind::I32:
      return makeScalar(
          ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    case Kind::ENUM:
      return makeScalar(
          ValueKind::Enum, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    case Kind::I64:
      return makeScalar(
          ValueKind::I64, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    case Kind::FLOAT:
      return makeScalar(ValueKind::F32, ReadFn::Fixed32BE, WriteFn::Fixed32BE);
    case Kind::DOUBLE:
      return makeScalar(ValueKind::F64, ReadFn::Fixed64BE, WriteFn::Fixed64BE);
    case Kind::STRING:
    case Kind::BINARY:
      return makeScalar(
          ValueKind::Bytes,
          ReadFn::LengthPrefixedVarint,
          WriteFn::LengthPrefixedVarint);
    case Kind::ANY:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
    case Kind::STRUCT:
    case Kind::UNION:
      throwUnsupportedTypeRef("Compact scalar codec", typeRef);
  }
  throwUnsupportedTypeRef("Compact scalar codec", typeRef);
}

ScalarOp protobufScalarOp(
    const type_system::TypeRef& typeRef, BoolContext /*boolCtx*/) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return protobufScalarOp(
          typeRef.asOpaqueAlias().targetType(), BoolContext::Container);
    case Kind::BOOL:
      return makeScalar(
          ValueKind::Bool, ReadFn::VarintAsBool, WriteFn::VarintAsBool);
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32: {
      auto kind = typeRef.kind() == Kind::BYTE ? ValueKind::I8
          : typeRef.kind() == Kind::I16        ? ValueKind::I16
                                               : ValueKind::I32;
      return makeScalar(kind, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    }
    case Kind::ENUM: {
      return makeScalar(
          ValueKind::Enum, ReadFn::UnsignedVarint, WriteFn::UnsignedVarint);
    }
    case Kind::I64:
      return makeScalar(
          ValueKind::I64, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint);
    case Kind::FLOAT:
      return makeScalar(ValueKind::F32, ReadFn::Fixed32LE, WriteFn::Fixed32LE);
    case Kind::DOUBLE:
      return makeScalar(ValueKind::F64, ReadFn::Fixed64LE, WriteFn::Fixed64LE);
    case Kind::STRING:
    case Kind::BINARY:
      return makeScalar(
          ValueKind::Bytes,
          ReadFn::LengthPrefixedVarint,
          WriteFn::LengthPrefixedVarint);
    case Kind::ANY:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
    case Kind::STRUCT:
    case Kind::UNION:
      throwUnsupportedTypeRef("Protobuf scalar codec", typeRef);
  }
  throwUnsupportedTypeRef("Protobuf scalar codec", typeRef);
}

// Thrift Binary: fixed-width big-endian for everything, i32-prefixed strings
ScalarOp binaryScalarOp(
    const type_system::TypeRef& typeRef, BoolContext /*boolCtx*/) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return binaryScalarOp(
          typeRef.asOpaqueAlias().targetType(), BoolContext::Container);
    case Kind::BOOL:
      return makeScalar(
          ValueKind::Bool, ReadFn::ByteAsBool, WriteFn::ByteAsBool);
    case Kind::BYTE:
      return makeScalar(ValueKind::I8, ReadFn::Fixed8, WriteFn::Fixed8);
    case Kind::I16:
      return makeScalar(ValueKind::I16, ReadFn::Fixed16BE, WriteFn::Fixed16BE);
    case Kind::I32:
      return makeScalar(ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE);
    case Kind::ENUM:
      return makeScalar(ValueKind::Enum, ReadFn::Fixed32BE, WriteFn::Fixed32BE);
    case Kind::I64:
      return makeScalar(ValueKind::I64, ReadFn::Fixed64BE, WriteFn::Fixed64BE);
    case Kind::FLOAT:
      return makeScalar(ValueKind::F32, ReadFn::Fixed32BE, WriteFn::Fixed32BE);
    case Kind::DOUBLE:
      return makeScalar(ValueKind::F64, ReadFn::Fixed64BE, WriteFn::Fixed64BE);
    case Kind::STRING:
    case Kind::BINARY:
      return makeScalar(
          ValueKind::Bytes,
          ReadFn::LengthPrefixedI32,
          WriteFn::LengthPrefixedI32);
    case Kind::ANY:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
    case Kind::STRUCT:
    case Kind::UNION:
      throwUnsupportedTypeRef("Binary scalar codec", typeRef);
  }
  throwUnsupportedTypeRef("Binary scalar codec", typeRef);
}

ScalarOp jsonScalarOp(
    const type_system::TypeRef& typeRef, BoolContext /*boolCtx*/) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return jsonScalarOp(
          typeRef.asOpaqueAlias().targetType(), BoolContext::Container);
    case Kind::BOOL:
      return makeScalar(
          ValueKind::Bool, ReadFn::ParseBoolKeyword, WriteFn::BoolToKeyword);
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32: {
      auto kind = typeRef.kind() == Kind::BYTE ? ValueKind::I8
          : typeRef.kind() == Kind::I16        ? ValueKind::I16
                                               : ValueKind::I32;
      return makeScalar(
          kind, ReadFn::ParseDecimalInt, WriteFn::IntToDecimalText);
    }
    case Kind::ENUM:
      return makeScalar(
          ValueKind::Enum,
          ReadFn::ParseEnumNameOrDecimalText,
          WriteFn::EnumNameOrDecimalText);
    case Kind::I64:
      return makeScalar(
          ValueKind::I64, ReadFn::ParseDecimalInt, WriteFn::IntToDecimalText);
    case Kind::FLOAT:
    case Kind::DOUBLE: {
      auto kind =
          typeRef.kind() == Kind::FLOAT ? ValueKind::F32 : ValueKind::F64;
      return makeScalar(
          kind, ReadFn::ParseDecimalFloat, WriteFn::FloatToDecimalText);
    }
    case Kind::STRING:
      return makeScalar(
          ValueKind::Bytes,
          ReadFn::ParseQuotedString,
          WriteFn::WriteQuotedString);
    case Kind::BINARY:
      return makeScalar(
          ValueKind::Bytes,
          ReadFn::ParseBase64String,
          WriteFn::WriteBase64String);
    case Kind::ANY:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
    case Kind::STRUCT:
    case Kind::UNION:
      throwUnsupportedTypeRef("JSON scalar codec", typeRef);
  }
  throwUnsupportedTypeRef("JSON scalar codec", typeRef);
}

// ─────────────────────────────────────────────────────────────────────────
// Type info (field header type byte) per protocol
// ─────────────────────────────────────────────────────────────────────────

uint8_t compactTypeInfo(const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return compactTypeInfo(typeRef.asOpaqueAlias().targetType());
    case Kind::BOOL:
      return wire::kCompactBooleanTrue; // adjusted at write time
    case Kind::BYTE:
      return wire::kCompactByte;
    case Kind::I16:
      return wire::kCompactI16;
    case Kind::I32:
    case Kind::ENUM:
      return wire::kCompactI32;
    case Kind::I64:
      return wire::kCompactI64;
    case Kind::DOUBLE:
      return wire::kCompactDouble;
    case Kind::FLOAT:
      return wire::kCompactFloat;
    case Kind::STRING:
    case Kind::BINARY:
      return wire::kCompactBinary;
    case Kind::LIST:
      return wire::kCompactList;
    case Kind::SET:
      return wire::kCompactSet;
    case Kind::MAP:
      return wire::kCompactMap;
    case Kind::STRUCT:
    case Kind::UNION:
      return wire::kCompactStruct;
    case Kind::ANY:
      throwUnsupportedTypeRef("Compact type info", typeRef);
  }
  throwUnsupportedTypeRef("Compact type info", typeRef);
}

// Thrift Binary TType values (standard Thrift TType enum)
uint8_t binaryTypeInfo(const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return binaryTypeInfo(typeRef.asOpaqueAlias().targetType());
    case Kind::BOOL:
      return wire::kBinaryBool;
    case Kind::BYTE:
      return wire::kBinaryByte;
    case Kind::I16:
      return wire::kBinaryI16;
    case Kind::I32:
    case Kind::ENUM:
      return wire::kBinaryI32;
    case Kind::I64:
      return wire::kBinaryI64;
    case Kind::DOUBLE:
      return wire::kBinaryDouble;
    case Kind::FLOAT:
      return wire::kBinaryFloat;
    case Kind::STRING:
    case Kind::BINARY:
      return wire::kBinaryString;
    case Kind::LIST:
      return wire::kBinaryList;
    case Kind::SET:
      return wire::kBinarySet;
    case Kind::MAP:
      return wire::kBinaryMap;
    case Kind::STRUCT:
    case Kind::UNION:
      return wire::kBinaryStruct;
    case Kind::ANY:
      throwUnsupportedTypeRef("Binary type info", typeRef);
  }
  throwUnsupportedTypeRef("Binary type info", typeRef);
}

// Protobuf's own wire types (not Thrift protocol type-ids, so not in WireType).
constexpr uint8_t kProtoWireVarint = 0;
constexpr uint8_t kProtoWire64Bit = 1;
constexpr uint8_t kProtoWireLengthDelimited = 2;
constexpr uint8_t kProtoWire32Bit = 5;

// Raw protobuf wire type (0=varint, 1=64-bit, 2=length-delimited, 5=32-bit)
uint8_t protoWireType(const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return protoWireType(typeRef.asOpaqueAlias().targetType());
    case Kind::BOOL:
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32:
    case Kind::I64:
    case Kind::ENUM:
      return kProtoWireVarint;
    case Kind::FLOAT:
      return kProtoWire32Bit;
    case Kind::DOUBLE:
      return kProtoWire64Bit;
    case Kind::STRING:
    case Kind::BINARY:
    case Kind::STRUCT:
    case Kind::UNION:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
      return kProtoWireLengthDelimited;
    case Kind::ANY:
      throwUnsupportedTypeRef("Protobuf wire type", typeRef);
  }
  throwUnsupportedTypeRef("Protobuf wire type", typeRef);
}

// Protobuf wire type + 1 (0 is reserved for end-of-message)
uint8_t protoTypeInfo(const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return protoTypeInfo(typeRef.asOpaqueAlias().targetType());
    case Kind::BOOL:
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32:
    case Kind::I64:
    case Kind::ENUM:
      return kProtoWireVarint + 1;
    case Kind::FLOAT:
      return kProtoWire32Bit + 1;
    case Kind::DOUBLE:
      return kProtoWire64Bit + 1;
    case Kind::STRING:
    case Kind::BINARY:
    case Kind::STRUCT:
    case Kind::UNION:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
      return kProtoWireLengthDelimited + 1;
    case Kind::ANY:
      throwUnsupportedTypeRef("Protobuf type info", typeRef);
  }
  throwUnsupportedTypeRef("Protobuf type info", typeRef);
}

bool isProtoPackable(const type_system::TypeRef& typeRef) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::OPAQUE_ALIAS:
      return isProtoPackable(typeRef.asOpaqueAlias().targetType());
    case Kind::BOOL:
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32:
    case Kind::I64:
    case Kind::FLOAT:
    case Kind::DOUBLE:
    case Kind::ENUM:
      return true;
    case Kind::STRING:
    case Kind::BINARY:
    case Kind::ANY:
    case Kind::LIST:
    case Kind::SET:
    case Kind::MAP:
    case Kind::STRUCT:
    case Kind::UNION:
      return false;
  }
  return false;
}

// ─────────────────────────────────────────────────────────────────────────
// Protocol configuration bundle
// ─────────────────────────────────────────────────────────────────────────

using ScalarFn = ScalarOp (*)(const type_system::TypeRef&, BoolContext);
using TypeInfoFn = uint8_t (*)(const type_system::TypeRef&);

struct ProtocolOps {
  ScalarFn scalarFn;
  TypeInfoFn typeInfoFn;
  FieldIdent fieldIdent;
  FieldProto fieldProto;

  // Struct framing
  std::string readFieldHeader;
  std::string writeFieldHeader;
  std::string readStructEnd;
  std::string writeStructEnd;
  std::string skipField;

  // Container framing (inline codegen, not intrinsics)
  ContainerFraming containerFraming;
};

// Forward declaration
Command commandForType(
    const type_system::TypeRef& typeRef,
    const ProtocolOps& ops,
    BoolContext boolCtx);

template <typename Structured>
StructOp makeStructOp(const Structured& node, const ProtocolOps& ops) {
  StructOp op;
  op.fieldIdent = ops.fieldIdent;
  op.writeFieldIdent = ops.fieldIdent;
  op.readFieldProto = ops.fieldProto;
  op.writeFieldProto = ops.fieldProto;
  op.readFieldHeader = ops.readFieldHeader;
  op.writeFieldHeader = ops.writeFieldHeader;
  op.readEnd = ops.readStructEnd;
  op.writeEnd = ops.writeStructEnd;
  op.skipField = ops.skipField;
  auto schemaType = node.asRef();
  op.schemaType = std::make_shared<type_system::TypeRef>(schemaType);
  const bool isUnion = schemaType.isUnion();

  for (const auto& field : node.fields()) {
    using Kind = type_system::TypeRef::Kind;
    const auto& fieldType = resolveOpaqueAlias(field.type());
    FieldEntry entry;
    entry.fieldId = folly::to_underlying(field.identity().id());
    entry.fieldName = std::string(field.identity().name());
    entry.readTypeInfo = ops.typeInfoFn(field.type());
    entry.writeTypeInfo = ops.typeInfoFn(field.type());
    entry.optional =
        field.presence() == type_system::PresenceQualifier::OPTIONAL_;
    entry.required = !isUnion &&
        field.presence() == type_system::PresenceQualifier::UNQUALIFIED;
    entry.hasCustomDefault = field.customDefault() != nullptr;

    // Protobuf: maps and list/set of non-packable elements are encoded as
    // repeated field occurrences under the same field ID.
    if (ops.containerFraming == ContainerFraming::None) {
      auto kind = fieldType.kind();
      if (kind == Kind::LIST || kind == Kind::SET) {
        auto elemType = (kind == Kind::LIST) ? fieldType.asList().elementType()
                                             : fieldType.asSet().elementType();
        if (!isProtoPackable(elemType)) {
          entry.isRepeated = true;
          // For unpacked repeated: the command is the ELEMENT command,
          // not a SeqOp. The struct loop handles iteration via the
          // repeated-field state machine.
          entry.command = std::make_unique<Command>(
              commandForType(elemType, ops, BoolContext::Container));
          op.fields.push_back(std::move(entry));
          continue; // skip the default commandForType below
        }
      } else if (kind == Kind::MAP) {
        entry.isRepeated = true;
      }
    }

    entry.command = std::make_unique<Command>(
        commandForType(field.type(), ops, BoolContext::StructField));
    op.fields.push_back(std::move(entry));
  }
  return op;
}

SeqOp makeSeqOp(
    const type_system::TypeRef& elementType, const ProtocolOps& ops) {
  SeqOp op;
  op.readFraming = ops.containerFraming;
  op.writeFraming = ops.containerFraming;
  op.readElemType = ops.typeInfoFn(elementType);
  op.writeElemType = ops.typeInfoFn(elementType);
  op.element = std::make_unique<Command>(
      commandForType(elementType, ops, BoolContext::Container));
  return op;
}

MapOp makeMapOp(
    const type_system::TypeRef& keyType,
    const type_system::TypeRef& valueType,
    const ProtocolOps& ops) {
  MapOp op;
  op.readFraming = ops.containerFraming;
  op.writeFraming = ops.containerFraming;
  op.readKeyType = ops.typeInfoFn(keyType);
  op.readValueType = ops.typeInfoFn(valueType);
  op.writeKeyType = ops.typeInfoFn(keyType);
  op.writeValueType = ops.typeInfoFn(valueType);

  // Protobuf maps: entries are length-delimited submessages
  if (ops.containerFraming == ContainerFraming::None) {
    op.readEntryIsSubmessage = true;
    op.writeEntryIsSubmessage = true;
    op.readKeyWireType = protoWireType(keyType);
    op.readValueWireType = protoWireType(valueType);
    op.writeKeyWireType = protoWireType(keyType);
    op.writeValueWireType = protoWireType(valueType);
  }

  op.key = std::make_unique<Command>(
      commandForType(keyType, ops, BoolContext::Container));
  op.value = std::make_unique<Command>(
      commandForType(valueType, ops, BoolContext::Container));
  return op;
}

Command commandForType(
    const type_system::TypeRef& typeRef,
    const ProtocolOps& ops,
    BoolContext boolCtx) {
  using Kind = type_system::TypeRef::Kind;
  switch (typeRef.kind()) {
    case Kind::STRUCT: {
      auto structOp = makeStructOp(typeRef.asStruct(), ops);
      // Protobuf nested structs are length-delimited
      if (ops.containerFraming == ContainerFraming::None) {
        structOp.readLengthDelimited = true;
        structOp.writeLengthDelimited = true;
      }
      return Command{std::move(structOp)};
    }
    case Kind::UNION: {
      auto structOp = makeStructOp(typeRef.asUnion(), ops);
      if (ops.containerFraming == ContainerFraming::None) {
        structOp.readLengthDelimited = true;
        structOp.writeLengthDelimited = true;
      }
      return Command{std::move(structOp)};
    }
    case Kind::OPAQUE_ALIAS:
      return commandForType(typeRef.asOpaqueAlias().targetType(), ops, boolCtx);
    case Kind::LIST: {
      auto elemTypeRef = typeRef.asList().elementType();
      auto seqOp = makeSeqOp(elemTypeRef, ops);
      // Protobuf: scalar element types use packed (ByBytes),
      // non-scalar (string, struct, list, map) use unpacked (repeated field
      // tags)
      if (ops.containerFraming == ContainerFraming::None) {
        if (isProtoPackable(elemTypeRef)) {
          seqOp.readLoopKind = LoopKind::ByBytes;
          seqOp.writeLoopKind = LoopKind::ByBytes;
        }
        // Unpacked non-scalar types are handled as repeated fields
        // in the parent StructOp via isRepeated flag on FieldEntry.
        // The SeqOp itself still uses ByCount on the Thrift side.
      }
      return Command{std::move(seqOp)};
    }
    case Kind::SET: {
      auto elemTypeRef = typeRef.asSet().elementType();
      auto seqOp = makeSeqOp(elemTypeRef, ops);
      if (ops.containerFraming == ContainerFraming::None) {
        if (isProtoPackable(elemTypeRef)) {
          seqOp.readLoopKind = LoopKind::ByBytes;
          seqOp.writeLoopKind = LoopKind::ByBytes;
        }
      }
      return Command{std::move(seqOp)};
    }
    case Kind::MAP:
      // makeMapOp emits protobuf-specific framing (length-delimited submessage
      // entries with key=1/value=2) when the protocol has no container framing.
      return Command{makeMapOp(
          typeRef.asMap().keyType(), typeRef.asMap().valueType(), ops)};
    case Kind::BOOL:
    case Kind::BYTE:
    case Kind::I16:
    case Kind::I32:
    case Kind::I64:
    case Kind::FLOAT:
    case Kind::DOUBLE:
    case Kind::STRING:
    case Kind::BINARY:
    case Kind::ENUM: {
      auto scalar = ops.scalarFn(typeRef, boolCtx);
      if (typeRef.kind() == Kind::ENUM) {
        scalar.enumNames = makeEnumNames(typeRef.asEnum());
      }
      return Command{std::move(scalar)};
    }
    case Kind::ANY:
      throwUnsupportedTypeRef("Transcode codec", typeRef);
  }
  throwUnsupportedTypeRef("Transcode codec", typeRef);
}

// ─────────────────────────────────────────────────────────────────────────
// Protocol definitions
// ─────────────────────────────────────────────────────────────────────────

const ProtocolOps kCompactOps{
    .scalarFn = compactScalarOp,
    .typeInfoFn = compactTypeInfo,
    .fieldIdent = FieldIdent::ById,
    .fieldProto = FieldProto::Compact,
    .readFieldHeader = "thrift_transcode_compact_read_field_header",
    .writeFieldHeader = "thrift_transcode_compact_write_field_header",
    .readStructEnd = "",
    .writeStructEnd = "thrift_transcode_compact_write_stop",
    .skipField = "thrift_transcode_compact_skip_field",
    .containerFraming = ContainerFraming::Compact,
};

const ProtocolOps kProtobufOps{
    .scalarFn = protobufScalarOp,
    .typeInfoFn = protoTypeInfo,
    .fieldIdent = FieldIdent::ById,
    .fieldProto = FieldProto::Protobuf,
    .readFieldHeader = "thrift_transcode_proto_read_field_header",
    .writeFieldHeader = "thrift_transcode_proto_write_field_header",
    .readStructEnd = "",
    .writeStructEnd = "thrift_transcode_proto_write_stop",
    .skipField = "thrift_transcode_proto_skip_field",
    .containerFraming = ContainerFraming::None, // protobuf uses repeated fields
};

const ProtocolOps kBinaryOps{
    .scalarFn = binaryScalarOp,
    .typeInfoFn = binaryTypeInfo,
    .fieldIdent = FieldIdent::ById,
    .fieldProto = FieldProto::Binary,
    .readFieldHeader = "thrift_transcode_binary_read_field_header",
    .writeFieldHeader = "thrift_transcode_binary_write_field_header",
    .readStructEnd = "",
    .writeStructEnd = "thrift_transcode_binary_write_stop",
    .skipField = "thrift_transcode_binary_skip_field",
    .containerFraming = ContainerFraming::Binary,
};

const ProtocolOps kJsonOps{
    .scalarFn = jsonScalarOp,
    .typeInfoFn = compactTypeInfo, // unused for JSON — no type bytes
    .fieldIdent = FieldIdent::ByName,
    .fieldProto = FieldProto::Unsupported,
    .readFieldHeader = "", // inlined
    .writeFieldHeader = "", // inlined
    .readStructEnd = "", // inlined
    .writeStructEnd = "", // inlined
    .skipField = "thrift_transcode_skip_json_value",
    .containerFraming = ContainerFraming::Json,
};

} // namespace

Codec makeThriftCompactCodec(const type_system::StructNode& node) {
  Codec codec;
  codec.name = "compact_" + std::string(node.debugName());
  codec.protocol = WireProtocol::ThriftCompact;
  codec.root = Command{makeStructOp(node, kCompactOps)};
  return codec;
}

Codec makeThriftCompactCodec(const type_system::UnionNode& node) {
  Codec codec;
  codec.name = "compact_" + std::string(node.debugName());
  codec.protocol = WireProtocol::ThriftCompact;
  codec.root = Command{makeStructOp(node, kCompactOps)};
  return codec;
}

Codec makeThriftBinaryCodec(const type_system::StructNode& node) {
  Codec codec;
  codec.name = "binary_" + std::string(node.debugName());
  codec.protocol = WireProtocol::ThriftBinary;
  codec.root = Command{makeStructOp(node, kBinaryOps)};
  return codec;
}

Codec makeThriftBinaryCodec(const type_system::UnionNode& node) {
  Codec codec;
  codec.name = "binary_" + std::string(node.debugName());
  codec.protocol = WireProtocol::ThriftBinary;
  codec.root = Command{makeStructOp(node, kBinaryOps)};
  return codec;
}

Codec makeProtobufBinaryCodec(const type_system::StructNode& node) {
  Codec codec;
  codec.name = "protobuf_" + std::string(node.debugName());
  codec.protocol = WireProtocol::ProtobufBinary;
  codec.root = Command{makeStructOp(node, kProtobufOps)};
  return codec;
}

Codec makeProtobufBinaryCodec(const type_system::UnionNode& node) {
  Codec codec;
  codec.name = "protobuf_" + std::string(node.debugName());
  codec.root = Command{makeStructOp(node, kProtobufOps)};
  return codec;
}

Codec makeJsonCodec(const type_system::StructNode& node) {
  Codec codec;
  codec.name = "json_" + std::string(node.debugName());
  codec.protocol = WireProtocol::Json;
  codec.root = Command{makeStructOp(node, kJsonOps)};
  return codec;
}

Codec makeJsonCodec(const type_system::UnionNode& node) {
  Codec codec;
  codec.name = "json_" + std::string(node.debugName());
  codec.protocol = WireProtocol::Json;
  codec.root = Command{makeStructOp(node, kJsonOps)};
  return codec;
}

} // namespace apache::thrift::transcode
