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

#include <folly/Overload.h>
#include <folly/Range.h>
#include <folly/Utility.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {

class BinaryProtocolWriter;
class CompactProtocolWriter;
class SimpleJSONProtocolWriter;

namespace op {
namespace detail {

template <typename T, typename Tag>
FOLLY_INLINE_VARIABLE constexpr bool kIsStrongType =
    std::is_enum<folly::remove_cvref_t<T>>::value&&
        type::is_a_v<Tag, type::integral_c>;

template <typename T, typename Tag>
FOLLY_INLINE_VARIABLE constexpr bool kIsIntegral =
    type::is_a_v<Tag, type::integral_c>;

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
template <typename T, typename Tag>
struct TypeTagToTType<type::cpp_type<T, Tag>> {
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
  uint32_t operator()(Protocol& prot, folly::StringPiece s) const {
    return prot.serializedSizeString(s);
  }
};

template <>
struct SerializedSize<false, type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, folly::StringPiece s) const {
    return prot.serializedSizeBinary(s);
  }

  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const folly::IOBuf& s) const {
    return prot.serializedSizeBinary(s);
  }
};

template <>
struct SerializedSize<true, type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, folly::StringPiece s) const {
    return prot.serializedSizeZCBinary(s);
  }
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const folly::IOBuf& s) const {
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
    : SerializedSize<ZeroCopy, Tag> {
  template <typename Protocol, typename U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    if constexpr (kIsStrongType<U, Tag>) {
      return SerializedSize<ZeroCopy, Tag>{}(
          prot, static_cast<type::native_type<Tag>>(m));
    } else {
      return SerializedSize<ZeroCopy, Tag>{}(prot, m);
    }
  }
};

template <bool ZeroCopy, typename Adapter, typename Tag>
struct SerializedSize<ZeroCopy, type::adapted<Adapter, Tag>> {
  template <typename Protocol, typename U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    return folly::overload(
        [&](auto adapter)
            -> decltype(decltype(adapter)::
                            template serializedSize<ZeroCopy, Tag>(prot, m)) {
          return decltype(adapter)::template serializedSize<ZeroCopy, Tag>(
              prot, m);
        },
        [&](auto...) {
          return SerializedSize<ZeroCopy, Tag>{}(prot, Adapter::toThrift(m));
        })(Adapter{});
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
  uint32_t operator()(Protocol& prot, folly::StringPiece s) const {
    return prot.writeString(s);
  }
};

template <>
struct Encode<type::binary_t> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const folly::IOBuf& s) const {
    return prot.writeBinary(s);
  }
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, folly::StringPiece s) const {
    return prot.writeBinary(s);
  }
};

template <class Tag>
struct ShouldWrite {
  template <typename T>
  bool operator()(field_ref<T>) const {
    return true;
  }
  template <typename T>
  bool operator()(required_field_ref<T>) const {
    return true;
  }
  template <typename T>
  bool operator()(optional_field_ref<T> opt) const {
    return opt.has_value();
  }
  template <typename T>
  bool operator()(optional_boxed_field_ref<T> opt) const {
    return opt.has_value();
  }
  template <typename T>
  bool operator()(union_field_ref<T> opt) const {
    return opt.has_value();
  }
  template <typename T>
  bool operator()(terse_field_ref<T> val) const {
    return !isEmpty<Tag>(*val);
  }
  template <typename T>
  bool operator()(terse_intern_boxed_field_ref<T> val) const {
    return !isEmpty<Tag>(*val);
  }
  template <typename T>
  bool operator()(const std::unique_ptr<T>& ptr) const {
    return ptr != nullptr;
  }
  template <typename T>
  bool operator()(const std::shared_ptr<T>& ptr) const {
    return ptr != nullptr;
  }
};

template <class Tag>
FOLLY_INLINE_VARIABLE constexpr ShouldWrite<Tag> should_write{};

template <typename T>
struct StructEncode {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& t) const {
    uint32_t s = 0;
    s += prot.writeStructBegin(op::get_class_name_v<T>.data());
    op::for_each_ordinal<T>([&](auto id) {
      using Id = decltype(id);
      using Tag = op::get_type_tag<T, Id>;
      auto&& field = op::get<Id>(t);
      if (!should_write<Tag>(field)) {
        return;
      }

      s += prot.writeFieldBegin(
          &*op::get_name_v<T, Id>.begin(),
          typeTagToTType<Tag>,
          folly::to_underlying(op::get_field_id<T, Id>::value));
      s += Encode<Tag>{}(prot, *field);
      s += prot.writeFieldEnd();
    });
    s += prot.writeFieldStop();
    s += prot.writeStructEnd();
    return s;
  }
};

