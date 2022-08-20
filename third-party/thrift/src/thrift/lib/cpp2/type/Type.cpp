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

#include <thrift/lib/cpp2/type/Type.h>

#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache {
namespace thrift {
namespace type {

bool Type::isFull(const TypeUri& typeUri) {
  return typeUri.getType() == TypeUri::Type::uri;
}

bool Type::isFull(const TypeName& typeName) {
  switch (typeName.getType()) {
    case TypeName::Type::enumType:
      return isFull(*typeName.enumType_ref());
    case TypeName::Type::structType:
      return isFull(*typeName.structType_ref());
    case TypeName::Type::unionType:
      return isFull(*typeName.unionType_ref());
    case TypeName::Type::exceptionType:
      return isFull(*typeName.exceptionType_ref());
    default:
      return true;
  }
}

bool Type::isFull(const TypeStruct& type) {
  for (const auto& param : *type.params()) {
    if (!isFull(param)) {
      return false;
    }
  }
  return isFull(*type.name());
}

void Type::checkName(const std::string& name) {
  validateUniversalName(name);
}

folly::Optional<protocol::TType> Type::toTType() const noexcept {
  switch (data_.name()->getType()) {
    case TypeName::Type::boolType:
      return protocol::TType::T_BOOL;
    case TypeName::Type::byteType:
      return protocol::TType::T_BYTE;
    case TypeName::Type::i16Type:
      return protocol::TType::T_I16;
    case TypeName::Type::i32Type:
      return protocol::TType::T_I32;
    case TypeName::Type::i64Type:
      return protocol::TType::T_I64;
    case TypeName::Type::floatType:
      return protocol::TType::T_FLOAT;
    case TypeName::Type::doubleType:
      return protocol::TType::T_DOUBLE;
    case TypeName::Type::stringType:
      return protocol::TType::T_STRING;
    case TypeName::Type::binaryType:
      return protocol::TType::T_STRING;
    case TypeName::Type::enumType:
      return protocol::TType::T_I32;
    case TypeName::Type::structType:
      return protocol::TType::T_STRUCT;
    case TypeName::Type::unionType:
      return protocol::TType::T_STRUCT;
    case TypeName::Type::exceptionType:
      return protocol::TType::T_STRUCT;
    case TypeName::Type::listType:
      return protocol::TType::T_LIST;
    case TypeName::Type::setType:
      return protocol::TType::T_SET;
    case TypeName::Type::mapType:
      return protocol::TType::T_MAP;
    default:
      return {};
  }
}

} // namespace type
} // namespace thrift
} // namespace apache
