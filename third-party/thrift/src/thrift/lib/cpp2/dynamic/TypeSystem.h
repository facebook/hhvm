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
#include <thrift/lib/cpp2/dynamic/detail/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/CppAttributes.h>
#include <folly/Synchronized.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>
#include <folly/lang/Assume.h>
#include <folly/lang/SafeAssert.h>
#include <folly/memory/not_null.h>

#include <fmt/core.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

// TypeSystem.h
// ────────────────────────────────────────────────────────────────────────────
// A C++ interface encapsulating a Thrift type system (as described by the
// Object Model)
//
// A Thrift type system is a collection of types, where each type, and all the
// types it refers to, are fully defined within the same type system.
//
// A valid type system must, therefore, include:
//   - All primitive types (i32, string, etc.)
//   - All instantiable container types (list, set, map)
//   - For every included structured type (struct, union):
//       - all types referenced in the structured type's fields
//   - For every included opaque alias type:
//       - the underlying target type
//
// clang-format off
//                                             ┌─────────────┐
//     ┌──────────────────────────────────────▶│ Thrift Type │◀───────────────────────────────────┐
//     │                                       └─────────────┘                                    │
//     │ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─                  │                                           │
//     ││    User-defined Types     │                 │                                           │
//     │                                              ▼                                           │
//     ││ ┌──────────┐              │              ┌ ─ ─ ┐           ┌─────────┐                  │
//     │  │   Enum   │◀────────────────────┬─────── oneof ──────────▶│Container│                  │
//     ││ └──────────┘              │      │       └ ─ ─ ┘           └─────────┘    ┌──────────┐  │
//     │                                   │          │                   │     ┌──▶│   List   │  │
//     ││                           │      │          │                   │     │   ├──────────┘  │
//     │  ┌──────────────┐                 │          │                   │     │     Element  │──┤
//     ││ │ Opaque Alias │◀─────────┼──────┤          ▼                   │     │   └ ─ ─ ─ ─ ─   │
//     │  ├──────────────┘                 │     ┌─────────┐              │     │   ┌──────────┐  │
//     ├┼─     Target    │          │      │     │Primitive│              ▼     ├──▶│   Set    │  │
//     │  └ ─ ─ ─ ─ ─ ─ ─                  │     └─────────┘           ┌ ─ ─ ┐  │   ├──────────┘  │
//     ││                           │      │          │                 oneof ──┤     Element  │──┤
//     │                                   │          │                └ ─ ─ ┘  │   └ ─ ─ ─ ─ ─   │
//     ││             ┌────────────┐│      │          ▼                         │   ┌──────────┐  │
//     │              │ Structured │◀──────┘       ┌ ─ ─ ┐                      └──▶│   Map    │  │
//     ││             └────────────┘│               oneof ────────────┐             ├──────────┘  │
//     │                     │                     └ ─ ─ ┘            │                 Key    │──┤
//     ││                    │      │                 │               │             ├ ─ ─ ─ ─ ─   │
//     │  ┌──────────┐       │                        │               │                Value   │──┤
//     ││ │  Struct  │◀┐     │      │                 ▼               │             └ ─ ─ ─ ─ ─   │
//     │  ├──────────┘ │     ▼           ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─    │                           │
//     ├┼─   Fields  │ │  ┌ ─ ─ ┐   │     ┌─────┬─────┐┌─────────┐│   │                           │
//     │  └ ─ ─ ─ ─ ─  ├── oneof         ││Byte │Int32││ Float32 │    │                           │
//     ││              │  └ ─ ─ ┘   │     ├─────┼─────┤├─────────┤│   │                           │
//     │  ┌──────────┐ │                 ││Int16│Int64││ Float64 │    │                           │
//     ││ │  Union   │◀┘            │     └─────┴─────┘└─────────┘│   │             ┌──────────┐  │
//     │  ├──────────┘                   │┌────┐┌─────────┐┌─────┐    └────────────▶│   Any    │  │
//     └┼─   Fields  │              │     │Text││ByteArray││Bool ││                 ├──────────┘  │
//        └ ─ ─ ─ ─ ─                    │└────┘└─────────┘└─────┘                     Value   │──┘
//      └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘     ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘                 └ ─ ─ ─ ─ ─
//
// clang-format on
//
// ────────────────────────────────────────────────────────────────────────────
// TypeSystem Graph
// ────────────────────────────────────────────────────────────────────────────
// The TypeSystem abstract class represents a type system as a graph.
// The nodes of the graph are types, and the edges are references between types.
//
// A graph node is one of:
//   - Concrete — capturing user-defined types.
//   - Virtual  — capturing primitive and container types
//
// For example, given the following IDL...
//
//    struct FooStruct {
//      1: i32 field1;
//      2: map<string, FooStruct> field2;
//    }
//
// ...the equivalent TypeSystem graph representation would be...
//                                    ┌ ─ ─ ─ ┐         ┌ ─ ─ ─ ─
//     ╔═══════════════════╗   ┌─────▶   i32              string │
//     ║     FooStruct     ║   │      └ ─ ─ ─ ┘         └ ─ ─ ─ ─
//     ║ ┌───────────────┐ ║   │                             ▲
//     ║ │ (1, "field1") │─╬───┘                             │
//     ║ ├───────────────┤ ║                    ┌────────────┘
//     ║ │ (2, "field2") │─╬───┐                │
//     ║ └───────────────┘ ║   │                │
//     ╚══════════════▲════╝   │      ┌ ─ ─ ─ ─ ┴ ─ ─ ─ ─ ─ ─ ─ ─
//                    │        └─────▶   map<string, FooStruct>  │
//                    │               └ ─ ─ ─ ─ ─ ─ ─ ─ ─│─ ─ ─ ─
//                    │                                  │
//                    └──────────────────────────────────┘
// ...where solid boxes are concrete nodes, and dashed boxes are virtual nodes.
//
// ────────────────────────────────────────────────────────────────────────────
// Concrete Nodes
// ────────────────────────────────────────────────────────────────────────────
// A concrete node represents a user-defined type, so is one of:
//   - StructNode
//   - UnionNode
//   - EnumNode
//   - OpaqueAliasNode
//
// All concrete node objects are owned by the TypeSystem.
// Furthermore, there is exactly one node object for each user-defined type.
// That means, two nodes have the same address if and only if they represent the
// same type.
//
// DefinitionRef objects are references to concrete nodes. They are not part of
// the graph themselves.
// DefinitionRef objects may be copied and moved freely. However, they are
// invalidated if the underlying TypeSystem is destroyed.
//
// ────────────────────────────────────────────────────────────────────────────
// Virtual Nodes
// ────────────────────────────────────────────────────────────────────────────
// A virtual node represents a primitive or container type. They are called
// "virtual" because they are consistent across all type systems and do not
// need to be materialized by the TypeSystem implementation.
//
// ────────────────────────────────────────────────────────────────────────────
// Edges
// ────────────────────────────────────────────────────────────────────────────
// TypeRef objects are references to any node (concrete or virtual). They
// represent edges in the graph.
// TypeRef objects may be copied and moved freely. However, they are invalidated
// if the underlying TypeSystem is destroyed.
//
// ────────────────────────────────────────────────────────────────────────────
// Summary of Types
// ────────────────────────────────────────────────────────────────────────────
// TypeSystem:
//   Represents a type system as a graph. Owns all concrete node objects.
//
// StructNode:
//   Represents a Thrift struct definition. Owns all field objects.
//
// UnionNode:
//   Represents a Thrift union definition. Owns all field objects.
//
// EnumNode:
//   Represents a Thrift enum definition.
//
// OpaqueAliasNode:
//   Represents a Thrift opaque alias definition.
//
// FieldDefinition:
//   Represents a Thrift struct or union field.
//
// DefinitionRef:
//   A reference to a concrete node.
//
// TypeRef:
//   A reference to any node (concrete or virtual).

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::type_system {

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
 * A non-unique Thrift IDL source-based identifier for a user-defined type in a
 * Thrift type system.
 *
 * Unlike URIs, source names may not be unique, even within a single type
 * system. Every type must have exactly one source name name, but a single name
 * may refer to multiple types.
 *
 * A SourceIdentifier has two components:
 *   - location:
 *       A URI of the resource (typically a file) containing the source Thrift
 *       IDL for a user-defined type. Examples:
 *         - "file:///thrift/lib/thrift/standard.thrift"
 *         - "fbsource://xplat/thrift/lib/thrift/standard.thrift"
 *   - name:
 *       The name of the definition within the source Thrift IDL pointed to by
 *       the location above.
 */
struct SourceIdentifierView {
  std::string_view location;
  std::string_view name;

  friend bool operator==(
      SourceIdentifierView lhs, SourceIdentifierView rhs) noexcept {
    return std::tie(lhs.location, lhs.name) == std::tie(rhs.location, rhs.name);
  }
  friend bool operator!=(
      SourceIdentifierView lhs, SourceIdentifierView rhs) noexcept {
    return !(lhs == rhs);
  }
};
struct SourceIdentifier {
  std::string location;
  std::string name;

  /* implicit */ operator SourceIdentifierView() const noexcept {
    return {location, name};
  }

  friend bool operator==(
      const SourceIdentifier& lhs, const SourceIdentifier& rhs) noexcept {
    return std::tie(lhs.location, lhs.name) == std::tie(rhs.location, rhs.name);
  }
  friend bool operator!=(
      const SourceIdentifier& lhs, const SourceIdentifier& rhs) noexcept {
    return !(lhs == rhs);
  }
};

inline bool operator==(
    SourceIdentifierView lhs, const SourceIdentifier& rhs) noexcept {
  return lhs == SourceIdentifierView(rhs);
}
inline bool operator==(
    const SourceIdentifier& lhs, SourceIdentifierView rhs) noexcept {
  return SourceIdentifierView(lhs) == rhs;
}

inline bool operator!=(
    SourceIdentifierView lhs, const SourceIdentifier& rhs) noexcept {
  return !(lhs == rhs);
}
inline bool operator!=(
    const SourceIdentifier& lhs, SourceIdentifierView rhs) noexcept {
  return !(lhs == rhs);
}

class SourceIdentifierHash {
 public:
  using is_transparent = void;

  std::size_t operator()(const SourceIdentifier& sourceIdentifier) const {
    return operator()(SourceIdentifierView(sourceIdentifier));
  }
  std::size_t operator()(SourceIdentifierView sourceIdentifier) const {
    return folly::hash::hash_combine(
        sourceIdentifier.location, sourceIdentifier.name);
  }
};

namespace detail {

struct ContainerTypeCache;

/**
 * A TypeRef of a list type, including the underlying element type.
 */
class ListTypeRef final {
 public:
  explicit ListTypeRef(TypeRef elementType, ContainerTypeCache& cache);

  const TypeRef& elementType() const { return *elementType_; }

  // Helper to construct a ListTypeRef from a type-system node
  // e.g. ListTypeRef::of(TypeRef::Bool())
  template <typename T>
  static ListTypeRef of(T&&, ContainerTypeCache& cache);

  ~ListTypeRef() noexcept = default;
  ListTypeRef(const ListTypeRef&) = default;
  ListTypeRef& operator=(const ListTypeRef&) = default;
  ListTypeRef(ListTypeRef&&) noexcept = default;
  ListTypeRef& operator=(ListTypeRef&&) noexcept = default;

  // See TypeRef::id()
  TypeId id() const;

 private:
  // Pointer into the per-TypeSystem container type cache.
  folly::not_null<const TypeRef*> elementType_;
};

/**
 * A TypeRef of a set type, including the underlying element type.
 */
class SetTypeRef final {
 public:
  explicit SetTypeRef(TypeRef elementType, ContainerTypeCache& cache);

  const TypeRef& elementType() const { return *elementType_; }

  // Helper to construct a SetTypeRef from a type-system node
  // e.g. SetTypeRef::of(TypeRef::Bool())
  template <typename T>
  static SetTypeRef of(T&&, ContainerTypeCache& cache);

  ~SetTypeRef() noexcept = default;
  SetTypeRef(const SetTypeRef&) = default;
  SetTypeRef& operator=(const SetTypeRef&) = default;
  SetTypeRef(SetTypeRef&&) noexcept = default;
  SetTypeRef& operator=(SetTypeRef&&) noexcept = default;

  // See TypeRef::id()
  TypeId id() const;

 private:
  // Pointer into the per-TypeSystem container type cache.
  folly::not_null<const TypeRef*> elementType_;
};

/**
 * A TypeRef of a map type, including the underlying key and value types.
 */
class MapTypeRef final {
 public:
  MapTypeRef(TypeRef keyType, TypeRef valueType, ContainerTypeCache& cache);

  const TypeRef& keyType() const { return *keyType_; }
  const TypeRef& valueType() const { return *valueType_; }

  // Helpers to construct a MapTypeRef from type-system nodes
  // e.g. MapTypeRef::of(TypeRef::I32(), TypeRef::String())
  template <typename K, typename V>
  static MapTypeRef of(K&&, V&&, ContainerTypeCache& cache);

  ~MapTypeRef() noexcept = default;
  MapTypeRef(const MapTypeRef&) = default;
  MapTypeRef& operator=(const MapTypeRef&) = default;
  MapTypeRef(MapTypeRef&&) noexcept = default;
  MapTypeRef& operator=(MapTypeRef&&) noexcept = default;

  // See TypeRef::id()
  TypeId id() const;

 private:
  // Pointers into the per-TypeSystem container type cache.
  folly::not_null<const TypeRef*> keyType_;
  folly::not_null<const TypeRef*> valueType_;
};

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
      throwAccessInactiveKind();
    }
    return asStructUnchecked();
  }
  const StructNode& asStructUnchecked() const noexcept {
    return **std::get_if<StructPtr>(&type_);
  }

  const UnionNode& asUnion() const {
    if (kind() != Kind::UNION) {
      throwAccessInactiveKind();
    }
    return asUnionUnchecked();
  }
  const UnionNode& asUnionUnchecked() const noexcept {
    return **std::get_if<UnionPtr>(&type_);
  }

  const EnumNode& asEnum() const {
    if (kind() != Kind::ENUM) {
      throwAccessInactiveKind();
    }
    return asEnumUnchecked();
  }
  const EnumNode& asEnumUnchecked() const noexcept {
    return **std::get_if<EnumPtr>(&type_);
  }

  const OpaqueAliasNode& asOpaqueAlias() const {
    if (kind() != Kind::OPAQUE_ALIAS) {
      throwAccessInactiveKind();
    }
    return asOpaqueAliasUnchecked();
  }
  const OpaqueAliasNode& asOpaqueAliasUnchecked() const noexcept {
    return **std::get_if<OpaqueAliasPtr>(&type_);
  }

  const List& asList() const {
    if (kind() != Kind::LIST) {
      throwAccessInactiveKind();
    }
    return asListUnchecked();
  }
  const List& asListUnchecked() const noexcept {
    return *std::get_if<List>(&type_);
  }

  const Set& asSet() const {
    if (kind() != Kind::SET) {
      throwAccessInactiveKind();
    }
    return asSetUnchecked();
  }
  const Set& asSetUnchecked() const noexcept {
    return *std::get_if<Set>(&type_);
  }

  const Map& asMap() const {
    if (kind() != Kind::MAP) {
      throwAccessInactiveKind();
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
   * Returns true if the other TypeRef has the same TypeId as this type.
   *
   * The other TypeRef object is allowed to originate from a different
   * TypeSystem instance. This function does not perform a deep equality to
   * compare that the type definitions match in this case.
   *
   * Equivalent to:
   *     this->id() == other.id() (but more efficient)
   */
  bool isEqualIdentityTo(const TypeRef& other) const noexcept;

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
        [&](auto&&) -> const T& { throwAccessInactiveKind(); });
  }
  template <Kind k>
  const std::variant_alternative_t<folly::to_underlying(k), Alternative>&
  asKind() const {
    return asType<
        std::variant_alternative_t<folly::to_underlying(k), Alternative>>();
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
  explicit TypeRef(const StructNode& structDef) noexcept
      : type_(std::in_place_type<StructPtr>, &structDef) {}
  explicit TypeRef(const UnionNode& unionDef) noexcept
      : type_(std::in_place_type<UnionPtr>, &unionDef) {}
  explicit TypeRef(const EnumNode& enumDef) noexcept
      : type_(std::in_place_type<EnumPtr>, &enumDef) {}
  explicit TypeRef(const OpaqueAliasNode& opaqueAliasDef) noexcept
      : type_(std::in_place_type<OpaqueAliasPtr>, &opaqueAliasDef) {}

  /**
   * Creates a reference to a user-defined type.
   */
  static TypeRef fromDefinition(DefinitionRef);

  template <Kind k>
  using KindConstant = std::integral_constant<Kind, k>;
  template <typename... Cases>
  using MatchKindResult = decltype(folly::overload(FOLLY_DECLVAL(Cases)...)(
      KindConstant<Kind::BOOL>{}));

  /**
   * Invokes the provided visitor function with `KindConstant<kind>` where
   * `kind` is provided at runtime. For example:
   *
   *     matchKind(
   *       []<Kind k>(KindConstant<k>) {
   *         // This will be called with k == Kind::I32
   *       }
   *     );
   *
   * Preconditions:
   *   - Kind is one of the enumerated (named) values. Otherwise,
   *     the behavior is undefined.
   */
  template <typename... F>
  MatchKindResult<F...> matchKind(F&&... visitors) const;

 private:
  Alternative type_;

  [[noreturn]] void throwAccessInactiveKind() const;
};
static_assert(std::is_trivially_copyable_v<TypeRef>);
static_assert(std::is_trivially_copy_assignable_v<TypeRef>);
static_assert(std::is_trivially_move_constructible_v<TypeRef>);
static_assert(std::is_trivially_move_assignable_v<TypeRef>);