template <typename T>
struct Encode<type::struct_t<T>> {
  template <typename Protocol>
  uint32_t operator()(Protocol& prot, const T& t) const {
    // Is protocol is pre-compiled, use `write` method since it's faster
    // than `StructEncode`.
    constexpr bool useWrite =
        folly::IsOneOf<Protocol, CompactProtocolWriter, BinaryProtocolWriter>::
            value ||
        (std::is_same<Protocol, SimpleJSONProtocolWriter>::value &&
         decltype(apache::thrift::detail::st::struct_private_access::
                      __fbthrift_cpp2_gen_json<T>())::value);
    if constexpr (useWrite) {
      return t.write(&prot);
    } else {
      return StructEncode<T>{}(prot, t);
    }
  }
};

// TODO: Use `union_match` to optimize union serialization
template <typename T>
struct Encode<type::union_t<T>> : Encode<type::struct_t<T>> {};

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
struct ListEncode {
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
struct Encode<type::list<Tag>> : ListEncode<Tag> {};

template <typename Tag>
struct SetEncode {
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

template <typename Tag>
struct Encode<type::set<Tag>> : SetEncode<Tag> {};

template <typename Key, typename Value>
struct MapEncode {
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

template <typename Key, typename Value>
struct Encode<type::map<Key, Value>> : MapEncode<Key, Value> {};

template <typename T, typename Tag>
struct CppTypeEncode {
  template <class Protocol, class U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    if constexpr (kIsStrongType<U, Tag>) {
      return Encode<Tag>{}(prot, static_cast<type::native_type<Tag>>(m));
    } else {
      return Encode<Tag>{}(prot, m);
    }
  }
};

template <typename T, typename Tag>
struct Encode<type::cpp_type<T, Tag>> : CppTypeEncode<T, Tag> {};

template <typename Adapter, typename Tag>
struct AdaptedEncode {
  template <typename Protocol, typename U>
  uint32_t operator()(Protocol& prot, const U& m) const {
    return folly::overload(
        [&](auto adapter) -> decltype(decltype(adapter)::template encode<Tag>(
                              prot, m)) {
          return decltype(adapter)::template encode<Tag>(prot, m);
        },
        [&](auto...) { return Encode<Tag>{}(prot, Adapter::toThrift(m)); })(
        Adapter{});
  }
};

template <typename Adapter, typename Tag>
struct Encode<type::adapted<Adapter, Tag>> : AdaptedEncode<Adapter, Tag> {};

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
    auto consumeElem = [&] {
      auto&& elem = apache::thrift::detail::pm::emplace_back_default(list);
      Decode<Tag>{}(prot, elem);
    };
    TType t;
    uint32_t s;
    prot.readListBegin(t, s);
    list = ListType();
    if (prot.kOmitsContainerSizes()) {
      // list size unknown, SimpleJSON protocol won't know type, either
      // so let's just hope that it spits out something that makes sense
      while (prot.peekList()) {
        consumeElem();
      }
    } else if (typeTagToTType<Tag> == t) {
      apache::thrift::detail::pm::reserve_if_possible(&list, s);
      while (s--) {
        consumeElem();
      }
    } else {
      while (s--) {
        prot.skip(t);
      }
    }

    prot.readListEnd();
  }
};

template <typename Container, typename... Args>
using emplace_hint_t = decltype(FOLLY_DECLVAL(Container).emplace_hint(
    FOLLY_DECLVAL(Container).end(), FOLLY_DECLVAL(Args)...));
template <typename Container, typename... Args>
using emplace_t = decltype(FOLLY_DECLVAL(Container).emplace(
    FOLLY_DECLVAL(Container).end(), FOLLY_DECLVAL(Args)...));
