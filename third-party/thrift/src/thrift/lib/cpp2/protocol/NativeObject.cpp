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

#include <thrift/lib/cpp2/protocol/NativeObject.h>

namespace apache::thrift::protocol::experimental {

// ---- Value ---- //

Value::Value(const Value& other) : kind_(other.kind_) {}
Value::Value(const Value::Kind& kind) : kind_(kind) {}

Value::Value(Value::Kind&& kind) noexcept : kind_(std::move(kind)) {}
Value::Value(Bool&& b) noexcept : kind_(std::move(b)) {}
Value::Value(I8&& i) noexcept : kind_(std::move(i)) {}
Value::Value(I16&& i) noexcept : kind_(std::move(i)) {}
Value::Value(I32&& i) noexcept : kind_(std::move(i)) {}
Value::Value(I64&& i) noexcept : kind_(std::move(i)) {}
Value::Value(Float&& f) noexcept : kind_(std::move(f)) {}
Value::Value(Double&& d) noexcept : kind_(std::move(d)) {}
Value::Value(Bytes&& s) noexcept : kind_(std::move(s)) {}
Value::Value(String&& s) noexcept : kind_(std::move(s)) {}
Value::Value(NativeList&& list) noexcept : kind_(std::move(list)) {}
Value::Value(NativeSet&& set) noexcept : kind_(std::move(set)) {}
Value::Value(NativeMap&& map) noexcept : kind_(std::move(map)) {}
Value::Value(Object&& strct) noexcept : kind_(std::move(strct)) {}

Value::Value(const Bool& b) : kind_(b) {}
Value::Value(const I8& i8) : kind_(i8) {}
Value::Value(const I16& i16) : kind_(i16) {}
Value::Value(const I32& i32) : kind_(i32) {}
Value::Value(const I64& i64) : kind_(i64) {}
Value::Value(const Float& f) : kind_(f) {}
Value::Value(const Double& d) : kind_(d) {}
Value::Value(const Bytes& b) : kind_(b) {}
Value::Value(const String& s) : kind_(s) {}
Value::Value(const NativeList& list) : kind_(list) {}
Value::Value(const NativeSet& set) : kind_(set) {}
Value::Value(const NativeMap& map) : kind_(map) {}
Value::Value(const Object& strct) : kind_(strct) {}

const Value::Kind& Value::inner() const {
  return kind_;
}

// ------- parsing functions ------- //

template <typename Protocol, typename T>
T read_primitive_as(Protocol& prot) {
  if constexpr (std::is_same_v<T, Bool>) {
    Bool b;
    prot.readBool(b);
    return b;
  } else if constexpr (std::is_same_v<T, I8>) {
    I8 i;
    prot.readByte(i);
    return i;
  } else if constexpr (std::is_same_v<T, I16>) {
    I16 i;
    prot.readI16(i);
    return i;
  } else if constexpr (std::is_same_v<T, I32>) {
    I32 i;
    prot.readI32(i);
    return i;
  } else if constexpr (std::is_same_v<T, I64>) {
    I64 i;
    prot.readI64(i);
    return i;
  } else if constexpr (std::is_same_v<T, Float>) {
    Float f;
    prot.readFloat(f);
    return f;
  } else if constexpr (std::is_same_v<T, Double>) {
    Double d;
    prot.readDouble(d);
    return d;
  } else if constexpr (std::is_same_v<T, Bytes>) {
    Bytes b;
    prot.readBinary(b.buf_);
    return b;
  } else if constexpr (std::is_same_v<T, String>) {
    String s;
    prot.readString(s);
    return s;
  } else {
    static_assert(false, "Unhandled primitive type");
  }
}

template <typename Protocol, bool StringToBinary>
NativeList read_list(Protocol& prot);

template <typename Protocol, bool StringToBinary>
NativeSet read_set(Protocol& prot);

template <typename Protocol, bool StringToBinary>
NativeMap read_map(Protocol& prot);

template <typename Protocol, bool StringToBinary>
Object read_struct(Protocol& prot);

constexpr bool is_primitive(TType type) {
  switch (type) {
    case protocol::T_BOOL:
    case protocol::T_BYTE:
    case protocol::T_I16:
    case protocol::T_I32:
    case protocol::T_I64:
    case protocol::T_DOUBLE:
    case protocol::T_FLOAT:
    case protocol::T_STRING:
      return true;
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_STRUCT:
    case T_MAP:
    case T_SET:
    case T_LIST:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      return false;
  }
}

template <typename Protocol, bool StringToBinary>
NativeList read_list(Protocol& prot) {
  std::ignore = prot;
  return NativeList{};
}

template <typename Protocol, bool StringToBinary>
NativeSet read_set(Protocol& prot) {
  std::ignore = prot;
  return NativeSet{};
}

template <typename Protocol, bool StringToBinary>
NativeMap read_map(Protocol& prot) {
  std::ignore = prot;
  return NativeMap{};
}

template <class Protocol, bool StringToBinary>
Object read_struct(Protocol& prot) {
  Object strct{};
  std::ignore = prot;
  return strct;
}

Object detail::parseObjectVia(
    ::apache::thrift::BinaryProtocolReader& prot, bool string_to_binary) {
  if (string_to_binary) {
    return read_struct<::apache::thrift::BinaryProtocolReader, true>(prot);
  } else {
    return read_struct<::apache::thrift::BinaryProtocolReader, false>(prot);
  }
}

Object detail::parseObjectVia(
    ::apache::thrift::CompactProtocolReader& prot, bool string_to_binary) {
  if (string_to_binary) {
    return read_struct<::apache::thrift::CompactProtocolReader, true>(prot);
  } else {
    return read_struct<::apache::thrift::CompactProtocolReader, false>(prot);
  }
}

// ---- Serialization ---- //

#define PROTOTYPE_WRITE(CLASS) \
  template <typename Protocol> \
  std::uint32_t write(Protocol& prot, const CLASS& val);

PROTOTYPE_WRITE(Bool)
PROTOTYPE_WRITE(I8)
PROTOTYPE_WRITE(I16)
PROTOTYPE_WRITE(I32)
PROTOTYPE_WRITE(I64)
PROTOTYPE_WRITE(Float)
PROTOTYPE_WRITE(Double)
PROTOTYPE_WRITE(Bytes)
PROTOTYPE_WRITE(String)
PROTOTYPE_WRITE(Object)
PROTOTYPE_WRITE(NativeList)
PROTOTYPE_WRITE(NativeSet)
PROTOTYPE_WRITE(NativeMap)

template <typename Protocol>
std::uint32_t write(Protocol&, const std::monostate&) {
  return 0;
}

#define WRITE_IMPL_VIA(CLASS, METHOD)                     \
  template <typename Protocol>                            \
  std::uint32_t write(Protocol& prot, const CLASS& val) { \
    return prot.METHOD(val);                              \
  }

WRITE_IMPL_VIA(Bool, writeBool)
WRITE_IMPL_VIA(I8, writeByte)
WRITE_IMPL_VIA(I16, writeI16)
WRITE_IMPL_VIA(I32, writeI32)
WRITE_IMPL_VIA(I64, writeI64)
WRITE_IMPL_VIA(Float, writeFloat)
WRITE_IMPL_VIA(Double, writeDouble)

template <typename Protocol>
std::uint32_t write(Protocol& prot, const Bytes& val) {
  return prot.writeBinary(
      folly::ByteRange{val.data(), val.data() + val.size()});
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const String& val) {
  return prot.writeBinary(
      folly::StringPiece{val.data(), val.data() + val.size()});
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const Object& obj) {
  uint32_t serializedSize = 0;
  std::ignore = prot;
  std::ignore = obj;
  return serializedSize;
}

inline void ensure_same_type(const Value& a, TType b) {
  if (a.get_ttype() != b) {
    TProtocolException::throwInvalidFieldData();
  }
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeList& list) {
  std::ignore = prot;
  std::ignore = list;
  return 0;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeSet& set) {
  std::ignore = prot;
  std::ignore = set;
  return 0;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const NativeMap& map) {
  std::ignore = prot;
  std::ignore = map;
  return 0;
}

template <typename Protocol>
std::uint32_t write(Protocol& prot, const Value& value) {
  return folly::variant_match(
      value.inner(), [&](const auto& v) { return write(prot, v); });
}

std::uint32_t detail::serializeObjectVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const Object& obj) {
  return write(prot, obj);
}

