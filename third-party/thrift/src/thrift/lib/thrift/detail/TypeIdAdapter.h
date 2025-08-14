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

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

#include <folly/Overload.h>
#include <folly/lang/Assume.h>
#include <folly/lang/Exception.h>

#include <fmt/core.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

// This file is intended to be used to adapt types in type_id.thrift.
// Do not include this file directly! Use TypeId.h instead.
namespace apache::thrift::type_system {

// From type_id.thrift
class BoolTypeIdStruct;
class ByteTypeIdStruct;
class I16TypeIdStruct;
class I32TypeIdStruct;
class I64TypeIdStruct;
class FloatTypeIdStruct;
class DoubleTypeIdStruct;
class StringTypeIdStruct;
class BinaryTypeIdStruct;
class AnyTypeIdStruct;
class ListTypeIdStruct;
class SetTypeIdStruct;
class MapTypeIdStruct;
class TypeIdUnion;

namespace detail {

/**
 * The adapted type for TypeIdUnion — a unique identifier for a Thrift type
 * within a type system, colloquially called its "typeid".
 *
 * Primitive types have a well-known identity across all type systems. These
 * identities are captured in the `typeids::` namespace. For example,
 * `typeids::Bool`, `typeids::Int32` etc.
 *
 * User-defined types are identified by their URIs. The "kind" of definition
 * (e.g. struct vs union) is not part of the type's identity.
 *
 * Container types are composed of other type identities. For example, a list of
 * bool is represented by `typeids::list(typeids::Bool)`.
 *
 * Every type identity has a canonical, stable, human-readable string
 * representation. This can be retrieved via `TypeId::name()`.
 *
 * TypeId models Copyable and Movable. Copies of TypeId compare equal to each
 * other.
 */
template <typename>
class TypeIdWrapper;
using TypeId = TypeIdWrapper<TypeIdUnion>;
using TypeIdAdapter = InlineAdapter<TypeId>;

/**
 * The adapted type for ListTypeIdStruct.
 */
template <typename>
class ListTypeIdWrapper;
using ListTypeId = ListTypeIdWrapper<ListTypeIdStruct>;
using ListTypeIdAdapter = InlineAdapter<ListTypeId>;

/**
 * The adapted type for SetTypeIdStruct.
 */
template <typename>
class SetTypeIdWrapper;
using SetTypeId = SetTypeIdWrapper<SetTypeIdStruct>;

/**
 * The adapted type for MapTypeIdStruct.
 */
template <typename>
class MapTypeIdWrapper;
using MapTypeId = MapTypeIdWrapper<MapTypeIdStruct>;
using SetTypeIdAdapter = InlineAdapter<SetTypeId>;

/**
 * The adapted type for primitive type tags such as Bool, I16, I32 etc.
 *
 * The template parameter (PrimitiveTagStruct) is one of the type tags (empty
 * structs):
 *   - BoolTypeIdStruct
 *   - ByteTypeIdStruct
 *   - ...
 */
template <typename PrimitiveTagStruct>
class PrimitiveTypeIdWrapper final {
 public:
  // Empty structs of the same type always compare equal.
  friend bool operator==(
      const PrimitiveTypeIdWrapper&, const PrimitiveTypeIdWrapper&) {
    return true;
  }
  friend bool operator!=(
      const PrimitiveTypeIdWrapper&, const PrimitiveTypeIdWrapper&) {
    return false;
  }
};
/**
 * Type tags are empty structs so the adapter implementation can throw away all
 * data in both toThrift and fromThrift.
 */
template <typename PrimitiveTagStruct>
class PrimitiveTypeIdAdapter final {
 private:
  using DefaultType = PrimitiveTagStruct;
  using AdaptedType = PrimitiveTypeIdWrapper<PrimitiveTagStruct>;

 public:
  template <typename U>
  static DefaultType toThrift(U&&) {
    return DefaultType{};
  }

