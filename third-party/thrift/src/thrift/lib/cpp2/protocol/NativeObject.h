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

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/type/Type.h>

/**
 * DISCLAIMER: This is an experimental library for Thrift. The API and
 * functionality are subject to change and may not be stable. Use at your
 * own risk and discretion.
 */

/**
 * This library provides a high-performance alternative to protocol::Value and
 * protocol::Object, utilizing std::variant to enable inline specializations
 * for container types. Unlike protocol::Value, containers in this library
 * do not contain protocol::Value elements. Instead, they can have a specialized
 * representation for their internal element types. For example, a list<i32>
 * would be specialized as std::vector<std::int32_t>, and a map<i16, String>
 * would be represented as folly::F14FastMap<std::int16_t, ValueHolder>.
 *
 * Despite these optimizations, the library can still represent arbitrary
 * Thrift hierarchies by falling back to a generic representation of `Value`
 * elements when necessary.
 *
 * Usage Examples:
 *
 * // Example of traversing and reading values from a NativeList
 * auto intList = make_list_of<std::int32_t>({1, 2, 3, 4, 5});
 * intList.visit([](const auto& list) {
 *   for (const auto& value : list) {
 *     std::cout << value << std::endl;
 *   }
 * });

 * // Example of traversing and reading values from a NativeObject
 * NativeObject obj;
 * obj.emplace(1, "hello");
 * obj.emplace(2, "example");
 * for (const auto& [fieldId, value] : obj) {
 *     std::cout << "Field ID: " << fieldId
 *      << ", Value: " << value.as<std::string>()
 *      << std::endl;
 * }
 */

namespace apache::thrift::protocol::experimental {
class NativeObject;
class NativeValue;
struct Bytes;
class ValueHolder;
class NativeList;
class NativeSet;
class NativeMap;

namespace detail {
size_t hash_value(const NativeValue& v);
size_t hash_value(const NativeObject& o);
size_t hash_value(const Bytes& s);
size_t hash_value(const ValueHolder& v);
} // namespace detail

} // namespace apache::thrift::protocol::experimental

// ---- std::hash specialization for container types ---- //

template <>
struct ::std::hash<apache::thrift::protocol::experimental::NativeValue> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::NativeValue& s)
      const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(s);
  }
};

template <>
struct ::std::hash<
    std::unique_ptr<apache::thrift::protocol::experimental::NativeValue>> {
  std::size_t operator()(const std::unique_ptr<
                         apache::thrift::protocol::experimental::NativeValue>&
                             s) const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(*s);
  }
};

template <>
struct std::hash<apache::thrift::protocol::experimental::NativeObject> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::NativeObject& s)
      const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(s);
  }
};

template <>
struct std::hash<apache::thrift::protocol::experimental::Bytes> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::Bytes& s) const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(s);
  }
};

template <>
struct std::hash<apache::thrift::protocol::experimental::ValueHolder> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::ValueHolder& s)
      const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(s);
  }
};

namespace apache::thrift::protocol::experimental {

// ---- Primitive types ---- //

struct Bytes {
  // TODO(sadroeck) - We could manually refcount & keep IOBuf on the stack
  using Buf = std::unique_ptr<folly::IOBuf>;
  Buf buf_;

  Bytes() noexcept = default;
  explicit Bytes(Buf buf) noexcept : buf_(std::move(buf)) {}
  explicit Bytes(const std::string& s) : buf_(folly::IOBuf::fromString(s)) {}
  explicit Bytes(std::string_view v)
      : buf_(std::make_unique<folly::IOBuf>(
            folly::IOBuf::COPY_BUFFER, v.data(), v.size())) {}

  Bytes(const Bytes& other) noexcept
      : buf_(std::make_unique<folly::IOBuf>(*other.buf_)) {}
  Bytes(Bytes&& other) noexcept = default;
  ~Bytes() noexcept = default;
  Bytes& operator=(const Bytes& other) noexcept {
    buf_ = std::make_unique<folly::IOBuf>(*other.buf_);
    return *this;
  }
  Bytes& operator=(Bytes&& other) noexcept = default;

  static Bytes fromIOBuf(const folly::IOBuf& buf) {
    return Bytes{std::make_unique<folly::IOBuf>(buf)};
  }
  static Bytes fromIOBuf(folly::IOBuf&& buf) {
    return Bytes{std::make_unique<folly::IOBuf>(std::move(buf))};
  }

  bool empty() const { return buf_->empty(); }
  const std::uint8_t* data() const { return buf_->data(); }
  std::size_t size() const { return buf_->length(); }
  folly::IOBuf& inner() { return *buf_; }
  const folly::IOBuf& inner() const { return *buf_; }

