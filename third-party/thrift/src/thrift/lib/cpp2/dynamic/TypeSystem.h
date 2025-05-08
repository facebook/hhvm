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

#pragma once

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/detail/Traits.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/CppAttributes.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/span.h>
#include <folly/lang/Assume.h>
#include <folly/lang/Exception.h>
#include <folly/memory/not_null.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::dynamic {

/**
 * Base class for structured types (structs, unions).
 */
class StructuredNode;
/**
 * A Thrift struct definition which includes all fields, and types transitively
 * referenced by those fields.
 */
class StructNode;
/**
 * A Thrift union definition includes all fields, and types transitively
 * referenced by those fields.
 */
class UnionNode;
/**
 * A Thrift enum definition.
 */
class EnumNode;
/**
 * An (URI-named) alias to a non-user-defined Thrift type.
 */
class OpaqueAliasNode;
/**
 * A definition refers to a user-defined Thrift type. One of:
 *   - StructNode
 *   - UnionNode
 *   - EnumNode
 *   - OpaqueAliasNode
 *
 * `DefinitionRef` models both Copyable and Movable.
 *
 * NOTE: `DefinitionRef` objects are only valid if the `TypeSystem` object that
 * it originates from is still alive.
 */
class DefinitionRef;
/**
 * A descriptor of a data type within Thrift. A `TypeRef` is one of:
 *   - primitive Thrift type (i32, string etc.)
 *   - user definitions (structured, enum, or alias)
 *   - container type (list, set, map)
 *
 * All `DefinitionRef` objects can form valid `TypeRef` objects.
 *
 * `TypeRef` models both Copyable and Movable.
 *
 * NOTE: `TypeRef` objects are only valid if the `TypeSystem` object that it
 * originates from is still alive.
 */
class TypeRef;

/**
 * An interface for a Thrift "type system", which is a store of schema
 * information.
 *
 * A type system is a collection of types where each type (and all types it
 * refers to) are fully defined within the same type system.
 *
 * This interface is used to abstract away the source of runtime schema
 * information from its usage. For example, a type system may be backed by:
 *   - Schema information bundled within the binary as part of Thrift's code
 *     generation.
 *   - Schema information fetched from a remote source.
 *   - Schema information created programmatically at runtime.
 */
class TypeSystem {
 public:
  virtual ~TypeSystem() noexcept = default;

  /**
   * Resolves the definition of a user-defined type, and implicitly, all
   * transitively referenced types.
   *
   * Throws:
   *   - InvalidTypeError if the type is not defined in the type system.
   */
  virtual DefinitionRef getUserDefinedType(UriView) = 0;
};

/**
 * Exception type that is thrown if there an attempt to form a definition with
 * invalid schema information. Some common scenario that result in this
 * exception being thrown:
 *   - There is an unresolved TypeId.
 *   - Union has non-optional fields.
 *   - Structured type has duplicate field ids or names.
 *   - Enum has duplicate names or values.
 */
class InvalidTypeError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

namespace detail {

/**
 * A TypeRef of a list type, including the underlying element type.
 */
class ListTypeRef final {
 public:
  explicit ListTypeRef(TypeRef elementType);

  const TypeRef& elementType() const { return *elementType_; }

  ~ListTypeRef() noexcept = default;
  ListTypeRef(const ListTypeRef&);
  ListTypeRef& operator=(const ListTypeRef&);
  ListTypeRef(ListTypeRef&&) noexcept = default;
  ListTypeRef& operator=(ListTypeRef&&) noexcept = default;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;
};

/**
 * A TypeRef of a set type, including the underlying element type.
 */
class SetTypeRef final {
 public:
  explicit SetTypeRef(TypeRef elementType);

  const TypeRef& elementType() const { return *elementType_; }

  ~SetTypeRef() noexcept = default;
  SetTypeRef(const SetTypeRef&);
  SetTypeRef& operator=(const SetTypeRef&);
  SetTypeRef(SetTypeRef&&) noexcept = default;
  SetTypeRef& operator=(SetTypeRef&&) noexcept = default;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;
};

/**
 * A TypeRef of a map type, including the underlying key and value types.
 */
class MapTypeRef final {
 public:
  MapTypeRef(TypeRef keyType, TypeRef valueType);

  const TypeRef& keyType() const { return *keyType_; }
  const TypeRef& valueType() const { return *valueType_; }

