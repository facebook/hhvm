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

#include <utility>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

using apache::thrift::protocol::TType;

template <typename>
struct TypeTagToTType;

template <>
struct TypeTagToTType<type::bool_t> {
  static constexpr TType value = TType::T_BOOL;
};
template <>
struct TypeTagToTType<type::byte_t> {
  static constexpr TType value = TType::T_BYTE;
};
template <>
struct TypeTagToTType<type::i16_t> {
  static constexpr TType value = TType::T_I16;
};
template <>
struct TypeTagToTType<type::i32_t> {
  static constexpr TType value = TType::T_I32;
};
template <>
struct TypeTagToTType<type::i64_t> {
  static constexpr TType value = TType::T_I64;
};
template <>
struct TypeTagToTType<type::float_t> {
  static constexpr TType value = TType::T_FLOAT;
};
template <>
struct TypeTagToTType<type::double_t> {
  static constexpr TType value = TType::T_DOUBLE;
};
template <>
struct TypeTagToTType<type::string_t> {
  static constexpr TType value = TType::T_STRING;
};
template <>
struct TypeTagToTType<type::binary_t> {
  static constexpr TType value = TType::T_STRING;
};
template <typename Tag>
struct TypeTagToTType<type::list<Tag>> {
  static constexpr TType value = TType::T_LIST;
};
template <typename Tag>
struct TypeTagToTType<type::set<Tag>> {
  static constexpr TType value = TType::T_SET;
};
template <typename KTag, typename ValueTag>
struct TypeTagToTType<type::map<KTag, ValueTag>> {
  static constexpr TType value = TType::T_MAP;
};
template <typename Tag>
struct TypeTagToTType<type::enum_t<Tag>> {
  static constexpr TType value = TType::T_I32;
};
template <typename Tag>
struct TypeTagToTType<type::struct_t<Tag>> {
  static constexpr TType value = TType::T_STRUCT;
};
template <typename Tag>
struct TypeTagToTType<type::union_t<Tag>> {
  static constexpr TType value = TType::T_STRUCT;
};
template <typename Tag>
struct TypeTagToTType<type::exception_t<Tag>> {
  static constexpr TType value = TType::T_STRUCT;
};
template <typename Adapter, typename Tag>
struct TypeTagToTType<type::adapted<Adapter, Tag>> {
  static constexpr TType value = TypeTagToTType<Tag>::value;
};

template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr apache::thrift::protocol::TType typeTagToTType =
    detail::TypeTagToTType<Tag>::value;

template <bool, typename>
struct SerializedSize;

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::bool_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, bool t) const {
    return prot.serializedSizeBool(t);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::byte_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int8_t i) const {
    return prot.serializedSizeByte(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::i16_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int16_t i) const {
    return prot.serializedSizeI16(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::i32_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int32_t i) const {
    return prot.serializedSizeI32(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::i64_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int64_t i) const {
    return prot.serializedSizeI64(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::float_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, float i) const {
    return prot.serializedSizeFloat(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::double_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, double i) const {
    return prot.serializedSizeDouble(i);
  }
};

template <bool ZeroCopy>
struct SerializedSize<ZeroCopy, type::string_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const std::string& s) const {
    return prot.serializedSizeString(s);
  }
};

template <>
struct SerializedSize<false, type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const std::string& s) const {
    return prot.serializedSizeBinary(s);
  }
};

template <>
struct SerializedSize<true, type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const std::string& s) const {
    return prot.serializedSizeZCBinary(s);
  }
};

template <typename T>
struct SerializedSize<false, type::struct_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSize(&prot);
  }
};

template <typename T>
struct SerializedSize<true, type::struct_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSizeZC(&prot);
  }
};

template <typename T>
struct SerializedSize<false, type::union_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSize(&prot);
  }
};

template <typename T>
struct SerializedSize<true, type::union_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSizeZC(&prot);
  }
};

template <typename T>
struct SerializedSize<false, type::exception_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSize(&prot);
  }
};

