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

#include <thrift/lib/cpp2/dynamic/AnnotationValue.h>

#include <span>
#include <stdexcept>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/json/dynamic.h>
#include <folly/lang/Assume.h>

#include <thrift/lib/cpp2/dynamic/Binary.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/String.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/Union.h>

namespace apache::thrift::dynamic {

DynamicValue toDynamicValue(
    const folly::dynamic& value, const type_system::TypeRef& type) {
  using type_system::TypeRef;
  switch (type.kind()) {
    case TypeRef::Kind::BOOL:
      return DynamicValue::makeBool(value.asBool());
    case TypeRef::Kind::BYTE:
      return DynamicValue::makeByte(static_cast<int8_t>(value.asInt()));
    case TypeRef::Kind::I16:
      return DynamicValue::makeI16(static_cast<int16_t>(value.asInt()));
    case TypeRef::Kind::I32:
      return DynamicValue::makeI32(static_cast<int32_t>(value.asInt()));
    case TypeRef::Kind::I64:
      return DynamicValue::makeI64(value.asInt());
    case TypeRef::Kind::FLOAT:
      return DynamicValue::makeFloat(static_cast<float>(value.asDouble()));
    case TypeRef::Kind::DOUBLE:
      return DynamicValue::makeDouble(value.asDouble());
    case TypeRef::Kind::STRING:
      return DynamicValue::makeString(value.asString());
    case TypeRef::Kind::BINARY:
      return DynamicValue::makeBinary(
          folly::IOBuf::copyBuffer(value.asString()));
    case TypeRef::Kind::ENUM: {
      const auto& enumNode = type.asEnum();
      if (value.isInt()) {
        return DynamicValue::makeEnum(
            enumNode, static_cast<int32_t>(value.asInt()));
      }
      const auto name = value.asString();
      for (const auto& enumValue : enumNode.values()) {
        if (enumValue.name == name) {
          return DynamicValue::makeEnum(enumNode, enumValue.i32);
        }
      }
      throw std::runtime_error("Unknown enum value in annotation: " + name);
    }
    case TypeRef::Kind::LIST: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& elementType = type.asList().elementType();
      auto& list = result.asList();
      for (const auto& element : value) {
        list.push_back(toDynamicValue(element, elementType));
      }
      return result;
    }
    case TypeRef::Kind::SET: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& elementType = type.asSet().elementType();
      auto& set = result.asSet();
      for (const auto& element : value) {
        set.insert(toDynamicValue(element, elementType));
      }
      return result;
    }
    case TypeRef::Kind::MAP: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& keyType = type.asMap().keyType();
      const auto& valueType = type.asMap().valueType();
      auto& map = result.asMap();
      for (const auto& [key, mapped] : value.items()) {
        map.insert(
            toDynamicValue(key, keyType), toDynamicValue(mapped, valueType));
      }
      return result;
    }
    case TypeRef::Kind::STRUCT: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& structNode = type.asStruct();
      auto& structValue = result.asStruct();
      for (const auto& [key, fieldValue] : value.items()) {
        const auto fieldName = key.asString();
        if (!structNode.hasField(fieldName)) {
          continue;
        }
        structValue.setField(
            fieldName,
            toDynamicValue(fieldValue, structNode.at(fieldName).type()));
      }
      return result;
    }
    case TypeRef::Kind::UNION: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& unionNode = type.asUnion();
      auto& unionValue = result.asUnion();
      for (const auto& [key, fieldValue] : value.items()) {
        const auto fieldName = key.asString();
        if (!unionNode.hasField(fieldName)) {
          continue;
        }
        unionValue.setField(
            fieldName,
            toDynamicValue(fieldValue, unionNode.at(fieldName).type()));
        break;
      }
      return result;
    }
    case TypeRef::Kind::OPAQUE_ALIAS:
      return toDynamicValue(value, type.asOpaqueAlias().targetType());
    case TypeRef::Kind::ANY:
      throw std::runtime_error("Any-typed annotation values are not supported");
  }
  throw std::runtime_error("Unsupported annotation value type");
}