  ~MapTypeRef() noexcept = default;
  MapTypeRef(const MapTypeRef&);
  MapTypeRef& operator=(const MapTypeRef&);
  MapTypeRef(MapTypeRef&&) noexcept = default;
  MapTypeRef& operator=(MapTypeRef&&) noexcept = default;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> keyType_;
  folly::not_null_unique_ptr<TypeRef> valueType_;
};

[[noreturn]] void throwTypeRefAccessInactiveKind(std::string_view actualKind);
[[noreturn]] void throwDefinitionRefAccessInactiveKind(
    std::string_view actualKind);

} // namespace detail

class TypeRef final {
 public:
  using Bool = TypeId::Bool;
  using Byte = TypeId::Byte;
  using I16 = TypeId::I16;
  using I32 = TypeId::I32;
  using I64 = TypeId::I64;
  using Float = TypeId::Float;
  using Double = TypeId::Double;
  using String = TypeId::String;
  using Binary = TypeId::Binary;
  using Any = TypeId::Any;
  using List = detail::ListTypeRef;
  using Set = detail::SetTypeRef;
  using Map = detail::MapTypeRef;

 private:
  using StructPtr = folly::not_null<const StructNode*>;
  using UnionPtr = folly::not_null<const UnionNode*>;
  using EnumPtr = folly::not_null<const EnumNode*>;
  using OpaqueAliasPtr = folly::not_null<const OpaqueAliasNode*>;

  using Alternative = std::variant<
      Bool,
      Byte,
      I16,
      I32,
      I64,
      Float,
      Double,
      String,
      Binary,
      Any,
      List,
      Set,
      Map,
      StructPtr,
      UnionPtr,
      EnumPtr,
      OpaqueAliasPtr>;

 public:
  enum class Kind {
    BOOL = detail::IndexOf<Alternative, Bool>,
    BYTE = detail::IndexOf<Alternative, Byte>,
    I16 = detail::IndexOf<Alternative, I16>,
    I32 = detail::IndexOf<Alternative, I32>,
    I64 = detail::IndexOf<Alternative, I64>,
    FLOAT = detail::IndexOf<Alternative, Float>,
    DOUBLE = detail::IndexOf<Alternative, Double>,
    STRING = detail::IndexOf<Alternative, String>,
    BINARY = detail::IndexOf<Alternative, Binary>,
    ANY = detail::IndexOf<Alternative, Any>,
    LIST = detail::IndexOf<Alternative, List>,
    SET = detail::IndexOf<Alternative, Set>,
    MAP = detail::IndexOf<Alternative, Map>,
    STRUCT = detail::IndexOf<Alternative, StructPtr>,
    UNION = detail::IndexOf<Alternative, UnionPtr>,
    ENUM = detail::IndexOf<Alternative, EnumPtr>,
    OPAQUE_ALIAS = detail::IndexOf<Alternative, OpaqueAliasPtr>,
  };
  Kind kind() const { return static_cast<Kind>(type_.index()); }

  bool isBool() const { return kind() == Kind::BOOL; }
  bool isByte() const { return kind() == Kind::BYTE; }
  bool isI16() const { return kind() == Kind::I16; }
  bool isI32() const { return kind() == Kind::I32; }
  bool isI64() const { return kind() == Kind::I64; }
  bool isFloat() const { return kind() == Kind::FLOAT; }
  bool isDouble() const { return kind() == Kind::DOUBLE; }
  bool isString() const { return kind() == Kind::STRING; }
  bool isBinary() const { return kind() == Kind::BINARY; }
  bool isAny() const { return kind() == Kind::ANY; }
  bool isStruct() const { return kind() == Kind::STRUCT; }
  bool isUnion() const { return kind() == Kind::UNION; }
  bool isEnum() const { return kind() == Kind::ENUM; }
  bool isOpaqueAlias() const { return kind() == Kind::OPAQUE_ALIAS; }
  bool isList() const { return kind() == Kind::LIST; }
  bool isSet() const { return kind() == Kind::SET; }
  bool isMap() const { return kind() == Kind::MAP; }

  const StructNode& asStruct() const {
    if (kind() != Kind::STRUCT) {
      throwTypeRefAccessInactiveKind();
    }
    return asStructUnchecked();
  }
  const StructNode& asStructUnchecked() const noexcept {
    return **std::get_if<StructPtr>(&type_);
  }