template <typename T>
struct SerializedSize<true, type::exception_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.serializedSizeZC(&prot);
  }
};

template <bool ZeroCopy, typename T>
struct SerializedSize<ZeroCopy, type::enum_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return prot.serializedSizeI32(static_cast<int32_t>(s));
  }
};

inline uint32_t checked_container_size(size_t size) {
  const size_t limit = std::numeric_limits<int32_t>::max();
  if (size > limit) {
    TProtocolException::throwExceededSizeLimit(size, limit);
  }
  return static_cast<uint32_t>(size);
}

template <bool ZeroCopy, typename Tag>
struct SerializedSize<ZeroCopy, type::list<Tag>> {
  template <typename Protocol, typename ListType>
  uint32_t operator()(Protocol& prot, const ListType& list) const {
    uint32_t xfer = 0;
    xfer += prot.serializedSizeListBegin(
        typeTagToTType<Tag>, checked_container_size(list.size()));
    for (const auto& elem : list) {
      xfer += SerializedSize<ZeroCopy, Tag>{}(prot, elem);
    }
    xfer += prot.serializedSizeListEnd();
    return xfer;
  }
};

template <bool ZeroCopy, typename Tag>
struct SerializedSize<ZeroCopy, type::set<Tag>> {
  template <typename Protocol, typename SetType>
  uint32_t operator()(Protocol& prot, const SetType& set) const {
    uint32_t xfer = 0;
    xfer += prot.serializedSizeSetBegin(
        typeTagToTType<Tag>, checked_container_size(set.size()));
    for (const auto& elem : set) {
      xfer += SerializedSize<ZeroCopy, Tag>{}(prot, elem);
    }
    xfer += prot.serializedSizeSetEnd();
    return xfer;
  }
};

template <bool ZeroCopy, typename Key, typename Value>
struct SerializedSize<ZeroCopy, type::map<Key, Value>> {
  template <typename Protocol, typename MapType>
  uint32_t operator()(Protocol& prot, const MapType& map) const {
    uint32_t xfer = 0;
    xfer += prot.serializedSizeMapBegin(
        typeTagToTType<Key>,
        typeTagToTType<Value>,
        checked_container_size(map.size()));
    for (const auto& kv : map) {
      xfer += SerializedSize<ZeroCopy, Key>{}(prot, kv.first);
      xfer += SerializedSize<ZeroCopy, Value>{}(prot, kv.second);
    }
    xfer += prot.serializedSizeMapEnd();
    return xfer;
  }
};

template <bool ZeroCopy, typename T, typename Tag>
struct SerializedSize<ZeroCopy, type::cpp_type<T, Tag>>
    : SerializedSize<ZeroCopy, Tag> {};

// TODO: Use serializedSize in adapter to optimize.
template <bool ZeroCopy, typename Adapter, typename Tag>
struct SerializedSize<ZeroCopy, type::adapted<Adapter, Tag>> {
  template <typename Protocol, typename U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    return SerializedSize<ZeroCopy, Tag>{}(prot, Adapter::toThrift(m));
  }
};

template <typename>
struct Encode;

template <>
struct Encode<type::bool_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, bool t) const {
    return prot.writeBool(t);
  }
};

template <>
struct Encode<type::byte_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int8_t i) const {
    return prot.writeByte(i);
  }
};

template <>
struct Encode<type::i16_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int16_t i) const {
    return prot.writeI16(i);
  }
};

template <>
struct Encode<type::i32_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int32_t i) const {
    return prot.writeI32(i);
  }
};

template <>
struct Encode<type::i64_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, int64_t i) const {
    return prot.writeI64(i);
  }
};

template <>
struct Encode<type::float_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, float i) const {
    return prot.writeFloat(i);
  }
};

template <>
struct Encode<type::double_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, double i) const {
    return prot.writeDouble(i);
  }
};

template <>
struct Encode<type::string_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const std::string& s) const {
    return prot.writeString(s);
  }
};

