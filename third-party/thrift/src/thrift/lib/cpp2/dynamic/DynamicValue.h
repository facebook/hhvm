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

#include <thrift/lib/cpp2/dynamic/Path.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <iosfwd>
#include <optional>
#include <string>
#include <variant>

// DynamicValue.h
// ────────────────────────────────────────────────────────────────────────────
// A C++ API for type-erased, type-safe Thrift values
//
// DynamicValue is the dynamic analog to Thrift's conventional code generation
// layer. It provides a way to operate on Thrift values whose types are not
// known at compile-time, while still maintaining full type safety through
// runtime schema validation via the TypeSystem API.
//
// ────────────────────────────────────────────────────────────────────────────
// Static Thrift vs. Dynamic Thrift
// ────────────────────────────────────────────────────────────────────────────
//
// Static Thrift (compile-time schema):
//
//     Foo f;
//     f.field_1() = 75;
//     f.field_2() = "Hello world";
//     serialize(f);
//
// Dynamic Thrift (run-time schema):
//
//     const type_system::TypeSystem& ts = ...;
//     const type_system::StructNode& schema = ts.getUserDefinedTypeOrThrow(
//         "meta.com/foo/Foo").asStruct();
//     DynamicValue d = DynamicValue::makeDefault(schema.asRef());
//     Struct& s = d.asStruct();
//     s.setField("field_1", DynamicValue::makeI64(75));
//     s.setField(FieldId{2}, DynamicValue::makeString("Hello world"));
//     serialize(d);
//
// ────────────────────────────────────────────────────────────────────────────
// Key Properties
// ────────────────────────────────────────────────────────────────────────────
//
// Type-Safe:
//   Every operation is validated against the runtime schema (via TypeSystem).
//   Accessing the wrong type or a nonexistent field throws an exception.
//
// Owning:
//   DynamicValue owns its data using C++ value semantics (copy and move).
//   This means a DynamicValue may be mutable.
//
// ────────────────────────────────────────────────────────────────────────────
// Class Hierarchy
// ────────────────────────────────────────────────────────────────────────────
//
// clang-format off
//
//   ┌─────────────────────────────────────────────────────────────────────────┐
//   │                          DynamicValue                                   │
//   │                    (owning, type-safe value)                            │
//   │                                                                         │
//   │  - Owns its data (copy/move semantics)                                  │
//   │  - Mutable access via as*() methods                                     │
//   │  - Factory methods: makeBool(), makeI32(), makeStruct(), etc.           │
//   └─────────────────────────────────────────────────────────────────────────┘
//                                       │
//                              can be viewed through
//                                       │
//                ┌──────────────────────┴────────────────────────┐
//                │                                               │
//                ▼                                               ▼
//   ┌─────────────────────────┐                     ┌─────────────────────────┐
//   │      DynamicRef         │      implicit       │    DynamicConstRef      │
//   │                         │     conversion      │                         │
//   │  - Non-owning           │────────────────────▶│  - Non-owning           │
//   │  - Mutable access       │                     │  - Read-only access     │
//   └─────────────────────────┘                     └─────────────────────────┘
//
// clang-format on
//
// ────────────────────────────────────────────────────────────────────────────
// Supported Thrift Types
// ────────────────────────────────────────────────────────────────────────────
//
// All Thrift types are supported, including Any and opaque alias types.
//
// ────────────────────────────────────────────────────────────────────────────
// Comparison with Other APIs
// ────────────────────────────────────────────────────────────────────────────
//
// vs. protocol::Value:
//   DynamicValue is type-safe, supports all Thrift serialization protocols, and
//   is generally more performant. It should replace most scenarios where
//   protocol::Value is used but type information is available.
//   protocol::Value is still useful for unschematized views of
//   compact/binary-encoded data.
//
// vs. thrift::Any:
//   Any is for shuttling data whose schema you don't care about across system
//   boundaries. DynamicValue is for interacting with data whose schema you do
//   care about, but is only available at runtime.
//   These may be combined — a service method may accept an `Any` and its schema
//   as arguments and operate on it using `DynamicValue`.
//
// vs. Protobuf DynamicMessage:
//   DynamicValue is the Thrift equivalent, but with stricter type safety and
//   full protocol support.
//
// ────────────────────────────────────────────────────────────────────────────
// Summary of Types
// ────────────────────────────────────────────────────────────────────────────
//
// DynamicValue:
//   An owning, type-safe, type-erased Thrift value. Supports mutable
//   operations and follows C++ value semantics (copy/move).
//
// DynamicRef:
//   A non-owning, mutable reference to a DynamicValue. Use this to provide
//   a view into container elements without copying. The referenced data must
//   remain valid for the lifetime of the DynamicRef.
//
// DynamicConstRef:
//   A non-owning, const reference to a DynamicValue. Use this for read-only
//   access to container elements. Implicitly convertible from DynamicRef.
//   The referenced data must remain valid for the lifetime of the
//   DynamicConstRef.
//
// Handle Types (defined in separate headers):
//   - String:  UTF-8 encoded string value
//   - Binary:  Arbitrary byte sequence (via folly::IOBuf)
//   - Any:     Type-erased Thrift value (via type::AnyData)
//   - List:    Ordered sequence of homogeneous elements
//   - Set:     Unordered collection of unique elements
//   - Map:     Key-value associative container
//   - Struct:  Named, typed fields with field ID access
//   - Union:   Exactly one active field at a time

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::dynamic {

// Forward declarations
namespace detail {
class StructImpl;
}

/**
 * A non-owning const reference to a type-erased Thrift value.
 *
 * This is similar to DynamicRef but holds const pointers for read-only access.
 * Use this when you want to provide a const view into a container element.
 *
 * Lifetime: The referenced TypeRef and data must remain valid for
 * the lifetime of this DynamicConstRef.
 */
class DynamicConstRef final {
 public:
  /**
   * Create a const reference from a const DynamicValue.
   */
  /*implicit*/ DynamicConstRef(const DynamicValue& value);

  /**
   * Create a const reference from a mutable DynamicRef.
   * This allows implicit conversion from DynamicRef to DynamicConstRef.
   */
  /*implicit*/ DynamicConstRef(const DynamicRef& ref);

  DynamicConstRef(const DynamicConstRef&) = default;
  DynamicConstRef(DynamicConstRef&&) = default;
  ~DynamicConstRef() = default;

  /**
   * Assignment operators are deleted because the semantics are ambiguous.
   */
  DynamicConstRef& operator=(const DynamicConstRef&) = delete;
  DynamicConstRef& operator=(DynamicConstRef&&) = delete;

  /**
   * Copy the referenced value into a new DynamicValue.
   */
  DynamicValue copy() const;

  /**
   * Returns the type of the referenced value.
   */
  const type_system::TypeRef& type() const { return *type_; }

  /**
   * Checks if this value is of the given datum kind.
   */
  template <type_system::TypeRef::Kind k>
  bool is() const {
    return type_->kind() == k;
  }

  /**
   * Type-safe const accessors. These check both the datum kind and the type.
   * Always return const T& for read-only access.
   * Throws:
   *   - std::runtime_error if type/kind doesn't match
   */
  template <type_system::TypeRef::Kind k>
  decltype(auto) as() const;

  bool asBool() const;
  int8_t asByte() const;
  int16_t asI16() const;
  int32_t asI32() const;
  int64_t asI64() const;
  float asFloat() const;
  double asDouble() const;
  int32_t asEnum() const { return asI32(); }
  const String& asString() const;
  const Binary& asBinary() const;
  const Any& asAny() const;
  const List& asList() const;
  const Set& asSet() const;
  const Map& asMap() const;
  const Struct& asStruct() const;
  const Union& asUnion() const;

  /**
   * Equality comparison.
   */
  friend bool operator==(
      const DynamicConstRef& lhs, const DynamicConstRef& rhs) noexcept;

  friend bool operator==(
      const DynamicConstRef& lhs, const DynamicValue& rhs) noexcept;

  friend bool operator==(
      const DynamicValue& lhs, const DynamicConstRef& rhs) noexcept {
    return rhs == lhs;
  }

  /**
   * Human-readable string representation.
   */
  std::string debugString() const;

  /**
   * Traverse this value using a Path and return a reference to the value
   * at the end of the traversal.
   *
   * Returns nullopt if the traversal reaches a missing value (unset optional
   * field, out-of-bounds list index, missing map key, missing set element,
   * or inactive union field).
   *
   * Throws:
   *   - InvalidPathAccessError if the path is incompatible with the current
   *     type (e.g., FieldAccess on a list, ListElement on a struct)
   *   - std::runtime_error if AnyType traversal is attempted (not supported)
   */
  std::optional<DynamicConstRef> traverse(const Path& path) const;

 private:
  /**
   * Template helper for traverse() implementation.
   * Used by both DynamicConstRef and DynamicRef to avoid code duplication.
   */
  template <typename RefType>
  static std::optional<RefType> traverseImpl(RefType start, const Path& path);

  template <typename T>
  DynamicConstRef(const type_system::TypeRef& type, const T& value)
      : type_(&type), ptr_(std::in_place_type<const T*>, &value) {}

  /**
   * Helper to dereference the pointer variant and get the actual value.
   */
  template <typename T>
  const T& deref() const {
    return std::visit(
        [](auto ptr) -> const T& {
          using PtrType = decltype(ptr);
          if constexpr (std::is_same_v<PtrType, const detail::Datum*>) {
            return ptr->template as<T>();
          } else if constexpr (std::is_same_v<PtrType, const T*>) {
            return *ptr;
          } else {
            throw std::runtime_error("Type mismatch in DynamicConstRef::deref");
          }
        },
        ptr_);
  }

  friend std::ostream& operator<<(
      std::ostream& os, const DynamicConstRef& value);
  friend class Set;
  friend class Set::ConstIterator;
  friend class Map;
  friend class Map::Iterator;
  friend class Map::ConstIterator;
  friend class Struct;
  friend class Union;
  friend class detail::StructImpl;
  template <typename T>
  friend class detail::ConcreteList;
  template <typename T>
  friend class detail::ConcreteSet;
  template <typename K, typename V>
  friend class detail::ConcreteMap;
  friend struct detail::DatumHash;
  friend class DynamicRef;

  /**
   * Variant of const pointers to all possible Thrift value types.
   */
  using PointerVariant = std::variant<
      const detail::Datum*,
      const bool*,
      const int8_t*,
      const int16_t*,
      const int32_t*,
      const int64_t*,
      const float*,
      const double*,
      const String*,
      const Binary*,
      const Any*,
      const List*,
      const Set*,
      const Map*,
      const Struct*,
      const Union*>;

  folly::not_null<const type_system::TypeRef*> type_;
  PointerVariant ptr_;
};

/**
 * A non-owning reference to a type-erased Thrift value.
 *
 * This is similar to DynamicValue but holds pointers instead of owning
 * the data. Use this when you want to provide a view into a container
 * element without copying.
 *
 * Lifetime: The referenced TypeRef and data must remain valid for
 * the lifetime of this DynamicRef.
 */
class DynamicRef final {
 public:
  /**
   * Create a reference from a DynamicValue.
   */
  /*implicit*/ DynamicRef(DynamicValue& value);
  DynamicRef(const DynamicRef&) = default;
  DynamicRef(DynamicRef&&) = default;
  ~DynamicRef() = default;

  /**
   * Assignment operators are deleted because the semantics are ambiguous:
   * should assignment modify the referenced value or rebind the reference?
   * Use copy() to create a DynamicValue, assign() to copy a value, or
   * explicitly rebind by creating a new DynamicRef.
   */
  DynamicRef& operator=(const DynamicRef&) = delete;
  DynamicRef& operator=(DynamicRef&&) = delete;
  DynamicRef& operator=(const DynamicValue&) = delete;

  /**
   * Copy the referenced value into a new DynamicValue.
   */
  DynamicValue copy() const;

  /**
   * Assign the value from another DynamicRef/DynamicValue to the value this
   * references. Types must match.
   * Throws:
   *   - std::runtime_error if types don't match
   */
  void assign(const DynamicRef& other);
  void assign(DynamicValue&& other);

  /**
   * Returns the type of the referenced value.
   */
  const type_system::TypeRef& type() const { return *type_; }

  /**
   * Checks if this value is of the given datum kind.
   */
  template <type_system::TypeRef::Kind k>
  bool is() const {
    return type_->kind() == k;
  }

  /**
   * Type-safe accessors. These check both the datum kind and the type.
   * Always return T& for mutable access.
   * Throws:
   *   - std::runtime_error if type/kind doesn't match
   */
  template <type_system::TypeRef::Kind k>
  decltype(auto) as();

  bool& asBool();
  int8_t& asByte();
  int16_t& asI16();
  int32_t& asI32();
  int64_t& asI64();
  float& asFloat();
  double& asDouble();
  int32_t& asEnum() { return asI32(); }
  String& asString();
  Binary& asBinary();
  Any& asAny();
  List& asList();
  Set& asSet();
  Map& asMap();
  Struct& asStruct();
  Union& asUnion();

  /**
   * Equality comparison.
   * Two refs are equal if they point to equal types and data.
   */
  friend bool operator==(const DynamicRef& lhs, const DynamicRef& rhs) noexcept;

  friend bool operator==(
      const DynamicRef& lhs, const DynamicValue& rhs) noexcept;

  friend bool operator==(
      const DynamicValue& lhs, const DynamicRef& rhs) noexcept {
    return rhs == lhs;
  }

  /**
   * Human-readable string representation.
   */
  std::string debugString() const;

  /**
   * Traverse this value using a Path and return a reference to the value
   * at the end of the traversal.
   *
   * Returns nullopt if the traversal reaches a missing value (unset optional
   * field, out-of-bounds list index, missing map key, missing set element,
   * or inactive union field).
   *
   * Throws:
   *   - InvalidPathAccessError if the path is incompatible with the current
   *     type (e.g., FieldAccess on a list, ListElement on a struct)
   *   - std::runtime_error if AnyType traversal is attempted (not supported)
   */
  std::optional<DynamicRef> traverse(const Path& path);

 private:
  template <typename T>
  DynamicRef(const type_system::TypeRef& type, T& value)
      : type_(&type), ptr_(std::in_place_type<T*>, &value) {}

  /**
   * Helper to dereference the pointer variant and get the actual value.
   */
  template <typename T>
  T& deref() {
    return std::visit(
        [](auto ptr) -> T& {
          using PtrType = decltype(ptr);
          if constexpr (std::is_same_v<PtrType, detail::Datum*>) {
            return ptr->template as<T>();
          } else if constexpr (std::is_same_v<PtrType, T*>) {
            return *ptr;
          } else {
            throw std::runtime_error("Type mismatch in DynamicRef::deref");
          }
        },
        ptr_);
  }

  friend std::ostream& operator<<(std::ostream& os, const DynamicRef& value);
  friend class DynamicConstRef;
  friend class Map;
  friend class Map::Iterator;
  friend class Struct;
  friend class detail::StructImpl;
  friend class Union;
  template <typename T>
  friend class detail::ConcreteList;
  template <typename T>
  friend class detail::ConcreteSet;
  template <typename K, typename V>
  friend class detail::ConcreteMap;

  /**
   * Variant of pointers to all possible Thrift value types.
   * This allows DynamicRef to point to either a Datum or directly to a concrete
   * type.
   */
  using PointerVariant = std::variant<
      detail::Datum*,
      bool*,
      int8_t*,
      int16_t*,
      int32_t*,
      int64_t*,
      float*,
      double*,
      String*,
      Binary*,
      Any*,
      List*,
      Set*,
      Map*,
      Struct*,
      Union*>;

  folly::not_null<const type_system::TypeRef*> type_;
  PointerVariant ptr_;
};

/**
 * A type-erased Thrift value that knows its type.
 *
 * This provides type-safe operations and conversions.
 */
class DynamicValue final {
 public:
  /**
   * Factory methods for creating typed values.
   */
  static DynamicValue makeBool(bool value);
  static DynamicValue makeByte(int8_t value);
  static DynamicValue makeI16(int16_t value);
  static DynamicValue makeI32(int32_t value);
  static DynamicValue makeI64(int64_t value);
  static DynamicValue makeFloat(float value);
  static DynamicValue makeDouble(double value);
  static DynamicValue makeString(
      std::string_view sv, std::pmr::memory_resource* mr = nullptr);
  static DynamicValue makeBinary(std::unique_ptr<folly::IOBuf> buf);
  static DynamicValue makeAny(type::AnyData anyData);
  static DynamicValue makeAny(
      const DynamicValue& value,
      type::StandardProtocol protocol = type::StandardProtocol::Compact);

  /**
   * Factory method for creating enum values.
   */
  static DynamicValue makeEnum(
      const type_system::EnumNode& enumType, int32_t value);

  /**
   * Factory method for creating a default value for the type.
   */
  static DynamicValue makeDefault(
      type_system::TypeRef type, std::pmr::memory_resource* mr = nullptr);

  /**
   * Returns the type of this value.
   */
  const type_system::TypeRef& type() const { return type_; }

  /**
   * Checks if this value is of the given datum kind.
   */
  template <type_system::TypeRef::Kind k>
  bool is() const;

  /**
   * Type-safe accessors. These check both the datum kind and the type.
   * Throws:
   *   - std::runtime_error if type/kind doesn't match
   */
  template <type_system::TypeRef::Kind k>
  decltype(auto) as() const&;
  template <type_system::TypeRef::Kind k>
  decltype(auto) as() &;

  bool asBool() const&;
  int8_t asByte() const&;
  int16_t asI16() const&;
  int32_t asI32() const&;
  int64_t asI64() const&;
  float asFloat() const&;
  double asDouble() const&;
  int32_t asEnum() const&;
  const String& asString() const&;
  const Binary& asBinary() const&;
  const Any& asAny() const&;
  const List& asList() const&;
  const Set& asSet() const&;
  const Map& asMap() const&;
  const Struct& asStruct() const&;
  const Union& asUnion() const&;

  bool& asBool() &;
  int8_t& asByte() &;
  int16_t& asI16() &;
  int32_t& asI32() &;
  int64_t& asI64() &;
  float& asFloat() &;
  double& asDouble() &;
  int32_t& asEnum() &;
  String& asString() &;
  Binary& asBinary() &;
  Any& asAny() &;
  List& asList() &;
  Set& asSet() &;
  Map& asMap() &;
  Struct& asStruct() &;
  Union& asUnion() &;

  /**
   * Move accessors for handle types.
   * Throws:
   *   - std::runtime_error if type/kind doesn't match
   */
  template <type_system::TypeRef::Kind k>
  decltype(auto) as() &&;
  bool asBool() &&;
  int8_t asByte() &&;
  int16_t asI16() &&;
  int32_t asI32() &&;
  int64_t asI64() &&;
  float asFloat() &&;
  double asDouble() &&;
  String asString() &&;
  Binary asBinary() &&;
  Any asAny() &&;
  List asList() &&;
  Set asSet() &&;
  Map asMap() &&;
  Struct asStruct() &&;
  Union asUnion() &&;

  /**
   * Equality comparison.
   * Two values are equal if they have the same type and datum.
   */
  friend bool operator==(
      const DynamicValue& lhs, const DynamicValue& rhs) noexcept;

  /**
   * Human-readable string representation.
   */
  std::string debugString() const;

  /**
   * Traverse this value using a Path and return a reference to the value
   * at the end of the traversal.
   *
   * Returns nullopt if the traversal reaches a missing value (unset optional
   * field, out-of-bounds list index, missing map key, missing set element,
   * or inactive union field).
   *
   * Throws:
   *   - InvalidPathAccessError if the path is incompatible with the current
   *     type (e.g., FieldAccess on a list, ListElement on a struct)
   *   - std::runtime_error if AnyType traversal is attempted (not supported)
   */
  std::optional<DynamicRef> traverse(const Path& path) &;
  std::optional<DynamicConstRef> traverse(const Path& path) const&;

 private:
  /**
   * Create a value with the given type and datum.
   * Throws:
   *   - std::runtime_error if the datum kind doesn't match the type
   */
  DynamicValue(type_system::TypeRef type, detail::Datum datum);

  friend std::ostream& operator<<(std::ostream& os, const DynamicValue& value);

  template <typename ProtocolWriter>
  friend void serializeValue(ProtocolWriter& prot, const DynamicConstRef& v);

  template <typename ProtocolReader>
  friend DynamicValue deserializeValue(
      ProtocolReader& prot,
      type_system::TypeRef type,
      std::pmr::memory_resource* mr);

  friend class DynamicConstRef;
  friend class DynamicRef;
  friend struct detail::DatumHash;
  template <typename T>
  friend class detail::ConcreteList;
  template <typename T>
  friend class detail::ConcreteSet;
  template <typename K, typename V>
  friend class detail::ConcreteMap;
  friend class Struct;
  friend class detail::StructImpl;
  friend class Union;

  // Friend declarations for fromRecord functions
  friend List fromRecord(
      const type_system::SerializableRecord& r,
      const type_system::TypeRef::List& listType,
      std::pmr::memory_resource* mr);
  friend Struct fromRecord(
      const type_system::SerializableRecord& r,
      const type_system::StructNode& structType,
      std::pmr::memory_resource* mr);

  /**
   * Returns the datum stored in this value.
   */
  const detail::Datum& datum() const& { return datum_; }
  detail::Datum& datum() & { return datum_; }
  detail::Datum&& datum() && { return std::move(datum_); }

  /**
   * Returns the datum kind.
   */
  detail::Datum::Kind kind() const { return datum_.kind(); }

  type_system::TypeRef type_;
  detail::Datum datum_;
};

template <type_system::TypeRef::Kind k>
decltype(auto) DynamicValue::as() const& {
  return datum_.as<detail::Datum::kind_of_type_kind<k>>();
}

template <type_system::TypeRef::Kind k>
decltype(auto) DynamicValue::as() & {
  return datum_.as<detail::Datum::kind_of_type_kind<k>>();
}

template <type_system::TypeRef::Kind k>
decltype(auto) DynamicValue::as() && {
  return datum_.as<detail::Datum::kind_of_type_kind<k>>();
}

template <type_system::TypeRef::Kind k>
bool DynamicValue::is() const {
  return datum_.is<detail::Datum::kind_of_type_kind<k>>();
}

template <type_system::TypeRef::Kind k>
decltype(auto) DynamicConstRef::as() const {
  return deref<detail::Datum::type_of_type_kind<k>>();
}

template <type_system::TypeRef::Kind k>
decltype(auto) DynamicRef::as() {
  return deref<detail::Datum::type_of_type_kind<k>>();
}

} // namespace apache::thrift::dynamic