  std::string_view as_string_view() const {
    if (buf_->isChained()) {
      throw std::runtime_error("Non-contiguous buffer");
    }
    return std::string_view{reinterpret_cast<const char*>(data()), size()};
  }

  bool operator==(const Bytes& other) const {
    return folly::IOBufCompare{}(*buf_, *other.buf_) == folly::ordering::eq;
  }

  bool operator==(const folly::IOBuf& buf) const {
    return folly::IOBufCompare{}(*buf_, buf) == folly::ordering::eq;
  }

  bool operator==(const std::string& str) const {
    return folly::IOBufCompare{}(
               *buf_,
               folly::IOBuf{
                   folly::IOBuf::WRAP_BUFFER, str.data(), str.size()}) ==
        folly::ordering::eq;
  }
  bool operator==(const std::string_view& str) const {
    return folly::IOBufCompare{}(
               *buf_,
               folly::IOBuf{
                   folly::IOBuf::WRAP_BUFFER, str.data(), str.size()}) ==
        folly::ordering::eq;
  }
  bool operator==(const char* str) const {
    return folly::IOBufCompare{}(
               *buf_,
               folly::IOBuf{
                   folly::IOBuf::WRAP_BUFFER, str, std::strlen(str)}) ==
        folly::ordering::eq;
  }
};

struct PrimitiveTypes {
  using Bool = bool;
  using I8 = std::int8_t;
  using I16 = std::int16_t;
  using I32 = std::int32_t;
  using I64 = std::int64_t;
  using Float = float;
  using Double = double;
  using Bytes = Bytes;
};

// Enumeration of the possible types of a NativeValue
// Note: This is the equivalent of the `protocol::Value::getType()` method
enum class ValueType {
  Empty,
  Bool,
  I8,
  I16,
  I32,
  I64,
  Float,
  Double,
  String,
  Bytes,
  List,
  Set,
  Map,
  Struct
};

// Provides a mapping of the valid states of a non-empty NativeValue instance
// into their appropriate thrift type.
// Note: `ValueType::Empty` is not a valid state to (de-)serialize, so this will
// throw an exception
TType value_type_into_ttype(ValueType type);

namespace detail {

// ---- Value's type system --- //
// NOTE: This contains the base case only

template <typename T>
struct native_value_type {
  using type = T;
  using tag = ::apache::thrift::type::infer_tag<T>;
};

template <typename T>
using native_value_type_t = typename detail::native_value_type<T>::type;

template <typename T>
constexpr bool is_numeric_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T>::tag,
    ::apache::thrift::type::number_c>;

template <typename T>
constexpr bool is_primitive_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T>::tag,
    ::apache::thrift::type::primitive_c>;

template <typename T>
constexpr bool is_structured_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T>::tag,
    ::apache::thrift::type::struct_c>;

template <typename T>
constexpr bool is_container_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T>::tag,
    ::apache::thrift::type::container_c>;

template <typename T>
constexpr bool is_native_object_type_v =
    is_primitive_v<T> || is_container_v<T> || is_structured_v<T>;

} // namespace detail

// ---- Container types ---- //

template <typename... Ts>
using ListOf = std::vector<Ts...>;

template <typename... Ts>
using SetOf = folly::F14FastSet<Ts...>;

template <typename... Ts>
using MapOf = folly::F14FastMap<Ts...>;

// ---- ValueAccess API ---- //

#define FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(TYPE, NAME) \
  bool is_##NAME() const;                             \
  const TYPE& as_##NAME() const;                      \
  TYPE& as_##NAME();                                  \
  const TYPE* if_##NAME() const;                      \
  TYPE* if_##NAME();                                  \
  TYPE& ensure_##NAME();                              \
  template <typename... Args>                         \
  TYPE& emplace_##NAME(Args&&... args);

// The type T (CRTP) provides the implementation on how to access the underlying
// wrapper as a `Value`
template <typename T>
class ValueAccess {
 public:
  ValueType get_type() const;

  // Allow implicit coercion into Value
  operator const NativeValue&() const noexcept;
  operator NativeValue&() noexcept;

  template <typename Ty>
  bool is_type() const noexcept;

  template <typename Ty>
  const detail::native_value_type_t<Ty>& as_type() const;

  template <typename Ty>
  detail::native_value_type_t<Ty>& as_type();

  template <typename Ty>
  const detail::native_value_type_t<Ty>* if_type() const noexcept;

  template <typename Ty>
  detail::native_value_type_t<Ty>* if_type() noexcept;

  bool operator==(const NativeValue& other) const;
  bool operator!=(const NativeValue& other) const;

  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::Bool, bool)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::I8, byte)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::I16, i16)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::I32, i32)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::I64, i64)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::Float, float)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::Double, double)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(PrimitiveTypes::Bytes, binary)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeList, list)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeSet, set)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeMap, map)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeObject, object)

 protected:
  ValueAccess() = default;

 private:
  NativeValue& value() noexcept { return static_cast<T&>(*this).as_value(); }
  const NativeValue& value() const noexcept {
    return static_cast<const T&>(*this).as_value();
  }
};

