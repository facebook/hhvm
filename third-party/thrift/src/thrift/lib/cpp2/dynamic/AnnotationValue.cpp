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

#include <stdexcept>

#include <folly/io/IOBuf.h>
#include <folly/json/dynamic.h>

#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
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

} // namespace apache::thrift::dynamic