std::uint32_t detail::serializeObjectVia(
    ::apache::thrift::CompactProtocolWriter& prot, const Object& obj) {
  return write(prot, obj);
}

std::uint32_t detail::serializeValueVia(
    ::apache::thrift::BinaryProtocolWriter& prot, const Value& value) {
  return write(prot, value);
}
std::uint32_t detail::serializeValueVia(
    ::apache::thrift::CompactProtocolWriter& prot, const Value& value) {
  return write(prot, value);
}

// ---- Hashing utilities ---- //

struct ValueHasher {
  size_t operator()(const Value& v) const;
  size_t operator()(const Bool& b) const;
  size_t operator()(const I8& i) const;
  size_t operator()(const I16& i) const;
  size_t operator()(const I32& i) const;
  size_t operator()(const I64& i) const;
  size_t operator()(const Float& f) const;
  size_t operator()(const Double& d) const;
  size_t operator()(const Bytes& s) const;
  size_t operator()(const String& s) const;
  size_t operator()(const Object& o) const;
  size_t operator()(const NativeList& l) const;
  size_t operator()(const NativeSet& s) const;
  size_t operator()(const NativeMap& m) const;
  size_t operator()(const std::monostate&) const;
};

size_t ValueHasher::operator()(const Value& v) const {
  return v.inner().index() + folly::variant_match(v.inner(), ValueHasher{});
}
size_t ValueHasher::operator()(const Bool& b) const {
  return std::hash<bool>{}(b);
}
size_t ValueHasher::operator()(const I8& i) const {
  return std::hash<int8_t>{}(i);
}
size_t ValueHasher::operator()(const I16& i) const {
  return std::hash<int16_t>{}(i);
}
size_t ValueHasher::operator()(const I32& i) const {
  return std::hash<int32_t>{}(i);
}
size_t ValueHasher::operator()(const I64& i) const {
  return std::hash<int64_t>{}(i);
}
size_t ValueHasher::operator()(const Float& f) const {
  return std::hash<float>{}(f);
}
size_t ValueHasher::operator()(const Double& d) const {
  return std::hash<double>{}(d);
}
size_t ValueHasher::operator()(const Bytes& s) const {
  return folly::IOBufHash{}(s.buf_);
}
size_t ValueHasher::operator()(const String& s) const {
  return std::hash<std::string>{}(s);
}

size_t ValueHasher::operator()(const Object& o) const {
  std::ignore = o;
  return 0;
}
size_t ValueHasher::operator()(const NativeList& l) const {
  std::ignore = l;
  return 0;
}
size_t ValueHasher::operator()(const NativeSet& s) const {
  std::ignore = s;
  return 0;
}
size_t ValueHasher::operator()(const NativeMap& m) const {
  std::ignore = m;
  return 0;
}

size_t ValueHasher::operator()(const std::monostate&) const {
  return 0;
}

template <typename T>
size_t hash_value(const T& t) {
  return ValueHasher{}(t);
}

size_t detail::hash_value(const Value& v) {
  return ValueHasher{}(v);
}

size_t detail::hash_value(const Object& o) {
  return ValueHasher{}(o);
}

size_t detail::hash_value(const Bytes& s) {
  return ValueHasher{}(s);
}

} // namespace apache::thrift::protocol::experimental