namespace detail {
// Cache for container type instantiations within a TypeSystem.
// Uses F14NodeMap to ensure stable references even during rehashing.
struct ContainerTypeCache {
  folly::Synchronized<folly::F14NodeMap<TypeId, TypeRef>> cache;
};
} // namespace detail

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
 *
 * This interface supports looking up types in two ways:
 *   - by URIs — this is the primary (unique) identity for a user-defined type.
 *   - by source names — a secondary, non-unique, and optional identity for a
 *     user-defined type. Typically, source information for a type system is
 *     derived from the Thrift IDL source file that produced it.
 */
class TypeSystem {
 public:
  virtual ~TypeSystem() noexcept = default;

  /**
   * Resolves the definition of a user-defined type, and implicitly, all
   * transitively referenced types.
   *
   * Returns an empty optional if the requested type does not exist in the type
   * system.
   */
  virtual std::optional<DefinitionRef> getUserDefinedType(UriView) const = 0;
  /**
   * Same as getUserDefinedType except throws an exception if the type is not
   * found.
   *
   * Throws:
   *   - InvalidTypeError if the type is not defined in the type system.
   */
  DefinitionRef getUserDefinedTypeOrThrow(UriView) const;
  /**
   * Resolves an arbitrary TypeId ito a TypeRef.
   *
   * Throws:
   *   - InvalidTypeError if the TypeId references user-defined types that are
   *     not defined in the type system.
   */
  TypeRef resolveTypeId(const TypeId& typeId) const;

