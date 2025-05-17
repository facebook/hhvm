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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>

#include <folly/Memory.h>

#include <fmt/core.h>

#include <memory>

namespace apache::thrift::dynamic {

StructuredNode::StructuredNode(
    Uri uri, std::vector<FieldNode> fields, bool isSealed)
    : uri_(std::move(uri)), fields_(std::move(fields)), isSealed_(isSealed) {
  std::uint16_t ordinal = 1;
  for (const FieldNode& field : fields_) {
    bool emplaced =
        fieldHandleById_
            .emplace(field.identity().id(), FastFieldHandle(ordinal))
            .second;
    if (!emplaced) {
      folly::throw_exception<InvalidTypeError>(fmt::format(
          "duplicate field id '{}' in struct '{}'",
          field.identity().id(),
          uri_));
    }
    emplaced = fieldHandleByName_
                   .emplace(field.identity().name(), FastFieldHandle(ordinal))
                   .second;
    if (!emplaced) {
      folly::throw_exception<InvalidTypeError>(fmt::format(
          "duplicate field name '{}' in struct '{}'",
          field.identity().name(),
          uri_));
    }
    ++ordinal;
  }
}

StructNode::StructNode(Uri uri, std::vector<FieldNode> fields, bool isSealed)
    : StructuredNode(std::move(uri), std::move(fields), isSealed) {}

UnionNode::UnionNode(Uri uri, std::vector<FieldNode> fields, bool isSealed)
    : StructuredNode(std::move(uri), std::move(fields), isSealed) {
  for (const FieldNode& field : this->fields()) {
    if (field.presence() != PresenceQualifier::OPTIONAL) {
      folly::throw_exception<InvalidTypeError>(fmt::format(
          "field {} must be optional in a Union", field.identity()));
    }
  }
}

namespace detail {

ListTypeRef::ListTypeRef(TypeRef elementType)
    : elementType_(folly::copy_to_unique_ptr(std::move(elementType))) {}

ListTypeRef::ListTypeRef(const ListTypeRef& other)
    : elementType_(folly::copy_to_unique_ptr(other.elementType())) {}

ListTypeRef& ListTypeRef::operator=(const ListTypeRef& other) {
  elementType_ = folly::copy_to_unique_ptr(other.elementType());
  return *this;
}

SetTypeRef::SetTypeRef(TypeRef elementType)
    : elementType_(folly::copy_to_unique_ptr(std::move(elementType))) {}

SetTypeRef::SetTypeRef(const SetTypeRef& other)
    : elementType_(folly::copy_to_unique_ptr(other.elementType())) {}

SetTypeRef& SetTypeRef::operator=(const SetTypeRef& other) {
  elementType_ = folly::copy_to_unique_ptr(other.elementType());
  return *this;
}

MapTypeRef::MapTypeRef(TypeRef keyType, TypeRef valueType)
    : keyType_(folly::copy_to_unique_ptr(std::move(keyType))),
      valueType_(folly::copy_to_unique_ptr(std::move(valueType))) {}

MapTypeRef::MapTypeRef(const MapTypeRef& other)
    : keyType_(folly::copy_to_unique_ptr(other.keyType())),
      valueType_(folly::copy_to_unique_ptr(other.valueType())) {}

MapTypeRef& MapTypeRef::operator=(const MapTypeRef& other) {
  keyType_ = folly::copy_to_unique_ptr(other.keyType());
  valueType_ = folly::copy_to_unique_ptr(other.valueType());
  return *this;
}

[[noreturn]] void throwTypeRefAccessInactiveKind(std::string_view actualKind) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access TypeRef with inactive kind, actual kind was: {}",
      actualKind));
}

[[noreturn]] void throwDefinitionRefAccessInactiveKind(
    std::string_view actualKind) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access DefinitionRef with inactive kind, actual kind was: {}",
      actualKind));
}

} // namespace detail

const StructuredNode& TypeRef::asStructured() const {
  switch (kind()) {
    case Kind::STRUCT:
      return asStruct();
    case Kind::UNION:
      return asUnion();
    default:
      break;
  }
  throwTypeRefAccessInactiveKind();
}

TypeId TypeRef::id() const {
  return visit(
      [](const Bool&) { return TypeIds::Bool; },
      [](const Byte&) { return TypeIds::Byte; },
      [](const I16&) { return TypeIds::I16; },
      [](const I32&) { return TypeIds::I32; },
      [](const I64&) { return TypeIds::I64; },
      [](const Float&) { return TypeIds::Float; },
      [](const Double&) { return TypeIds::Double; },
      [](const String&) { return TypeIds::String; },
      [](const Binary&) { return TypeIds::Binary; },
      [](const Any&) { return TypeIds::Any; },
      [](const StructNode& structDef) { return TypeIds::uri(structDef.uri()); },
      [](const UnionNode& unionDef) { return TypeIds::uri(unionDef.uri()); },
      [](const EnumNode& enumDef) { return TypeIds::uri(enumDef.uri()); },
      [](const OpaqueAliasNode& opaqueAliasDef) {
        return TypeIds::uri(opaqueAliasDef.uri());
      },
      [](const List& list) { return TypeIds::list(list.elementType().id()); },
      [](const Set& set) { return TypeIds::set(set.elementType().id()); },
      [](const Map& map) {
        return TypeIds::map(map.keyType().id(), map.valueType().id());
      });
}

