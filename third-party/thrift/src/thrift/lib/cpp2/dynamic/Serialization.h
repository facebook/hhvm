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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/Union.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteList.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/Traits.h>

#include <glog/logging.h>
#include <folly/Conv.h>
#include <folly/lang/Assume.h>
#include <folly/memory/not_null.h>

#include <memory_resource>

namespace apache::thrift::dynamic {

namespace detail {

// Default validation callbacks that skip unknown fields and throw on errors
struct ThrowingValidationCallbacks {
  void onUnknownField(
      const type_system::StructuredNode&,
      int16_t,
      std::string_view,
      protocol::TType) {}

  void onTypeMismatch(
      std::string_view context,
      protocol::TType expected,
      protocol::TType actual) {
    throw std::runtime_error(
        fmt::format(
            "type mismatch in {}: {} vs {}", context, expected, actual));
  }

  void onMultipleUnionFields(const type_system::UnionNode&, uint32_t) {
    throw std::runtime_error(
        "Union cannot have more than one field during deserialization");
  }
};

} // namespace detail

// ============================================================================
// serialize - converts Datum to wire format
// ============================================================================

// Serialize arithmetic types
template <typename T, typename ProtocolWriter>
  requires std::is_arithmetic_v<T>
void serialize(ProtocolWriter& writer, T value) {
  op::encode<type::infer_tag<T>>(writer, value);
}

// String
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const String& str) {
  writer.writeString(str.view());
}

// Binary
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Binary& binary) {
  if (binary.data_) {
    writer.writeBinary(*binary.data_);
  } else {
    // Empty binary
    writer.writeBinary(folly::IOBuf());
  }
}

// Any
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Any& any) {
  // Serialize the underlying AnyData as an AnyStruct
  op::encode<type::struct_t<type::AnyStruct>>(writer, any.data_.toThrift());
}

// List
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const List& list);

// Set
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Set& set) {
  auto elemType = set.elementType();
  writer.writeSetBegin(
      type_system::ToTTypeFn{}(elemType), folly::to_narrow(set.size()));

  for (auto elemRef : set) {
    serializeValue(writer, elemRef);
  }

  writer.writeSetEnd();
}

// Map
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Map& map) {
  auto keyType = map.keyType();
  auto valueType = map.valueType();
  writer.writeMapBegin(
      type_system::ToTTypeFn{}(keyType),
      type_system::ToTTypeFn{}(valueType),
      folly::to_narrow(map.size()));

  for (auto [keyRef, valueRef] : map) {
    serializeValue(writer, keyRef);
    serializeValue(writer, valueRef);
  }

  writer.writeMapEnd();
}

// Struct
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Struct&);

// Union
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Union&);

// Null
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, Null) {
  throw std::logic_error("Serializing Null is not possible.");
}

// ============================================================================
// deserialize - converts wire format to Datum
// ============================================================================

// Deserialize functions for TypeRef types
template <typename ProtocolReader>
bool deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Bool,
    std::pmr::memory_resource*) {
  bool ret;
  op::decode<type::bool_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int8_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Byte,
    std::pmr::memory_resource*) {
  int8_t ret;
  op::decode<type::byte_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int16_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I16,
    std::pmr::memory_resource*) {
  int16_t ret;
  op::decode<type::i16_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int32_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I32,
    std::pmr::memory_resource*) {
  int32_t ret;
  op::decode<type::i32_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int64_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I64,
    std::pmr::memory_resource*) {
  int64_t ret;
  op::decode<type::i64_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
float deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Float,
    std::pmr::memory_resource*) {
  float ret;
  op::decode<type::float_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
double deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Double,
    std::pmr::memory_resource*) {
  double ret;
  op::decode<type::double_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int32_t deserialize(
    ProtocolReader& reader,
    const type_system::EnumNode&,
    std::pmr::memory_resource*) {
  int32_t ret;
  op::decode<type::i32_t>(reader, ret);
  return ret;
}

// String
template <typename ProtocolReader>
String deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::String,
    std::pmr::memory_resource* mr) {
  std::string temp;
  reader.readString(temp);
  return String(temp, mr);
}

