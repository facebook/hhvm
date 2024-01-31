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

bool Type::isFull(const TypeUri& typeUri, bool validate_uri) {
  if (typeUri.getType() != TypeUri::Type::uri) {
    return false;
  }
  if (validate_uri) {
    try {
      checkName(typeUri.get_uri());
    } catch (std::exception&) {
      return false;
    }
  }
  return true;
}

bool Type::isFull(const TypeName& typeName, bool validate_uri) {
  switch (typeName.getType()) {
    case TypeName::Type::enumType:
      return isFull(*typeName.enumType_ref(), validate_uri);
    case TypeName::Type::structType:
      return isFull(*typeName.structType_ref(), validate_uri);
    case TypeName::Type::unionType:
      return isFull(*typeName.unionType_ref(), validate_uri);
    case TypeName::Type::exceptionType:
      return isFull(*typeName.exceptionType_ref(), validate_uri);
    case TypeName::Type::__EMPTY__:
      return false;
    default:
      return true;
  }
}

bool Type::isFull(
    const TypeStruct& type, bool ensure_params, bool validate_uri) {
  auto type_name = *type.name();
  if (ensure_params) {
    size_t expected_params_count = 0;
    switch (type_name.getType()) {
      case TypeName::Type::listType:
      case TypeName::Type::setType:
        expected_params_count = 1;
        break;
      case TypeName::Type::mapType:
        expected_params_count = 2;
        break;
      default:
        break;
    }
    if (expected_params_count != type.params()->size()) {
      return false;
    }
  }
  for (const auto& param : *type.params()) {
    if (!isFull(param, ensure_params, validate_uri)) {
      return false;
    }
  }
  return isFull(type_name, validate_uri);
}

void Type::checkName(const std::string& name) {
  validateUniversalName(name);
}

} // namespace type
} // namespace thrift
} // namespace apache