// ---- Structured types ---- //

namespace detail {

// We don't know the exaxct size of what `Value` is going to be at compile time,
// however we can derive it from its properties. As `NativeObject` contains a
// F14FastMap of `field id` -> `Value`, we assume the static size of the map
// is equivalent for all `large` structures, so we use a large array here as a
// equivalent.
using QuasiValue = std::array<std::uint8_t, 512>;
using QuasiObject =
    folly::F14FastMap<std::int16_t, std::array<std::uint8_t, 512>>;

using QuasiList =
    std::variant<ListOf<PrimitiveTypes::Bool>, ListOf<QuasiObject>>;
using QuasiSet = std::variant<SetOf<bool>, SetOf<QuasiObject>>;
using QuasiMap = std::variant<
    MapOf<PrimitiveTypes::I16, QuasiObject>,
    MapOf<QuasiObject, QuasiObject>>;
using QuasiValueKind = std::variant<
    PrimitiveTypes::Bool,
    PrimitiveTypes::I8,
    PrimitiveTypes::I16,
    PrimitiveTypes::I32,
    PrimitiveTypes::I64,
    PrimitiveTypes::Float,
    PrimitiveTypes::Double,
    Bytes,
    QuasiList,
    QuasiSet,
    QuasiMap,
    QuasiObject>;
} // namespace detail

constexpr std::size_t SIZE_OF_VALUE = sizeof(detail::QuasiValueKind);
constexpr std::size_t ALIGN_OF_VALUE = 8;

// TODO(sadroeck) - Figure out a better name for this
class alignas(ALIGN_OF_VALUE) ValueHolder : public ValueAccess<ValueHolder> {
 public:
  ValueHolder() noexcept = default;

  /* implicit */ ValueHolder(const NativeValue&);
  ValueHolder(const ValueHolder&);

  /* implicit */ ValueHolder(NativeValue&&) noexcept;
  ValueHolder(ValueHolder&&) noexcept;

  ValueHolder& operator=(const ValueHolder&);
  ValueHolder& operator=(ValueHolder&&) noexcept;

  ~ValueHolder();

  NativeValue& as_value() noexcept;
  const NativeValue& as_value() const noexcept;

  bool operator==(const ValueHolder& other) const;
  bool operator!=(const ValueHolder& other) const;

 private:
  alignas(ALIGN_OF_VALUE) std::array<std::uint8_t, SIZE_OF_VALUE> data_;
};

template <typename... Ts>
using FieldMapOf = folly::F14FastMap<Ts...>;

class NativeObject {
 public:
  using FieldId = std::int16_t;
  using Fields = FieldMapOf<FieldId, ValueHolder>;

  NativeObject() noexcept = default;
  NativeObject(NativeObject&&) noexcept = default;
  ~NativeObject() = default;
  NativeObject& operator=(const NativeObject&) = default;
  NativeObject& operator=(NativeObject&&) noexcept = default;
  NativeObject(const NativeObject&) = default;

  bool operator==(const NativeObject& other) const = default;

  NativeValue& operator[](FieldId i);
  NativeValue& operator[](apache::thrift::FieldId i);
  NativeValue& at(FieldId i);
  NativeValue& at(apache::thrift::FieldId i);
  const NativeValue& at(FieldId i) const;
  const NativeValue& at(apache::thrift::FieldId i) const;
  bool contains(FieldId i) const;
  bool contains(apache::thrift::FieldId i) const;
  std::size_t erase(FieldId i);
  std::size_t erase(apache::thrift::FieldId i);
  NativeValue* if_contains(FieldId i);
  NativeValue* if_contains(apache::thrift::FieldId i);
  const NativeValue* if_contains(FieldId i) const;
  const NativeValue* if_contains(apache::thrift::FieldId i) const;
  template <typename... Args>
  NativeValue& emplace(FieldId id, Args... args);

  [[nodiscard]] Fields::iterator begin();
  [[nodiscard]] Fields::const_iterator begin() const;
  [[nodiscard]] Fields::iterator end();
  [[nodiscard]] Fields::const_iterator end() const;
  [[nodiscard]] size_t size() const { return fields.size(); }
  [[nodiscard]] bool empty() const { return fields.empty(); }

 private:
  Fields fields;
};

// ---- NativeList ---- //

namespace detail {

// Provides a mapping of native c++ element types into their appropriate
// ListOf<T> specialization, e.g.
// - list_t<std::int8_t> = ListOf<I8>
// - list_t<std::string> = ListOf<Bytes>
template <typename T>
using list_t = ListOf<std::conditional_t<
    is_primitive_v<T>,
    typename native_value_type<T>::type,
    std::conditional_t<is_structured_v<T>, NativeObject, ValueHolder>>>;

} // namespace detail

