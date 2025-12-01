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
void serialize(ProtocolWriter&, const Any&) {
  throw std::logic_error("Unimplemented: serialize(Any)");
}

// List
template <typename ProtocolWriter>
void serialize(ProtocolWriter& writer, const List& list);

// Set
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Set&) {
  throw std::logic_error("Unimplemented: serialize(Set)");
}

// Map
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Map&) {
  throw std::logic_error("Unimplemented: serialize(Map)");
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
    ProtocolReader&, type_system::TypeRef::Any, std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Any)");
}

// List
template <typename ProtocolReader>
List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc);

// Set
template <typename ProtocolReader>
Set deserialize(
    ProtocolReader&,
    const type_system::TypeRef::Set&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Set)");
}

// Map
template <typename ProtocolReader>
Map deserialize(
    ProtocolReader&,
    const type_system::TypeRef::Map&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Map)");
}

// Struct
template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader&,
    const type_system::StructNode&,
    std::pmr::memory_resource*);

// Union
template <typename ProtocolReader>
Union deserialize(
    ProtocolReader&, const type_system::UnionNode&, std::pmr::memory_resource*);

// Unsupported types
template <typename ProtocolReader>
int deserialize(
    ProtocolReader&,
    const type_system::OpaqueAliasNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(OpaqueAliasNode)");
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
 * Deserialize a DynamicValue from a protocol reader.
 */
template <typename ProtocolReader>
DynamicValue deserializeValue(
    ProtocolReader& prot,
    type_system::TypeRef type,
    std::pmr::memory_resource* mr) {
  return DynamicValue(type, type.visit([&](auto&& t) {
    return detail::Datum::make(deserialize(prot, t, mr));
  }));
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
template <typename ProtocolReader>
List deserialize(
    ProtocolReader& reader,
    const type_system::TypeRef::List& type,
    std::pmr::memory_resource* alloc) {
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

        DCHECK_EQ(impl->size(), 0);
        auto& data = impl->elements();

        protocol::TType ttype;
        uint32_t size;
        reader.readListBegin(ttype, size);
        if (!size) {
          reader.readListEnd();
          return List(detail::IList::Ptr(impl));
        }

        if (type_system::ToTTypeFn{}(type.elementType()) != ttype) {
          throw std::runtime_error(
              fmt::format(
                  "type mismatch in deserialization: {} vs {}",
                  ttype,
                  type_system::ToTTypeFn{}(type.elementType())));
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
            bool value = deserialize(reader, elemTypeNode, alloc);
            data.emplace_back(static_cast<std::byte>(value));
          }
        } else {
          data.reserve(size);
          for (; size > 0; --size) {
            data.emplace_back(deserialize(reader, elemTypeNode, alloc));
          }
        }
        reader.readListEnd();
        return List(detail::IList::Ptr(impl));
      });
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

template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader& reader,
    const type_system::StructNode& type,
    std::pmr::memory_resource* alloc) {
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

    // Find the field
    Struct::FieldId id{fid};
    auto handle = [&]() {
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
    }();

    if (handle.valid()) {
      const auto& field = type.at(handle);

      if (ftype != field.wireType()) {
        throw std::runtime_error(
            fmt::format(
                "Expected field {} on struct {} to have wire-type {} but got {}",
                field.identity().name(),
                type.uri(),
                field.wireType(),
                ftype));
      }

      // Use virtual interface to set the field
      ret.setField(handle, deserializeValue(reader, field.type(), alloc));
    } else {
      // Unknown field
      // TODO: store instead of skipping
      reader.skip(ftype);
    }

    reader.readFieldEnd();
  }

  reader.readStructEnd();

  return ret;
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

template <typename ProtocolReader>
Union deserialize(
    ProtocolReader& reader,
    const type_system::UnionNode& type,
    std::pmr::memory_resource* alloc) {
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

  // Find the field
  Union::FieldId id{fid};
  auto handle = type.fieldHandleFor(id);

  if (handle.valid()) {
    const auto& field = type.at(handle);

    if (ftype != field.wireType()) {
      throw std::runtime_error(
          fmt::format(
              "Expected field {} on union {} to have wire-type {} but got {}",
              field.identity().name(),
              type.uri(),
              field.wireType(),
              ftype));
    }

    ret.activeFieldDef_ = &field;
    ret.activeFieldData_ = field.type().visit([&](auto&& t) {
      return ret.makeDatumPtr(
          detail::Datum::make(deserialize(reader, t, alloc)));
    });
  } else {
    // Unknown field - skip it
    reader.skip(ftype);
  }

  reader.readFieldEnd();

  // Read field stop
  reader.readFieldBegin(name, ftype, fid);
  if (ftype != protocol::T_STOP) {
    throw std::runtime_error(
        "Union cannot have more than one field during deserialization");
  }

  reader.readStructEnd();

  return ret;
}

} // namespace apache::thrift::dynamic