  const UnionNode& asUnion() const {
    if (kind() != Kind::UNION) {
      throwTypeRefAccessInactiveKind();
    }
    return asUnionUnchecked();
  }
  const UnionNode& asUnionUnchecked() const noexcept {
    return **std::get_if<UnionPtr>(&type_);
  }

  const EnumNode& asEnum() const {
    if (kind() != Kind::ENUM) {
      throwTypeRefAccessInactiveKind();
    }
    return asEnumUnchecked();
  }
  const EnumNode& asEnumUnchecked() const noexcept {
    return **std::get_if<EnumPtr>(&type_);
  }

  const OpaqueAliasNode& asOpaqueAlias() const {
    if (kind() != Kind::OPAQUE_ALIAS) {
      throwTypeRefAccessInactiveKind();
    }
    return asOpaqueAliasUnchecked();
  }
  const OpaqueAliasNode& asOpaqueAliasUnchecked() const noexcept {
    return **std::get_if<OpaqueAliasPtr>(&type_);
  }

  const List& asList() const {
    if (kind() != Kind::LIST) {
      throwTypeRefAccessInactiveKind();
    }
    return asListUnchecked();
  }
  const List& asListUnchecked() const noexcept {
    return *std::get_if<List>(&type_);
  }

  const Set& asSet() const {
    if (kind() != Kind::SET) {
      throwTypeRefAccessInactiveKind();
    }
    return asSetUnchecked();
  }
  const Set& asSetUnchecked() const noexcept {
    return *std::get_if<Set>(&type_);
  }

  const Map& asMap() const {
    if (kind() != Kind::MAP) {
      throwTypeRefAccessInactiveKind();
    }
    return asMapUnchecked();
  }
  const Map& asMapUnchecked() const noexcept {
    return *std::get_if<Map>(&type_);
  }

  bool isStructured() const { return isStruct() || isUnion(); }
  const StructuredNode& asStructured() const;

  /**
   * Returns this type's unique type identifier.
   *
   * This type will always have the same type identifier, and it is the only
   * type within a type system that has this identifier.
   */
  TypeId id() const;

  /**
   * Two types are equal iff they have the same identity.
   */
  friend bool operator==(const TypeRef& lhs, const TypeRef& rhs);
  friend bool operator!=(const TypeRef& lhs, const TypeRef& rhs) {
    return !(lhs == rhs);
  }

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying type.
   *
   * See: https://en.cppreference.com/w/cpp/utility/variant/visit
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const;

  /**
   * `as<T>` produces the contained T, assuming it is the currently active
   * variant alternative.
   *
   * Pre-conditions:
   *   - T is the active variant alternative, else throws `std::runtime_error`
   */
  template <typename T>
  const T& asType() const {
    return visit(
        [](const T& value) -> const T& { return value; },
        [&](auto&&) -> const T& { throwTypeRefAccessInactiveKind(); });
  }

  // TypeRef to primitives
  explicit TypeRef(Bool) noexcept : type_(std::in_place_type<Bool>) {}
  explicit TypeRef(Byte) noexcept : type_(std::in_place_type<Byte>) {}
  explicit TypeRef(I16) noexcept : type_(std::in_place_type<I16>) {}
  explicit TypeRef(I32) noexcept : type_(std::in_place_type<I32>) {}
  explicit TypeRef(I64) noexcept : type_(std::in_place_type<I64>) {}
  explicit TypeRef(Float) noexcept : type_(std::in_place_type<Float>) {}
  explicit TypeRef(Double) noexcept : type_(std::in_place_type<Double>) {}
  explicit TypeRef(String) noexcept : type_(std::in_place_type<String>) {}
  explicit TypeRef(Binary) noexcept : type_(std::in_place_type<Binary>) {}
  explicit TypeRef(Any) noexcept : type_(std::in_place_type<Any>) {}

  // TypeRef to containers
  explicit TypeRef(List list) noexcept : type_(std::move(list)) {}
  explicit TypeRef(Set set) noexcept : type_(std::move(set)) {}
  explicit TypeRef(Map map) noexcept : type_(std::move(map)) {}