class NativeList {
 public:
  using Kind = std::variant<
      std::monostate,
      // Specialization for Primitive elements
      ListOf<PrimitiveTypes::Bool>, // TODO(sadroeck) - This can be more
                                    // efficient
      ListOf<PrimitiveTypes::I8>,
      ListOf<PrimitiveTypes::I16>,
      ListOf<PrimitiveTypes::I32>,
      ListOf<PrimitiveTypes::I64>,
      ListOf<PrimitiveTypes::Float>,
      ListOf<PrimitiveTypes::Double>,
      ListOf<PrimitiveTypes::Bytes>,
      ListOf<NativeObject>,
      // Fallback for list/map/set
      ListOf<ValueHolder>>;

  template <typename T>
  using Specialized = detail::list_t<std::remove_cv_t<typename T::value_type>>;

  const Kind& inner() const noexcept;
  std::size_t size() const noexcept;
  bool empty() const noexcept;

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) const {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  // Default ops
  NativeList() = default;
  ~NativeList() = default;
  NativeList(const NativeList& other);
  NativeList(NativeList&& other) noexcept = default;
  NativeList& operator=(const NativeList& other) = default;
  NativeList& operator=(NativeList&& other) noexcept = default;

  // Variant ctors
  template <typename T>
  /* implicit */ NativeList(ListOf<T>&& l);
  template <typename T>
  /* implicit */ NativeList(const ListOf<T>& l);

  bool operator==(const NativeList& other) const;
  bool operator!=(const NativeList& other) const;

  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<PrimitiveTypes::Bool>, list_of_bool)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<PrimitiveTypes::I8>, list_of_i8)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<PrimitiveTypes::I16>, list_of_i16)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<PrimitiveTypes::I32>, list_of_i32)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<PrimitiveTypes::I64>, list_of_i64)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(
      ListOf<PrimitiveTypes::Float>, list_of_float)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(
      ListOf<PrimitiveTypes::Double>, list_of_double)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(
      ListOf<PrimitiveTypes::Bytes>, list_of_bytes)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<NativeObject>, list_of_object)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(ListOf<ValueHolder>, list_of_value)

  template <typename T>
  bool has_element() const noexcept;

  template <typename T>
  bool is_type() const noexcept;

  template <typename T>
  const Specialized<T>& as_type() const;

  template <typename T>
  Specialized<T>& as_type();

  template <typename T>
  const Specialized<T>* if_type() const noexcept;

  template <typename T>
  Specialized<T>* if_type() noexcept;

 private:
  Kind kind_;
};

namespace detail {

template <typename T>
using set_t = std::conditional_t<
    is_primitive_v<T> || is_structured_v<T>,
    SetOf<typename native_value_type<T>::type>,
    SetOf<ValueHolder>>;

}

class NativeSet {
 public:
  using Kind = std::variant<
      std::monostate,
      // Specialization for Primitive elements
      SetOf<PrimitiveTypes::Bool>,
      SetOf<PrimitiveTypes::I8>,
      SetOf<PrimitiveTypes::I16>,
      SetOf<PrimitiveTypes::I32>,
      SetOf<PrimitiveTypes::I64>,
      SetOf<PrimitiveTypes::Float>,
      SetOf<PrimitiveTypes::Double>,
      SetOf<PrimitiveTypes::Bytes>,
      SetOf<NativeObject>,
      // Fallback for list/map//set
      SetOf<ValueHolder>>;

  template <typename T>
  using Specialized = detail::set_t<std::remove_cv_t<typename T::value_type>>;

  // Default ops
  NativeSet() = default;
  ~NativeSet() = default;
  NativeSet(const NativeSet& other);
  NativeSet(NativeSet&& other) noexcept = default;
  NativeSet& operator=(const NativeSet& other) = default;
  NativeSet& operator=(NativeSet&& other) noexcept = default;

  // Variant ctors
  /* implicit */ NativeSet(Kind&& kind);
  template <typename T>
  /* implicit */ NativeSet(SetOf<T>&& s);

  const Kind& inner() const;
  std::size_t size() const noexcept;
  bool empty() const noexcept;
  bool contains(const NativeValue& value) const noexcept;

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) const {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  bool operator==(const NativeSet& other) const;
  bool operator!=(const NativeSet& other) const;

  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::Bool>, set_of_bool)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::I8>, set_of_i8)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::I16>, set_of_i16)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::I32>, set_of_i32)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::I64>, set_of_i64)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::Float>, set_of_float)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(
      SetOf<PrimitiveTypes::Double>, set_of_double)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<PrimitiveTypes::Bytes>, set_of_bytes)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<NativeObject>, set_of_object)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(SetOf<ValueHolder>, set_of_value)

  template <typename T>
  bool has_element() const noexcept;

  template <typename T>
  bool is_type() const noexcept;

  template <typename T>
  const Specialized<T>& as_type() const;

  template <typename T>
  Specialized<T>& as_type();

  template <typename T>
  const Specialized<T>* if_type() const noexcept;

  template <typename T>
  Specialized<T>* if_type() noexcept;

 private:
  Kind kind_;
};