namespace {

using type_system::SerializableRecord;

SerializableRecord::ByteArray toByteArray(const Binary& binary) {
  auto cursor = binary.cursor();
  return folly::IOBuf::copyBuffer(
      cursor.readFixedString(binary.computeChainDataLength()));
}

} // namespace

SerializableRecord toSerializableRecord(DynamicConstRef value) {
  using type_system::TypeRef;
  const TypeRef* type = &value.type();
  while (type->kind() == TypeRef::Kind::OPAQUE_ALIAS) {
    type = &type->asOpaqueAlias().targetType();
  }
  switch (type->kind()) {
    case TypeRef::Kind::BOOL:
      return SerializableRecord{SerializableRecord::Bool(value.asBool())};
    case TypeRef::Kind::BYTE:
      return SerializableRecord{SerializableRecord::Int8(value.asByte())};
    case TypeRef::Kind::I16:
      return SerializableRecord{SerializableRecord::Int16(value.asI16())};
    case TypeRef::Kind::I32:
      return SerializableRecord{SerializableRecord::Int32(value.asI32())};
    case TypeRef::Kind::I64:
      return SerializableRecord{SerializableRecord::Int64(value.asI64())};
    case TypeRef::Kind::FLOAT:
      return SerializableRecord{SerializableRecord::Float32(value.asFloat())};
    case TypeRef::Kind::DOUBLE:
      return SerializableRecord{SerializableRecord::Float64(value.asDouble())};
    case TypeRef::Kind::ENUM:
      return SerializableRecord{SerializableRecord::Int32(value.asEnum())};
    case TypeRef::Kind::STRING:
      return SerializableRecord{
          SerializableRecord::Text{value.asString().view()}};
    case TypeRef::Kind::BINARY:
      return SerializableRecord{toByteArray(value.asBinary())};
    case TypeRef::Kind::LIST: {
      SerializableRecord::List list;
      const auto& elements = value.asList();
      list.reserve(elements.size());
      for (DynamicConstRef element : elements) {
        list.push_back(toSerializableRecord(element));
      }
      return SerializableRecord{std::move(list)};
    }
    case TypeRef::Kind::SET: {
      SerializableRecord::Set set;
      const auto& elements = value.asSet();
      set.reserve(elements.size());
      for (DynamicConstRef element : elements) {
        set.insert(toSerializableRecord(element));
      }
      return SerializableRecord{std::move(set)};
    }
    case TypeRef::Kind::MAP: {
      SerializableRecord::Map map;
      const auto& entries = value.asMap();
      map.reserve(entries.size());
      for (const auto [key, mapped] : entries) {
        map.emplace(toSerializableRecord(key), toSerializableRecord(mapped));
      }
      return SerializableRecord{std::move(map)};
    }
    case TypeRef::Kind::STRUCT: {
      SerializableRecord::FieldSet fields;
      const auto& structValue = value.asStruct();
      for (const auto& field : type->asStruct().fields()) {
        const auto id = field.identity().id();
        if (auto fieldValue = structValue.getField(id)) {
          fields.emplace(id, toSerializableRecord(*fieldValue));
        }
      }
      return SerializableRecord{std::move(fields)};
    }
    case TypeRef::Kind::UNION: {
      SerializableRecord::FieldSet fields;
      const auto& unionValue = value.asUnion();
      if (!unionValue.isEmpty()) {
        const auto handle = unionValue.activeField();
        const auto id =
            type->asUnion().fields()[handle.index()].identity().id();
        fields.emplace(id, toSerializableRecord(unionValue.getField(handle)));
      }
      return SerializableRecord{std::move(fields)};
    }
    case TypeRef::Kind::OPAQUE_ALIAS:
      folly::assume_unreachable();
    case TypeRef::Kind::ANY:
      throw std::runtime_error("Any-typed annotation values are not supported");
  }
  throw std::runtime_error("Unsupported annotation value type");
}

