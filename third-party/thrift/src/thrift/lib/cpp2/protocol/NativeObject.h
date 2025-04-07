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
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/type/Type.h>

namespace apache::thrift::protocol::experimental {
class Object;
class Value;
struct Bytes;
class ValueHolder;
class NativeList;
class NativeSet;
class NativeMap;

namespace detail {
size_t hash_value(const Value& v);
size_t hash_value(const Object& o);
size_t hash_value(const Bytes& s);
size_t hash_value(const ValueHolder& v);
} // namespace detail

} // namespace apache::thrift::protocol::experimental

// ---- std::hash specialization for container types ---- //

template <>
struct ::std::hash<apache::thrift::protocol::experimental::Value> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::Value& s) const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(s);
  }
};

template <>
struct ::std::hash<
    std::unique_ptr<apache::thrift::protocol::experimental::Value>> {
  std::size_t operator()(
      const std::unique_ptr<apache::thrift::protocol::experimental::Value>& s)
      const noexcept {
    return apache::thrift::protocol::experimental::detail::hash_value(*s);
  }
};

template <>
struct std::hash<apache::thrift::protocol::experimental::Object> {
  std::size_t operator()(
      const apache::thrift::protocol::experimental::Object& s) const noexcept {
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

using Bool = bool;
using I8 = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;
using Float = float;
using Double = double;
using String = std::string;

struct Bytes {
  // TODO(sadroeck) - We could manually refcount & keep IOBuf on the stack
  using Buf = std::unique_ptr<folly::IOBuf>;
  Buf buf_;

  Bytes() noexcept = default;
  ~Bytes() noexcept = default;
  explicit Bytes(Buf buf) noexcept : buf_(std::move(buf)) {}
  Bytes(const Bytes& other) noexcept
      : buf_(std::make_unique<folly::IOBuf>(*other.buf_)) {}
  Bytes(Bytes&& other) noexcept = default;
  Bytes& operator=(const Bytes& other) noexcept {
    buf_ = std::make_unique<folly::IOBuf>(*other.buf_);
    return *this;
  }
  Bytes& operator=(Bytes&& other) noexcept = default;

  static Bytes fromStdString(const std::string& v) {
    return Bytes{folly::IOBuf::fromString(v)};
  }

  const std::uint8_t* data() const { return buf_->data(); }
  std::size_t size() const { return buf_->length(); }

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
};

namespace detail {

// ---- Value's type system --- //
// NOTE: This contains the base case only

template <typename T, bool StringToBinary>
struct native_value_type;

template <typename T, bool StringToBinary = true>
using native_value_type_t =
    typename detail::native_value_type<T, StringToBinary>::type;

} // namespace detail

// ---- ValueAccess API ---- //

#define FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(TYPE, NAME) \
  bool is_##NAME() const;                             \
  const TYPE& as_##NAME() const;                      \
  TYPE& as_##NAME();                                  \
  const TYPE* if_##NAME() const;                      \
  TYPE* if_##NAME();                                  \
  decltype(auto) ensure_##NAME();                     \
  template <typename... Args>                         \
  decltype(auto) emplace_##NAME(Args&&... args);

// The type T (CRTP) provides the implementation on how to access the underlying
// wrapper as a `Value`
template <typename T>
class ValueAccess {
 public:
  TType get_ttype() const;

  // Allow implicit coercion into Value
  operator const Value&() const noexcept;
  operator Value&() noexcept;

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

  bool operator==(const Value& other) const;
  bool operator!=(const Value& other) const;

  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(Bool, bool)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(I8, byte)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(I16, i16)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(I32, i32)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(I64, i64)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(Float, float)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(Double, double)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(Bytes, bytes)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(String, string)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeList, list)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeSet, set)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(NativeMap, map)
  FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD(Object, object)

 protected:
  ValueAccess() = default;

 private:
  Value& as_value() noexcept { return static_cast<T&>(*this).as_value(); }
  const Value& as_value() const noexcept {
    return static_cast<const T&>(*this).as_value();
  }
  const Value& as_const_value() const noexcept {
    return static_cast<const T&>(*this).as_value();
  }
};

// ---- Structured types ---- //

constexpr std::size_t SIZE_OF_VALUE = 56;
constexpr std::size_t ALIGN_OF_VALUE = 8;