  // TypeRef to definitions
  explicit TypeRef(const StructNode& structDef)
      : type_(std::in_place_type<StructPtr>, &structDef) {}
  explicit TypeRef(const UnionNode& unionDef)
      : type_(std::in_place_type<UnionPtr>, &unionDef) {}
  explicit TypeRef(const EnumNode& enumDef)
      : type_(std::in_place_type<EnumPtr>, &enumDef) {}
  explicit TypeRef(const OpaqueAliasNode& opaqueAliasDef)
      : type_(std::in_place_type<OpaqueAliasPtr>, &opaqueAliasDef) {}

  /**
   * Creates a reference to a user-defined type.
   */
  static TypeRef fromDefinition(DefinitionRef);

 private:
  Alternative type_;

  static std::string_view kindToString(Kind) noexcept;
  [[noreturn]] void throwTypeRefAccessInactiveKind() const {
    detail::throwTypeRefAccessInactiveKind(kindToString(kind()));
  }
};
static_assert(std::is_copy_constructible_v<TypeRef>);
static_assert(std::is_copy_assignable_v<TypeRef>);
static_assert(std::is_move_constructible_v<TypeRef>);
static_assert(std::is_move_assignable_v<TypeRef>);

class FieldNode final : folly::MoveOnly {
 public:
  FieldNode(
      FieldIdentity identity,
      PresenceQualifier presence,
      TypeRef type,
      std::optional<SerializableRecord> customDefaultValue)
      : identity_(std::move(identity)),
        presence_(presence),
        type_(std::move(type)),
        customDefaultValue_(std::move(customDefaultValue)) {}

  const FieldIdentity& identity() const { return identity_; }
  PresenceQualifier presence() const { return presence_; }
  const TypeRef& type() const { return type_; }
  const SerializableRecord* FOLLY_NULLABLE customDefault() const {
    return customDefaultValue_.has_value()
        ? std::addressof(*customDefaultValue_)
        : nullptr;
  }

 private:
  FieldIdentity identity_;
  PresenceQualifier presence_;
  TypeRef type_;
  std::optional<SerializableRecord> customDefaultValue_;
};

class StructuredNode {
 public:
  const Uri& uri() const noexcept { return uri_; }
  folly::span<const FieldNode> fields() const noexcept { return fields_; }
  bool isSealed() const noexcept { return isSealed_; }

 protected:
  Uri uri_;
  std::vector<FieldNode> fields_;
  bool isSealed_;

  StructuredNode(Uri uri, std::vector<FieldNode> fields, bool isSealed)
      : uri_(std::move(uri)), fields_(std::move(fields)), isSealed_(isSealed) {}
};

class StructNode final : folly::MoveOnly, public StructuredNode {
 public:
  StructNode(Uri, std::vector<FieldNode>, bool isSealed);
};

class UnionNode final : folly::MoveOnly, public StructuredNode {
 public:
  UnionNode(Uri, std::vector<FieldNode>, bool isSealed);
};

class EnumNode final : folly::MoveOnly {
 public:
  struct Value {
    std::string name;
    std::int32_t i32;

    friend bool operator==(const Value& lhs, const Value& rhs) noexcept {
      return std::tie(lhs.name, lhs.i32) == std::tie(rhs.name, rhs.i32);
    }
    friend bool operator!=(const Value& lhs, const Value& rhs) noexcept {
      return !(lhs == rhs);
    }
  };

  const Uri& uri() const noexcept { return uri_; }
  folly::span<const Value> values() const noexcept { return values_; }

  explicit EnumNode(Uri uri, std::vector<Value> values)
      : uri_(std::move(uri)), values_(std::move(values)) {}

 private:
  Uri uri_;
  std::vector<Value> values_;
};

class OpaqueAliasNode final : folly::MoveOnly {
 public:
  const Uri& uri() const noexcept { return uri_; }
  const TypeRef& targetType() const noexcept { return targetType_; }

  explicit OpaqueAliasNode(Uri uri, TypeRef targetType)
      : uri_(std::move(uri)), targetType_(std::move(targetType)) {}

 private:
  Uri uri_;
  TypeRef targetType_;
};

class DefinitionRef final {
 private:
  using StructPtr = folly::not_null<const StructNode*>;
  using UnionPtr = folly::not_null<const UnionNode*>;
  using EnumPtr = folly::not_null<const EnumNode*>;
  using OpaqueAliasPtr = folly::not_null<const OpaqueAliasNode*>;
  using Alternative =
      std::variant<StructPtr, UnionPtr, EnumPtr, OpaqueAliasPtr>;