// Binary
template <typename ProtocolReader>
Binary deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Binary,
    std::pmr::memory_resource* mr) {
  std::unique_ptr<folly::IOBuf> buf;
  reader.readBinary(buf);
  return Binary(std::move(buf), mr);
}

// Any
template <typename ProtocolReader>
Any deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Any,
    std::pmr::memory_resource* mr) {
  // Deserialize an AnyStruct from the protocol
  type::AnyStruct anyStruct;
  op::decode<type::struct_t<type::AnyStruct>>(reader, anyStruct);
  return Any(type::AnyData(std::move(anyStruct)), mr);
}

// List
template <typename ProtocolReader>
List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc);

template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>
List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks);

// Set
template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>
Set deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::Set& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks) {
  Set ret(type, alloc);

  protocol::TType ttype;
  uint32_t size;
  reader.readSetBegin(ttype, size);

  // Handle protocols that omit container sizes (e.g., SimpleJSON)
  if constexpr (ProtocolReader::kOmitsContainerSizes()) {
    while (reader.peekSet()) {
      ret.insert(
          deserializeValue(reader, type.elementType(), alloc, callbacks));
    }
    reader.readSetEnd();
    return ret;
  }

  if (!size) {
    reader.readSetEnd();
    return ret;
  }

  // T_VOID indicates protocol doesn't encode type information (e.g.,
  // SimpleJSON)
  auto expected = type_system::ToTTypeFn{}(type.elementType());
  if (ttype != protocol::TType::T_VOID && expected != ttype) {
    // If callback didn't throw, skip remaining set data and return empty
    for (; size > 0; --size) {
      callbacks.onTypeMismatch("set element", expected, ttype);
      reader.skip(ttype);
    }
    reader.readSetEnd();
    return ret;
  }

  if (!apache::thrift::canReadNElements(reader, size, {ttype})) {
    protocol::TProtocolException::throwTruncatedData();
  }

  ret.reserve(size);
  for (; size > 0; --size) {
    ret.insert(deserializeValue(reader, type.elementType(), alloc, callbacks));
  }

  reader.readSetEnd();
  return ret;
}

template <typename ProtocolReader>
Set deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::Set& type,
    std::pmr::memory_resource* alloc) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserialize(reader, type, alloc, callbacks);
}

// Map
template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

Map deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::Map& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks) {
  Map ret(type, alloc);

  protocol::TType keyTType;
  protocol::TType valueTType;
  uint32_t size;
  reader.readMapBegin(keyTType, valueTType, size);

  // Handle protocols that omit container sizes (e.g., SimpleJSON)
  if constexpr (ProtocolReader::kOmitsContainerSizes()) {
    while (reader.peekMap()) {
      auto key = deserializeValue(reader, type.keyType(), alloc, callbacks);
      auto value = deserializeValue(reader, type.valueType(), alloc, callbacks);
      ret.insert(std::move(key), std::move(value));
    }
    reader.readMapEnd();
    return ret;
  }

  if (!size) {
    reader.readMapEnd();
    return ret;
  }

  // T_VOID indicates protocol doesn't encode type information (e.g.,
  // SimpleJSON)
  auto expectedKey = type_system::ToTTypeFn{}(type.keyType());
  if (keyTType != protocol::TType::T_VOID && expectedKey != keyTType) {
    // If callback didn't throw, skip remaining map data and return empty
    for (; size > 0; --size) {
      callbacks.onTypeMismatch("map key", expectedKey, keyTType);
      reader.skip(keyTType);
      reader.skip(valueTType);
    }
    reader.readMapEnd();
    return ret;
  }

  auto expectedValue = type_system::ToTTypeFn{}(type.valueType());
  if (valueTType != protocol::TType::T_VOID && expectedValue != valueTType) {
    // If callback didn't throw, skip remaining map data and return empty
    for (; size > 0; --size) {
      callbacks.onTypeMismatch("map value", expectedValue, valueTType);
      reader.skip(keyTType);
      reader.skip(valueTType);
    }
    reader.readMapEnd();
    return ret;
  }

  if (!apache::thrift::canReadNElements(reader, size, {keyTType, valueTType})) {
    protocol::TProtocolException::throwTruncatedData();
  }

  ret.reserve(size);
  for (; size > 0; --size) {
    auto key = deserializeValue(reader, type.keyType(), alloc, callbacks);
    auto value = deserializeValue(reader, type.valueType(), alloc, callbacks);
    ret.insert(std::move(key), std::move(value));
  }

  reader.readMapEnd();
  return ret;
}