// TODO(sadroeck) - Figure out a better name for this
class alignas(ALIGN_OF_VALUE) ValueHolder : public ValueAccess<ValueHolder> {
 public:
  ValueHolder() noexcept = default;

  /* implicit */ ValueHolder(const Value&);
  ValueHolder(const ValueHolder&);

  /* implicit */ ValueHolder(Value&&) noexcept;
  ValueHolder(ValueHolder&&) noexcept;

  ValueHolder& operator=(const ValueHolder&);
  ValueHolder& operator=(ValueHolder&&) noexcept;

  ~ValueHolder();

  Value& as_value() noexcept;
  const Value& as_value() const noexcept;

  bool operator==(const ValueHolder& other) const;
  bool operator!=(const ValueHolder& other) const;

 private:
  alignas(ALIGN_OF_VALUE) std::array<std::uint8_t, SIZE_OF_VALUE> data_;
};

// ---- placeholders ---- //
class Object {
 public:
  bool operator==(const Object&) const = default;
};
class NativeList {
 public:
  bool operator==(const NativeList&) const = default;
};
class NativeSet {
 public:
  bool operator==(const NativeSet&) const = default;
};
class NativeMap {
 public:
  bool operator==(const NativeMap&) const = default;
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

template <bool StringToBinary>
struct native_value_type<Bool, StringToBinary> {
  using type = Bool;
  using tag = ::apache::thrift::type::bool_t;
};

template <bool StringToBinary>
struct native_value_type<I8, StringToBinary> {
  using type = I8;
  using tag = ::apache::thrift::type::byte_t;
};

template <bool StringToBinary>
struct native_value_type<I16, StringToBinary> {
  using type = I16;
  using tag = ::apache::thrift::type::i16_t;
};

template <bool StringToBinary>
struct native_value_type<I32, StringToBinary> {
  using type = I32;
  using tag = ::apache::thrift::type::i32_t;
};

template <bool StringToBinary>
struct native_value_type<I64, StringToBinary> {
  using type = I64;
  using tag = ::apache::thrift::type::i64_t;
};

template <bool StringToBinary>
struct native_value_type<Float, StringToBinary> {
  using type = Float;
  using tag = ::apache::thrift::type::float_t;
};

template <bool StringToBinary>
struct native_value_type<Double, StringToBinary> {
  using type = Double;
  using tag = ::apache::thrift::type::double_t;
};

template <bool StringToBinary>
struct native_value_type<Bytes, StringToBinary> {
  using type = Bytes;
  using tag = ::apache::thrift::type::binary_t;
};

template <bool StringToBinary>
struct native_value_type<String, StringToBinary> {
  using type = std::conditional_t<StringToBinary, Bytes, String>;
  using tag = ::apache::thrift::type::string_t;
};

// TODO(sadroeck) - Implement this
struct ValueLike {};

template <bool StringToBinary>
struct native_value_type<NativeList, StringToBinary> {
  using type = NativeList;
  using tag =
      ::apache::thrift::type::list<::apache::thrift::type::struct_t<ValueLike>>;
};

template <bool StringToBinary>
struct native_value_type<NativeSet, StringToBinary> {
  using type = NativeSet;
  using tag =
      ::apache::thrift::type::set<::apache::thrift::type::struct_t<ValueLike>>;
};

template <bool StringToBinary>
struct native_value_type<NativeMap, StringToBinary> {
  using type = NativeMap;
  using tag = ::apache::thrift::type::map<
      ::apache::thrift::type::struct_t<ValueLike>,
      ::apache::thrift::type::struct_t<ValueLike>>;
};

template <typename T, bool StringToBinary = true>
constexpr bool is_primitive_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T, StringToBinary>::tag,
    ::apache::thrift::type::primitive_c>;

template <typename T, bool StringToBinary = true>
constexpr bool is_structured_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T, StringToBinary>::tag,
    ::apache::thrift::type::struct_c>;

template <typename T, bool StringToBinary = true>
constexpr bool is_container_v = ::apache::thrift::type::is_a_v<
    typename native_value_type<T, StringToBinary>::tag,
    ::apache::thrift::type::container_c>;

template <typename T, bool StringToBinary = true>
constexpr bool is_native_object_type_v = is_primitive_v<T, StringToBinary> ||
    is_container_v<T, StringToBinary> || is_structured_v<T, StringToBinary>;

} // namespace detail

// ---- Definition of a Thrift Value ---- //