template <>
struct Encode<type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const std::string& s) const {
    return prot.writeBinary(s);
  }
};

template <typename T>
struct Encode<type::struct_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.write(&prot);
  }
};

template <typename T>
struct Encode<type::union_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.write(&prot);
  }
};

template <typename T>
struct Encode<type::exception_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return s.write(&prot);
  }
};

template <typename T>
struct Encode<type::enum_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& s) const {
    return prot.writeI32(static_cast<int32_t>(s));
  }
};

// TODO: add optimization used in protocol_methods.h
template <typename Tag>
struct Encode<type::list<Tag>> {
  template <typename Protocol, typename T>
  uint32_t operator()(Protocol& prot, const T& list) const {
    uint32_t xfer = 0;
    xfer += prot.writeListBegin(
        typeTagToTType<Tag>, checked_container_size(list.size()));
    for (const auto& elem : list) {
      xfer += Encode<Tag>{}(prot, elem);
    }
    xfer += prot.writeListEnd();
    return xfer;
  }
};

template <typename Tag>
struct Encode<type::set<Tag>> {
  template <typename Protocol, typename T>
  uint32_t operator()(Protocol& prot, const T& set) const {
    uint32_t xfer = 0;
    xfer += prot.writeSetBegin(
        typeTagToTType<Tag>, checked_container_size(set.size()));
    for (const auto& elem : set) {
      xfer += Encode<Tag>{}(prot, elem);
    }
    xfer += prot.writeSetEnd();
    return xfer;
  }
};

template <typename Key, typename Value>
struct Encode<type::map<Key, Value>> {
  template <typename Protocol, typename T>
  uint32_t operator()(Protocol& prot, const T& map) const {
    uint32_t xfer = 0;
    xfer += prot.writeMapBegin(
        typeTagToTType<Key>,
        typeTagToTType<Value>,
        checked_container_size(map.size()));
    for (const auto& kv : map) {
      xfer += Encode<Key>{}(prot, kv.first);
      xfer += Encode<Value>{}(prot, kv.second);
    }
    xfer += prot.writeMapEnd();
    return xfer;
  }
};

template <typename T, typename Tag>
struct Encode<type::cpp_type<T, Tag>> : Encode<Tag> {};

template <typename Adapter, typename Tag>
struct Encode<type::adapted<Adapter, Tag>> {
  template <typename Protocol, typename U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    return Encode<Tag>{}(prot, Adapter::toThrift(m));
  }
};

template <typename>
struct Decode;

template <>
struct Decode<type::bool_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, bool& b) const {
    prot.readBool(b);
  }

  template <typename Protocol>
  void operator()(Protocol& prot, std::vector<bool>::reference t) const {
    bool b;
    prot.readBool(b);
    t = b;
  }
};

template <>
struct Decode<type::byte_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, int8_t& i) const {
    prot.readByte(i);
  }
};

template <>
struct Decode<type::i16_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, int16_t& i) const {
    prot.readI16(i);
  }
};

template <>
struct Decode<type::i32_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, int32_t& i) const {
    prot.readI32(i);
  }
};

template <>
struct Decode<type::i64_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, int64_t& i) const {
    prot.readI64(i);
  }
};

template <>
struct Decode<type::float_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, float& f) const {
    prot.readFloat(f);
  }
};

template <>
struct Decode<type::double_t> {
  template <typename Protocol>
  void operator()(Protocol& prot, double& d) const {
    prot.readDouble(d);
  }
};

template <>
struct Decode<type::string_t> {
  template <typename Protocol, typename StrType>
  void operator()(Protocol& prot, StrType& s) const {
    prot.readString(s);
  }
};

template <>
struct Decode<type::binary_t> {
  template <typename Protocol, typename StrType>
  void operator()(Protocol& prot, StrType& s) const {
    prot.readBinary(s);
  }
};

template <typename T>
struct Decode<type::struct_t<T>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& s) const {
    s.read(&prot);
  }
};

template <typename T>
struct Decode<type::union_t<T>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& s) const {
    s.read(&prot);
  }
};

