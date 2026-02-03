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
      return isFull(*typeName.enumType(), validate_uri);
    case TypeName::Type::structType:
      return isFull(*typeName.structType(), validate_uri);
    case TypeName::Type::unionType:
      return isFull(*typeName.unionType(), validate_uri);
    case TypeName::Type::exceptionType:
      return isFull(*typeName.exceptionType(), validate_uri);
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

void Type::checkName(folly::cstring_view name) {
  validateUniversalName(name);
}

bool identicalTypeUri(const TypeUri& lhs, const TypeUri& rhs) {
  // We don't consider 'scopedName' here since it is there for backward
  // compatible reason and couldn't be generated with any of our standard
  // offerings.
  if (lhs.getType() != rhs.getType()) {
    if (lhs.uri().has_value() && rhs.typeHashPrefixSha2_256().has_value()) {
      return getUniversalHashPrefix(
                 getUniversalHash(
                     type::UniversalHashAlgorithm::Sha2_256,

                     lhs.uri().value()),
                 kDefaultTypeHashBytes) == rhs.typeHashPrefixSha2_256().value();
    } else if (
        lhs.typeHashPrefixSha2_256().has_value() && rhs.uri().has_value()) {
      return lhs.typeHashPrefixSha2_256().value() ==
          getUniversalHashPrefix(
                 getUniversalHash(
                     type::UniversalHashAlgorithm::Sha2_256, rhs.uri().value()),
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
    case TypeName::Type::__EMPTY__:
      return false;
    // primitives
    case TypeName::Type::boolType:
    case TypeName::Type::byteType:
    case TypeName::Type::i16Type:
    case TypeName::Type::i32Type:
    case TypeName::Type::i64Type:
    case TypeName::Type::floatType:
    case TypeName::Type::doubleType:
    case TypeName::Type::stringType:
    case TypeName::Type::binaryType:
      return true;
    // definitions
    case TypeName::Type::enumType:
      return identicalTypeUri(
          lhs.name()->enumType().value(), rhs.name()->enumType().value());
    case TypeName::Type::typedefType:
      return identicalTypeUri(
          lhs.name()->typedefType().value(), rhs.name()->typedefType().value());
    case TypeName::Type::structType:
      return identicalTypeUri(
          lhs.name()->structType().value(), rhs.name()->structType().value());
    case TypeName::Type::unionType:
      return identicalTypeUri(
          lhs.name()->unionType().value(), rhs.name()->unionType().value());
    case TypeName::Type::exceptionType:
      return identicalTypeUri(
          lhs.name()->exceptionType().value(),
          rhs.name()->exceptionType().value());
    // containers
    case TypeName::Type::listType:
    case TypeName::Type::setType:
      if (lhs.params()->size() != 1 || rhs.params()->size() != 1) {
        folly::throw_exception<std::runtime_error>("Invalid params");
      }
      return identicalTypeStruct(
          lhs.params().value()[0], rhs.params().value()[0]);
    case TypeName::Type::mapType:
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
  auto pos = uri.find_last_of('/');
  if (pos == std::string::npos) {
    return "?";
  } else {
    return std::string(uri.begin() + pos + 1, uri.end());
  }
}

// a.b.c.D -> D
std::string debugScopedName(std::string_view name) {
  auto pos = name.find_last_of('.');
  if (pos == std::string::npos) {
    return "?";
  } else {
    return std::string(name.begin() + pos + 1, name.end());
  }
}

std::string debugUri(const TypeUri& type) {
  switch (type.getType()) {
    case TypeUri::Type::uri:
      return debugUri(*type.uri());
    case TypeUri::Type::scopedName:
      return debugScopedName(*type.scopedName());
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
    case TypeName::Type::boolType:
      return "bool";
    case TypeName::Type::byteType:
      return "byte";
    case TypeName::Type::i16Type:
      return "i16";
    case TypeName::Type::i32Type:
      return "i32";
    case TypeName::Type::i64Type:
      return "i64";
    case TypeName::Type::floatType:
      return "float";
    case TypeName::Type::doubleType:
      return "double";
    case TypeName::Type::stringType:
      return "string";
    case TypeName::Type::binaryType:
      return "binary";
    case TypeName::Type::enumType:
      return fmt::format("enum<{}>", debugUri(*type.name()->enumType()));
    case TypeName::Type::typedefType:
      // Need schema to resolve :(
      return fmt::format("typedef<{}>", debugUri(*type.name()->typedefType()));
    case TypeName::Type::structType:
      return fmt::format("struct<{}>", debugUri(*type.name()->structType()));
    case TypeName::Type::unionType:
      return fmt::format("union<{}>", debugUri(*type.name()->unionType()));
    case TypeName::Type::exceptionType:
      return fmt::format(
          "exception<{}>", debugUri(*type.name()->exceptionType()));
    case TypeName::Type::listType:
      return fmt::format("list<{}>", debugStringImpl(type.params()->at(0)));
    case TypeName::Type::setType:
      return fmt::format("set<{}>", debugStringImpl(type.params()->at(0)));
    case TypeName::Type::mapType:
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

namespace {

struct UnionHasher {
  template <class U>
    requires(is_thrift_union_v<U>)
  std::size_t operator()(const U& u) {
    return op::visit_union_with_tag(
        u,
        [](auto tag, auto& value) {
          using Ident = folly::type_list_element_t<0, decltype(tag)>;
          constexpr auto id = op::get_field_id_v<U, Ident>;
          static_assert(id != FieldId{0});
          return folly::hash::hash_combine(id, UnionHasher{}(value));
        },
        [] { return size_t{0}; });
  }

  // Non-union values (e.g., integers, strings) are passed as they are.
  template <class T>
    requires(!is_thrift_union_v<T>)
  const T& operator()(const T& t) {
    return t;
  }
};

size_t hashTypeStruct(const TypeStruct& type) {
  size_t hash = UnionHasher{}(*type.name());
  for (const auto& param : *type.params()) {
    hash = folly::hash::hash_combine(hash, hashTypeStruct(param));
  }
  return hash;
}

} // namespace
} // namespace apache::thrift::type

size_t std::hash<apache::thrift::type::Type>::operator()(
    const apache::thrift::type::Type& value) const {
  return apache::thrift::type::hashTypeStruct(value.toThrift());
}
