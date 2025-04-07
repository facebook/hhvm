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

// ---- ValueHolder ---- //

ValueHolder::ValueHolder(const Value& val) : data_{} {
  static_cast<Value&>(*this) = val;
}

ValueHolder::ValueHolder(const ValueHolder& val) : data_{} {
  static_cast<Value&>(*this) = static_cast<const Value&>(val);
}

ValueHolder::ValueHolder(Value&& val) noexcept : data_{} {
  static_cast<Value&>(*this) = std::move(val);
}

ValueHolder::ValueHolder(ValueHolder&& val) noexcept
    : data_{std::move(val.data_)} {
  val.data_ = {};
}

Value& ValueHolder::as_value() noexcept {
  return reinterpret_cast<Value&>(data_);
}

const Value& ValueHolder::as_value() const noexcept {
  return reinterpret_cast<const Value&>(data_);
}

ValueHolder& ValueHolder::operator=(const ValueHolder& val) {
  static_cast<Value&>(*this) = static_cast<const Value&>(val);
  return *this;
}

ValueHolder& ValueHolder::operator=(ValueHolder&& val) noexcept {
  data_ = std::move(val.data_);
  val.data_ = {};
  return *this;
}

ValueHolder::~ValueHolder() {
  // TODO(sadroeck) - Invalidate data_ by putting sentinal value as last byte ?
  // Note: this should always be the std::variant index
  if (data_ != decltype(data_){}) {
    static_cast<Value&>(*this).~Value();
  }
}

bool ValueHolder::operator==(const ValueHolder& other) const {
  return static_cast<const Value&>(*this) == static_cast<const Value&>(other);
}

bool ValueHolder::operator!=(const ValueHolder& other) const {
  return !(*this == other);
}

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
template <typename... Args>
auto make_value(Args&&... args) {
  return ValueHolder{Value(std::forward<Args>(args)...)};
}

template <typename Protocol, bool StringToBinary>
void read_struct_field(
    Protocol& prot, TType field_type, Object& strct, Object::FieldId id) {
  switch (field_type) {
    case T_BOOL: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, Bool>(prot)));
      return;
    }
    case T_BYTE: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, I8>(prot)));
      return;
    }
    case T_I16: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, I16>(prot)));
      return;
    }
    case T_I32: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, I32>(prot)));
      return;
    }
    case T_I64: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, I64>(prot)));
      return;
    }
    case T_FLOAT: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, Float>(prot)));
      return;
    }
    case T_DOUBLE: {
      strct.emplace(id, make_value(read_primitive_as<Protocol, Double>(prot)));
      return;
    }
    case T_STRING: {
      if constexpr (StringToBinary) {
        strct.emplace(id, make_value(read_primitive_as<Protocol, Bytes>(prot)));
      } else {
        strct.emplace(
            id, make_value(read_primitive_as<Protocol, String>(prot)));
      }
      return;
    }
    case protocol::T_STRUCT: {
      strct.emplace(
          id, make_value(read_struct<Protocol, StringToBinary>(prot)));
      return;
    }
    case protocol::T_LIST: {
      strct.emplace(id, make_value(read_list<Protocol, StringToBinary>(prot)));
      return;
    }
    case protocol::T_SET: {
      strct.emplace(id, make_value(read_set<Protocol, StringToBinary>(prot)));
      return;
    }
    case protocol::T_MAP: {
      strct.emplace(id, make_value(read_map<Protocol, StringToBinary>(prot)));
      return;
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
      TProtocolException::throwInvalidSkipType(field_type);
      break;
  }
}

template <class Protocol, bool StringToBinary>
Object read_struct(Protocol& prot) {
  std::string name;
  int16_t fid{};
  TType ftype{};
  Object strct{};
  prot.readStructBegin(name);
  while (true) {
    prot.readFieldBegin(name, ftype, fid);
    if (ftype == protocol::T_STOP) {
      break;
    }
    read_struct_field<Protocol, StringToBinary>(prot, ftype, strct, fid);
    prot.readFieldEnd();
  }
  prot.readStructEnd();
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
  serializedSize += prot.writeStructBegin("");
  for (const auto& [field_id, field_val] : obj) {
    auto fieldType = field_val.get_ttype();
    serializedSize += prot.writeFieldBegin("", fieldType, field_id);
    serializedSize += write(prot, field_val);
    serializedSize += prot.writeFieldEnd();
  }
  serializedSize += prot.writeFieldStop();
  serializedSize += prot.writeStructEnd();
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

const std::size_t OBJECT_HASH_SEED = 0xFE << 24;

struct ValueHasher {
  size_t operator()(const Value& v) const;
  size_t operator()(const ValueHolder& v) const;
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
  template <typename T, typename U>
  size_t operator()(const std::pair<T, U>& p) const;
};

size_t ValueHasher::operator()(const Value& v) const {
  return v.inner().index() + folly::variant_match(v.inner(), ValueHasher{});
}
size_t ValueHasher::operator()(const ValueHolder& v) const {
  return ValueHasher{}(v.as_value());
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
  return folly::hash::commutative_hash_combine_range_generic(
      OBJECT_HASH_SEED, ValueHasher{}, o.begin(), o.end());
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

template <typename T, typename U>
size_t ValueHasher::operator()(const std::pair<T, U>& p) const {
  return folly::hash::hash_combine_generic(ValueHasher{}, p.first, p.second);
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

size_t detail::hash_value(const ValueHolder& v) {
  return ValueHasher{}(v);
}

// ---- Object ---- //

Object::Fields::iterator Object::begin() {
  return fields.begin();
}

Object::Fields::iterator Object::end() {
  return fields.end();
}

Object::Fields::const_iterator Object::begin() const {
  return fields.begin();
}

Object::Fields::const_iterator Object::end() const {
  return fields.end();
}

Value& Object::operator[](FieldId i) {
  return fields[i];
}
Value& Object::at(FieldId i) {
  return fields.at(i).as_value();
}
const Value& Object::at(FieldId i) const {
  return fields.at(i).as_value();
}
bool Object::contains(FieldId i) const {
  return fields.find(i) != fields.end();
}
std::size_t Object::erase(FieldId i) {
  return fields.erase(i);
}
Value* Object::if_contains(FieldId i) {
  auto* ptr = folly::get_ptr(fields, i);
  return ptr ? &ptr->as_value() : nullptr;
}
const Value* Object::if_contains(FieldId i) const {
  const auto* ptr = folly::get_ptr(fields, i);
  return ptr ? &ptr->as_value() : nullptr;
}

} // namespace apache::thrift::protocol::experimental