  /**
   * Generates a set of all user-defined type URIs currently known to the type
   * system.
   *
   * For every URI returned by this function, `getUserDefinedType` must succeed.
   * For every URI that is NOT returned by this function, `getUserDefinedType`
   * must fail.
   *
   * If the set of URIs is not finitely enumerable, then this function should
   * return an empty optional.
   */
  virtual std::optional<folly::F14FastSet<Uri>> getKnownUris() const = 0;

  /**
   * Resolves the defintion of a user-defined type referred to by a source
   * identifier, if it exists.
   *
   * Note that source information is optional — not all user-defined types may
   * have a source identifier.
   */
  virtual std::optional<DefinitionRef> getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView) const = 0;

  /**
   * Retrieves the source identifier for a user-defined type, if it exists. This
   * is the inverse of `getUserDefinedTypeBySourceIdentifier`.
   *
   * Note that source information is optional — not all user-defined types may
   * have a source identifier.
   */
  virtual std::optional<SourceIdentifierView>
      getSourceIdentiferForUserDefinedType(DefinitionRef) const = 0;

  using NameToDefinitionsMap = folly::F14FastMap<std::string, DefinitionRef>;
  /**
   * Resolves all definitions of user-defined types at a provided location URI.
   * Typically, this URI points to a source Thrift IDL file.
   *
   * The result is a mapping of definition name to a reference to the
   * definition.
   *
   * For every name in the result, `getUserDefinedTypeBySourceIdentifier` must
   * also succeed.
   */
  virtual NameToDefinitionsMap getUserDefinedTypesAtLocation(
      std::string_view location) const = 0;

  /**
   * Creates a TypeRef for the `bool` type — either true or false.
   */
  static TypeRef Bool() noexcept;
  /**
   * Creates a TypeRef for the `byte` type — 8-bit signed integer.
   */
  static TypeRef Byte() noexcept;
  /**
   * Creates a TypeRef for the `i16` type — 16-bit signed integer.
   */
  static TypeRef I16() noexcept;
  /**
   * Creates a TypeRef for the `i32` type — 32-bit signed integer.
   */
  static TypeRef I32() noexcept;
  /**
   * Creates a TypeRef for the `i64` type — 64-bit signed integer.
   */
  static TypeRef I64() noexcept;
  /**
   * Creates a TypeRef for the `float` type — IEEE 754 binary32.
   */
  static TypeRef Float() noexcept;
  /**
   * Creates a TypeRef for the `double` type — IEEE 754 binary64.
   */
  static TypeRef Double() noexcept;
  /**
   * Creates a TypeRef for the `string` type — UTF-8 encoded bytes.
   */
  static TypeRef String() noexcept;
  /**
   * Creates a TypeRef for the `binary` type — a sequence of bytes.
   */
  static TypeRef Binary() noexcept;
  /**
   * Creates a TypeRef for the `any` type — a type-erased Thrift value.
   */
  static TypeRef Any() noexcept;

  /**
   * Creates a TypeRef for a `list` type.
   * The provided element type must from the same type system.
   */
  TypeRef ListOf(TypeRef elementType) const;
  /**
   * Creates a TypeRef for a `set` type.
   * The provided element type must from the same type system.
   */
  TypeRef SetOf(TypeRef elementType) const;
  /**
   * Creates a TypeRef for a `list` type.
   * The provided key and value types must from the same type system.
   */
  TypeRef MapOf(TypeRef keyType, TypeRef valueType) const;

  /**
   * Creates a TypeRef for a user-defined type.
   *
   * Throws:
   *   - InvalidTypeError if the type is not defined in the type system.
   */
  TypeRef UserDefined(UriView) const;

  /**
   * Get the container type cache for this TypeSystem instance.
   * Thread-safe access is ensured using folly::Synchronized.
   */
  detail::ContainerTypeCache& containerTypeCache() const {
    return *containerTypeCache_;
  }

 private:
  // Cache for container type instantiations, ensuring each unique container
  // type is only instantiated once. Uses F14NodeMap for stable references.
  // Thread-safety is managed by folly::Synchronized.
  mutable std::unique_ptr<detail::ContainerTypeCache> containerTypeCache_ =
      std::make_unique<detail::ContainerTypeCache>();
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

