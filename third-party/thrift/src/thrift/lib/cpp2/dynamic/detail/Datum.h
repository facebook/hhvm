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

// Minimal includes to avoid pulling in <variant> too early

#include <thrift/lib/cpp2/dynamic/Binary.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/String.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>

#include <fmt/core.h>
#include <folly/CPortability.h>
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/functional/Invoke.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>
#include <folly/lang/Exception.h>

#include <cstdint>
#include <functional>
#include <memory_resource>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace apache::thrift::dynamic {

class Any {};
class Set {};
class Map {};
class Union {};

bool operator==(const Any&, const Any&) noexcept;
bool operator==(const Set&, const Set&) noexcept;
bool operator==(const Map&, const Map&) noexcept;
bool operator==(const Union&, const Union&) noexcept;

struct Null final {
  friend constexpr bool operator==(const Null&, const Null&) noexcept {
    return true;
  }
};

namespace detail {

// Helper function to check type compatibility
void expectType(
    const type_system::TypeRef& expected, const type_system::TypeRef& actual);

/**
 * A Thrift "datum" which serves as the "machine representation" for any value
 * in Thrift's type system. This is a private API.
 *
 * Every Thrift value is formed by the tuple:
 *   (type, datum), or concretely,
 *   (`type_system::TypeRef`, <this class>).
 *
 * A Datum by itself is *not* a value, since it does not (necessarily) contain
 * the type. Instead, Datum's have "kinds" (see `Datum::Kind`), and every
 * Thrift type has a corresponding kind (see `Datum::kind_of`).
 *
 * For example, all enum types (and `i32`), share the same "kind",
 * which is `Datum::Kind::I32`. However, the following values...
 *   - (type = MyEnum,        datum = I32(3))
 *   - (type = SomeOtherEnum, datum = I32(3))
 *   - (type = i32,           datum = I32(3))
 * ...are distinct, even though they share the same Datum representation.
 *
 * NOTE: the distinction between datum and value here comes from Thrift's Object
 * Model. See:
 *   https://www.internalfb.com/intern/staticdocs/thrift/docs/object-model/#definition
 */
class Datum final {
 public:
  using Alternative = std::variant<
      Null, // Not actually a datum, but used to represent unset optional
            // fields.
      bool,
      int8_t,
      int16_t,
      int32_t,
      int64_t,
      float,
      double,
      String,
      Binary,
      Any,
      List,
      Set,
      Map,
      Struct,
      Union>;

  /**
   * An enum encoding the possible kinds of Datum.
   *
   * This is an alias to DatumKind defined in fwd.h
   */
  using Kind = DatumKind;

  /**
   * Produces a human-readable string representation of the given kind.
   */
  static std::string_view nameOf(Kind k) noexcept;

  /**
   * Produces the currently active kind of data stored within this Datum.
   */
  Kind kind() const noexcept { return static_cast<Kind>(alternative_.index()); }

  /**
   * Determines if the given type is one of the possible alternatives stored
   * inside a Datum.
   *
   *     is_alternative<bool>   == true  // stored directly as bool
   *     is_alternative<Struct> == true  // stored directly as Struct
   */
  template <typename T>
  static constexpr bool is_alternative =
      folly::type_list_find_v<T, Alternative> <
      folly::type_list_size_v<Alternative>;

  /**
   * Given a valid Datum alternative type, returns the corresponding kind at
   * compile-time.
   *
   * This is the inverse of Datum::type_of<Kind>.
   */
  template <typename T>
    requires is_alternative<T>
  static constexpr Kind kind_of =
      static_cast<Kind>(folly::type_list_find_v<T, Alternative>);

  /**
   * Given a Datum::Kind, returns the corresponding alternative type that is
   * used to store the value.
   *
   * This is the inverse of Datum::kind_of<Kind>.
   */
  template <Kind k>
  using type_of = datum_kind_to_type<k>;

  template <Kind k>
  using KindConstant = std::integral_constant<Kind, k>;