template <typename ProtocolReader>
Map deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::Map& type,
    std::pmr::memory_resource* alloc) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserialize(reader, type, alloc, callbacks);
}

// Struct
template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader&,
    const type_system::StructNode&,
    std::pmr::memory_resource*);

template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

Struct deserialize(
    ProtocolReader&,
    const type_system::StructNode&,
    std::pmr::memory_resource*,
    Callbacks&);

// Union
template <typename ProtocolReader>
Union deserialize(
    ProtocolReader&, const type_system::UnionNode&, std::pmr::memory_resource*);

template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

Union deserialize(
    ProtocolReader&,
    const type_system::UnionNode&,
    std::pmr::memory_resource*,
    Callbacks&);

// Unsupported types
template <typename ProtocolReader>
int deserialize(
    ProtocolReader&,
    const type_system::OpaqueAliasNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(OpaqueAliasNode)");
}

template <
    typename ProtocolReader,
    typename T,
    DeserializeValidationCallbacks Callbacks>
  requires(!folly::is_instantiation_of_v<folly::not_null, std::decay_t<T>>)
auto deserialize(
    ProtocolReader& reader,
    T&& type,
    std::pmr::memory_resource* mr,
    Callbacks&) {
  return deserialize(reader, std::forward<T>(type), mr);
}

// Helper overload for not_null pointers
template <typename ProtocolReader, typename T>
auto deserialize(
    ProtocolReader& reader, folly::not_null<const T*> t, auto&&... args) {
  return deserialize(reader, *t, std::forward<decltype(args)>(args)...);
}

// ============================================================================
// serializeValue/deserializeValue - high-level serialization for DynamicValue
// ============================================================================

/**
 * Deserialize a DynamicValue from a protocol reader with validation callbacks.
 */
template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

DynamicValue deserializeValue(
    ProtocolReader& prot,
    type_system::TypeRef type,
    std::pmr::memory_resource* mr,
    Callbacks& callbacks) {
  return DynamicValue(type, type.visit([&](auto&& t) {
    return detail::Datum::make(deserialize(prot, t, mr, callbacks));
  }));
}

/**
 * Deserialize a DynamicValue from a protocol reader.
 */
template <typename ProtocolReader>
DynamicValue deserializeValue(
    ProtocolReader& prot,
    type_system::TypeRef type,
    std::pmr::memory_resource* mr) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserializeValue(prot, type, mr, callbacks);
}

/**
 * Serialize a DynamicConstRef to a protocol writer.
 */
template <typename ProtocolWriter>
void serializeValue(ProtocolWriter& prot, const DynamicConstRef& v) {
  using Kind = type_system::TypeRef::Kind;
  v.type().matchKind([&]<Kind k>(type_system::TypeRef::KindConstant<k>) {
    serialize(prot, v.as<k>());
  });
}

// ============================================================================
// List serialization - needs ConcreteList to be complete
// ============================================================================