using AnnotationsMap = detail::AnnotationsMap;

class FieldDefinition final : folly::MoveOnly {
 public:
  FieldDefinition(
      FieldIdentity identity,
      PresenceQualifier presence,
      TypeRef type,
      std::optional<SerializableRecord> customDefaultPartialRecord,
      AnnotationsMap annotations);

  const FieldIdentity& identity() const { return identity_; }
  PresenceQualifier presence() const { return presence_; }
  const TypeRef& type() const { return type_; }
  const SerializableRecord* FOLLY_NULLABLE customDefault() const {
    return customDefaultPartialRecord_.has_value()
        ? std::addressof(*customDefaultPartialRecord_)
        : nullptr;
  }

  const AnnotationsMap& annotations() const noexcept { return annotations_; }

  const SerializableRecord* FOLLY_NULLABLE
  getAnnotationOrNull(UriView uri) const {
    return folly::get_ptr(annotations_, uri);
  }

  protocol::TType wireType() const { return wireType_; }

 private:
  FieldIdentity identity_;
  PresenceQualifier presence_;
  TypeRef type_;
  protocol::TType wireType_;
  std::optional<SerializableRecord> customDefaultPartialRecord_;
  AnnotationsMap annotations_;
};

/**
 * A transparent handle that refers to a field within a `StructuredNode` using a
 * 1-based index. Rather than a map lookup (either field ID or field name), this
 * handle allows very fast array-based lookup.
 *
 * A FastFieldHandle is only valid for the StructuredNode instance (either
 * StructNode or UnionNode) that created it. Using a FastFieldHandle with a
 * different instance produces undefined behavior.
 */