  /**
   * Invokes the provided visitor function with `KindConstant<kind>` where
   * `kind` is provided at runtime. For example:
   *
   *     matchKind(
   *       Kind::I32,
   *       []<Kind k>(KindConstant<k>) {
   *         // This will be called with k == Kind::I32
   *       }
   *     );
   *
   * Preconditions:
   *   - Datum::Kind is one of the enumerated (named) values. Otherwise,
   *     the behavior is undefined.
   */
  template <typename... F>
  static FOLLY_ALWAYS_INLINE decltype(auto) matchKind(
      Datum::Kind kind, F&&... visitors) {
    const auto invokeWith = [&](auto tag) -> decltype(auto) {
      return std::invoke(folly::overload(std::forward<F>(visitors)...), tag);
    };
    switch (kind) {
      case Kind::Null:
        return invokeWith(KindConstant<Kind::Null>{});
      case Kind::Bool:
        return invokeWith(KindConstant<Kind::Bool>{});
      case Kind::Byte:
        return invokeWith(KindConstant<Kind::Byte>{});
      case Kind::I16:
        return invokeWith(KindConstant<Kind::I16>{});
      case Kind::I32:
        return invokeWith(KindConstant<Kind::I32>{});
      case Kind::I64:
        return invokeWith(KindConstant<Kind::I64>{});
      case Kind::Float:
        return invokeWith(KindConstant<Kind::Float>{});
      case Kind::Double:
        return invokeWith(KindConstant<Kind::Double>{});
      case Kind::String:
        return invokeWith(KindConstant<Kind::String>{});
      case Kind::Binary:
        return invokeWith(KindConstant<Kind::Binary>{});
      case Kind::Any:
        return invokeWith(KindConstant<Kind::Any>{});
      case Kind::List:
        return invokeWith(KindConstant<Kind::List>{});
      case Kind::Set:
        return invokeWith(KindConstant<Kind::Set>{});
      case Kind::Map:
        return invokeWith(KindConstant<Kind::Map>{});
      case Kind::Struct:
        return invokeWith(KindConstant<Kind::Struct>{});
      case Kind::Union:
        return invokeWith(KindConstant<Kind::Union>{});
    }
  }

 public:
  /**
   * Provides a best-effort mapping of TypeRef::Kind to the alternative that
   * would be used to store a value of that type, if the mapping is known at
   * compile-time.
   *
   *     type_of_type_kind<TypeRef::Kind::I32>    → int32_t
   *     type_of_type_kind<TypeRef::Kind::STRING> → String
   */
  template <type_system::TypeRef::Kind k>
  using type_of_type_kind = detail::type_of_type_kind<k>;

  /**
   * Provides a best-effort mapping of TypeRef::Kind to Datum::Kind, where the
   * mapping is known at compile-time.
   *
   *     kind_of_type_kind<TypeRef::Kind::I32>    → Kind::I32
   *     kind_of_type_kind<TypeRef::Kind::STRING> → Kind::String
   */
  template <type_system::TypeRef::Kind k>
  static constexpr Kind kind_of_type_kind = detail::kind_of_type_kind<k>;

  /**
   * Given a possible Datum alternative type, determines if the data is stored
   * inline with the Datum (primitive types), as opposed to larger container
   * types that manage heap allocation internally.
   */
  template <typename T>
    requires is_alternative<T>
  static constexpr bool is_stored_inline = folly::is_one_of_v<
      T,
      Null,
      bool,
      int8_t,
      int16_t,
      int32_t,
      int64_t,
      float,
      double>;

  /**
   * Same as std::is_signed_integral_v<type_of<k>>.
   */
  template <Datum::Kind k>
  static constexpr bool is_signed_integral =
      k == Kind::Byte || k == Kind::I16 || k == Kind::I32 || k == Kind::I64;

  /**
   * Same as std::is_floating_point_v<type_of<k>>.
   */
  template <Datum::Kind k>
  static constexpr bool is_floating_point =
      k == Kind::Float || k == Kind::Double;