DynamicValue fromSerializableRecord(
    const SerializableRecord& record, const type_system::TypeRef& type) {
  using type_system::TypeRef;
  switch (type.kind()) {
    case TypeRef::Kind::BOOL:
      return DynamicValue::makeBool(record.asBool());
    case TypeRef::Kind::BYTE:
      return DynamicValue::makeByte(record.asInt8());
    case TypeRef::Kind::I16:
      return DynamicValue::makeI16(record.asInt16());
    case TypeRef::Kind::I32:
      return DynamicValue::makeI32(record.asInt32());
    case TypeRef::Kind::I64:
      return DynamicValue::makeI64(record.asInt64());
    case TypeRef::Kind::FLOAT:
      return DynamicValue::makeFloat(record.asFloat32());
    case TypeRef::Kind::DOUBLE:
      return DynamicValue::makeDouble(record.asFloat64());
    case TypeRef::Kind::ENUM:
      return DynamicValue::makeEnum(type.asEnum(), record.asInt32());
    case TypeRef::Kind::STRING:
      return DynamicValue::makeString(record.asText());
    case TypeRef::Kind::BINARY:
      return DynamicValue::makeBinary(record.asByteArray()->clone());
    case TypeRef::Kind::LIST: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& elementType = type.asList().elementType();
      auto& list = result.asList();
      for (const auto& element : record.asList()) {
        list.push_back(fromSerializableRecord(element, elementType));
      }
      return result;
    }
    case TypeRef::Kind::SET: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& elementType = type.asSet().elementType();
      auto& set = result.asSet();
      for (const auto& element : record.asSet()) {
        set.insert(fromSerializableRecord(element, elementType));
      }
      return result;
    }
    case TypeRef::Kind::MAP: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& keyType = type.asMap().keyType();
      const auto& valueType = type.asMap().valueType();
      auto& map = result.asMap();
      for (const auto& [key, mapped] : record.asMap()) {
        map.insert(
            fromSerializableRecord(key, keyType),
            fromSerializableRecord(mapped, valueType));
      }
      return result;
    }
    case TypeRef::Kind::STRUCT: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& structNode = type.asStruct();
      auto& structValue = result.asStruct();
      for (const auto& [id, fieldRecord] : record.asFieldSet()) {
        if (!structNode.hasField(id)) {
          continue;
        }
        const auto& field = structNode.at(id);
        structValue.setField(
            field.identity().name(),
            fromSerializableRecord(fieldRecord, field.type()));
      }
      return result;
    }
    case TypeRef::Kind::UNION: {
      DynamicValue result = DynamicValue::makeDefault(type);
      const auto& unionNode = type.asUnion();
      auto& unionValue = result.asUnion();
      for (const auto& [id, fieldRecord] : record.asFieldSet()) {
        if (!unionNode.hasField(id)) {
          continue;
        }
        const auto& field = unionNode.at(id);
        unionValue.setField(
            field.identity().name(),
            fromSerializableRecord(fieldRecord, field.type()));
      }
      return result;
    }
    case TypeRef::Kind::OPAQUE_ALIAS:
      return fromSerializableRecord(record, type.asOpaqueAlias().targetType());
    case TypeRef::Kind::ANY:
      throw std::runtime_error("Any-typed annotation values are not supported");
  }
  throw std::runtime_error("Unsupported annotation value type");
}

folly::F14FastMap<std::string, type_system::SerializableRecordUnion>
serializeAnnotations(std::span<const DynamicValue> annotations) {
  folly::F14FastMap<std::string, type_system::SerializableRecordUnion> result;
  result.reserve(annotations.size());
  for (const auto& annotation : annotations) {
    const auto& uri = annotation.type().asStruct().uri();
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    result.emplace(
        uri, SerializableRecord::toThrift(toSerializableRecord(annotation)));
  }
  return result;
}

std::vector<DynamicValue> deserializeAnnotations(
    const folly::F14FastMap<std::string, type_system::SerializableRecordUnion>&
        annotations,
    const type_system::TypeSystem& typeSystem) {
  std::vector<DynamicValue> result;
  result.reserve(annotations.size());
  for (const auto& [uri, record] : annotations) {
    result.push_back(fromSerializableRecord(
        SerializableRecord::fromThrift(
            type_system::SerializableRecordUnion(record)),
        typeSystem.UserDefined(uri)));
  }
  return result;
}

} // namespace apache::thrift::dynamic