namespace detail {

// Helper functions for serializing list elements
template <typename ProtocolWriter, typename T>
  requires(
      ProtocolWriter::kSupportsArithmeticVectors() &&
      !std::is_same_v<T, bool> && std::is_arithmetic_v<T>)
void serializeListElements(
    ProtocolWriter& writer, const ConcreteList<T>& data) {
  writer.template writeArithmeticVector<T>(
      data.elements().data(), data.elements().size());
}

// Specialization for bool (which uses std::byte storage)
template <typename ProtocolWriter>
void serializeListElements(
    ProtocolWriter& writer, const ConcreteList<bool>& data) {
  for (const auto& elt : data.elements()) {
    serialize(writer, static_cast<bool>(elt));
  }
}

template <typename ProtocolWriter, typename T>
void serializeListElements(
    ProtocolWriter& writer, const ConcreteList<T>& data) {
  for (const T& elt : data.elements()) {
    serialize(writer, elt);
  }
}

} // namespace detail

// Serialize List - overrides the placeholder in the serialize section above
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const List& list) {
  const detail::IList* impl = list.impl_.get();
  if (!impl) {
    writer.writeListBegin(protocol::TType::T_VOID, 0);
    writer.writeListEnd();
    return;
  }
  impl->visit([&]<typename T>(const detail::ConcreteList<T>& data) {
    writer.writeListBegin(
        type_system::ToTTypeFn{}(data.elementType()),
        folly::to_narrow(data.size()));
    detail::serializeListElements(writer, data);
    writer.writeListEnd();
  });
}

// Deserialize List - overrides the placeholder in the deserialize section above
template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks) {
  return type.elementType().matchKind(
      [&]<type_system::TypeRef::Kind elemTypeKind>(
          type_system::TypeRef::KindConstant<elemTypeKind>) -> List {
        using elemDatumType = detail::type_of_type_kind<elemTypeKind>;
        using elemStorageType = detail::storage_type_t<elemDatumType>;
        const auto& elemTypeNode = type.elementType().asKind<elemTypeKind>();

        // Allocate ConcreteList using pmr
        auto* impl = alloc
            ? std::pmr::polymorphic_allocator<>(alloc)
                  .template new_object<detail::ConcreteList<elemDatumType>>(
                      type, alloc)
            : new detail::ConcreteList<elemDatumType>(type, alloc);
        detail::IList::Ptr implPtr(impl);

        DCHECK_EQ(impl->size(), 0);
        auto& data = impl->elements();

        protocol::TType ttype;
        uint32_t size;
        reader.readListBegin(ttype, size);

        // Handle protocols that omit container sizes (e.g., SimpleJSON)
        if constexpr (ProtocolReader::kOmitsContainerSizes()) {
          while (reader.peekList()) {
            if constexpr (std::is_same_v<elemDatumType, bool>) {
              bool value = deserialize(reader, elemTypeNode, alloc, callbacks);
              data.emplace_back(static_cast<std::byte>(value));
            } else {
              data.emplace_back(
                  deserialize(reader, elemTypeNode, alloc, callbacks));
            }
          }
          reader.readListEnd();
          return List(std::move(implPtr));
        } else {
          if (!size) {
            reader.readListEnd();
            return List(std::move(implPtr));
          }

          // T_VOID indicates protocol doesn't encode type information
          auto expected = type_system::ToTTypeFn{}(type.elementType());
          if (ttype != protocol::TType::T_VOID && expected != ttype) {
            // If callback didn't throw, skip remaining list data, return empty
            for (; size > 0; --size) {
              callbacks.onTypeMismatch("list element", expected, ttype);
              reader.skip(ttype);
            }
            reader.readListEnd();
            return List(std::move(implPtr));
          }

          if (!apache::thrift::canReadNElements(reader, size, {ttype})) {
            protocol::TProtocolException::throwTruncatedData();
          }

          if constexpr (
              ProtocolReader::kSupportsArithmeticVectors() &&
              !std::is_same_v<elemDatumType, bool> &&
              std::is_arithmetic_v<elemStorageType>) {
            data.resize(size);
            reader.template readArithmeticVector<elemStorageType>(
                data.data(), size);
          } else if constexpr (std::is_same_v<elemDatumType, bool>) {
            // For bool elements (stored as std::byte), deserialize and cast
            data.reserve(size);
            for (; size > 0; --size) {
              bool value = deserialize(reader, elemTypeNode, alloc, callbacks);
              data.emplace_back(static_cast<std::byte>(value));
            }
          } else {
            data.reserve(size);
            for (; size > 0; --size) {
              data.emplace_back(
                  deserialize(reader, elemTypeNode, alloc, callbacks));
            }
          }
          reader.readListEnd();
          return List(std::move(implPtr));
        }
      });
}