  /**
   * A numeric kind is one of {byte, i16, i32, i64, float, double}.
   */
  template <Datum::Kind k>
  static constexpr bool is_numeric =
      is_signed_integral<k> || is_floating_point<k>;

  /**
   * Determines if a trivial conversion (i.e. static_cast) from one numeric
   * datum kind to another is guaranteed never lose precision.
   */
  template <Datum::Kind from, Datum::Kind to>
  static constexpr bool is_trivially_widenable =
      is_numeric<from> && is_numeric<to> && std::invoke([] {
        if constexpr (is_signed_integral<from>) {
          if constexpr (is_signed_integral<to>) {
            return sizeof(type_of<from>) <= sizeof(type_of<to>);
          } else {
            // when converting from signed integral → floating point, we need to
            // be able to store the value within the precision (mantissa) bits
            // only.
            //   float → 24 precision bits
            //   double → 53 precision bits
            if constexpr (to == Kind::Float) {
              return from == Kind::Byte || from == Kind::I16;
            } else {
              return from != Kind::I64;
            }
          }
        } else {
          // The following casts are allowed:
          //   - float  → float
          //   - float  → double
          //   - double → double
          return is_floating_point<to> &&
              (to == Kind::Double || from == Kind::Float);
        }
      });
  static_assert(is_trivially_widenable<Kind::I32, Kind::I64>);
  static_assert(!is_trivially_widenable<Kind::I64, Kind::I32>);
  static_assert(!is_trivially_widenable<Kind::Float, Kind::I32>);
  static_assert(is_trivially_widenable<Kind::I16, Kind::Float>);
  static_assert(!is_trivially_widenable<Kind::I32, Kind::Float>);
  static_assert(is_trivially_widenable<Kind::I32, Kind::Double>);
  static_assert(is_trivially_widenable<Kind::Float, Kind::Double>);

  // Initialize with Null
  constexpr Datum() noexcept : alternative_(Null()) {}

  // Initialize with a given value
  template <typename T>
  static Datum make(T&& value) {
    return Datum(std::forward<T>(value));
  }

  static Datum makeNull() noexcept { return Datum(); }
  static Datum makeBool(bool value) noexcept { return Datum(value); }
  static Datum makeByte(int8_t value) noexcept { return Datum(value); }
  static Datum makeI16(int16_t value) noexcept { return Datum(value); }
  static Datum makeI32(int32_t value) noexcept { return Datum(value); }
  static Datum makeI64(int64_t value) noexcept { return Datum(value); }
  static Datum makeFloat(float value) noexcept { return Datum(value); }
  static Datum makeDouble(double value) noexcept { return Datum(value); }

  template <typename T>
  bool is() const noexcept;

  template <Kind k>
  bool is() const noexcept {
    return is<type_of<k>>();
  }

  bool isNull() const noexcept { return is<Kind::Null>(); }
  bool isBool() const noexcept { return is<Kind::Bool>(); }
  bool isByte() const noexcept { return is<Kind::Byte>(); }
  bool isI16() const noexcept { return is<Kind::I16>(); }
  bool isI32() const noexcept { return is<Kind::I32>(); }
  bool isI64() const noexcept { return is<Kind::I64>(); }
  bool isFloat() const noexcept { return is<Kind::Float>(); }
  bool isDouble() const noexcept { return is<Kind::Double>(); }
  bool isString() const noexcept { return is<Kind::String>(); }
  bool isBinary() const noexcept { return is<Kind::Binary>(); }
  bool isAny() const noexcept { return is<Kind::Any>(); }
  bool isList() const noexcept { return is<Kind::List>(); }
  bool isSet() const noexcept { return is<Kind::Set>(); }
  bool isMap() const noexcept { return is<Kind::Map>(); }
  bool isStruct() const noexcept { return is<Kind::Struct>(); }
  bool isUnion() const noexcept { return is<Kind::Union>(); }

  /**
   * Throws:
   *   - std::runtime_error if kind() != k
   */
  void throwIfNot(Kind k) const;

  /**
   * Throws:
   *   - std::runtime_error if kind() == Kind::Null
   */
  void throwIfNull() const;

