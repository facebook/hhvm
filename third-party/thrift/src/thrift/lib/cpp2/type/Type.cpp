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

#include <folly/lang/Exception.h>
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

bool identicalTypeUri(const TypeUri& lhs, const TypeUri& rhs) {
  // We don't consider 'scopedName' here since it is there for backward
  // compatible reason and couldn't be generated with any of our standard
  // offerings.
  if (lhs.getType() != rhs.getType()) {
    if (lhs.uri_ref().has_value() &&
        rhs.typeHashPrefixSha2_256_ref().has_value()) {
      return getUniversalHashPrefix(
                 getUniversalHash(
                     type::UniversalHashAlgorithm::Sha2_256,

                     lhs.uri_ref().value()),
                 kDefaultTypeHashBytes) ==
          rhs.typeHashPrefixSha2_256_ref().value();
    } else if (
        lhs.typeHashPrefixSha2_256_ref().has_value() &&
        rhs.uri_ref().has_value()) {
      return lhs.typeHashPrefixSha2_256_ref().value() ==
          getUniversalHashPrefix(
                 getUniversalHash(
                     type::UniversalHashAlgorithm::Sha2_256,
                     rhs.uri_ref().value()),
                 kDefaultTypeHashBytes);
    }
    return false;
  }
  return lhs == rhs;
}

bool identicalTypeStruct(const TypeStruct& lhs, const TypeStruct& rhs) {
  if (lhs.name()->getType() != rhs.name()->getType()) {
    return false;
  }

  switch (lhs.name()->getType()) {
    case TypeName::__EMPTY__:
      return false;
    // primitives
    case TypeName::boolType:
    case TypeName::byteType:
    case TypeName::i16Type:
    case TypeName::i32Type:
    case TypeName::i64Type:
    case TypeName::floatType:
    case TypeName::doubleType:
    case TypeName::stringType:
    case TypeName::binaryType:
      return true;
    // definitions
    case TypeName::enumType:
      return identicalTypeUri(
          lhs.name()->enumType_ref().value(),
          rhs.name()->enumType_ref().value());
    case TypeName::typedefType:
      return identicalTypeUri(
          lhs.name()->typedefType_ref().value(),
          rhs.name()->typedefType_ref().value());
    case TypeName::structType:
      return identicalTypeUri(
          lhs.name()->structType_ref().value(),
          rhs.name()->structType_ref().value());
    case TypeName::unionType:
      return identicalTypeUri(
          lhs.name()->unionType_ref().value(),
          rhs.name()->unionType_ref().value());
    case TypeName::exceptionType:
      return identicalTypeUri(
          lhs.name()->exceptionType_ref().value(),
          rhs.name()->exceptionType_ref().value());
    // containers
    case TypeName::listType:
    case TypeName::setType:
      if (lhs.params()->size() != 1 || rhs.params()->size() != 1) {
        folly::throw_exception<std::runtime_error>("Invalid params");
      }
      return identicalTypeStruct(
          lhs.params().value()[0], rhs.params().value()[0]);
    case TypeName::mapType:
      if (lhs.params()->size() != 2 || rhs.params()->size() != 2) {
        folly::throw_exception<std::runtime_error>("Invalid params");
      }
      return identicalTypeStruct(
                 lhs.params().value()[0], rhs.params().value()[0]) &&
          identicalTypeStruct(lhs.params().value()[1], rhs.params().value()[1]);
    default:
      // unreachable
      folly::terminate_with<std::runtime_error>("Invalid type.");
  }
}

} // namespace type
} // namespace thrift
} // namespace apache