// ---- NativeMap ---- //

namespace detail {

template <typename K, typename V>
using map_with_primitive_key_t =
    MapOf<typename native_value_type<K>::type, ValueHolder>;

template <typename K, typename V>
using map_t = std::conditional_t<
    is_primitive_v<K>,
    map_with_primitive_key_t<K, V>,
    MapOf<ValueHolder, ValueHolder>>;

template <typename K, typename V>
bool native_map_emplace(NativeMap& map, K&& key, V&& val);

bool native_map_emplace(NativeMap& map, NativeValue&& key, NativeValue&& val);

template <typename K, typename V>
bool native_map_insert_or_assign(NativeMap& map, K&& key, V&& val);

bool native_map_insert_or_assign(
    NativeMap& map, NativeValue&& key, NativeValue&& val);

} // namespace detail

class NativeMap {
 public:
  using Kind = std::variant<
      std::monostate,

      // Specialization of primitive -> Value maps
      MapOf<PrimitiveTypes::Bool, ValueHolder>,
      MapOf<PrimitiveTypes::I8, ValueHolder>,
      MapOf<PrimitiveTypes::I16, ValueHolder>,
      MapOf<PrimitiveTypes::I32, ValueHolder>,
      MapOf<PrimitiveTypes::I64, ValueHolder>,
      MapOf<PrimitiveTypes::Float, ValueHolder>,
      MapOf<PrimitiveTypes::Double, ValueHolder>,
      MapOf<PrimitiveTypes::Bytes, ValueHolder>,

      // Unspecialized as fallback
      MapOf<ValueHolder, ValueHolder>>;

  template <typename T>
  using Specialized = detail::map_t<
      std::remove_cv_t<typename T::key_type>,
      std::remove_cv_t<typename T::mapped_type>>;

  using Generic = MapOf<ValueHolder, ValueHolder>;

  const Kind& inner() const;
  std::size_t size() const noexcept;
  bool empty() const noexcept;

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) const {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  // Returns whether the value was inserted
  template <typename K, typename V>
  bool emplace(K&& key, V&& val) {
    return detail::native_map_emplace(
        *this, std::forward<K>(key), std::forward<V>(val));
  }

  // Returns whether the value was inserted
  template <typename K, typename V>
  bool insert_or_assign(K&& key, V&& val) {
    return detail::native_map_insert_or_assign(
        *this, std::forward<K>(key), std::forward<V>(val));
  }

  bool contains(const NativeValue& key) const noexcept;

  // Default ops
  NativeMap() = default;
  ~NativeMap() = default;
  NativeMap(const NativeMap& other);
  NativeMap(NativeMap&& other) noexcept = default;
  NativeMap& operator=(const NativeMap& other) = default;
  NativeMap& operator=(NativeMap&& other) noexcept = default;

  // Variant ctors
  /* implicit */ NativeMap(const Kind& kind);
  /* implicit */ NativeMap(Kind&& kind) noexcept;
  template <typename... Args>
  /* implicit */ NativeMap(MapOf<Args...>&& m) noexcept;

  bool operator==(const NativeMap& other) const;
  bool operator!=(const NativeMap& other) const;

  template <typename T>
  bool is_type() const noexcept;

  template <typename T>
  const Specialized<T>& as_type() const;

  template <typename T>
  Specialized<T>& as_type();

  template <typename T>
  const Specialized<T>* if_type() const noexcept;

  template <typename T>
  Specialized<T>* if_type() noexcept;

 private:
  Kind kind_;
};

