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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <iosfwd>
#include <string>
#include <variant>

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

 private:
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
  friend class Struct;
  friend class Union;
  friend class detail::StructImpl;
  template <typename T>
  friend class detail::ConcreteList;

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
  friend class Struct;
  friend class detail::StructImpl;
  friend class Union;
  template <typename T>
  friend class detail::ConcreteList;

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
  template <typename T>
  friend class detail::ConcreteList;
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