struct FastFieldHandle {
  /**
   * The 1-based index into a StructuredNode's fields array.
   */
  std::uint16_t ordinal;
  /**
   * The 0-based index into a StructuredNode's fields array.
   *
   * Preconditions:
   *   - valid() == true
   */
  std::uint16_t index() const noexcept {
    FOLLY_SAFE_DCHECK(valid(), "invalid handle");
    return ordinal - 1;
  }

  /**
   * This is a sentinel value that indicates an invalid handle — i.e. a field is
   * not present.
   */
  static constexpr FastFieldHandle invalid() noexcept {
    return FastFieldHandle{0};
  }
  bool valid() const noexcept { return ordinal != 0; }
  explicit operator bool() const noexcept { return valid(); }

  static FastFieldHandle fromIndex(std::uint16_t index) noexcept {
    return FastFieldHandle{std::uint16_t(index + 1)};
  }

  friend bool operator==(
      const FastFieldHandle& lhs, const FastFieldHandle& rhs) {
    return lhs.ordinal == rhs.ordinal;
  }
  friend bool operator!=(
      const FastFieldHandle& lhs, const FastFieldHandle& rhs) {
    return !(lhs == rhs);
  }
};

class DefinitionNode {
 public:
  /**
   * Returns the URI associated with this definition.
   * Throws `InvalidTypeError` if the type does not have a URI.
   */
  const Uri& uri() const;

