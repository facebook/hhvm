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

#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/id_types.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

#include <folly/Overload.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/lang/Exception.h>

#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// This file is intended to be used to adapt types in record.thrift.
// Do not include this file directly! Use Record.h instead.
namespace apache::thrift::dynamic {

using FieldId = type::FieldId;

// From record.thrift
class SerializableRecordUnion;

namespace detail {

bool areByteArraysEqual(const type::ByteBuffer&, const type::ByteBuffer&);

[[noreturn]] void throwSerializableRecordAccessInactiveKind(
    std::string_view actualKind);

/**
 * If the input string is not valid UTF-8, then throws `std::invalid_argument`.
 */
void ensureUTF8OrThrow(std::string_view);

template <typename T, std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
void ensureValidFloatOrThrow(T datum) {
  if (std::isnan(datum)) {
    folly::throw_exception<std::invalid_argument>(
        "NaN is not a valid Thrift datum");
  }
  if (datum == T(0) && std::signbit(datum)) {
    folly::throw_exception<std::invalid_argument>(
        "-0.0 is not a valid Thrift datum");
  }
}

/**
 * The adapted type for SerializableRecord — a concrete, machine-readable,
 * description of every datum that is possible to represent in Thrift.
 *
 * A record captures the structure of datums without association to a Thrift
 * type. This structure derives from de-facto standard constructs in the
 * computing ecosystem. Most (if not all) useful programming languages and
 * computer hardware can efficiently support these structures.
 */
template <typename>
class SerializableRecordWrapper;
using SerializableRecord = SerializableRecordWrapper<SerializableRecordUnion>;

template <typename T = SerializableRecord>
class SerializableRecordAdapter : public InlineAdapter<T> {
  // SerializableRecord is incomplete at this point. We only use templates
  // to delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, SerializableRecord>);

 public:
  template <typename U>
  static T fromThrift(U&& value) {
    static_assert(std::is_rvalue_reference_v<U&&>);
    // NOTE: The constructors below perform validation on the input data. It may
    // throw exceptions.
    switch (value.getType()) {
      case U::Type::boolDatum:
        return T(typename T::Bool(*value.boolDatum_ref()));
      case U::Type::int8Datum:
        return T(typename T::Int8(*value.int8Datum_ref()));
      case U::Type::int16Datum:
        return T(typename T::Int16(*value.int16Datum_ref()));
      case U::Type::int32Datum:
        return T(typename T::Int32(*value.int32Datum_ref()));
      case U::Type::int64Datum:
        return T(typename T::Int64(*value.int64Datum_ref()));
      case U::Type::float32Datum:
        return T(typename T::Float32(*value.float32Datum_ref()));
      case U::Type::float64Datum:
        return T(typename T::Float64(*value.float64Datum_ref()));
      case U::Type::textDatum:
        return T(typename T::Text(std::move(*value.textDatum_ref())));
      case U::Type::byteArrayDatum:
        // union_field_ref::value() elides the unique_ptr, which we need here.
        return T(typename T::ByteArray(value.move_byteArrayDatum()));
      case U::Type::fieldSetDatum:
        return T(typename T::FieldSet(std::move(*value.fieldSetDatum_ref())));
      case U::Type::listDatum:
        return T(typename T::List(std::move(*value.listDatum_ref())));
      case U::Type::setDatum:
        return T(typename T::Set(std::move(*value.setDatum_ref())));
      case U::Type::mapDatum:
        return T(typename T::Map(std::move(*value.mapDatum_ref())));
      case U::Type::__EMPTY__:
        folly::throw_exception<std::invalid_argument>(
            "SerializableRecord cannot be empty");
      default:
        break;
    }
    folly::assume_unreachable();
  }
};

class SerializableRecordHasher {
 public:
  std::size_t operator()(const SerializableRecord&) const noexcept;
};

/**
 * A wrapper around primitive types (such as bool, std::int8_t etc.) to prevent
 * implicit conversions.
 *
 * Note that "primitive" here refers to C++ types, not Thrift types, albeit
 * there is a large overlap.
 */
template <typename T>
struct PrimitiveDatum {
  T datum;
  /* implicit */ operator T() const { return datum; }