  template <typename U>
  static AdaptedType fromThrift(U&&) {
    return AdaptedType{};
  }
};

using BoolTypeId = PrimitiveTypeIdWrapper<BoolTypeIdStruct>;
using BoolTypeIdAdapter = PrimitiveTypeIdAdapter<BoolTypeIdStruct>;

using ByteTypeId = PrimitiveTypeIdWrapper<ByteTypeIdStruct>;
using ByteTypeIdAdapter = PrimitiveTypeIdAdapter<ByteTypeIdStruct>;

using I16TypeId = PrimitiveTypeIdWrapper<I16TypeIdStruct>;
using I16TypeIdAdapter = PrimitiveTypeIdAdapter<I16TypeIdStruct>;

using I32TypeId = PrimitiveTypeIdWrapper<I32TypeIdStruct>;
using I32TypeIdAdapter = PrimitiveTypeIdAdapter<I32TypeIdStruct>;

using I64TypeId = PrimitiveTypeIdWrapper<I64TypeIdStruct>;
using I64TypeIdAdapter = PrimitiveTypeIdAdapter<I64TypeIdStruct>;

using FloatTypeId = PrimitiveTypeIdWrapper<FloatTypeIdStruct>;
using FloatTypeIdAdapter = PrimitiveTypeIdAdapter<FloatTypeIdStruct>;

using DoubleTypeId = PrimitiveTypeIdWrapper<DoubleTypeIdStruct>;
using DoubleTypeIdAdapter = PrimitiveTypeIdAdapter<DoubleTypeIdStruct>;

using StringTypeId = PrimitiveTypeIdWrapper<StringTypeIdStruct>;
using StringTypeIdAdapter = PrimitiveTypeIdAdapter<StringTypeIdStruct>;

using BinaryTypeId = PrimitiveTypeIdWrapper<BinaryTypeIdStruct>;
using BinaryTypeIdAdapter = PrimitiveTypeIdAdapter<BinaryTypeIdStruct>;

using AnyTypeId = PrimitiveTypeIdWrapper<AnyTypeIdStruct>;
using AnyTypeIdAdapter = PrimitiveTypeIdAdapter<AnyTypeIdStruct>;

std::string toTypeidName(const TypeId&);
std::string toTypeidName(const ListTypeId&);
std::string toTypeidName(const SetTypeId&);
std::string toTypeidName(const MapTypeId&);

[[noreturn]] void throwListBadElementType();
[[noreturn]] void throwSetBadElementType();
[[noreturn]] void throwMapBadKeyType();
[[noreturn]] void throwMapBadValueType();
[[noreturn]] void throwTypeIdAccessInactiveKind(std::string_view actualKind);

template <typename T>
class ListTypeIdWrapper final
    : public type::detail::EqWrap<ListTypeIdWrapper<T>, T> {
 private:
  // ListTypeIdStruct is incomplete at this point. We only use templates to
  // delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, ListTypeIdStruct>);
  using Base = type::detail::EqWrap<ListTypeIdWrapper<T>, T>;

 public:
  using Base::Base;

  // Explicitly using two functions taking by-reference instead of one taking
  // by-value because TypeId is an incomplete type.
  explicit ListTypeIdWrapper(const TypeId& elementType) {
    this->data_.elementType() = elementType;
  }
  explicit ListTypeIdWrapper(TypeId&& elementType) noexcept {
    this->data_.elementType() = std::move(elementType);
  }

  const TypeId& elementType() const& noexcept {
    if (auto elementType = this->data_.elementType()) {
      return *elementType;
    }
    throwListBadElementType();
  }
  TypeId&& elementType() && noexcept {
    if (auto elementType = std::move(this->data_).elementType()) {
      return std::move(*elementType);
    }
    throwListBadElementType();
  }

  std::string name() const { return toTypeidName(*this); }
};

template <typename T>
class SetTypeIdWrapper final
    : public type::detail::EqWrap<SetTypeIdWrapper<T>, T> {
 private:
  // SetTypeIdStruct is incomplete at this point. We only use templates to
  // delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, SetTypeIdStruct>);
  using Base = type::detail::EqWrap<SetTypeIdWrapper<T>, T>;

 public:
  using Base::Base;

  // Explicitly using two functions taking by-reference instead of one taking
  // by-value because TypeId is an incomplete type.
  explicit SetTypeIdWrapper(const TypeId& elementType) {
    this->data_.elementType() = elementType;
  }
  explicit SetTypeIdWrapper(TypeId&& elementType) noexcept {
    this->data_.elementType() = std::move(elementType);
  }

  const TypeId& elementType() const& noexcept {
    if (auto elementType = this->data_.elementType()) {
      return *elementType;
    }
    throwSetBadElementType();
  }
  TypeId&& elementType() && noexcept {
    if (auto elementType = std::move(this->data_).elementType()) {
      return std::move(*elementType);
    }
    throwSetBadElementType();
  }

  std::string name() const { return toTypeidName(*this); }
};

template <typename T>
class MapTypeIdWrapper final
    : public type::detail::EqWrap<MapTypeIdWrapper<T>, T> {
 private:
  // MapTypeIdStruct is incomplete at this point. We only use templates to
  // delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, MapTypeIdStruct>);
  using Base = type::detail::EqWrap<MapTypeIdWrapper<T>, T>;

 public:
  using Base::Base;

  // Explicitly using two functions taking by-reference instead of one taking
  // by-value because TypeId is an incomplete type.
  explicit MapTypeIdWrapper(const TypeId& keyType, const TypeId& valueType) {
    this->data_.keyType() = keyType;
    this->data_.valueType() = valueType;
  }
  explicit MapTypeIdWrapper(TypeId&& keyType, const TypeId& valueType) {
    this->data_.keyType() = std::move(keyType);
    this->data_.valueType() = valueType;
  }
  explicit MapTypeIdWrapper(const TypeId& keyType, TypeId&& valueType) {
    this->data_.keyType() = keyType;
    this->data_.valueType() = std::move(valueType);
  }
  explicit MapTypeIdWrapper(TypeId&& keyType, TypeId&& valueType) noexcept {
    this->data_.keyType() = std::move(keyType);
    this->data_.valueType() = std::move(valueType);
  }

  const TypeId& keyType() const& noexcept {
    if (auto keyType = this->data_.keyType()) {
      return *keyType;
    }
    throwMapBadKeyType();
  }
  TypeId&& keyType() && noexcept {
    if (auto keyType = std::move(this->data_).keyType()) {
      return std::move(*keyType);
    }
    throwMapBadKeyType();
  }

  const TypeId& valueType() const& noexcept {
    if (auto valueType = this->data_.valueType()) {
      return *valueType;
    }
    throwMapBadValueType();
  }
  TypeId&& valueType() && noexcept {
    if (auto valueType = std::move(this->data_).valueType()) {
      return std::move(*valueType);
    }
    throwMapBadValueType();
  }

  std::string name() const { return toTypeidName(*this); }
};
using MapTypeIdAdapter = InlineAdapter<MapTypeId>;

using TypeIdKinds = folly::tag_t<
    BoolTypeId,
    ByteTypeId,
    I16TypeId,
    I32TypeId,
    I64TypeId,
    FloatTypeId,
    DoubleTypeId,
    StringTypeId,
    BinaryTypeId,
    AnyTypeId,
    ListTypeId,
    SetTypeId,
    MapTypeId>;

/**
 * Determines if T is one of the union alternatives of TypeId.
 */
template <typename T>
constexpr bool isTypeIdKind = folly::type_list_find_v<T, TypeIdKinds> !=
    folly::type_list_size_v<TypeIdKinds>;

template <typename T>
class TypeIdWrapper final : public type::detail::EqWrap<TypeIdWrapper<T>, T> {
 private:
  // TypeIdUnion is incomplete at this point. We only use templates to
  // delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, TypeIdUnion>);
  using Base = type::detail::EqWrap<TypeIdWrapper<T>, T>;