template <typename Container, typename Key, typename Value>
auto& emplace_at_end(Container& container, Key&& key, Value&& val) {
  if constexpr (folly::is_detected_v<emplace_hint_t, Container&, Key, Value>) {
    return container
        .emplace_hint(
            container.end(), std::forward<Key>(key), std::forward<Value>(val))
        ->second;
  } else if constexpr (folly::
                           is_detected_v<emplace_t, Container&, Key, Value>) {
    return container.emplace(std::forward<Key>(key), std::forward<Value>(val))
        .first->second;
  } else {
    return container[std::forward<Key>(key)];
  }
}
template <typename Container, typename Value>
void emplace_at_end(Container& container, Value&& val) {
  if constexpr (folly::is_detected_v<emplace_hint_t, Container&, Value>) {
    container.emplace_hint(container.end(), std::forward<Value>(val));
  } else if constexpr (folly::is_detected_v<emplace_t, Container&, Value>) {
    container.emplace(std::forward<Value>(val));
  } else {
    container.insert(std::forward<Value>(val));
  }
}

template <typename Tag>
struct Decode<type::set<Tag>> {
 private:
  // Handles set with sorted_unique property
  template <typename Set, typename Protocol>
  static std::enable_if_t<
      apache::thrift::detail::pm::sorted_unique_constructible_v<Set>>
  decode_known_length_set(Protocol& prot, Set& set, std::uint32_t set_size) {
    if (set_size == 0) {
      return;
    }

    bool sorted = true;
    typename Set::container_type tmp(set.get_allocator());
    apache::thrift::detail::pm::reserve_if_possible(&tmp, set_size);
    {
      auto& elem0 = apache::thrift::detail::pm::emplace_back_default(tmp);
      Decode<Tag>{}(prot, elem0);
    }
    for (size_t i = 1; i < set_size; ++i) {
      auto& elem = apache::thrift::detail::pm::emplace_back_default(tmp);
      Decode<Tag>{}(prot, elem);
      sorted = sorted && set.key_comp()(tmp[i - 1], elem);
    }

    using folly::sorted_unique;
    set = sorted ? Set(sorted_unique, std::move(tmp)) : Set(std::move(tmp));
  }

  // Handles set without sorted_unique property
  template <typename Set, typename Protocol>
  static std::enable_if_t<
      !apache::thrift::detail::pm::sorted_unique_constructible_v<Set>>
  decode_known_length_set(Protocol& prot, Set& set, std::uint32_t set_size) {
    apache::thrift::detail::pm::reserve_if_possible(&set, set_size);

    for (auto i = set_size; i > 0; i--) {
      typename Set::value_type value =
          apache::thrift::detail::default_set_element(set);
      Decode<Tag>{}(prot, value);
      emplace_at_end(set, std::move(value));
    }
  }

