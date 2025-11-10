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
#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>

#include <folly/Memory.h>

#include <fmt/core.h>

#include <functional>
#include <memory>

namespace apache::thrift::type_system {

namespace detail {

// Helper to get or create a cached TypeRef for a container type.
// Uses folly::Synchronized for thread-safe access with optimistic concurrency.
// F14NodeMap ensures stable references even during rehashing.
const TypeRef* getOrCreateCachedType(
    ContainerTypeCache& cache, const TypeId& typeId, TypeRef&& typeRef) {
  // Optimistic read: check if already cached
  {
    auto rlock = cache.cache.rlock();
    auto it = rlock->find(typeId);
    if (it != rlock->end()) {
      return &it->second;
    }
  }

  // Not found, need to insert. Acquire write lock.
  auto wlock = cache.cache.wlock();

  // Double-check: another thread may have inserted while we were waiting
  auto [it, inserted] = wlock->try_emplace(typeId, typeRef);
  return &it->second;
}

ListTypeRef::ListTypeRef(TypeRef elementType, ContainerTypeCache& cache)
    : elementType_([&] {
        auto id = elementType.id();
        return getOrCreateCachedType(cache, id, std::move(elementType));
      }()) {}

TypeId ListTypeRef::id() const {
  return TypeIds::list(elementType().id());
}

SetTypeRef::SetTypeRef(TypeRef elementType, ContainerTypeCache& cache)
    : elementType_([&] {
        auto id = elementType.id();
        return getOrCreateCachedType(cache, id, std::move(elementType));
      }()) {}

TypeId SetTypeRef::id() const {
  return TypeIds::set(elementType().id());
}

MapTypeRef::MapTypeRef(
    TypeRef keyType, TypeRef valueType, ContainerTypeCache& cache)
    : keyType_([&] {
        auto id = keyType.id();
        return getOrCreateCachedType(cache, id, std::move(keyType));
      }()),
      valueType_([&] {
        auto id = valueType.id();
        return getOrCreateCachedType(cache, id, std::move(valueType));
      }()) {}

TypeId MapTypeRef::id() const {
  return TypeIds::map(keyType().id(), valueType().id());
}

} // namespace detail

StructuredNode::StructuredNode(
    Uri uri,
    std::vector<FieldDefinition> fields,
    bool isSealed,
    AnnotationsMap annotations)
    : DefinitionNode(std::move(uri)),
      fields_(std::move(fields)),
      isSealed_(isSealed),
      annotations_(std::move(annotations)) {
  std::uint16_t ordinal = 1;
  for (const FieldDefinition& field : fields_) {
    bool emplaced =
        fieldHandleById_
            .emplace(field.identity().id(), FastFieldHandle{ordinal})
            .second;
    if (!emplaced) {
      folly::throw_exception<InvalidTypeError>(fmt::format(
          "duplicate field id '{}' in struct '{}'",
          folly::to_underlying(field.identity().id()),
          uri_));
    }
    emplaced = fieldHandleByName_
                   .emplace(field.identity().name(), FastFieldHandle{ordinal})
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

StructNode::StructNode(
    Uri uri,
    std::vector<FieldDefinition> fields,
    bool isSealed,
    AnnotationsMap annotations)
    : StructuredNode(
          std::move(uri), std::move(fields), isSealed, std::move(annotations)) {
}

UnionNode::UnionNode(
    Uri uri,
    std::vector<FieldDefinition> fields,
    bool isSealed,
    AnnotationsMap annotations)
    : StructuredNode(
          std::move(uri), std::move(fields), isSealed, std::move(annotations)) {
  for (const FieldDefinition& field : this->fields()) {
    if (field.presence() != PresenceQualifier::OPTIONAL_) {
      folly::throw_exception<InvalidTypeError>(fmt::format(
          "field {} must be optional in a Union", field.identity()));
    }
  }
}

FieldDefinition::FieldDefinition(
    FieldIdentity identity,
    PresenceQualifier presence,
    TypeRef type,
    std::optional<SerializableRecord> customDefaultPartialRecord,
    AnnotationsMap annotations)
    : identity_(std::move(identity)),
      presence_(presence),
      type_(type),
      wireType_(ToTTypeFn{}(type_)),
      customDefaultPartialRecord_(std::move(customDefaultPartialRecord)),
      annotations_(std::move(annotations)) {}

const StructuredNode& TypeRef::asStructured() const {
  switch (kind()) {
    case Kind::STRUCT:
      return asStruct();
    case Kind::UNION:
      return asUnion();
    default:
      break;
  }
  throwAccessInactiveKind();
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
      [](const List& list) { return list.id(); },
      [](const Set& set) { return set.id(); },
      [](const Map& map) { return map.id(); });
}

bool TypeRef::isEqualIdentityTo(const TypeRef& rhs) const noexcept {
  if (this == std::addressof(rhs)) {
    return true;
  }
  return visit(
      // For non-primitive types, we are able to compare by the address of the
      // definition for TypeRef within one TypeSystem. This is because:
      //   * Two types cannot have the same URI, and
      //   * There can only be one definition per type.
      //
      // For TypeRef across TypeSystem instances, we have to fallback to
      // TypeId-based comparison.
      [&](const StructNode& structDef) {
        if (!rhs.isStruct()) {
          return false;
        }
        const StructNode& other = rhs.asStruct();
        if (std::addressof(structDef) == std::addressof(other)) {
          return true;
        }
        return structDef.uri() == other.uri();
      },
      [&](const UnionNode& unionDef) {
        if (!rhs.isUnion()) {
          return false;
        }
        const UnionNode& other = rhs.asUnion();
        if (std::addressof(unionDef) == std::addressof(other)) {
          return true;
        }
        return unionDef.uri() == other.uri();
      },
      [&](const EnumNode& enumDef) {
        if (!rhs.isEnum()) {
          return false;
        }
        const EnumNode& other = rhs.asEnum();
        if (std::addressof(enumDef) == std::addressof(other)) {
          return true;
        }
        return enumDef.uri() == other.uri();
      },
      [&](const OpaqueAliasNode& opaqueAliasDef) {
        if (!rhs.isOpaqueAlias()) {
          return false;
        }
        const OpaqueAliasNode& other = rhs.asOpaqueAlias();
        if (std::addressof(opaqueAliasDef) == std::addressof(other)) {
          return true;
        }
        return opaqueAliasDef.uri() == other.uri();
      },
      // Container types
      [&](const TypeRef::List& list) {
        return rhs.isList() &&
            list.elementType().isEqualIdentityTo(rhs.asList().elementType());
      },
      [&](const TypeRef::Set& set) {
        return rhs.isSet() &&
            set.elementType().isEqualIdentityTo(rhs.asSet().elementType());
      },
      [&](const TypeRef::Map& map) {
        if (!rhs.isMap()) {
          return false;
        }
        const TypeRef::Map& other = rhs.asMap();
        return map.keyType().isEqualIdentityTo(other.keyType()) &&
            map.valueType().isEqualIdentityTo(other.valueType());
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

namespace {
std::string_view kindToString(TypeRef::Kind k) noexcept {
  using Kind = TypeRef::Kind;
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
} // namespace

[[noreturn]] void TypeRef::throwAccessInactiveKind() const {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access TypeRef with inactive kind, actual kind was: {}",
      kindToString(kind())));
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

const Uri& DefinitionNode::uri() const {
  if (uri_.empty()) {
    folly::throw_exception<InvalidTypeError>("Type does not have URI set.");
  }
  return uri_;
}

const Uri& DefinitionRef::uri() const {
  return visit(std::mem_fn(&DefinitionNode::uri));
}

namespace {
// A DefinitionRef is a variant of pointers to definition nodes. There is
// exactly one node object per definition. Therefore, the address of the
// node is a unique identifier for the definition.
std::uintptr_t addressOfDefinition(DefinitionRef ref) {
  return ref.visit([](const auto& def) {
    return reinterpret_cast<std::uintptr_t>(std::addressof(def));
  });
}
} // namespace

bool operator==(const DefinitionRef& lhs, const DefinitionRef& rhs) noexcept {
  return addressOfDefinition(lhs) == addressOfDefinition(rhs);
}

namespace {
std::string_view kindToString(DefinitionRef::Kind k) noexcept {
  using Kind = DefinitionRef::Kind;
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
} // namespace

[[noreturn]] void DefinitionRef::throwAccessInactiveKind() const {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access DefinitionRef with inactive kind, actual kind was: {}",
      kindToString(kind())));
}

DefinitionRef TypeSystem::getUserDefinedTypeOrThrow(UriView uri) const {
  if (std::optional<DefinitionRef> def = this->getUserDefinedType(uri)) {
    return *def;
  }
  folly::throw_exception<InvalidTypeError>(fmt::format(
      "Type with URI '{}' is not defined in this TypeSystem.", uri));
}

namespace {
struct TypeIdResolver {
  const TypeSystem& typeSystem_;
  detail::ContainerTypeCache& cache_;

  TypeRef operator()(const TypeId& typeId) const { return typeId.visit(*this); }

  TypeRef operator()(const TypeId::Bool&) const {
    return TypeRef(TypeRef::Bool());
  }
  TypeRef operator()(const TypeId::Byte&) const {
    return TypeRef(TypeRef::Byte());
  }

  TypeRef operator()(const TypeId::I16&) const {
    return TypeRef(TypeRef::I16());
  }

  TypeRef operator()(const TypeId::I32&) const {
    return TypeRef(TypeRef::I32());
  }

  TypeRef operator()(const TypeId::I64&) const {
    return TypeRef(TypeRef::I64());
  }

  TypeRef operator()(const TypeId::Float&) const {
    return TypeRef(TypeRef::Float());
  }

  TypeRef operator()(const TypeId::Double&) const {
    return TypeRef(TypeRef::Double());
  }

  TypeRef operator()(const TypeId::String&) const {
    return TypeRef(TypeRef::String());
  }

  TypeRef operator()(const TypeId::Binary&) const {
    return TypeRef(TypeRef::Binary());
  }

  TypeRef operator()(const TypeId::Any&) const {
    return TypeRef(TypeRef::Any());
  }

  TypeRef operator()(const TypeId::Uri& uriId) const {
    auto defn = typeSystem_.getUserDefinedTypeOrThrow(uriId);
    return TypeRef::fromDefinition(std::move(defn));
  }

  TypeRef operator()(const TypeId::List& listId) const {
    auto elementType = (*this)(listId.elementType());
    return TypeRef(TypeRef::List(elementType, cache_));
  }

  TypeRef operator()(const TypeId::Set& setId) const {
    auto elementType = (*this)(setId.elementType());
    return TypeRef(TypeRef::Set(elementType, cache_));
  }

  TypeRef operator()(const TypeId::Map& mapId) const {
    auto keyType = (*this)(mapId.keyType());
    auto valueType = (*this)(mapId.valueType());
    return TypeRef(TypeRef::Map(keyType, valueType, cache_));
  }
};
} // namespace

TypeRef TypeSystem::resolveTypeId(const TypeId& typeId) const {
  return TypeIdResolver{*this, containerTypeCache()}(typeId);
}
} // namespace apache::thrift::type_system

std::size_t std::hash<apache::thrift::type_system::DefinitionRef>::operator()(
    const apache::thrift::type_system::DefinitionRef& ref) const {
  return std::hash<std::uintptr_t>{}(
      apache::thrift::type_system::addressOfDefinition(ref));
}