 public:
  enum class Kind {
    STRUCT = detail::IndexOf<Alternative, StructPtr>,
    UNION = detail::IndexOf<Alternative, UnionPtr>,
    ENUM = detail::IndexOf<Alternative, EnumPtr>,
    OPAQUE_ALIAS = detail::IndexOf<Alternative, OpaqueAliasPtr>
  };
  Kind kind() const { return static_cast<Kind>(definition_.index()); }

  bool isStruct() const noexcept { return kind() == Kind::STRUCT; }
  bool isUnion() const noexcept { return kind() == Kind::UNION; }
  bool isEnum() const noexcept { return kind() == Kind::ENUM; }
  bool isOpaqueAlias() const noexcept { return kind() == Kind::OPAQUE_ALIAS; }

  const StructNode& asStruct() const {
    if (kind() != Kind::STRUCT) {
      throwDefinitionRefAccessInactiveKind();
    }
    return *std::get<StructPtr>(definition_);
  }
  const UnionNode& asUnion() const {
    if (kind() != Kind::UNION) {
      throwDefinitionRefAccessInactiveKind();
    }
    return *std::get<UnionPtr>(definition_);
  }
  const EnumNode& asEnum() const {
    if (kind() != Kind::ENUM) {
      throwDefinitionRefAccessInactiveKind();
    }
    return *std::get<EnumPtr>(definition_);
  }
  const OpaqueAliasNode& asOpaqueAlias() const {
    if (kind() != Kind::OPAQUE_ALIAS) {
      throwDefinitionRefAccessInactiveKind();
    }
    return *std::get<OpaqueAliasPtr>(definition_);
  }

  bool isStructured() const noexcept { return isStruct() || isUnion(); }
  const StructuredNode& asStructured() const {
    switch (kind()) {
      case Kind::STRUCT:
        return asStruct();
      case Kind::UNION:
        return asUnion();
      default:
        break;
    }
    throwDefinitionRefAccessInactiveKind();
  }

  /**
   * Returns the URI associated with this definition. Since all user-defined
   * types must have URIs, this function is infallible.
   */
  const Uri& uri() const noexcept;

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying type.
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const {
    auto overloaded = folly::overload(std::forward<F>(visitors)...);
    return folly::variant_match(
        definition_,
        [&](auto&& nodePtr) -> decltype(auto) { return overloaded(*nodePtr); });
  }

  /**
   * `asType<T>` produces the contained T, assuming it is the currently active
   * variant alternative.
   *
   * Pre-conditions:
   *   - T is the active variant alternative, else throws `std::runtime_error`
   */
  template <typename T>
  const T& asType() const {
    return visit(
        [](const T& value) -> const T& { return value; },
        [&](const auto&) -> const T& {
          throwDefinitionRefAccessInactiveKind();
        });
  }

  explicit DefinitionRef(Alternative definition)
      : definition_(std::move(definition)) {}

 private:
  Alternative definition_;

  static std::string_view kindToString(Kind) noexcept;
  [[noreturn]] void throwDefinitionRefAccessInactiveKind() const {
    detail::throwDefinitionRefAccessInactiveKind(kindToString(kind()));
  }
};

template <typename... F>
decltype(auto) TypeRef::visit(F&&... visitors) const {
  auto overloaded = folly::overload(std::forward<F>(visitors)...);
  return folly::variant_match(
      type_,
      [&](StructPtr structDef) -> decltype(auto) {
        return overloaded(*structDef);
      },
      [&](UnionPtr unionDef) -> decltype(auto) {
        return overloaded(*unionDef);
      },
      [&](EnumPtr enumDef) -> decltype(auto) { return overloaded(*enumDef); },
      [&](OpaqueAliasPtr opaqueAliasDef) -> decltype(auto) {
        return overloaded(*opaqueAliasDef);
      },
      [&](const List& list) -> decltype(auto) { return overloaded(list); },
      [&](const Set& set) -> decltype(auto) { return overloaded(set); },
      [&](const Map& map) -> decltype(auto) { return overloaded(map); },
      [&](const auto& primitive) -> decltype(auto) {
        return overloaded(primitive);
      });
}

} // namespace apache::thrift::dynamic