 public:
  using Base::Base;

 public:
  using Bool = BoolTypeId;
  using Byte = ByteTypeId;
  using I16 = I16TypeId;
  using I32 = I32TypeId;
  using I64 = I64TypeId;
  using Float = FloatTypeId;
  using Double = DoubleTypeId;
  using String = StringTypeId;
  using Binary = BinaryTypeId;
  using Any = AnyTypeId;
  using Uri = std::string;
  using List = ListTypeId;
  using Set = SetTypeId;
  using Map = MapTypeId;

  /* implicit */ TypeIdWrapper(Bool) noexcept;
  /* implicit */ TypeIdWrapper(Byte) noexcept;
  /* implicit */ TypeIdWrapper(I16) noexcept;
  /* implicit */ TypeIdWrapper(I32) noexcept;
  /* implicit */ TypeIdWrapper(I64) noexcept;
  /* implicit */ TypeIdWrapper(Float) noexcept;
  /* implicit */ TypeIdWrapper(Double) noexcept;
  /* implicit */ TypeIdWrapper(String) noexcept;
  /* implicit */ TypeIdWrapper(Binary) noexcept;
  /* implicit */ TypeIdWrapper(Any) noexcept;
  /* implicit */ TypeIdWrapper(Uri uri) noexcept;
  /* implicit */ TypeIdWrapper(List&& list) noexcept;
  /* implicit */ TypeIdWrapper(Set&& set) noexcept;
  /* implicit */ TypeIdWrapper(Map&& map) noexcept;