template <typename ProtocolReader>
List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserialize(reader, type, alloc, callbacks);
}

// ============================================================================
// Struct serialization
// ============================================================================

template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Struct& structValue) {
  const auto& structNode = structValue.type().asStructUnchecked();
  const auto& fieldDefs = structNode.fields();

  writer.writeStructBegin(structNode.uri().data());

  // Iterate through fields using the type system and virtual interface
  for (size_t i = 0; i < fieldDefs.size(); ++i) {
    const auto& fieldDef = fieldDefs[i];
    auto handle = type_system::FastFieldHandle{static_cast<uint16_t>(i + 1)};

    // Skip unset optional fields
    if (!structValue.hasField(handle)) {
      continue;
    }

    writer.writeFieldBegin(
        fieldDef.identity().name().data(),
        fieldDef.wireType(),
        static_cast<int16_t>(fieldDef.identity().id()));

    // Get the field value through the virtual interface
    auto fieldRef = structValue.getField(handle);

    // Serialize the field value through the reference
    // We know fieldRef has a value because hasField returned true
    serializeValue(writer, *fieldRef);

    writer.writeFieldEnd();
  }

  writer.writeFieldStop();
  writer.writeStructEnd();
}

template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

Struct deserialize(
    ProtocolReader& reader,
    const type_system::StructNode& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks) {
  Struct ret(type, alloc);

  std::string name;
  int16_t fid;
  protocol::TType ftype;
  uint16_t pos = 0;

  reader.readStructBegin(name);

  while (true) {
    reader.readFieldBegin(name, ftype, fid);
    if (ftype == protocol::T_STOP) {
      break;
    }

    // Find the field - use field name for name-based protocols, ID otherwise
    auto handle = [&]() {
      if constexpr (ProtocolReader::kUsesFieldNames()) {
        // Field name-based protocol (e.g., SimpleJSON)
        return type.fieldHandleFor(name);
      } else {
        // Field ID-based protocol (e.g., Compact, Binary)
        Struct::FieldId id{fid};
        // Optimize the case where fields are serialized in order.
        for (; pos < type.fields().size(); pos++) {
          auto& nextField = type.fields()[pos];
          // Happy path: we read the next field
          if (nextField.identity().id() == id) {
            return type_system::FastFieldHandle{++pos};
          }
          // We might have skipped an optional/terse field, so try the next one.
        }
        // Unknown field or not in order
        return type.fieldHandleFor(id);
      }
    }();

    if (handle.valid()) {
      const auto& field = type.at(handle);

      // T_VOID indicates protocol doesn't encode type information (e.g.,
      // SimpleJSON)
      if (ftype != protocol::TType::T_VOID && ftype != field.wireType()) {
        callbacks.onTypeMismatch(
            fmt::format(
                "field '{}' on struct '{}'",
                field.identity().name(),
                type.uri()),
            field.wireType(),
            ftype);
        // If callback didn't throw, skip the mismatched field data
        reader.skip(ftype);
        reader.readFieldEnd();
        continue;
      }

      // Use virtual interface to set the field
      ret.setField(
          handle, deserializeValue(reader, field.type(), alloc, callbacks));
    } else {
      // Unknown field
      callbacks.onUnknownField(type, fid, name, ftype);
      reader.skip(ftype);
    }

    reader.readFieldEnd();
  }

  reader.readStructEnd();

  return ret;
}