namespace detail {

// ----- Type traits for Value's specialization ----- //

// This resolves to the variant a thrift type `T` will be stored in an
// `Value::Kind`. e.g.
// - numeric types will be specialized as their native c++ types
// - strings will be specialized as `Bytes`
// - lists will be specialized as `NativeList`
// - sets will be specialized as `NativeSet`
// - maps will be specialized as `NativeMap`
// - structs will be specialized as `Object`

template <>
struct native_value_type<std::monostate> {
  using type = std::monostate;
  using tag = ::apache::thrift::type::void_t;
};

template <>
struct native_value_type<PrimitiveTypes::Bool> {
  using type = PrimitiveTypes::Bool;
  using tag = ::apache::thrift::type::bool_t;
};

template <>
struct native_value_type<PrimitiveTypes::I8> {
  using type = PrimitiveTypes::I8;
  using tag = ::apache::thrift::type::byte_t;
};

template <>
struct native_value_type<PrimitiveTypes::I16> {
  using type = PrimitiveTypes::I16;
  using tag = ::apache::thrift::type::i16_t;
};

template <>
struct native_value_type<PrimitiveTypes::I32> {
  using type = PrimitiveTypes::I32;
  using tag = ::apache::thrift::type::i32_t;
};

template <>
struct native_value_type<PrimitiveTypes::I64> {
  using type = PrimitiveTypes::I64;
  using tag = ::apache::thrift::type::i64_t;
};

template <>
struct native_value_type<PrimitiveTypes::Float> {
  using type = PrimitiveTypes::Float;
  using tag = ::apache::thrift::type::float_t;
};

template <>
struct native_value_type<PrimitiveTypes::Double> {
  using type = PrimitiveTypes::Double;
  using tag = ::apache::thrift::type::double_t;
};

template <>
struct native_value_type<PrimitiveTypes::Bytes> {
  using type = PrimitiveTypes::Bytes;
  using tag = ::apache::thrift::type::binary_t;
};

template <>
struct native_value_type<std::string> {
  using type = PrimitiveTypes::Bytes;
  using tag = ::apache::thrift::type::string_t;
};

template <>
struct native_value_type<NativeObject> {
  using type = NativeObject;
  using tag = ::apache::thrift::type::struct_t<NativeObject>;
};

template <>
struct native_value_type<ValueHolder> {
  using type = ValueHolder;
  using tag = ::apache::thrift::type::struct_c;
};

template <>
struct native_value_type<NativeList> {
  using type = NativeList;
  using tag = ::apache::thrift::type::list<::apache::thrift::type::struct_c>;
};

template <typename... Ts>
struct native_value_type<ListOf<Ts...>> {
  using type = NativeList;
  using tag = ::apache::thrift::type::list<::apache::thrift::type::struct_c>;
};

template <>
struct native_value_type<NativeSet> {
  using type = NativeSet;
  using tag = ::apache::thrift::type::set<::apache::thrift::type::struct_c>;
};

template <typename... Ts>
struct native_value_type<std::set<Ts...>> {
  using type = NativeSet;
  using tag = ::apache::thrift::type::set<::apache::thrift::type::struct_c>;
};

template <typename... Ts>
struct native_value_type<SetOf<Ts...>> {
  using type = NativeSet;
  using tag = ::apache::thrift::type::set<::apache::thrift::type::struct_c>;
};

template <>
struct native_value_type<NativeMap> {
  using type = NativeMap;
  using tag = ::apache::thrift::type::
      map<::apache::thrift::type::struct_c, ::apache::thrift::type::struct_c>;
};

template <typename... Ts>
struct native_value_type<MapOf<Ts...>> {
  using type = NativeMap;
  using tag = ::apache::thrift::type::
      map<::apache::thrift::type::struct_c, ::apache::thrift::type::struct_c>;
};

template <typename... Ts>
struct native_value_type<std::map<Ts...>> {
  using type = NativeMap;
  using tag = ::apache::thrift::type::
      map<::apache::thrift::type::struct_c, ::apache::thrift::type::struct_c>;
};

// ---- Value mapping for e.g. NativeValue -> TType ---- //

template <typename T>
struct native_value_type_mapping;

#define FBTHRIFT_VALUE_TYPE_MAPPING(TYPE, NAME)         \
  template <>                                           \
  struct native_value_type_mapping<TYPE> {              \
    constexpr static ValueType value = ValueType::NAME; \
  };

FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::Bool, Bool)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::I8, I8)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::I16, I16)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::I32, I32)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::I64, I64)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::Float, Float)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::Double, Double)
FBTHRIFT_VALUE_TYPE_MAPPING(PrimitiveTypes::Bytes, Bytes)
FBTHRIFT_VALUE_TYPE_MAPPING(NativeList, List)
FBTHRIFT_VALUE_TYPE_MAPPING(NativeSet, Set)
FBTHRIFT_VALUE_TYPE_MAPPING(NativeMap, Map)
FBTHRIFT_VALUE_TYPE_MAPPING(NativeObject, Struct)

#undef FBTHRIFT_DEF_PRIMITIVE_TYPE_MAPPING

template <typename T>
constexpr ValueType native_value_type_mapping_v =
    native_value_type_mapping<T>::value;

} // namespace detail

// ---- Definition of a Thrift Value ---- //