  template <typename U = T, std::enable_if_t<std::is_same_v<U, T>>* = nullptr>
  /* implicit */ PrimitiveDatum(T datum) noexcept : datum(datum) {}

  template <
      typename U = T,
      std::enable_if_t<std::is_convertible_v<U, T>>* = nullptr>
  friend bool operator==(const PrimitiveDatum& lhs, const U& rhs) {
    return lhs.datum == rhs;
  }
  template <
      typename V,
      std::enable_if_t<!std::is_same_v<V, PrimitiveDatum>>* = nullptr>
  friend bool operator==(const V& lhs, const PrimitiveDatum& rhs) {
    return rhs == lhs;
  }

  // In C++20, operator!= can be synthesized from operator==.
  template <
      typename U = T,
      std::enable_if_t<std::is_convertible_v<U, T>>* = nullptr>
  friend bool operator!=(const PrimitiveDatum& lhs, const U& rhs) {
    return !(lhs == rhs);
  }
  template <
      typename V,
      std::enable_if_t<!std::is_same_v<V, PrimitiveDatum>>* = nullptr>
  friend bool operator!=(const V& lhs, const PrimitiveDatum& rhs) {
    return !(lhs == rhs);
  }
};

using SerializableRecordBool = PrimitiveDatum<bool>;
using SerializableRecordInt8 = PrimitiveDatum<std::int8_t>;
using SerializableRecordInt16 = PrimitiveDatum<std::int16_t>;
using SerializableRecordInt32 = PrimitiveDatum<std::int32_t>;
using SerializableRecordInt64 = PrimitiveDatum<std::int64_t>;
using SerializableRecordFloat32 = PrimitiveDatum<float>;
using SerializableRecordFloat64 = PrimitiveDatum<double>;
using SerializableRecordText = std::string;
using SerializableRecordByteArray = std::unique_ptr<type::ByteBuffer>;
using SerializableRecordFieldSet = std::map<FieldId, SerializableRecord>;
using SerializableRecordList = std::vector<SerializableRecord>;
using SerializableRecordSet =
    folly::F14FastSet<SerializableRecord, SerializableRecordHasher>;
using SerializableRecordMap = folly::F14FastMap<
    SerializableRecord,
    SerializableRecord,
    SerializableRecordHasher>;

/**
 * Determines if T is one of the union alternatives of SerializableRecord.
 */
template <typename T>
constexpr bool isSerializableRecordKind =
    std::is_same_v<T, SerializableRecordBool> ||
    std::is_same_v<T, SerializableRecordInt8> ||
    std::is_same_v<T, SerializableRecordInt16> ||
    std::is_same_v<T, SerializableRecordInt32> ||
    std::is_same_v<T, SerializableRecordInt64> ||
    std::is_same_v<T, SerializableRecordFloat32> ||
    std::is_same_v<T, SerializableRecordFloat64> ||
    std::is_same_v<T, SerializableRecordText> ||
    std::is_same_v<T, SerializableRecordByteArray> ||
    std::is_same_v<T, SerializableRecordFieldSet> ||
    std::is_same_v<T, SerializableRecordList> ||
    std::is_same_v<T, SerializableRecordSet> ||
    std::is_same_v<T, SerializableRecordMap>;

template <typename T>
class SerializableRecordWrapper final
    : public type::detail::EqWrap<SerializableRecordWrapper<T>, T> {
 private:
  // SerializableRecordUnion is incomplete at this point. We only use templates
  // to delay instantiation until the struct is defined.
  static_assert(std::is_same_v<T, SerializableRecordUnion>);
  using Base = type::detail::EqWrap<SerializableRecordWrapper<T>, T>;

 public:
  using Base::Base;
  // The underlying data should be validated by the adapter by using the other
  // constructors.
  /* implicit */ SerializableRecordWrapper(const SerializableRecordUnion&) =
      delete;
  /* implicit */ SerializableRecordWrapper(SerializableRecordUnion&&) = delete;

  // These aliases exist so that the user can write SerializableRecord::<type>
  // instead of reaching into the detail namesspace.
  using Bool = SerializableRecordBool;
  using Int8 = SerializableRecordInt8;
  using Int16 = SerializableRecordInt16;
  using Int32 = SerializableRecordInt32;
  using Int64 = SerializableRecordInt64;
  using Float32 = SerializableRecordFloat32;
  using Float64 = SerializableRecordFloat64;
  using Text = SerializableRecordText;
  using ByteArray = SerializableRecordByteArray;
  using FieldSet = SerializableRecordFieldSet;
  using List = SerializableRecordList;
  using Set = SerializableRecordSet;
  using Map = SerializableRecordMap;

  /* implicit */ SerializableRecordWrapper(Bool datum) noexcept {
    this->data_.boolDatum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Int8 datum) noexcept {
    this->data_.int8Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Int16 datum) noexcept {
    this->data_.int16Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Int32 datum) noexcept {
    this->data_.int32Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Int64 datum) noexcept {
    this->data_.int64Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Float32 datum) {
    detail::ensureValidFloatOrThrow(float(datum));
    this->data_.float32Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Float64 datum) {
    detail::ensureValidFloatOrThrow(double(datum));
    this->data_.float64Datum_ref() = datum;
  }
  /* implicit */ SerializableRecordWrapper(Text datum) {
    detail::ensureUTF8OrThrow(datum);
    this->data_.textDatum_ref() = std::move(datum);
  }
  /* implicit */ SerializableRecordWrapper(ByteArray datum) noexcept {
    // union_field_ref::operator=() elides the shared_ptr, which we need here.
    this->data_.set_byteArrayDatum(std::move(datum));
  }

  /* implicit */ SerializableRecordWrapper(FieldSet&& fieldSet) noexcept {
    this->data_.fieldSetDatum_ref() = std::move(fieldSet);
  }
  /* implicit */ SerializableRecordWrapper(List&& list) noexcept {
    this->data_.listDatum_ref() = std::move(list);
  }
  /* implicit */ SerializableRecordWrapper(Set&& set) noexcept {
    this->data_.setDatum_ref() = std::move(set);
  }
  /* implicit */ SerializableRecordWrapper(Map&& map) noexcept {
    this->data_.mapDatum_ref() = std::move(map);
  }

  /* implicit */ SerializableRecordWrapper(std::nullptr_t) = delete;
  /**
   * Arrays have the pesky behavior of implicitly converting to a pointer. This
   * explicit factory function ensures so such conversions are not possible.
   */
  template <std::size_t N>
  static Text text(const char (&str)[N]) {
    return text(std::string_view(str, N - 1));
  }
  static Text text(std::string_view str) { return Text(str); }

  enum class Kind {
    BOOL = folly::to_underlying(T::Type::boolDatum),
    INT8 = folly::to_underlying(T::Type::int8Datum),
    INT16 = folly::to_underlying(T::Type::int16Datum),
    INT32 = folly::to_underlying(T::Type::int32Datum),
    INT64 = folly::to_underlying(T::Type::int64Datum),
    FLOAT32 = folly::to_underlying(T::Type::float32Datum),
    FLOAT64 = folly::to_underlying(T::Type::float64Datum),
    TEXT = folly::to_underlying(T::Type::textDatum),
    BYTE_ARRAY = folly::to_underlying(T::Type::byteArrayDatum),
    FIELD_SET = folly::to_underlying(T::Type::fieldSetDatum),
    LIST = folly::to_underlying(T::Type::listDatum),
    SET = folly::to_underlying(T::Type::setDatum),
    MAP = folly::to_underlying(T::Type::mapDatum),
  };
  /**
   * Produces the current variant alternative.
   */
  Kind kind() const { return static_cast<Kind>(this->data_.getType()); }

  bool isBool() const { return kind() == Kind::BOOL; }
  bool isInt8() const { return kind() == Kind::INT8; }
  bool isInt16() const { return kind() == Kind::INT16; }
  bool isInt32() const { return kind() == Kind::INT32; }
  bool isInt64() const { return kind() == Kind::INT64; }
  bool isFloat32() const { return kind() == Kind::FLOAT32; }
  bool isFloat64() const { return kind() == Kind::FLOAT64; }
  bool isText() const { return kind() == Kind::TEXT; }
  bool isByteArray() const { return kind() == Kind::BYTE_ARRAY; }
  bool isFieldSet() const { return kind() == Kind::FIELD_SET; }
  bool isList() const { return kind() == Kind::LIST; }
  bool isSet() const { return kind() == Kind::SET; }
  bool isMap() const { return kind() == Kind::MAP; }

  Bool asBool() const {
    if (auto datum = this->data_.boolDatum_ref()) {
      return Bool(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Int8 asInt8() const {
    if (auto datum = this->data_.int8Datum_ref()) {
      return Int8(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Int16 asInt16() const {
    if (auto datum = this->data_.int16Datum_ref()) {
      return Int16(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Int32 asInt32() const {
    if (auto datum = this->data_.int32Datum_ref()) {
      return Int32(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Int64 asInt64() const {
    if (auto datum = this->data_.int64Datum_ref()) {
      return Int64(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Float32 asFloat32() const {
    if (auto datum = this->data_.float32Datum_ref()) {
      return Float32(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Float64 asFloat64() const {
    if (auto datum = this->data_.float64Datum_ref()) {
      return Float64(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const Text& asText() const& {
    if (auto datum = this->data_.textDatum_ref()) {
      return *datum;
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Text&& asText() && {
    if (auto datum = std::move(this->data_).textDatum_ref()) {
      return std::move(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const ByteArray& asByteArray() const& {
    // union_field_ref::value() elides the unique_ptr, which we need here.
    if (this->data_.getType() == T::Type::byteArrayDatum) {
      return this->data_.get_byteArrayDatum();
    }
    throwSerializableRecordAccessInactiveKind();
  }
  ByteArray&& asByteArray() && {
    // union_field_ref::value() elides the unique_ptr, which we need here.
    if (this->data_.getType() == T::Type::byteArrayDatum) {
      return std::move(this->data_.mutable_byteArrayDatum());
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const FieldSet& asFieldSet() const& {
    if (auto datum = this->data_.fieldSetDatum_ref()) {
      return *datum;
    }
    throwSerializableRecordAccessInactiveKind();
  }
  FieldSet&& asFieldSet() && {
    if (auto datum = std::move(this->data_).FieldSetDatum_ref()) {
      return std::move(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const List& asList() const& {
    if (auto datum = this->data_.listDatum_ref()) {
      return *datum;
    }
    throwSerializableRecordAccessInactiveKind();
  }
  List&& asList() && {
    if (auto datum = std::move(this->data_).listDatum_ref()) {
      return std::move(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const Set& asSet() const& {
    if (auto datum = this->data_.setDatum_ref()) {
      return *datum;
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Set&& asSet() && {
    if (auto datum = std::move(this->data_).setDatum_ref()) {
      return std::move(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  const Map& asMap() const& {
    if (auto datum = this->data_.mapDatum_ref()) {
      return *datum;
    }
    throwSerializableRecordAccessInactiveKind();
  }
  Map&& asMap() && {
    if (auto datum = std::move(this->data_).mapDatum_ref()) {
      return std::move(*datum);
    }
    throwSerializableRecordAccessInactiveKind();
  }

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying definition.
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const {
    auto overloaded = folly::overload(std::forward<F>(visitors)...);
    switch (kind()) {
      case Kind::BOOL:
        return overloaded(asBool());
      case Kind::INT8:
        return overloaded(asInt8());
      case Kind::INT16:
        return overloaded(asInt16());
      case Kind::INT32:
        return overloaded(asInt32());
      case Kind::INT64:
        return overloaded(asInt64());
      case Kind::FLOAT32:
        return overloaded(asFloat32());
      case Kind::FLOAT64:
        return overloaded(asFloat64());
      case Kind::TEXT:
        return overloaded(asText());
      case Kind::BYTE_ARRAY:
        return overloaded(asByteArray());
      case Kind::FIELD_SET:
        return overloaded(asFieldSet());
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
   *   - V is the active variant alternative, else throws `std::runtime_error`
   */
  template <typename V>
  const V& asType() const {
    return visit(
        [](const V& value) -> const V& { return value; },
        [&](const auto&) -> const V& {
          throwSerializableRecordAccessInactiveKind();
        });
  }

  friend bool operator==(const SerializableRecordWrapper& lhs, Bool rhs) {
    return lhs.isBool() && lhs.asBool() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Int8 rhs) {
    return lhs.isInt8() && lhs.asInt8() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Int16 rhs) {
    return lhs.isInt16() && lhs.asInt16() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Int32 rhs) {
    return lhs.isInt32() && lhs.asInt32() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Int64 rhs) {
    return lhs.isInt64() && lhs.asInt64() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Float32 rhs) {
    return lhs.isFloat32() && lhs.asFloat32() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, Float64 rhs) {
    return lhs.isFloat64() && lhs.asFloat64() == rhs;
  }
  friend bool operator==(
      const SerializableRecordWrapper& lhs, const Text& rhs) {
    return lhs.isText() && lhs.asText() == rhs;
  }
  friend bool operator==(
      const SerializableRecordWrapper& lhs, const ByteArray& rhs) {
    return lhs.isByteArray() &&
        detail::areByteArraysEqual(*lhs.asByteArray(), *rhs);
  }
  friend bool operator==(
      const SerializableRecordWrapper& lhs, const FieldSet& rhs) {
    return lhs.isFieldSet() && lhs.asFieldSet() == rhs;
  }
  friend bool operator==(
      const SerializableRecordWrapper& lhs, const List& rhs) {
    return lhs.isList() && lhs.asList() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, const Set& rhs) {
    return lhs.isSet() && lhs.asSet() == rhs;
  }
  friend bool operator==(const SerializableRecordWrapper& lhs, const Map& rhs) {
    return lhs.isMap() && lhs.asMap() == rhs;
  }

  template <
      typename V,
      std::enable_if_t<isSerializableRecordKind<std::remove_cvref_t<V>>>* =
          nullptr>
  friend bool operator==(const V& lhs, const SerializableRecordWrapper& rhs) {
    return rhs == lhs;
  }

  // In C++20, operator!= can be synthesized from operator==.
  template <
      typename V,
      std::enable_if_t<isSerializableRecordKind<std::remove_cvref_t<V>>>* =
          nullptr>
  friend bool operator!=(const SerializableRecordWrapper& lhs, const V& rhs) {
    return !(lhs == rhs);
  }
  template <
      typename V,
      std::enable_if_t<isSerializableRecordKind<std::remove_cvref_t<V>>>* =
          nullptr>
  friend bool operator!=(const V& lhs, const SerializableRecordWrapper& rhs) {
    return !(lhs == rhs);
  }

 private:
  [[noreturn]] void throwSerializableRecordAccessInactiveKind() const {
    detail::throwSerializableRecordAccessInactiveKind(
        util::enumNameSafe(this->data_.getType()));
  }
};

} // namespace detail

} // namespace apache::thrift::dynamic

template <>
struct std::hash<apache::thrift::dynamic::detail::SerializableRecord>
    : apache::thrift::dynamic::detail::SerializableRecordHasher {};

template <typename T>
struct std::hash<apache::thrift::dynamic::detail::PrimitiveDatum<T>> {
  std::size_t operator()(
      const apache::thrift::dynamic::detail::PrimitiveDatum<T>& primitive)
      const noexcept {
    return std::hash<T>{}(primitive.datum);
  }
};