 protected:
  Uri uri_;

  explicit DefinitionNode(Uri uri) noexcept : uri_(std::move(uri)) {}
};

class StructuredNode : public DefinitionNode {
 public:
  folly::span<const FieldDefinition> fields() const noexcept { return fields_; }
  bool isSealed() const noexcept { return isSealed_; }

  /**
   * Looks up a field by ID.
   *
   * Throws:
   *   - std::out_of_range if the field ID is not present.
   */
  const FieldDefinition& at(FieldId id) const { return at(fieldHandleFor(id)); }
  /**
   * Looks up a field by name.
   *
   * Throws:
   *   - std::out_of_range if the field ID is not present.
   */
  const FieldDefinition& at(std::string_view name) const {
    return at(fieldHandleFor(name));
  }
  /**
   * Looks up a field by a fast field handle, previously obtained from
   * fieldHandleFor(...);
   *
   * Preconditions:
   *   - The provided handle was obtained by calling fieldHandleFor(...) on this
   *     instance.
   *
   * Throws:
   *   - std::out_of_range if the field handle is invalid or out of range.
   */
  const FieldDefinition& at(FastFieldHandle handle) const {
    if (!handle.valid() || handle.ordinal > fields_.size()) {
      throw std::out_of_range(
          fmt::format("invalid field handle: {}", handle.ordinal));
    }
    return fields_.at(handle.ordinal - 1);
  }

  bool hasField(FieldId id) const noexcept {
    return fieldHandleById_.contains(id);
  }
  bool hasField(std::string_view name) const noexcept {
    return fieldHandleByName_.contains(name);
  }

  /**
   * Returns a field handle for the given field ID, if it exists, returning
   * `FastFieldHandle::invalid()` otherwise.
   */
  FastFieldHandle fieldHandleFor(FieldId id) const noexcept {
    if (const FastFieldHandle* handle = folly::get_ptr(fieldHandleById_, id)) {
      return *handle;
    }
    return FastFieldHandle::invalid();
  }
  /**
   * Returns a field handle for the given field name, if it exists, returning
   * `FastFieldHandle::invalid()` otherwise.
   */
  FastFieldHandle fieldHandleFor(std::string_view name) const noexcept {
    if (const FastFieldHandle* handle =
            folly::get_ptr(fieldHandleByName_, name)) {
      return *handle;
    }
    return FastFieldHandle::invalid();
  }

  const AnnotationsMap& annotations() const noexcept { return annotations_; }

  const SerializableRecord* FOLLY_NULLABLE
  getAnnotationOrNull(UriView uri) const {
    return folly::get_ptr(annotations_, uri);
  }

