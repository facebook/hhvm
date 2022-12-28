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
    case TypeName::Type::__EMPTY__:
      return false;
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

} // namespace type
} // namespace thrift
} // namespace apache