  /**
   * Returns true if the underlying data is the empty union.
   *
   * An empty TypeId is an invalid TypeId — most operations will fail. However,
   * empty TypeIds may be created as a result of deserialization so it should be
   * considered as part of input sanitization.
   */
  bool empty() const noexcept {
    return this->data_.getType() == T::Type::__EMPTY__;
  }

  enum class Kind {
    BOOL = folly::to_underlying(T::Type::boolType),
    BYTE = folly::to_underlying(T::Type::byteType),
    I16 = folly::to_underlying(T::Type::i16Type),
    I32 = folly::to_underlying(T::Type::i32Type),
    I64 = folly::to_underlying(T::Type::i64Type),
    FLOAT = folly::to_underlying(T::Type::floatType),
    DOUBLE = folly::to_underlying(T::Type::doubleType),
    STRING = folly::to_underlying(T::Type::stringType),
    BINARY = folly::to_underlying(T::Type::binaryType),
    ANY = folly::to_underlying(T::Type::anyType),
    URI = folly::to_underlying(T::Type::userDefinedType),
    LIST = folly::to_underlying(T::Type::listType),
    SET = folly::to_underlying(T::Type::setType),
    MAP = folly::to_underlying(T::Type::mapType),
  };
  /**
   * Produces the current variant alternative.
   *
   * Pre-conditions:
   *   - empty() == false, else throws `std::runtime_error`
   */
  Kind kind() const {
    if (empty()) {
      folly::throw_exception<std::runtime_error>(
          "tried to access empty TypeId");
    }
    return static_cast<Kind>(this->data_.getType());
  }

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
  bool isUri() const { return kind() == Kind::URI; }
  bool isList() const { return kind() == Kind::LIST; }
  bool isSet() const { return kind() == Kind::SET; }
  bool isMap() const { return kind() == Kind::MAP; }

  const Uri& asUri() const& {
    if (auto uri = this->data_.userDefinedType_ref()) {
      return *uri;
    }
    throwTypeIdAccessInactiveKind();
  }
  Uri&& asUri() && {
    if (auto uri = std::move(this->data_).userDefinedType_ref()) {
      return std::move(*uri);
    }
    throwTypeIdAccessInactiveKind();
  }

  const List& asList() const& {
    if (auto list = this->data_.listType_ref()) {
      return *list;
    }
    throwTypeIdAccessInactiveKind();
  }
  List&& asList() && {
    if (auto list = std::move(this->data_).listType_ref()) {
      return std::move(*list);
    }
    throwTypeIdAccessInactiveKind();
  }

  const Set& asSet() const& {
    if (auto set = this->data_.setType_ref()) {
      return *set;
    }
    throwTypeIdAccessInactiveKind();
  }
  Set&& asSet() && {
    if (auto set = std::move(this->data_).setType_ref()) {
      return std::move(*set);
    }
    throwTypeIdAccessInactiveKind();
  }

  const Map& asMap() const& {
    if (auto map = this->data_.mapType_ref()) {
      return *map;
    }
    throwTypeIdAccessInactiveKind();
  }
  Map&& asMap() && {
    if (auto map = std::move(this->data_).mapType_ref()) {
      return std::move(*map);
    }
    throwTypeIdAccessInactiveKind();
  }

  /**
   * Produces the human-readable canonical typeid name of this instance.
   *
   * Pre-conditions:
   *   - empty() == false
   */
  std::string name() const { return toTypeidName(*this); }

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying definition.
   *
   * Pre-conditions:
   *   - empty() == false
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const {
    auto overloaded = folly::overload(std::forward<F>(visitors)...);
    switch (kind()) {
      case Kind::BOOL:
        return overloaded(Bool());
      case Kind::BYTE:
        return overloaded(Byte());
      case Kind::I16:
        return overloaded(I16());
      case Kind::I32:
        return overloaded(I32());
      case Kind::I64:
        return overloaded(I64());
      case Kind::FLOAT:
        return overloaded(Float());
      case Kind::DOUBLE:
        return overloaded(Double());
      case Kind::STRING:
        return overloaded(String());
      case Kind::BINARY:
        return overloaded(Binary());
      case Kind::ANY:
        return overloaded(Any());
      case Kind::URI:
        return overloaded(asUri());
      case Kind::LIST:
        return overloaded(asList());
      case Kind::SET:
        return overloaded(asSet());
      case Kind::MAP:
        return overloaded(asMap());
      default:
        break;
    }
    // The call to kind() above would have thrown.
    folly::assume_unreachable();
  }