 protected:
  std::vector<FieldDefinition> fields_;
  folly::F14FastMap<FieldId, FastFieldHandle> fieldHandleById_;
  folly::F14FastMap<std::string_view, FastFieldHandle> fieldHandleByName_;
  bool isSealed_;
  AnnotationsMap annotations_;

  StructuredNode(
      Uri uri,
      std::vector<FieldDefinition> fields,
      bool isSealed,
      AnnotationsMap annotations);
};

class StructNode final : folly::MoveOnly, public StructuredNode {
 public:
  StructNode(
      Uri,
      std::vector<FieldDefinition>,
      bool isSealed,
      AnnotationsMap annotations);

  TypeRef asRef() const noexcept { return TypeRef(*this); }
};

class UnionNode final : folly::MoveOnly, public StructuredNode {
 public:
  UnionNode(
      Uri,
      std::vector<FieldDefinition>,
      bool isSealed,
      AnnotationsMap annotations);

  TypeRef asRef() const noexcept { return TypeRef(*this); }
};

class EnumNode final : folly::MoveOnly, public DefinitionNode {
 public:
  struct Value {
    std::string name;
    std::int32_t i32;
    AnnotationsMap annotations_;

    friend bool operator==(const Value& lhs, const Value& rhs) noexcept {
      return std::tie(lhs.name, lhs.i32) == std::tie(rhs.name, rhs.i32);
    }
    friend bool operator!=(const Value& lhs, const Value& rhs) noexcept {
      return !(lhs == rhs);
    }
    const AnnotationsMap& annotations() const noexcept { return annotations_; }
    const SerializableRecord* FOLLY_NULLABLE
    getAnnotationOrNull(UriView uri) const {
      return folly::get_ptr(annotations_, uri);
    }
  };

  folly::span<const Value> values() const noexcept { return values_; }

  const AnnotationsMap& annotations() const noexcept { return annotations_; }

  const SerializableRecord* FOLLY_NULLABLE
  getAnnotationOrNull(UriView uri) const {
    return folly::get_ptr(annotations_, uri);
  }

  explicit EnumNode(
      Uri uri, std::vector<Value> values, AnnotationsMap annotations)
      : DefinitionNode(std::move(uri)),
        values_(std::move(values)),
        annotations_(std::move(annotations)) {}

  TypeRef asRef() const noexcept { return TypeRef(*this); }

 private:
  std::vector<Value> values_;
  AnnotationsMap annotations_;
};

class OpaqueAliasNode final : folly::MoveOnly, public DefinitionNode {
 public:
  const TypeRef& targetType() const noexcept { return targetType_; }

  const AnnotationsMap& annotations() const noexcept { return annotations_; }

  const SerializableRecord* FOLLY_NULLABLE
  getAnnotationOrNull(UriView uri) const {
    return folly::get_ptr(annotations_, uri);
  }

  explicit OpaqueAliasNode(
      Uri uri, TypeRef targetType, AnnotationsMap annotations)
      : DefinitionNode(std::move(uri)),
        targetType_(targetType),
        annotations_(std::move(annotations)) {}

  TypeRef asRef() const noexcept { return TypeRef(*this); }

 private:
  TypeRef targetType_;
  AnnotationsMap annotations_;
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
      throwAccessInactiveKind();
    }
    return *std::get<StructPtr>(definition_);
  }
  const UnionNode& asUnion() const {
    if (kind() != Kind::UNION) {
      throwAccessInactiveKind();
    }
    return *std::get<UnionPtr>(definition_);
  }
  const EnumNode& asEnum() const {
    if (kind() != Kind::ENUM) {
      throwAccessInactiveKind();
    }
    return *std::get<EnumPtr>(definition_);
  }
  const OpaqueAliasNode& asOpaqueAlias() const {
    if (kind() != Kind::OPAQUE_ALIAS) {
      throwAccessInactiveKind();
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
      case Kind::ENUM:
      case Kind::OPAQUE_ALIAS:
      default:
        break;
    }
    throwAccessInactiveKind();
  }

  /**
   * Returns the URI associated with this definition.
   * Throws `InvalidTypeError` if the type does not have a URI.
   */
  const Uri& uri() const;

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
        [&](const auto&) -> const T& { throwAccessInactiveKind(); });
  }

  /**
   * Returns true if two DefinitionRef objects refer to the exact same node
   * object (by address).
   *
   * Note that there is always exactly one node object for each definition, and
   * so the address of the node object is sufficient to uniquely identify the
   * definition.
   */
  friend bool operator==(const DefinitionRef&, const DefinitionRef&) noexcept;

  explicit DefinitionRef(Alternative definition)
      : definition_(std::move(definition)) {}

 private:
  Alternative definition_;

  [[noreturn]] void throwAccessInactiveKind() const;
};

template <typename... F>
decltype(auto) TypeRef::visit(F&&... visitors) const {
  return folly::variant_match(type_, [&](const auto& val) -> decltype(auto) {
    return folly::overload(std::forward<F>(visitors)...)(
        detail::maybe_deref(val));
  });
}