class NativeValue : public ValueAccess<NativeValue> {
 public:
  using Kind = std::variant<
      std::monostate, // Allows default construction of NativeValue
      PrimitiveTypes::Bool,
      PrimitiveTypes::I8,
      PrimitiveTypes::I16,
      PrimitiveTypes::I32,
      PrimitiveTypes::I64,
      PrimitiveTypes::Float,
      PrimitiveTypes::Double,
      PrimitiveTypes::Bytes,
      NativeList,
      NativeSet,
      NativeMap,
      NativeObject>;

  const Kind& inner() const noexcept;
  Kind& inner() noexcept;
  bool is_empty() const noexcept;

  NativeValue& as_value() noexcept { return *this; }
  const NativeValue& as_value() const noexcept { return *this; }

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  template <typename... Fs>
  decltype(auto) visit(Fs&&... fs) const {
    return folly::variant_match(kind_, std::forward<Fs>(fs)...);
  }

  // Default ops
  NativeValue() noexcept;
  NativeValue(NativeValue&& obj) noexcept = default;
  NativeValue(const NativeValue&);
  NativeValue& operator=(const NativeValue&) noexcept = default;
  NativeValue& operator=(NativeValue&&) noexcept = default;
  ~NativeValue() noexcept = default;

  // Variant ctors
  /* implicit */ NativeValue(Kind&& kind) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::Bool&& b) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::I8&& i8) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::I16&& i16) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::I32&& i32) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::I64&& i64) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::Float&& f) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::Double&& d) noexcept;
  /* implicit */ NativeValue(PrimitiveTypes::Bytes&& b) noexcept;
  /* implicit */ NativeValue(NativeList&& list) noexcept;
  /* implicit */ NativeValue(NativeSet&& set) noexcept;
  /* implicit */ NativeValue(NativeMap&& map) noexcept;
  /* implicit */ NativeValue(NativeObject&& strct) noexcept;

  /* implicit */ NativeValue(const Kind& kind);
  /* implicit */ NativeValue(const PrimitiveTypes::Bool& b);
  /* implicit */ NativeValue(const PrimitiveTypes::I8& i8);
  /* implicit */ NativeValue(const PrimitiveTypes::I16& i16);
  /* implicit */ NativeValue(const PrimitiveTypes::I32& i32);
  /* implicit */ NativeValue(const PrimitiveTypes::I64& i64);
  /* implicit */ NativeValue(const PrimitiveTypes::Float& f);
  /* implicit */ NativeValue(const PrimitiveTypes::Double& d);
  /* implicit */ NativeValue(const PrimitiveTypes::Bytes& b);
  /* implicit */ NativeValue(const NativeList& list);
  /* implicit */ NativeValue(const NativeSet& set);
  /* implicit */ NativeValue(const NativeMap& map);
  /* implicit */ NativeValue(const NativeObject& strct);

 private:
  Kind kind_;
}; // namespace apache::thrift::protocol::experimental

static_assert(
    sizeof(NativeValue) == SIZE_OF_VALUE,
    "The size of Value must match the size of ValueHolder");
static_assert(
    alignof(NativeValue) == alignof(ValueHolder),
    "The alignment of Value must match the alignment of ValueHolder");

#undef FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD

namespace detail {

// ------- Type traits to verify the ListOf type system ------- //

template <typename T>
constexpr bool is_list_v = ::apache::thrift::type::
    is_a_v<typename native_value_type<T>::tag, ::apache::thrift::type::list_c>;

// ------- Type traits to verify the SetOf type system ------- //

template <typename T>
constexpr bool is_set_v = ::apache::thrift::type::
    is_a_v<typename native_value_type<T>::tag, ::apache::thrift::type::set_c>;

// ------- Type traits to verify the MapOf type system ------- //

template <typename T>
constexpr bool is_map_v = ::apache::thrift::type::
    is_a_v<typename native_value_type<T>::tag, ::apache::thrift::type::map_c>;

// ---- Parsing functions ---- //

NativeObject parseObjectVia(
    ::apache::thrift::BinaryProtocolReader& prot, bool string_to_binary);
NativeObject parseObjectVia(
    ::apache::thrift::CompactProtocolReader& prot, bool string_to_binary);
NativeValue parseValueVia(
    ::apache::thrift::BinaryProtocolReader& prot, protocol::TType ttype);
NativeValue parseValueVia(
    ::apache::thrift::CompactProtocolReader& prot, protocol::TType ttype);
std::uint32_t serializeObjectVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const NativeObject& obj);
std::uint32_t serializeObjectVia(
    ::apache::thrift::CompactProtocolWriter& prot, const NativeObject& obj);
std::uint32_t serializeValueVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const NativeValue& value);
std::uint32_t serializeValueVia(
    ::apache::thrift::CompactProtocolWriter& prot, const NativeValue& value);

} // namespace detail

template <class Protocol>
NativeObject parseObject(Protocol& prot, bool string_to_binary = true) {
  return detail::parseObjectVia(prot, string_to_binary);
}