  /**
   * Throws:
   *   - std::runtime_error if active kind does not match the provided type
   */
  template <typename T>
  const T& as() const&;
  template <typename T>
  T& as() &;
  template <typename T>
  T&& as() &&;

  /**
   * Throws:
   *   - std::runtime_error if active kind is not the same as provided kind
   */
  template <Kind k>
  const type_of<k>& as() const& {
    return as<type_of<k>>();
  }
  template <Kind k>
  type_of<k>& as() & {
    return as<type_of<k>>();
  }
  template <Kind k>
  type_of<k>&& as() && {
    return std::move(*this).as<type_of<k>>();
  }

  bool asBool() const& { return as<Kind::Bool>(); }
  int8_t asByte() const& { return as<Kind::Byte>(); }
  int16_t asI16() const& { return as<Kind::I16>(); }
  int32_t asI32() const& { return as<Kind::I32>(); }
  int64_t asI64() const& { return as<Kind::I64>(); }
  float asFloat() const& { return as<Kind::Float>(); }
  double asDouble() const& { return as<Kind::Double>(); }
  const String& asString() const& { return as<Kind::String>(); }
  const Binary& asBinary() const& { return as<Kind::Binary>(); }
  const Any& asAny() const& { return as<Kind::Any>(); }
  const List& asList() const& { return as<Kind::List>(); }
  const Set& asSet() const& { return as<Kind::Set>(); }
  const Map& asMap() const& { return as<Kind::Map>(); }
  const Struct& asStruct() const& { return as<Kind::Struct>(); }
  const Union& asUnion() const& { return as<Kind::Union>(); }

  bool& asBool() & { return as<Kind::Bool>(); }
  int8_t& asByte() & { return as<Kind::Byte>(); }
  int16_t& asI16() & { return as<Kind::I16>(); }
  int32_t& asI32() & { return as<Kind::I32>(); }
  int64_t& asI64() & { return as<Kind::I64>(); }
  float& asFloat() & { return as<Kind::Float>(); }
  double& asDouble() & { return as<Kind::Double>(); }
  String& asString() & { return as<Kind::String>(); }
  Binary& asBinary() & { return as<Kind::Binary>(); }
  Any& asAny() & { return as<Kind::Any>(); }
  List& asList() & { return as<Kind::List>(); }
  Set& asSet() & { return as<Kind::Set>(); }
  Map& asMap() & { return as<Kind::Map>(); }
  Struct& asStruct() & { return as<Kind::Struct>(); }
  Union& asUnion() & { return as<Kind::Union>(); }

  bool asBool() && { return std::move(*this).as<Kind::Bool>(); }
  int8_t asByte() && { return std::move(*this).as<Kind::Byte>(); }
  int16_t asI16() && { return std::move(*this).as<Kind::I16>(); }
  int32_t asI32() && { return std::move(*this).as<Kind::I32>(); }
  int64_t asI64() && { return std::move(*this).as<Kind::I64>(); }
  float asFloat() && { return std::move(*this).as<Kind::Float>(); }
  double asDouble() && { return std::move(*this).as<Kind::Double>(); }
  String asString() && { return std::move(*this).as<Kind::String>(); }
  Binary asBinary() && { return std::move(*this).as<Kind::Binary>(); }
  Any asAny() && { return std::move(*this).as<Kind::Any>(); }
  List asList() && { return std::move(*this).as<Kind::List>(); }
  Set asSet() && { return std::move(*this).as<Kind::Set>(); }
  Map asMap() && { return std::move(*this).as<Kind::Map>(); }
  Struct asStruct() && { return std::move(*this).as<Kind::Struct>(); }
  Union asUnion() && { return std::move(*this).as<Kind::Union>(); }

  template <typename... F>
  decltype(auto) visit(F&&... visitors) const&;
  template <typename... F>
  decltype(auto) visit(F&&... visitors) &&;

  bool operator==(const Datum& other) const;

 private:
  template <typename T>
  explicit Datum(T value) : alternative_(std::move(value)) {}

