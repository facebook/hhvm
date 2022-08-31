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
template <typename KeyTag, typename ValueTag>
struct TypeTagToTType<type::map<KeyTag, ValueTag>> {
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
  template <typename Protocol>
  uint32_t operator()(
      Protocol& prot, const type::native_type<type::list<Tag>>& list) const {
    std::size_t xfer = 0;
    xfer += prot.writeListBegin(typeTagToTType<Tag>, list.size());
    for (const auto& elem : list) {
      xfer += Encode<Tag>{}(prot, elem);
    }
    xfer += prot.writeListEnd();
    return xfer;
  }
};

template <typename Tag>
struct Encode<type::set<Tag>> {
  template <typename Protocol>
  uint32_t operator()(
      Protocol& prot, const type::native_type<type::set<Tag>>& set) const {
    std::size_t xfer = 0;
    xfer += prot.writeSetBegin(typeTagToTType<Tag>, set.size());
    for (const auto& elem : set) {
      xfer += Encode<Tag>{}(prot, elem);
    }
    xfer += prot.writeSetEnd();
    return xfer;
  }
};

template <typename Key, typename Value>
struct Encode<type::map<Key, Value>> {
  template <typename Protocol>
  uint32_t operator()(
      Protocol& prot,
      const type::native_type<type::map<Key, Value>>& map) const {
    std::size_t xfer = 0;
    xfer += prot.writeMapBegin(
        typeTagToTType<Key>, typeTagToTType<Value>, map.size());
    for (const auto& kv : map) {
      xfer += Encode<Key>{}(prot, kv.first);
      xfer += Encode<Value>{}(prot, kv.second);
    }
    xfer += prot.writeMapEnd();
    return xfer;
  }
};

// TODO: Handle cpp_type with containers that cannot use static_cast.
template <typename T, typename Tag>
struct Encode<type::cpp_type<T, Tag>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& t) const {
    return Encode<Tag>{}(prot, static_cast<type::native_type<Tag>>(t));
  }
};

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
struct Decode<type::enum_t<T>> {
  template <typename Protocol>
  void operator()(Protocol& prot, T& t) const {
    int32_t i;
    prot.readI32(i);
    t = static_cast<T>(i);
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