template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader& reader,
    const type_system::StructNode& type,
    std::pmr::memory_resource* alloc) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserialize(reader, type, alloc, callbacks);
}

// ============================================================================
// Union serialization
// ============================================================================

template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const Union& unionValue) {
  const auto& unionNode = unionValue.type().asUnionUnchecked();

  writer.writeStructBegin(unionNode.uri().data());

  // Serialize the active field if the union is not empty
  if (!unionValue.isEmpty()) {
    const auto& fieldDef = *unionValue.activeFieldDef_;
    const auto& datum = *unionValue.activeFieldData_;

    writer.writeFieldBegin(
        fieldDef.identity().name().data(),
        fieldDef.wireType(),
        static_cast<int16_t>(fieldDef.identity().id()));

    // Serialize the field value
    datum.visit([&](const auto& value) { serialize(writer, value); });

    writer.writeFieldEnd();
  }

  writer.writeFieldStop();
  writer.writeStructEnd();
}

template <typename ProtocolReader, DeserializeValidationCallbacks Callbacks>

Union deserialize(
    ProtocolReader& reader,
    const type_system::UnionNode& type,
    std::pmr::memory_resource* alloc,
    Callbacks& callbacks) {
  Union ret(type, alloc);

  std::string name;
  int16_t fid;
  protocol::TType ftype;

  reader.readStructBegin(name);

  // Read the first (and only) field
  reader.readFieldBegin(name, ftype, fid);

  if (ftype == protocol::T_STOP) {
    // Empty union
    reader.readStructEnd();
    return ret;
  }

  // Find the field - use field name for name-based protocols, ID otherwise
  auto handle = [&]() {
    if constexpr (ProtocolReader::kUsesFieldNames()) {
      // Field name-based protocol (e.g., SimpleJSON)
      return type.fieldHandleFor(name);
    } else {
      // Field ID-based protocol (e.g., Compact, Binary)
      Union::FieldId id{fid};
      return type.fieldHandleFor(id);
    }
  }();

  if (handle.valid()) {
    const auto& field = type.at(handle);

    // T_VOID indicates protocol doesn't encode type information (e.g.,
    // SimpleJSON)
    if (ftype != protocol::TType::T_VOID && ftype != field.wireType()) {
      callbacks.onTypeMismatch(
          fmt::format(
              "field '{}' on union '{}'", field.identity().name(), type.uri()),
          field.wireType(),
          ftype);
      // If callback didn't throw, skip the mismatched field data
      reader.skip(ftype);
    } else {
      ret.activeFieldDef_ = &field;
      ret.activeFieldData_ = field.type().visit([&](auto&& t) {
        return ret.makeDatumPtr(
            detail::Datum::make(deserialize(reader, t, alloc, callbacks)));
      });
    }
  } else {
    // Unknown field
    callbacks.onUnknownField(type, fid, name, ftype);
    reader.skip(ftype);
  }

  reader.readFieldEnd();

  // Read field stop
  reader.readFieldBegin(name, ftype, fid);
  if (ftype != protocol::T_STOP) {
    // Drain remaining fields and count them
    uint32_t totalFields = 1; // first field already deserialized
    while (ftype != protocol::T_STOP) {
      totalFields++;
      reader.skip(ftype);
      reader.readFieldEnd();
      reader.readFieldBegin(name, ftype, fid);
    }
    callbacks.onMultipleUnionFields(type, totalFields);
  }

  reader.readStructEnd();

  return ret;
}

template <typename ProtocolReader>
Union deserialize(
    ProtocolReader& reader,
    const type_system::UnionNode& type,
    std::pmr::memory_resource* alloc) {
  detail::ThrowingValidationCallbacks callbacks;
  return deserialize(reader, type, alloc, callbacks);
}

} // namespace apache::thrift::dynamic
