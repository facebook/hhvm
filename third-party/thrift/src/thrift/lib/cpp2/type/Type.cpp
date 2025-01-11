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

#include <fmt/format.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::type {

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

namespace {

// meta.com/a/B -> B
std::string debugUri(std::string_view uri) {
  auto pos = uri.find_last_of("/");
  if (pos == std::string::npos) {
    return "?";
  } else {
    return std::string(uri.begin() + pos + 1, uri.end());
  }
}

// a.b.c.D -> D
std::string debugScopedName(std::string_view name) {
  auto pos = name.find_last_of(".");
  if (pos == std::string::npos) {
    return "?";
  } else {
    return std::string(name.begin() + pos + 1, name.end());
  }
}

std::string debugUri(const TypeUri& type) {
  switch (type.getType()) {
    case TypeUri::Type::uri:
      return debugUri(*type.uri_ref());
    case TypeUri::Type::scopedName:
      return debugScopedName(*type.scopedName_ref());
    case TypeUri::Type::typeHashPrefixSha2_256:
    case TypeUri::Type::definitionKey:
      // Need schema information!
      return "?";
    default:
      folly::throw_exception<std::runtime_error>("Invalid type.");
  }
}

std::string debugStringImpl(const TypeStruct& type) {
  switch (type.name()->getType()) {
    case TypeName::boolType:
      return "bool";
    case TypeName::byteType:
      return "byte";
    case TypeName::i16Type:
      return "i16";
    case TypeName::i32Type:
      return "i32";
    case TypeName::i64Type:
      return "i64";
    case TypeName::floatType:
      return "float";
    case TypeName::doubleType:
      return "double";
    case TypeName::stringType:
      return "string";
    case TypeName::binaryType:
      return "binary";
    case TypeName::enumType:
      return fmt::format("enum<{}>", debugUri(*type.name()->enumType_ref()));
    case TypeName::typedefType:
      // Need schema to resolve :(
      return fmt::format(
          "typedef<{}>", debugUri(*type.name()->typedefType_ref()));
    case TypeName::structType:
      return fmt::format(
          "struct<{}>", debugUri(*type.name()->structType_ref()));
    case TypeName::unionType:
      return fmt::format("union<{}>", debugUri(*type.name()->unionType_ref()));
    case TypeName::exceptionType:
      return fmt::format(
          "exception<{}>", debugUri(*type.name()->exceptionType_ref()));
    case TypeName::listType:
      return fmt::format("list<{}>", debugStringImpl(type.params()->at(0)));
    case TypeName::setType:
      return fmt::format("set<{}>", debugStringImpl(type.params()->at(0)));
    case TypeName::mapType:
      return fmt::format(
          "map<{}, {}>",
          debugStringImpl(type.params()->at(0)),
          debugStringImpl(type.params()->at(1)));
    default:
      folly::throw_exception<std::runtime_error>("Invalid type.");
  }
}

} // namespace

std::string Type::debugString() const {
  return debugStringImpl(toThrift());
}

} // namespace apache::thrift::type