template <typename T>
struct Decode<type::exception_t<T>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& s) const {
    s.read(&prot);
  }
};

template <typename T>
struct Decode<type::enum_t<T>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& t) const {
    int32_t i;
    prot.readI32(i);
    t = static_cast<T>(i);
  }
};

// TODO: add optimization used in protocol_methods.h
template <typename Tag>
struct Decode<type::list<Tag>> {
  template <typename Protocol, typename ListType>
  void operator()(Protocol& prot, ListType& list) const {
    TType t;
    uint32_t s;
    prot.readListBegin(t, s);
    apache::thrift::detail::pm::reserve_if_possible(&list, s);
    if (typeTagToTType<Tag> == t) {
      while (s--) {
        auto&& elem = apache::thrift::detail::pm::emplace_back_default(list);
        Decode<Tag>{}(prot, elem);
      }
    } else {
      while (s--) {
        prot.skip(t);
      }
    }
    prot.readListEnd();
  }
};

template <typename Tag>
struct Decode<type::set<Tag>> {
  template <typename Protocol, typename SetType>
  void operator()(Protocol& prot, SetType& set) const {
    TType t;
    uint32_t s;
    prot.readSetBegin(t, s);
    apache::thrift::detail::pm::reserve_if_possible(&set, s);
    if (typeTagToTType<Tag> == t) {
      while (s--) {
        typename SetType::value_type value;
        Decode<Tag>{}(prot, value);
        set.emplace_hint(set.end(), std::move(value));
      }
    } else {
      while (s--) {
        prot.skip(t);
      }
    }
    prot.readSetEnd();
  }
};

template <typename Key, typename Value>
struct Decode<type::map<Key, Value>> {
  template <typename Protocol, typename MapType>
  void operator()(Protocol& prot, MapType& map) const {
    TType keyType, valueType;
    uint32_t s;
    prot.readMapBegin(keyType, valueType, s);
    apache::thrift::detail::pm::reserve_if_possible(&map, s);
    if (typeTagToTType<Key> == keyType && typeTagToTType<Value> == valueType) {
      while (s--) {
        typename MapType::key_type key;
        Decode<Key>{}(prot, key);
        auto iter = map.emplace_hint(
            map.end(), std::move(key), typename MapType::mapped_type{});
        Decode<Value>{}(prot, iter->second);
      }
    } else {
      while (s--) {
        prot.skip(keyType);
        prot.skip(valueType);
      }
    }
    prot.readMapEnd();
  }
};

template <typename T, typename Tag>
struct Decode<type::cpp_type<T, Tag>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& t) const {
    type::native_type<Tag> u;
    Decode<Tag>{}(prot, u);
    t = static_cast<T>(u);
  }
};

template <typename T, typename Tag>
struct Decode<type::cpp_type<T, type::list<Tag>>> : Decode<type::list<Tag>> {};

template <typename T, typename Tag>
struct Decode<type::cpp_type<T, type::set<Tag>>> : Decode<type::set<Tag>> {};

template <typename T, typename Key, typename Value>
struct Decode<type::cpp_type<T, type::map<Key, Value>>>
    : Decode<type::map<Key, Value>> {};

// TODO: Use inplace adapter deserialization as optimization.
template <typename Adapter, typename Tag>
struct Decode<type::adapted<Adapter, Tag>> {
  template <typename Protocol, typename U>
  void operator()(Protocol& prot, U& m) const {
    type::native_type<Tag> orig;
    Decode<Tag>{}(prot, orig);
    m = Adapter::fromThrift(std::move(orig));
  }

  template <typename FieldId, class Struct, class Protocol, class U>
  void operator()(FieldId, Struct& strct, Protocol& prot, U& m) const {
    type::native_type<Tag> orig;
    Decode<Tag>{}(prot, orig);
    m = adapt_detail::
        fromThriftField<Adapter, folly::to_underlying(FieldId::value)>(
            std::move(orig), strct);
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