  /**
   * `isType<V>` produces true if V is the currently active variant alternative.
   */
  template <typename V>
  bool isType() const {
    return visit(
        [](const V&) { return true; }, [](const auto&) { return false; });
  }

  /**
   * `asType<V>` produces the contained V, assuming it is the currently active
   * variant alternative.
   *
   * Pre-conditions:
   *   - V is the active variant alternative, else throws
   *     `bad_union_field_access`
   */
  template <typename V>
  const V& asType() const {
    return visit(
        [](const V& value) -> const V& { return value; },
        [&](const auto&) -> const V& { throwTypeIdAccessInactiveKind(); });
  }

  friend bool operator==(const TypeIdWrapper& lhs, Bool) {
    return lhs.isBool();
  }
  friend bool operator==(const TypeIdWrapper& lhs, Byte) {
    return lhs.isByte();
  }
  friend bool operator==(const TypeIdWrapper& lhs, I16) { return lhs.isI16(); }
  friend bool operator==(const TypeIdWrapper& lhs, I32) { return lhs.isI32(); }
  friend bool operator==(const TypeIdWrapper& lhs, I64) { return lhs.isI64(); }
  friend bool operator==(const TypeIdWrapper& lhs, Float) {
    return lhs.isFloat();
  }
  friend bool operator==(const TypeIdWrapper& lhs, Double) {
    return lhs.isDouble();
  }
  friend bool operator==(const TypeIdWrapper& lhs, String) {
    return lhs.isString();
  }
  friend bool operator==(const TypeIdWrapper& lhs, Binary) {
    return lhs.isBinary();
  }
  friend bool operator==(const TypeIdWrapper& lhs, Any) { return lhs.isAny(); }
  friend bool operator==(const TypeIdWrapper& lhs, const Uri& rhs) {
    return lhs.isUri() && lhs.asUri() == rhs;
  }
  friend bool operator==(const TypeIdWrapper& lhs, const List& rhs) {
    return lhs.isList() && lhs.asList() == rhs;
  }
  friend bool operator==(const TypeIdWrapper& lhs, const Set& rhs) {
    return lhs.isSet() && lhs.asSet() == rhs;
  }
  friend bool operator==(const TypeIdWrapper& lhs, const Map& rhs) {
    return lhs.isMap() && lhs.asMap() == rhs;
  }

  template <typename V, std::enable_if_t<isTypeIdKind<V>>* = nullptr>
  friend bool operator==(const V& lhs, const TypeIdWrapper& rhs) {
    return rhs == lhs;
  }

  // In C++20, operator!= can be synthesized from operator==.
  template <typename V, std::enable_if_t<isTypeIdKind<V>>* = nullptr>
  friend bool operator!=(const TypeIdWrapper& lhs, const V& rhs) {
    return !(lhs == rhs);
  }
  template <typename V, std::enable_if_t<isTypeIdKind<V>>* = nullptr>
  friend bool operator!=(const V& lhs, const TypeIdWrapper& rhs) {
    return !(lhs == rhs);
  }

 private:
  [[noreturn]] void throwTypeIdAccessInactiveKind() const {
    detail::throwTypeIdAccessInactiveKind(
        util::enumNameSafe(this->data_.getType()));
  }
};

} // namespace detail

} // namespace apache::thrift::type_system

template <>
struct fmt::formatter<apache::thrift::type_system::detail::TypeId>
    : formatter<std::string_view> {
 public:
  format_context::iterator format(
      const apache::thrift::type_system::detail::TypeId& typeId,
      format_context& ctx) const;
};

namespace std {
// Partial specialization for all Primitive TypeId variants
template <typename PrimitiveTagStruct>
class hash<apache::thrift::type_system::detail::PrimitiveTypeIdWrapper<
    PrimitiveTagStruct>> {
  using Self = apache::thrift::type_system::detail::PrimitiveTypeIdWrapper<
      PrimitiveTagStruct>;

 public:
  size_t operator()(const Self&) const noexcept {
    return ::apache::thrift::op::hash<
        apache::thrift::type::struct_t<PrimitiveTagStruct>>(
        PrimitiveTagStruct{});
  }
};
} // namespace std