namespace detail {

template <typename T>
/* static */ ListTypeRef ListTypeRef::of(
    T&& element, ContainerTypeCache& cache) {
  return ListTypeRef(TypeRef(std::forward<T>(element)), cache);
}

template <typename T>
/* static */ SetTypeRef SetTypeRef::of(T&& element, ContainerTypeCache& cache) {
  return SetTypeRef(TypeRef(std::forward<T>(element)), cache);
}

template <typename K, typename V>
/* static */ MapTypeRef MapTypeRef::of(
    K&& key, V&& value, ContainerTypeCache& cache) {
  return MapTypeRef(
      TypeRef(std::forward<K>(key)), TypeRef(std::forward<V>(value)), cache);
}

} // namespace detail

/* static */ inline TypeRef TypeSystem::Bool() noexcept {
  return TypeRef(TypeRef::Bool());
}

/* static */ inline TypeRef TypeSystem::Byte() noexcept {
  return TypeRef(TypeRef::Byte());
}

/* static */ inline TypeRef TypeSystem::I16() noexcept {
  return TypeRef(TypeRef::I16());
}

/* static */ inline TypeRef TypeSystem::I32() noexcept {
  return TypeRef(TypeRef::I32());
}

/* static */ inline TypeRef TypeSystem::I64() noexcept {
  return TypeRef(TypeRef::I64());
}

/* static */ inline TypeRef TypeSystem::Float() noexcept {
  return TypeRef(TypeRef::Float());
}

/* static */ inline TypeRef TypeSystem::Double() noexcept {
  return TypeRef(TypeRef::Double());
}

/* static */ inline TypeRef TypeSystem::String() noexcept {
  return TypeRef(TypeRef::String());
}

/* static */ inline TypeRef TypeSystem::Binary() noexcept {
  return TypeRef(TypeRef::Binary());
}

/* static */ inline TypeRef TypeSystem::Any() noexcept {
  return TypeRef(TypeRef::Any());
}

inline TypeRef TypeSystem::ListOf(TypeRef elementType) const {
  return TypeRef(TypeRef::List(elementType, containerTypeCache()));
}

inline TypeRef TypeSystem::SetOf(TypeRef elementType) const {
  return TypeRef(TypeRef::Set(elementType, containerTypeCache()));
}

inline TypeRef TypeSystem::MapOf(TypeRef keyType, TypeRef valueType) const {
  return TypeRef(TypeRef::Map(keyType, valueType, containerTypeCache()));
}

inline TypeRef TypeSystem::UserDefined(UriView uri) const {
  return TypeRef::fromDefinition(this->getUserDefinedTypeOrThrow(uri));
}

// TypeRef::matchKind implementation (defined after OpaqueAliasNode is complete)
template <typename... F>
TypeRef::MatchKindResult<F...> TypeRef::matchKind(F&&... visitors) const {
  if (isOpaqueAlias()) {
    return asOpaqueAlias().targetType().matchKind(std::forward<F>(visitors)...);
  }

  // Intentionally does not support pointer-to-member callables or &&-qualified
  // operator() for build speed.
  auto invokeWith = folly::overload(std::forward<F>(visitors)...);
  switch (kind()) {
    case Kind::BOOL:
      return invokeWith(KindConstant<Kind::BOOL>{});
    case Kind::BYTE:
      return invokeWith(KindConstant<Kind::BYTE>{});
    case Kind::I16:
      return invokeWith(KindConstant<Kind::I16>{});
    case Kind::I32:
      return invokeWith(KindConstant<Kind::I32>{});
    case Kind::I64:
      return invokeWith(KindConstant<Kind::I64>{});
    case Kind::FLOAT:
      return invokeWith(KindConstant<Kind::FLOAT>{});
    case Kind::DOUBLE:
      return invokeWith(KindConstant<Kind::DOUBLE>{});
    case Kind::STRING:
      return invokeWith(KindConstant<Kind::STRING>{});
    case Kind::BINARY:
      return invokeWith(KindConstant<Kind::BINARY>{});
    case Kind::ANY:
      return invokeWith(KindConstant<Kind::ANY>{});
    case Kind::LIST:
      return invokeWith(KindConstant<Kind::LIST>{});
    case Kind::SET:
      return invokeWith(KindConstant<Kind::SET>{});
    case Kind::MAP:
      return invokeWith(KindConstant<Kind::MAP>{});
    case Kind::STRUCT:
      return invokeWith(KindConstant<Kind::STRUCT>{});
    case Kind::UNION:
      return invokeWith(KindConstant<Kind::UNION>{});
    case Kind::ENUM:
      return invokeWith(KindConstant<Kind::ENUM>{});
    case Kind::OPAQUE_ALIAS:
      break;
  }
  folly::assume_unreachable();
}

} // namespace apache::thrift::type_system

namespace std {
template <>
struct hash<apache::thrift::type_system::DefinitionRef> {
  std::size_t operator()(
      const apache::thrift::type_system::DefinitionRef&) const;
};
} // namespace std