 public:
  template <typename Protocol, typename SetType>
  void operator()(Protocol& prot, SetType& set) const {
    auto consumeElem = [&] {
      typename SetType::value_type value;
      Decode<Tag>{}(prot, value);
      emplace_at_end(set, std::move(value));
    };
    TType t;
    uint32_t s;
    prot.readSetBegin(t, s);
    set = SetType();
    if (prot.kOmitsContainerSizes()) {
      while (prot.peekSet()) {
        consumeElem();
      }
    } else if (typeTagToTType<Tag> == t) {
      decode_known_length_set(prot, set, s);
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
 private:
  // Handles map with sorted_unique property
  template <typename Map, typename Protocol>
  static std::enable_if_t<
      apache::thrift::detail::pm::sorted_unique_constructible_v<Map>>
  decode_known_length_map(Protocol& prot, Map& map, std::uint32_t map_size) {
    if (map_size == 0) {
      return;
    }

    bool sorted = true;
    typename Map::container_type tmp(map.get_allocator());
    apache::thrift::detail::pm::reserve_if_possible(&tmp, map_size);
    {
      auto& elem0 =
          apache::thrift::detail::pm::emplace_back_default_map(tmp, map);
      Decode<Key>{}(prot, elem0.first);
      Decode<Value>{}(prot, elem0.second);
    }
    for (size_t i = 1; i < map_size; ++i) {
      auto& elem =
          apache::thrift::detail::pm::emplace_back_default_map(tmp, map);
      Decode<Key>{}(prot, elem.first);
      Decode<Value>{}(prot, elem.second);
      sorted = sorted && map.key_comp()(tmp[i - 1].first, elem.first);
    }

    using folly::sorted_unique;
    map = sorted ? Map(sorted_unique, std::move(tmp)) : Map(std::move(tmp));
  }

  // Handles map without sorted_unique property.
  template <typename Map, typename Protocol>
  static std::enable_if_t<
      !apache::thrift::detail::pm::sorted_unique_constructible_v<Map>>
  decode_known_length_map(Protocol& prot, Map& map, std::uint32_t map_size) {
    apache::thrift::detail::pm::reserve_if_possible(&map, map_size);

    for (auto i = map_size; i--;) {
      typename Map::key_type key = apache::thrift::detail::default_map_key(map);
      typename Map::mapped_type value =
          apache::thrift::detail::default_map_value(map);
      Decode<Key>{}(prot, key);
      Decode<Value>{}(prot, value);
      emplace_at_end(map, std::move(key), std::move(value));
    }
  }

 public:
  template <typename Protocol, typename MapType>
  void operator()(Protocol& prot, MapType& map) const {
    auto consumeElem = [&] {
      typename MapType::key_type key;
      Decode<Key>{}(prot, key);
      auto& val =
          emplace_at_end(map, std::move(key), typename MapType::mapped_type{});
      Decode<Value>{}(prot, val);
    };

    TType keyType, valueType;
    uint32_t s;
    prot.readMapBegin(keyType, valueType, s);
    map = MapType();
    if (prot.kOmitsContainerSizes()) {
      while (prot.peekMap()) {
        consumeElem();
      }
    } else if (
        typeTagToTType<Key> == keyType && typeTagToTType<Value> == valueType) {
      decode_known_length_map(prot, map, s);
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
struct Decode<type::cpp_type<T, Tag>> : Decode<Tag> {
  template <class Protocol, class U>
  void operator()(Protocol& prot, U& m) const {
    if constexpr (kIsIntegral<U, Tag>) {
      type::native_type<Tag> i;
      Decode<Tag>::operator()(prot, i);
      m = static_cast<U>(i);
    } else {
      Decode<Tag>::operator()(prot, m);
    }
  }
};

template <typename Adapter, typename Tag, typename U>
void adapter_clear(U& m) {
  if (typeTagToTType<Tag> == TType::T_LIST ||
      typeTagToTType<Tag> == TType::T_SET ||
      typeTagToTType<Tag> == TType::T_MAP) {
    adapt_detail::clear<Adapter>(m);
  }
}

template <typename Adapter, typename Tag>
struct Decode<type::adapted<Adapter, Tag>> {
  template <typename Protocol, typename U>
  void operator()(Protocol& prot, U& m) const {
    return folly::overload(
        [&](auto adapter) -> decltype(decltype(adapter)::template decode<Tag>(
                              prot, m)) {
          adapter_clear<Adapter, Tag, U>(m);
          decltype(adapter)::template decode<Tag>(prot, m);
        },
        [&](auto...) {
          constexpr bool hasInplaceToThrift = ::apache::thrift::adapt_detail::
              has_inplace_toThrift<Adapter, folly::remove_cvref_t<U>>::value;
          if constexpr (hasInplaceToThrift) {
            adapter_clear<Adapter, Tag, U>(m);
            Decode<Tag>{}(prot, Adapter::toThrift(m));
          } else {
            type::native_type<Tag> orig;
            Decode<Tag>{}(prot, orig);
            m = Adapter::fromThrift(std::move(orig));
          }
        })(Adapter{});
  }
};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct Decode<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  static_assert(type::is_concrete_v<field_adapted_tag>, "");

  template <typename Protocol, typename U, typename AdapterT = Adapter>
  constexpr adapt_detail::
      if_not_field_adapter<AdapterT, type::native_type<Tag>, Struct, void>
      operator()(Protocol& prot, U& m, Struct&) const {
    Decode<type::adapted<Adapter, Tag>>{}(prot, m);
  }

  template <typename Protocol, typename U, typename AdapterT = Adapter>
  constexpr adapt_detail::
      if_field_adapter<AdapterT, FieldId, type::native_type<Tag>, Struct, void>
      operator()(Protocol& prot, U& m, Struct& strct) const {
    // TODO(dokwon): in-place deserialization support for field adapter.
    folly::overload(
        [&](auto adapter) -> decltype(decltype(adapter)::template decode<Tag>(
                              prot, m)) {
          adapter_clear<Adapter, Tag, U>(m);
          decltype(adapter)::template decode<Tag>(prot, m);
        },
        [&](...) {
          type::native_type<Tag> orig;
          Decode<Tag>{}(prot, orig);
          m = adapt_detail::fromThriftField<Adapter, FieldId>(
              std::move(orig), strct);
        })(Adapter{});
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