class Value : public ValueAccess<Value> {
 public:
  using Kind = std::variant<
      Bool,
      I8,
      I16,
      I32,
      I64,
      Float,
      Double,
      Bytes,
      String,
      NativeList,
      NativeSet,
      NativeMap,
      Object>;

  const Kind& inner() const;

  Value& as_value() noexcept { return *this; }
  const Value& as_value() const noexcept { return *this; }

  // Default ops
  Value(Value&& obj) noexcept = default;
  Value(const Value&);
  Value& operator=(const Value&) noexcept = default;
  Value& operator=(Value&&) noexcept = default;
  ~Value() noexcept = default;

  // Variant ctors
  /* implicit */ Value(Kind&& kind) noexcept;
  /* implicit */ Value(Bool&& b) noexcept;
  /* implicit */ Value(I8&& i8) noexcept;
  /* implicit */ Value(I16&& i16) noexcept;
  /* implicit */ Value(I32&& i32) noexcept;
  /* implicit */ Value(I64&& i64) noexcept;
  /* implicit */ Value(Float&& f) noexcept;
  /* implicit */ Value(Double&& d) noexcept;
  /* implicit */ Value(Bytes&& b) noexcept;
  /* implicit */ Value(String&& s) noexcept;
  /* implicit */ Value(NativeList&& list) noexcept;
  /* implicit */ Value(NativeSet&& set) noexcept;
  /* implicit */ Value(NativeMap&& map) noexcept;
  /* implicit */ Value(Object&& strct) noexcept;

  /* implicit */ Value(const Kind& kind);
  /* implicit */ Value(const Bool& b);
  /* implicit */ Value(const I8& i8);
  /* implicit */ Value(const I16& i16);
  /* implicit */ Value(const I32& i32);
  /* implicit */ Value(const I64& i64);
  /* implicit */ Value(const Float& f);
  /* implicit */ Value(const Double& d);
  /* implicit */ Value(const Bytes& b);
  /* implicit */ Value(const String& s);
  /* implicit */ Value(const NativeList& list);
  /* implicit */ Value(const NativeSet& set);
  /* implicit */ Value(const NativeMap& map);
  /* implicit */ Value(const Object& strct);

 private:
  Kind kind_;
}; // namespace apache::thrift::protocol::experimental

// TODO(sadroeck) - Re-enable these after adding containers
// static_assert(
//     sizeof(Value) == SIZE_OF_VALUE,
//     "The size of Value must match the size of ValueHolder");
// static_assert(
//     alignof(Value) == alignof(ValueHolder),
//     "The alignment of Value must match the alignment of ValueHolder");

#undef FBTHRIFT_DEF_MAIN_TYPE_ACCESS_FWD

namespace detail {

Object parseObjectVia(
    ::apache::thrift::BinaryProtocolReader& prot, bool string_to_binary);
Object parseObjectVia(
    ::apache::thrift::CompactProtocolReader& prot, bool string_to_binary);
std::uint32_t serializeObjectVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const Object& obj);
std::uint32_t serializeObjectVia(
    ::apache::thrift::CompactProtocolWriter& prot, const Object& obj);
std::uint32_t serializeValueVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const Value& value);
std::uint32_t serializeValueVia(
    ::apache::thrift::CompactProtocolWriter& prot, const Value& value);

} // namespace detail

template <class Protocol>
Object parseObject(Protocol& prot, bool string_to_binary = true) {
  return detail::parseObjectVia(prot, string_to_binary);
}

template <class Protocol>
Object parseObject(folly::IOBuf& buf, bool string_to_binary = true) {
  Protocol prot{};
  prot.setInput(&buf);
  return parseObject<Protocol>(prot, string_to_binary);
}

template <class Protocol>
std::uint32_t serializeObject(Protocol& prot, const Object& obj) {
  return detail::serializeObjectVia(prot, obj);
}

template <class Protocol>
std::uint32_t serializeValue(Protocol& prot, const Value& val) {
  return detail::serializeValueVia(prot, val);
}

template <class Protocol>
void serializeObject(const Object& val, folly::IOBufQueue& queue) {
  Protocol prot{};
  prot.setOutput(&queue);
  serializeObject(prot, val);
}

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(const Object& val) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  serializeObject<Protocol>(val, queue);
  return queue.move();
}

} // namespace apache::thrift::protocol::experimental

#include <thrift/lib/cpp2/protocol/NativeObject-inl.h>