  Alternative alternative_;
};

} // namespace detail

///
// Implementation follows
///

template <typename T>
inline bool detail::Datum::is() const noexcept {
  return std::holds_alternative<T>(alternative_);
}

template <typename... F>
inline decltype(auto) detail::Datum::visit(F&&... visitors) const& {
  return folly::variant_match(alternative_, std::forward<F>(visitors)...);
}

template <typename... F>
inline decltype(auto) detail::Datum::visit(F&&... visitors) && {
  return folly::variant_match(
      std::move(alternative_), std::forward<F>(visitors)...);
}

namespace detail {
[[noreturn]] void throwInvalidDatumKindError(
    Datum::Kind expected, Datum::Kind actual);
} // namespace detail

template <typename T>
inline const T& detail::Datum::as() const& {
  if (const T* value = std::get_if<T>(&alternative_)) {
    return *value;
  }
  detail::throwInvalidDatumKindError(kind_of<T>, kind());
}

template <typename T>
inline T& detail::Datum::as() & {
  if (T* value = std::get_if<T>(&alternative_)) {
    return *value;
  }
  detail::throwInvalidDatumKindError(kind_of<T>, kind());
}

template <typename T>
inline T&& detail::Datum::as() && {
  static_assert(!std::is_reference_v<T>);
  if (T* value = std::get_if<T>(&alternative_)) {
    return std::move(*value);
  }
  detail::throwInvalidDatumKindError(kind_of<T>, kind());
}

// ============================================================================
// fromRecord - converts SerializableRecord to Datum
// ============================================================================

// Primitive numeric types
inline bool fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::Bool,
    std::pmr::memory_resource*) {
  return bool(r.asBool());
}

inline int8_t fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::Byte,
    std::pmr::memory_resource*) {
  return int8_t(r.asInt8());
}

inline int16_t fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::I16,
    std::pmr::memory_resource*) {
  return int16_t(r.asInt16());
}

inline int32_t fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::I32,
    std::pmr::memory_resource*) {
  return int32_t(r.asInt32());
}

inline int64_t fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::I64,
    std::pmr::memory_resource*) {
  return int64_t(r.asInt64());
}

inline float fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::Float,
    std::pmr::memory_resource*) {
  return float(r.asFloat32());
}

inline double fromRecord(
    const type_system::SerializableRecord& r,
    type_system::TypeRef::Double,
    std::pmr::memory_resource*) {
  return double(r.asFloat64());
}

inline int32_t fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::EnumNode&,
    std::pmr::memory_resource*) {
  return int32_t(r.asInt32());
}

// String
inline String fromRecord(
    const type_system::SerializableRecord&,
    type_system::TypeRef::String,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(TypeRef::String)");
}

// Binary
inline Binary fromRecord(
    const type_system::SerializableRecord&,
    type_system::TypeRef::Binary,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(TypeRef::Binary)");
}

// Any
inline Any fromRecord(
    const type_system::SerializableRecord&,
    type_system::TypeRef::Any,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(TypeRef::Any)");
}

// Set
inline Set fromRecord(
    const type_system::SerializableRecord&,
    const type_system::TypeRef::Set&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(TypeRef::Set)");
}

// Map
inline Map fromRecord(
    const type_system::SerializableRecord&,
    const type_system::TypeRef::Map&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(TypeRef::Map)");
}

// Union
inline Union fromRecord(
    const type_system::SerializableRecord&,
    const type_system::UnionNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(UnionNode)");
}

// OpaqueAliasNode
inline int fromRecord(
    const type_system::SerializableRecord&,
    const type_system::OpaqueAliasNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: fromRecord(OpaqueAliasNode)");
}

// Helper overload for not_null pointers
template <typename T>
auto fromRecord(
    const type_system::SerializableRecord& r,
    folly::not_null<const T*> t,
    std::pmr::memory_resource* mr = nullptr) {
  return fromRecord(r, *t, mr);
}

detail::Datum fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef& type,
    std::pmr::memory_resource* mr);

} // namespace apache::thrift::dynamic
