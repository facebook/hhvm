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

#include <thrift/lib/cpp2/dynamic/detail/Traits.h>
#include <thrift/lib/thrift/gen-cpp2/id_types.h>
#include <thrift/lib/thrift/gen-cpp2/record_types.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

#include <folly/Overload.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/container/Reserve.h>
#include <folly/lang/Assume.h>
#include <folly/lang/Exception.h>

#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::type_system {

using FieldId = type::FieldId;
class SerializableRecord;

namespace detail {

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
      typename U,
      std::enable_if_t<
          !std::is_same_v<U, PrimitiveDatum> && std::is_convertible_v<U, T>>* =
          nullptr>
  friend bool operator==(const U& lhs, const PrimitiveDatum& rhs) {
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
      typename U,
      std::enable_if_t<
          !std::is_same_v<U, PrimitiveDatum> && std::is_convertible_v<U, T>>* =
          nullptr>
  friend bool operator!=(const U& lhs, const PrimitiveDatum& rhs) {
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
using SerializableRecordMap = folly::F14VectorMap<
    SerializableRecord,
    SerializableRecord,
    SerializableRecordHasher>;

using SerializableRecordKinds = folly::tag_t<
    SerializableRecordBool,
    SerializableRecordInt8,
    SerializableRecordInt16,
    SerializableRecordInt32,
    SerializableRecordInt64,
    SerializableRecordFloat32,
    SerializableRecordFloat64,
    SerializableRecordText,
    SerializableRecordByteArray,
    SerializableRecordFieldSet,
    SerializableRecordList,
    SerializableRecordSet,
    SerializableRecordMap>;

/**
 * Determines if T is one of the union alternatives of SerializableRecord.
 */
template <typename T>
constexpr bool isSerializableRecordKind =
    folly::type_list_find_v<T, SerializableRecordKinds> !=
    folly::type_list_size_v<SerializableRecordKinds>;

bool areByteArraysEqual(const type::ByteBuffer&, const type::ByteBuffer&);

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

} // namespace detail

/**
 * A concrete, machine-readable, description of every datum that is possible to
 * represent in Thrift.
 *
 * A record captures the structure of datums without association to a Thrift
 * type. This structure derives from de-facto standard constructs in the
 * computing ecosystem. Most (if not all) useful programming languages and
 * computer hardware can efficiently support these structures.
 *
 * This class is an ergonomic API for SerializableRecordUnion that enforces the
 * invariants described in record.thrift but which are unenforceable in "pure"
 * Thrift schema. For example, uniqueness of values within a list.
 *
 * Conversion functions to and from the underlying serializable representation
 * are available as SerializableRecord::toThrift and
 * SerializableRecord::fromThrift respectively.
 */
class SerializableRecord final {
 public:
  // These aliases exist so that the user can write SerializableRecord::<type>
  // instead of reaching into the detail namespace.
  using Bool = detail::SerializableRecordBool;
  using Int8 = detail::SerializableRecordInt8;
  using Int16 = detail::SerializableRecordInt16;
  using Int32 = detail::SerializableRecordInt32;
  using Int64 = detail::SerializableRecordInt64;
  using Float32 = detail::SerializableRecordFloat32;
  using Float64 = detail::SerializableRecordFloat64;
  using Text = detail::SerializableRecordText;
  using ByteArray = detail::SerializableRecordByteArray;
  using FieldSet = detail::SerializableRecordFieldSet;
  using List = detail::SerializableRecordList;
  using Set = detail::SerializableRecordSet;
  using Map = detail::SerializableRecordMap;

 private:
  // These types are boxed because we cannot instantiate std::variant with
  // incomplete types.
  using FieldSetPtr = std::unique_ptr<FieldSet>;
  using ListPtr = std::unique_ptr<List>;
  using SetPtr = std::unique_ptr<Set>;
  using MapPtr = std::unique_ptr<Map>;

  using Alternative = std::variant<
      Bool,
      Int8,
      Int16,
      Int32,
      Int64,
      Float32,
      Float64,
      Text,
      ByteArray,
      FieldSetPtr,
      ListPtr,
      SetPtr,
      MapPtr>;
  Alternative datum_;

 public:
  enum class Kind {
    BOOL = detail::IndexOf<Alternative, Bool>,
    INT8 = detail::IndexOf<Alternative, Int8>,
    INT16 = detail::IndexOf<Alternative, Int16>,
    INT32 = detail::IndexOf<Alternative, Int32>,
    INT64 = detail::IndexOf<Alternative, Int64>,
    FLOAT32 = detail::IndexOf<Alternative, Float32>,
    FLOAT64 = detail::IndexOf<Alternative, Float64>,
    TEXT = detail::IndexOf<Alternative, Text>,
    BYTE_ARRAY = detail::IndexOf<Alternative, ByteArray>,
    FIELD_SET = detail::IndexOf<Alternative, FieldSetPtr>,
    LIST = detail::IndexOf<Alternative, ListPtr>,
    SET = detail::IndexOf<Alternative, SetPtr>,
    MAP = detail::IndexOf<Alternative, MapPtr>,
  };
  /**
   * Produces the current variant alternative.
   */
  Kind kind() const {
    if (datum_.valueless_by_exception()) {
      throwAccessEmpty();
    }
    return static_cast<Kind>(datum_.index());
  }

 public:
  /**
   * Unlike the underlying Thrift union, a SerializableRecord can never be
   * "empty", since there is no "nil" or "unit" type in Thrift.
   */
  SerializableRecord() = delete;
  ~SerializableRecord() noexcept;

  SerializableRecord(const SerializableRecord&);
  SerializableRecord& operator=(const SerializableRecord&);

  SerializableRecord(SerializableRecord&&) noexcept = default;
  SerializableRecord& operator=(SerializableRecord&&) noexcept = default;

  /* implicit */ SerializableRecord(Bool datum) noexcept : datum_(datum) {}
  /* implicit */ SerializableRecord(Int8 datum) noexcept : datum_(datum) {}
  /* implicit */ SerializableRecord(Int16 datum) noexcept : datum_(datum) {}
  /* implicit */ SerializableRecord(Int32 datum) noexcept : datum_(datum) {}
  /* implicit */ SerializableRecord(Int64 datum) noexcept : datum_(datum) {}
  /* implicit */ SerializableRecord(Float32 datum) : datum_(datum) {
    detail::ensureValidFloatOrThrow(float(datum));
  }
  /* implicit */ SerializableRecord(Float64 datum) : datum_(datum) {
    detail::ensureValidFloatOrThrow(double(datum));
  }
  /* implicit */ SerializableRecord(Text datum) : datum_(std::move(datum)) {
    detail::ensureUTF8OrThrow(asText());
  }
  /* implicit */ SerializableRecord(ByteArray datum) noexcept
      : datum_(std::move(datum)) {}

  /* implicit */ SerializableRecord(FieldSet&&) noexcept;
  /* implicit */ SerializableRecord(List&&) noexcept;
  /* implicit */ SerializableRecord(Set&&) noexcept;
  /* implicit */ SerializableRecord(Map&&) noexcept;

  /* implicit */ SerializableRecord(std::nullptr_t) = delete;

  /**
   * Arrays have the pesky behavior of implicitly converting to a pointer. This
   * explicit factory function ensures so such conversions are not possible.
   */
  template <std::size_t N>
  static Text text(const char (&str)[N]) {
    return text(std::string_view(str, N - 1));
  }
  static Text text(std::string_view str) { return Text(str); }

  /**
   * Creates a SerializableRecord from the raw Thrift union that represents its
   * schematized form.
   *
   * If the underlying data represents an invalid datum, then this throws
   * `std::invalid_argument`. This can happen, for example, if input is:
   *   - the empty union
   *   - has non-UTF-8 string
   *   - NaN floating point numbers
   *   - duplicate elements in a set
   *   - duplicate keys in a map
   */
  static SerializableRecord fromThrift(SerializableRecordUnion&&);
  /**
   * Converts a SerializableRecord to its underlying schematized form. Unlike
   * fromThrift(...), this should not fail.
   */
  static SerializableRecordUnion toThrift(const SerializableRecord&);

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
    if (auto* datum = std::get_if<Bool>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Int8 asInt8() const {
    if (auto* datum = std::get_if<Int8>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Int16 asInt16() const {
    if (auto* datum = std::get_if<Int16>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Int32 asInt32() const {
    if (auto* datum = std::get_if<Int32>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Int64 asInt64() const {
    if (auto* datum = std::get_if<Int64>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Float32 asFloat32() const {
    if (auto* datum = std::get_if<Float32>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Float64 asFloat64() const {
    if (auto* datum = std::get_if<Float64>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }

  const Text& asText() const& {
    if (auto* datum = std::get_if<Text>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  Text&& asText() && {
    if (auto* datum = std::get_if<Text>(&datum_)) {
      return std::move(*datum);
    }
    throwAccessInactiveKind();
  }

  const ByteArray& asByteArray() const& {
    if (auto* datum = std::get_if<ByteArray>(&datum_)) {
      return *datum;
    }
    throwAccessInactiveKind();
  }
  ByteArray&& asByteArray() && {
    if (auto* datum = std::get_if<ByteArray>(&datum_)) {
      return std::move(*datum);
    }
    throwAccessInactiveKind();
  }

  const FieldSet& asFieldSet() const& {
    if (auto* datum = std::get_if<FieldSetPtr>(&datum_)) {
      return **datum;
    }
    throwAccessInactiveKind();
  }
  FieldSet&& asFieldSet() && {
    if (auto* datum = std::get_if<FieldSetPtr>(&datum_)) {
      return std::move(**datum);
    }
    throwAccessInactiveKind();
  }

  const List& asList() const& {
    if (auto* datum = std::get_if<ListPtr>(&datum_)) {
      return **datum;
    }
    throwAccessInactiveKind();
  }
  List&& asList() && {
    if (auto* datum = std::get_if<ListPtr>(&datum_)) {
      return std::move(**datum);
    }
    throwAccessInactiveKind();
  }

  const Set& asSet() const& {
    if (auto* datum = std::get_if<SetPtr>(&datum_)) {
      return **datum;
    }
    throwAccessInactiveKind();
  }
  Set&& asSet() && {
    if (auto* datum = std::get_if<SetPtr>(&datum_)) {
      return std::move(**datum);
    }
    throwAccessInactiveKind();
  }

  const Map& asMap() const& {
    if (auto* datum = std::get_if<MapPtr>(&datum_)) {
      return **datum;
    }
    throwAccessInactiveKind();
  }
  Map&& asMap() && {
    if (auto* datum = std::get_if<MapPtr>(&datum_)) {
      return std::move(**datum);
    }
    throwAccessInactiveKind();
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
    }
    // The call to kind() above would have thrown.
    folly::assume_unreachable();
  }

  /**
   * `isType<V>` produces true if V is the currently active variant alternative.
   *
   * Preconditions:
   *   - kind() != Kind::EMPTY, otherwise throws `std::runtime_error`.
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
        [&](const auto&) -> const V& { throwAccessInactiveKind(); });
  }

  friend bool operator==(
      const SerializableRecord& lhs, const SerializableRecord& rhs);
  friend bool operator==(const SerializableRecord& lhs, Bool rhs) {
    return lhs.isBool() && lhs.asBool() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Int8 rhs) {
    return lhs.isInt8() && lhs.asInt8() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Int16 rhs) {
    return lhs.isInt16() && lhs.asInt16() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Int32 rhs) {
    return lhs.isInt32() && lhs.asInt32() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Int64 rhs) {
    return lhs.isInt64() && lhs.asInt64() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Float32 rhs) {
    return lhs.isFloat32() && lhs.asFloat32() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, Float64 rhs) {
    return lhs.isFloat64() && lhs.asFloat64() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, const Text& rhs) {
    return lhs.isText() && lhs.asText() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, const ByteArray& rhs) {
    return lhs.isByteArray() &&
        detail::areByteArraysEqual(*lhs.asByteArray(), *rhs);
  }
  friend bool operator==(const SerializableRecord& lhs, const FieldSet& rhs) {
    return lhs.isFieldSet() && lhs.asFieldSet() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, const List& rhs) {
    return lhs.isList() && lhs.asList() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, const Set& rhs) {
    return lhs.isSet() && lhs.asSet() == rhs;
  }
  friend bool operator==(const SerializableRecord& lhs, const Map& rhs) {
    return lhs.isMap() && lhs.asMap() == rhs;
  }

  template <
      typename V,
      std::enable_if_t<detail::isSerializableRecordKind<V>>* = nullptr>
  friend bool operator==(const V& lhs, const SerializableRecord& rhs) {
    return rhs == lhs;
  }

  // In C++20, operator!= can be synthesized from operator==.
  template <
      typename V,
      std::enable_if_t<detail::isSerializableRecordKind<V>>* = nullptr>
  friend bool operator!=(const SerializableRecord& lhs, const V& rhs) {
    return !(lhs == rhs);
  }
  template <
      typename V,
      std::enable_if_t<detail::isSerializableRecordKind<V>>* = nullptr>
  friend bool operator!=(const V& lhs, const SerializableRecord& rhs) {
    return !(lhs == rhs);
  }

 private:
  [[noreturn]] void throwAccessInactiveKind() const;
  [[noreturn]] static void throwAccessEmpty();
};

/**
 * Produces a string representation of a SerializableRecord that is useful for
 * debugging ONLY.
 *
 * This string is not guaranteed to be stable, nor is it guaranteed to include
 * complete information.
 */
std::string toDebugString(const SerializableRecord&);
std::ostream& operator<<(std::ostream&, const SerializableRecord&);

namespace detail {
template <typename Tag>
struct EmbedInplace;

template <>
struct EmbedInplace<type::bool_t> {
  void operator()(
      type::native_type<type::bool_t>& value,
      const SerializableRecord& record) const {
    value = record.asBool();
  }
};

template <typename Tag>
  requires type::is_a_v<Tag, type::number_c>
struct EmbedInplace<Tag> {
  void operator()(
      type::native_type<Tag>& value, const SerializableRecord& record) const {
    if constexpr (type::is_a_v<Tag, type::byte_t>) {
      value = record.asInt8();
    } else if constexpr (type::is_a_v<Tag, type::i16_t>) {
      value = record.asInt16();
    } else if constexpr (type::is_a_v<Tag, type::i32_t>) {
      value = record.asInt32();
    } else if constexpr (type::is_a_v<Tag, type::i64_t>) {
      value = record.asInt64();
    } else if constexpr (type::is_a_v<Tag, type::float_t>) {
      value = record.asFloat32();
    } else if constexpr (type::is_a_v<Tag, type::double_t>) {
      value = record.asFloat64();
    }
  }
};

template <>
struct EmbedInplace<type::string_t> {
  void operator()(std::string& value, const SerializableRecord& record) const {
    value = record.asText();
  }
};

template <>
struct EmbedInplace<type::binary_t> {
  void operator()(std::string& value, const SerializableRecord& record) const {
    value = record.asByteArray()->toString();
  }

  void operator()(folly::IOBuf& value, const SerializableRecord& record) const {
    value = record.asByteArray()->cloneAsValue();
  }

  void operator()(
      std::unique_ptr<folly::IOBuf>& value,
      const SerializableRecord& record) const {
    value = record.asByteArray()->clone();
  }
};

template <typename T>
struct EmbedInplace<type::enum_t<T>> {
  void operator()(T& value, const SerializableRecord& record) const {
    value = static_cast<T>(record.asInt32().datum);
  }
};

template <typename Tag>
struct EmbedInplace<type::list<Tag>> {
  template <typename ListType>
  void operator()(ListType& value, const SerializableRecord& record) const {
    const auto& list = record.asList();
    value.clear();
    folly::reserve_if_available(value, list.size());
    for (const auto& element : list) {
      auto& datum = value.emplace_back();
      EmbedInplace<Tag>{}(datum, element);
    }
  }
};

template <typename Tag>
struct EmbedInplace<type::set<Tag>> {
  template <typename SetType>
  void operator()(SetType& value, const SerializableRecord& record) const {
    const auto& set = record.asSet();
    value.clear();
    folly::reserve_if_available(value, set.size());
    for (const auto& element : set) {
      type::native_type<Tag> datum;
      EmbedInplace<Tag>{}(datum, element);
      value.insert(std::move(datum));
    }
  }
};

template <typename KeyTag, typename ValueTag>
struct EmbedInplace<type::map<KeyTag, ValueTag>> {
  template <typename MapType>
  void operator()(MapType& value, const SerializableRecord& record) const {
    const auto& map = record.asMap();
    value.clear();
    folly::reserve_if_available(value, map.size());
    for (const auto& [k, v] : map) {
      type::native_type<KeyTag> keyDatum;
      EmbedInplace<KeyTag>{}(keyDatum, k);
      type::native_type<ValueTag> valueDatum;
      EmbedInplace<ValueTag>{}(valueDatum, v);
      value.emplace(std::move(keyDatum), std::move(valueDatum));
    }
  }
};

template <typename T, typename Tag>
struct EmbedInplace<type::cpp_type<T, Tag>> : EmbedInplace<Tag> {};

// TODO: Add Field adapter if needed.
template <typename Adapter, typename Tag>
struct EmbedInplace<type::adapted<Adapter, Tag>> {
  template <typename U>
  void operator()(U& m, const SerializableRecord& record) const {
    // TODO: Optimize in-place adapter
    type::native_type<Tag> orig;
    EmbedInplace<Tag>{}(orig, record);
    m = Adapter::fromThrift(std::move(orig));
  }
};

template <class T>
struct EmbedInplaceStructure {
  void operator()(T& s, const SerializableRecord& record) const {
    for (const auto& [id, value] : record.asFieldSet()) {
      op::invoke_by_field_id<T>(
          static_cast<FieldId>(id),
          [&, v = value]<typename Id>(Id) {
            using FieldTag = op::get_type_tag<T, Id>;
            using FieldType = op::get_native_type<T, Id>;
            FieldType t;
            EmbedInplace<FieldTag>{}(t, v);

            using Ref = op::get_field_ref<T, Id>;
            if constexpr (apache::thrift::detail::is_shared_or_unique_ptr_v<
                              Ref>) {
              op::get<Id>(s) =
                  std::make_unique<op::get_native_type<T, Id>>(std::move(t));
            } else {
              op::get<Id>(s) = std::move(t);
            }
          },
          [&] {
            // Missing field in T.
          });
    }
  }
};

template <typename T>
struct EmbedInplace<type::struct_t<T>> : EmbedInplaceStructure<T> {};
template <typename T>
struct EmbedInplace<type::union_t<T>> : EmbedInplaceStructure<T> {};
template <typename T>
struct EmbedInplace<type::exception_t<T>> : EmbedInplaceStructure<T> {};

} // namespace detail

/**
 * Embed SerializableRecord to a corresponding Thrift type provided by Tag. If
 * SerializableRecord is partial, then the missing fields will be filled with
 * default. If the record contains unknown fields, it drops the data. This can
 * happen if the record is produced from different schema version.
 *
 * Pre-conditions:
 *   - Provided record is compatible with a corresponding Thrift type provided
 *     by Tag, else throws `std::runtime_error`
 */
template <typename Tag>
auto embed(const SerializableRecord& record) {
  type::native_type<Tag> value;
  detail::EmbedInplace<Tag>{}(value, record);
  return value;
}

} // namespace apache::thrift::type_system

template <>
struct std::hash<apache::thrift::type_system::SerializableRecord>
    : apache::thrift::type_system::detail::SerializableRecordHasher {};

template <typename T>
struct std::hash<apache::thrift::type_system::detail::PrimitiveDatum<T>> {
  std::size_t operator()(
      const apache::thrift::type_system::detail::PrimitiveDatum<T>& primitive)
      const noexcept {
    return std::hash<T>{}(primitive.datum);
  }
};