template <class Protocol>
NativeObject parseObject(folly::IOBuf& buf, bool string_to_binary = true) {
  Protocol prot{};
  prot.setInput(&buf);
  return parseObject<Protocol>(prot, string_to_binary);
}

template <class Protocol, typename Tag>
NativeValue parseValue(const folly::IOBuf& buf) {
  Protocol prot;
  prot.setInput(&buf);
  return detail::parseValueVia(
      prot, type::toTType(type::detail::getBaseType(Tag{})));
}

template <class Protocol, typename Tag>
NativeValue parseValue(Protocol& prot) {
  return detail::parseValueVia(
      prot, type::toTType(type::detail::getBaseType(Tag{})));
}

template <class Protocol>
std::uint32_t serializeObject(Protocol& prot, const NativeObject& obj) {
  return detail::serializeObjectVia(prot, obj);
}

template <class Protocol>
std::uint32_t serializeValue(Protocol& prot, const NativeValue& val) {
  return detail::serializeValueVia(prot, val);
}

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeValue(const NativeValue& val) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  Protocol prot;
  prot.setOutput(&queue);
  serializeValue(prot, val);
  return queue.move();
}

template <class Protocol>
void serializeObject(const NativeObject& val, folly::IOBufQueue& queue) {
  Protocol prot{};
  prot.setOutput(&queue);
  serializeObject(prot, val);
}

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(const NativeObject& val) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  serializeObject<Protocol>(val, queue);
  return queue.move();
}

template <typename TT, typename Tag = void>
struct ValueHelper;

template <typename TT, typename T = type::native_type<TT>>
NativeValue asValueStruct(T&& value) {
  return ValueHelper<TT>::into(std::forward<T>(value));
}

// ---- Container helpers ---- //

// Creates a specialized list type with a single element derived
// from the provided type T, e.g.
// make_list_of<int>(...) => ListOf<I32>
// make_list_of<std::string>(...) => ListOf<Bytes>
// make_list_of<NativeSet>(...) => ListOf<ValueHolder>
template <typename T>
NativeList make_list_of(T&& t) {
  using V = std::remove_cvref_t<T>;
  if constexpr (std::is_same_v<V, NativeValue>) {
    return t.visit(
        [](std::monostate&) { return NativeList{}; },
        [](const std::monostate&) { return NativeList{}; },
        [](auto&& v) { return make_list_of(std::forward<decltype(v)>(v)); });
  } else {
    using ListTy = detail::list_t<V>;
    using ListElemTy = typename ListTy::value_type;

    if constexpr (std::is_same_v<ListElemTy, ValueHolder>) {
      static_assert(detail::is_container_v<V>);
      return NativeList{ListOf<ValueHolder>{NativeValue{std::forward<T>(t)}}};
    } else if constexpr (detail::is_primitive_v<ListElemTy>) {
      if constexpr (
          std::is_same_v<V, std::string> ||
          std::is_same_v<V, std::string_view>) {
        return NativeList{ListTy{Bytes{std::forward<T>(t)}}};
      } else {
        return NativeList{ListTy{std::forward<T>(t)}};
      }
    } else {
      throw std::runtime_error(fmt::format(
          "Unsupported specialization for make_list_of<{}>",
          folly::pretty_name<V>()));
    }
  }
}

template <typename K, typename V>
NativeMap make_map_of(K&& key, V&& val) {
  if constexpr (
      std::is_same_v<K, NativeValue> && std::is_same_v<V, NativeValue>) {
    // Introspect the generic values to determine the correct specialization
    if (key.is_empty() && val.is_empty()) {
      return NativeMap{};
    }

    return key.visit(
        [](std::monostate&) -> NativeMap {
          throw std::runtime_error("Cannot create a map with null keys");
        },
        [&](auto&& k) -> NativeMap {
          using KeyTy = std::remove_cvref_t<decltype(k)>;
          if (val.is_empty()) {
            throw std::runtime_error("Cannot create a map with null values");
          }

          return val.visit(
              [](std::monostate&) -> NativeMap {

              },
              [&](auto&& v) -> NativeMap {
                using ValTy = std::remove_cvref_t<decltype(v)>;
                return make_map_of<KeyTy, ValTy>(
                    std::forward<KeyTy>(k), std::forward<ValTy>(v));
              });
        });
  } else {
    using KeyTy = std::remove_cvref_t<K>;
    using ValTy = std::remove_cvref_t<V>;
    using MapTy = detail::map_t<KeyTy, ValTy>;

    MapTy map{};
    map.emplace(std::forward<K>(key), std::forward<V>(val));
    return map;
  }
}

} // namespace apache::thrift::protocol::experimental

#include <thrift/lib/cpp2/protocol/NativeObject-inl.h>