bool operator==(const TypeRef& lhs, const TypeRef& rhs) {
  return lhs.visit(
      // For non-primitive types, we are able to compare by the address of the
      // definition. This is because:
      //   * Two types cannot have the same URI, and
      //   * There can only be one definition per type.
      [&](const StructNode& structDef) {
        return rhs.isStruct() &&
            std::addressof(structDef) == std::addressof(rhs.asStruct());
      },
      [&](const UnionNode& unionDef) {
        return rhs.isUnion() &&
            std::addressof(unionDef) == std::addressof(rhs.asUnion());
      },
      [&](const EnumNode& enumDef) {
        return rhs.isEnum() &&
            std::addressof(enumDef) == std::addressof(rhs.asEnum());
      },
      [&](const OpaqueAliasNode& opaqueAliasDef) {
        return rhs.isOpaqueAlias() &&
            std::addressof(opaqueAliasDef) ==
            std::addressof(rhs.asOpaqueAlias());
      },
      // Container types
      [&](const TypeRef::List& list) {
        return rhs.isList() && list.elementType() == rhs.asList().elementType();
      },
      [&](const TypeRef::Set& set) {
        return rhs.isSet() && set.elementType() == rhs.asSet().elementType();
      },
      [&](const TypeRef::Map& map) {
        if (!rhs.isMap()) {
          return false;
        }
        const TypeRef::Map& other = rhs.asMap();
        return std::tie(map.keyType(), map.valueType()) ==
            std::tie(other.keyType(), other.valueType());
      },
      // Primitive types
      [&](const TypeRef::Bool&) { return rhs.isBool(); },
      [&](const TypeRef::Byte&) { return rhs.isByte(); },
      [&](const TypeRef::I16&) { return rhs.isI16(); },
      [&](const TypeRef::I32&) { return rhs.isI32(); },
      [&](const TypeRef::I64&) { return rhs.isI64(); },
      [&](const TypeRef::Float&) { return rhs.isFloat(); },
      [&](const TypeRef::Double&) { return rhs.isDouble(); },
      [&](const TypeRef::String&) { return rhs.isString(); },
      [&](const TypeRef::Binary&) { return rhs.isBinary(); },
      [&](const TypeRef::Any&) { return rhs.isAny(); });
}

/* static */ std::string_view TypeRef::kindToString(Kind k) noexcept {
  switch (k) {
    case Kind::BOOL:
      return "BOOL";
    case Kind::BYTE:
      return "BYTE";
    case Kind::I16:
      return "I16";
    case Kind::I32:
      return "I32";
    case Kind::I64:
      return "I64";
    case Kind::FLOAT:
      return "FLOAT";
    case Kind::DOUBLE:
      return "DOUBLE";
    case Kind::STRING:
      return "STRING";
    case Kind::BINARY:
      return "BINARY";
    case Kind::ANY:
      return "ANY";
    case Kind::LIST:
      return "LIST";
    case Kind::SET:
      return "SET";
    case Kind::MAP:
      return "MAP";
    case Kind::STRUCT:
      return "STRUCT";
    case Kind::UNION:
      return "UNION";
    case Kind::ENUM:
      return "ENUM";
    case Kind::OPAQUE_ALIAS:
      return "OPAQUE_ALIAS";
    default:
      break;
  }
  return "<unknown>";
}

/* static */ TypeRef TypeRef::fromDefinition(DefinitionRef def) {
  switch (def.kind()) {
    case DefinitionRef::Kind::STRUCT:
      return TypeRef(def.asStruct());
    case DefinitionRef::Kind::UNION:
      return TypeRef(def.asUnion());
    case DefinitionRef::Kind::ENUM:
      return TypeRef(def.asEnum());
    case DefinitionRef::Kind::OPAQUE_ALIAS:
      return TypeRef(def.asOpaqueAlias());
    default:
      break;
  };
  folly::assume_unreachable();
}

const Uri& DefinitionRef::uri() const noexcept {
  return visit([](const auto& def) -> const Uri& { return def.uri(); });
}

/* static */ std::string_view DefinitionRef::kindToString(Kind k) noexcept {
  switch (k) {
    case Kind::STRUCT:
      return "STRUCT";
    case Kind::UNION:
      return "UNION";
    case Kind::ENUM:
      return "ENUM";
    case Kind::OPAQUE_ALIAS:
      return "OPAQUE_ALIAS";
    default:
      break;
  }
  return "<unknown>";
}

} // namespace apache::thrift::dynamic
