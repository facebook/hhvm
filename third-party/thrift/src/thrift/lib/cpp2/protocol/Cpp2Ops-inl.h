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

#include <type_traits>

#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/io/IOBuf.h>

namespace apache {
namespace thrift {

namespace detail {

template <
    typename C,
    typename T = decltype(std::declval<C&>().push_back(
        std::declval<typename C::value_type>()))>
struct push_back_result {
  using type = T;
};

template <
    typename C,
    typename T = decltype(std::declval<C&>().insert(
        std::declval<typename C::key_type>()))>
struct insert_key_result {
  using type = T;
};

template <
    typename C,
    typename T =
        decltype(std::declval<C&>()[std::declval<typename C::key_type>()])>
struct subscript_key_result {
  using type = T;
};

template <
    typename C,
    typename T = decltype(std::declval<C&>().reserve(
        std::declval<typename C::size_type>()))>
struct reserve_result {
  using type = T;
};

template <typename C, typename = void>
struct Reserver {
  static void reserve(C&, typename C::size_type) {}
};

template <typename C>
struct Reserver<C, folly::void_t<typename reserve_result<C>::type>> {
  static void reserve(C& container, typename C::size_type size) {
    container.reserve(size);
  }
};

template <typename Cont, typename Elem, typename Enable = void>
inline constexpr bool alloc_is_recursive = false;
template <typename Cont, typename Elem>
inline constexpr bool alloc_is_recursive<
    Cont,
    Elem,
    folly::
        void_t<typename Cont::allocator_type, typename Elem::allocator_type>> =
    std::uses_allocator<Elem, typename Cont::allocator_type>::value;
template <typename Cont, typename Elem>
inline constexpr bool alloc_should_propagate = alloc_is_recursive<Cont, Elem> &&
    !std::uses_allocator<Cont, std::allocator<char>>::value;

template <typename Map>
inline constexpr bool alloc_should_propagate_map =
    alloc_should_propagate<Map, typename Map::key_type> ||
    alloc_should_propagate<Map, typename Map::mapped_type>;

template <typename C>
std::enable_if_t<
    alloc_should_propagate<C, typename C::value_type>,
    typename C::value_type>
default_set_element(C& c) {
  return typename C::value_type(c.get_allocator());
}
template <typename C>
std::enable_if_t<
    !alloc_should_propagate<C, typename C::value_type>,
    typename C::value_type>
default_set_element(C&) {
  return typename C::value_type{};
}
template <typename Map>
std::enable_if_t<
    alloc_should_propagate<Map, typename Map::key_type>,
    typename Map::key_type>
default_map_key(Map& m) {
  return typename Map::key_type(m.get_allocator());
}
template <typename Map>
std::enable_if_t<
    !alloc_should_propagate<Map, typename Map::key_type>,
    typename Map::key_type>
default_map_key(Map&) {
  return typename Map::key_type{};
}
template <typename C>
std::enable_if_t<
    alloc_should_propagate<C, typename C::mapped_type>,
    typename C::mapped_type>
default_map_value(C& map) {
  return typename C::mapped_type(map.get_allocator());
}
template <typename C>
std::enable_if_t<
    !alloc_should_propagate<C, typename C::mapped_type>,
    typename C::mapped_type>
default_map_value(C&) {
  return typename C::mapped_type{};
}

} // namespace detail

template <>
class Cpp2Ops<folly::fbstring> {
 public:
  typedef folly::fbstring Type;
  static constexpr protocol::TType thriftType() { return protocol::T_STRING; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeString(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readString(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeString(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeString(*value);
  }
};

template <>
class Cpp2Ops<std::string> {
 public:
  typedef std::string Type;
  static constexpr protocol::TType thriftType() { return protocol::T_STRING; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeString(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readString(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeString(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeString(*value);
  }
};

template <>
class Cpp2Ops<int8_t> {
 public:
  typedef int8_t Type;
  static constexpr protocol::TType thriftType() { return protocol::T_BYTE; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeByte(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readByte(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeByte(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeByte(*value);
  }
};

template <>
class Cpp2Ops<int16_t> {
 public:
  typedef int16_t Type;
  static constexpr protocol::TType thriftType() { return protocol::T_I16; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI16(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readI16(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI16(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI16(*value);
  }
};

template <>
class Cpp2Ops<int32_t> {
 public:
  typedef int32_t Type;
  static constexpr protocol::TType thriftType() { return protocol::T_I32; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI32(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readI32(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(*value);
  }
};

template <>
class Cpp2Ops<int64_t> {
 public:
  typedef int64_t Type;
  static constexpr protocol::TType thriftType() { return protocol::T_I64; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI64(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readI64(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI64(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI64(*value);
  }
};

template <>
class Cpp2Ops<uint8_t> {
 public:
  using Type = uint8_t;
  using SignedType = std::make_signed_t<Type>;
  static constexpr protocol::TType thriftType() { return protocol::T_BYTE; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeByte(folly::to_signed(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    SignedType signedValue;
    prot->readByte(signedValue);
    *value = folly::to_unsigned(signedValue);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeByte(folly::to_signed(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeByte(folly::to_signed(*value));
  }
};

template <>
class Cpp2Ops<uint16_t> {
 public:
  using Type = uint16_t;
  using SignedType = std::make_signed_t<Type>;
  static constexpr protocol::TType thriftType() { return protocol::T_I16; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI16(folly::to_signed(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    SignedType signedValue;
    prot->readI16(signedValue);
    *value = folly::to_unsigned(signedValue);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI16(folly::to_signed(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI16(folly::to_signed(*value));
  }
};

template <>
class Cpp2Ops<uint32_t> {
 public:
  using Type = uint32_t;
  using SignedType = std::make_signed_t<Type>;
  static constexpr protocol::TType thriftType() { return protocol::T_I32; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI32(folly::to_signed(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    SignedType signedValue;
    prot->readI32(signedValue);
    *value = folly::to_unsigned(signedValue);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(folly::to_signed(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(folly::to_signed(*value));
  }
};

template <>
class Cpp2Ops<uint64_t> {
 public:
  using Type = uint64_t;
  using SignedType = std::make_signed_t<Type>;
  static constexpr protocol::TType thriftType() { return protocol::T_I64; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI64(folly::to_signed(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    SignedType signedValue;
    prot->readI64(signedValue);
    *value = folly::to_unsigned(signedValue);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI64(folly::to_signed(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI64(folly::to_signed(*value));
  }
};

template <>
class Cpp2Ops<bool> {
 public:
  typedef bool Type;
  static constexpr protocol::TType thriftType() { return protocol::T_BOOL; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeBool(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readBool(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeBool(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeBool(*value);
  }
};

template <>
class Cpp2Ops<double> {
 public:
  typedef double Type;
  static constexpr protocol::TType thriftType() { return protocol::T_DOUBLE; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeDouble(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readDouble(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeDouble(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeDouble(*value);
  }
};

template <class E>
class Cpp2Ops<E, typename std::enable_if<std::is_enum<E>::value>::type> {
 public:
  typedef E Type;
  static constexpr protocol::TType thriftType() { return protocol::T_I32; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeI32(static_cast<int32_t>(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    int32_t val;
    prot->readI32(val);
    *value = static_cast<Type>(val);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(static_cast<int32_t>(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeI32(static_cast<int32_t>(*value));
  }
};

template <>
class Cpp2Ops<float> {
 public:
  typedef float Type;
  static constexpr protocol::TType thriftType() { return protocol::T_FLOAT; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeFloat(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readFloat(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeFloat(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeFloat(*value);
  }
};

namespace detail {

template <class Protocol, class V>
void readIntoVector(Protocol* prot, V& vec) {
  typedef typename V::value_type ElemType;
  for (auto& e : vec) {
    Cpp2Ops<ElemType>::read(prot, &e);
  }
}

template <class Protocol>
void readIntoVector(Protocol* prot, std::vector<bool>& vec) {
  for (auto e : vec) {
    // e is a proxy object because the elements don't have distinct addresses
    // (packed into a bitvector). We actually copy the proxy during iteration
    // (can't use non-const reference because iteration returns by value, can't
    // use const reference because we modify it), but it still points to the
    // actual element.
    bool b;
    Cpp2Ops<bool>::read(prot, &b);
    e = b;
  }
}

} // namespace detail

template <class L>
class Cpp2Ops<
    L,
    folly::void_t<typename apache::thrift::detail::push_back_result<L>::type>> {
 private:
  // Need a resize func instead of c.resize(size, defaultElement(C)), because
  // some non-standard vectors do not support resize(size_type, T value = T()).
  template <typename C = L>
  std::enable_if_t<
      detail::alloc_should_propagate<C, typename C::value_type>,
      void> static resize(C& c, uint32_t size) {
    c.resize(size, typename C::value_type(c.get_allocator()));
  }
  template <typename C = L>
  std::enable_if_t<
      !detail::alloc_should_propagate<C, typename C::value_type>,
      void> static resize(C& c, uint32_t size) {
    c.resize(size);
  }

 public:
  typedef L Type;
  static constexpr protocol::TType thriftType() { return protocol::T_LIST; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    typedef typename Type::value_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->writeListBegin(
        Cpp2Ops<ElemType>::thriftType(), folly::to_narrow(value->size()));
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::write(prot, &e);
    }
    xfer += prot->writeListEnd();
    return xfer;
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    value->clear();
    uint32_t size;
    protocol::TType etype;
    prot->readListBegin(etype, size);
    resize(*value, size);
    apache::thrift::detail::readIntoVector(prot, *value);
    prot->readListEnd();
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    typedef typename Type::value_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeListBegin(
        Cpp2Ops<ElemType>::thriftType(), value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::serializedSize(prot, &e);
    }
    xfer += prot->serializedSizeListEnd();
    return xfer;
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    typedef typename Type::value_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeListBegin(
        Cpp2Ops<ElemType>::thriftType(), folly::to_narrow(value->size()));
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::serializedSizeZC(prot, &e);
    }
    xfer += prot->serializedSizeListEnd();
    return xfer;
  }
};

template <class S>
class Cpp2Ops<
    S,
    folly::void_t<
        typename apache::thrift::detail::insert_key_result<S>::type>> {
 private:
  template <typename C = S>
  std::enable_if_t<
      detail::alloc_should_propagate<C, typename C::key_type>,
      typename C::key_type> static defaultElement(C& c) {
    return typename C::key_type(c.get_allocator());
  }
  template <typename C = S>
  std::enable_if_t<
      !detail::alloc_should_propagate<C, typename C::key_type>,
      typename C::key_type> static defaultElement(C&) {
    return typename C::key_type{};
  }

 public:
  typedef S Type;
  static constexpr protocol::TType thriftType() { return protocol::T_SET; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    typedef typename Type::key_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->writeSetBegin(Cpp2Ops<ElemType>::thriftType(), value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::write(prot, &e);
    }
    xfer += prot->writeSetEnd();
    return xfer;
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    typedef typename Type::key_type ElemType;
    value->clear();
    uint32_t size;
    protocol::TType etype;
    prot->readSetBegin(etype, size);
    apache::thrift::detail::Reserver<Type>::reserve(*value, size);
    for (uint32_t i = 0; i < size; i++) {
      ElemType elem = defaultElement(*value);
      Cpp2Ops<ElemType>::read(prot, &elem);
      value->insert(std::move(elem));
    }
    prot->readSetEnd();
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    typedef typename Type::key_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeSetBegin(
        Cpp2Ops<ElemType>::thriftType(), value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::serializedSize(prot, &e);
    }
    xfer += prot->serializedSizeSetEnd();
    return xfer;
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    typedef typename Type::key_type ElemType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeSetBegin(
        Cpp2Ops<ElemType>::thriftType(), value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<ElemType>::serializedSizeZC(prot, &e);
    }
    xfer += prot->serializedSizeSetEnd();
    return xfer;
  }
};

template <class M>
class Cpp2Ops<
    M,
    folly::void_t<
        typename apache::thrift::detail::subscript_key_result<M>::type>> {
 private:
  template <typename Map, typename ValueType>
  std::enable_if_t<
      detail::alloc_should_propagate_map<Map>,
      ValueType&> static emplaceKey(Map& m, typename Map::key_type&& key) {
    return m.emplace(std::move(key), detail::default_map_value(m))
        .first->second;
  }
  template <typename Map, typename ValueType>
  std::enable_if_t<
      !detail::alloc_should_propagate_map<Map>,
      ValueType&> static emplaceKey(Map& m, typename Map::key_type&& key) {
    return m[std::move(key)];
  }

 public:
  typedef M Type;
  static constexpr protocol::TType thriftType() { return protocol::T_MAP; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    typedef typename Type::key_type KeyType;
    typedef typename std::remove_cv<
        typename std::remove_reference<decltype(*value->begin())>::type>::type
        PairType;
    typedef typename PairType::second_type ValueType;
    uint32_t xfer = 0;
    xfer += prot->writeMapBegin(
        Cpp2Ops<KeyType>::thriftType(),
        Cpp2Ops<ValueType>::thriftType(),
        value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<KeyType>::write(prot, &e.first);
      xfer += Cpp2Ops<ValueType>::write(prot, &e.second);
    }
    xfer += prot->writeMapEnd();
    return xfer;
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    typedef typename Type::key_type KeyType;
    // We do this dance with decltype rather than just using Type::mapped_type
    // because different map implementations (such as Google's dense_hash_map)
    // call it data_type.
    typedef typename std::remove_cv<
        typename std::remove_reference<decltype(*value->begin())>::type>::type
        PairType;
    typedef typename PairType::second_type ValueType;
    value->clear();
    uint32_t size;
    protocol::TType keytype, valuetype;
    prot->readMapBegin(keytype, valuetype, size);
    apache::thrift::detail::Reserver<Type>::reserve(*value, size);
    for (uint32_t i = 0; i < size; i++) {
      KeyType key = detail::default_map_key(*value);
      Cpp2Ops<KeyType>::read(prot, &key);
      ValueType& val = emplaceKey<Type, ValueType>(*value, std::move(key));
      Cpp2Ops<ValueType>::read(prot, &val);
    }
    prot->readMapEnd();
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    typedef typename Type::key_type KeyType;
    typedef typename std::remove_cv<
        typename std::remove_reference<decltype(*value->begin())>::type>::type
        PairType;
    typedef typename PairType::second_type ValueType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeMapBegin(
        Cpp2Ops<KeyType>::thriftType(),
        Cpp2Ops<ValueType>::thriftType(),
        value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<KeyType>::serializedSize(prot, &e.first);
      xfer += Cpp2Ops<ValueType>::serializedSize(prot, &e.second);
    }
    xfer += prot->serializedSizeMapEnd();
    return xfer;
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    typedef typename Type::key_type KeyType;
    typedef typename std::remove_cv<
        typename std::remove_reference<decltype(*value->begin())>::type>::type
        PairType;
    typedef typename PairType::second_type ValueType;
    uint32_t xfer = 0;
    xfer += prot->serializedSizeMapBegin(
        Cpp2Ops<KeyType>::thriftType(),
        Cpp2Ops<ValueType>::thriftType(),
        value->size());
    for (const auto& e : *value) {
      xfer += Cpp2Ops<KeyType>::serializedSizeZC(prot, &e.first);
      xfer += Cpp2Ops<ValueType>::serializedSizeZC(prot, &e.second);
    }
    xfer += prot->serializedSizeMapEnd();
    return xfer;
  }
};

template <>
class Cpp2Ops<folly::IOBuf> {
 public:
  typedef folly::IOBuf Type;
  static constexpr protocol::TType thriftType() { return protocol::T_STRING; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeBinary(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeZCBinary(*value);
  }
};

template <>
class Cpp2Ops<std::unique_ptr<folly::IOBuf>> {
 public:
  typedef std::unique_ptr<folly::IOBuf> Type;
  static constexpr protocol::TType thriftType() { return protocol::T_STRING; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return prot->writeBinary(*value);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    prot->readBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return prot->serializedSizeBinary(*value);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return prot->serializedSizeZCBinary(*value);
  }
};

template <class T>
class Cpp2Ops<
    T,
    std::enable_if_t<
        is_thrift_class_v<T> &&
        !folly::is_detected_v<detect_indirection_fn_t, T>>> {
 public:
  typedef T Type;
  static constexpr protocol::TType thriftType() { return protocol::T_STRUCT; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return value->write(prot);
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    value->readNoXfer(prot);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return value->serializedSize(prot);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return value->serializedSizeZC(prot);
  }
};

template <class T>
class Cpp2Ops<
    T,
    std::enable_if_t<folly::is_detected_v<detect_indirection_fn_t, T>>> {
 private:
  using S = folly::remove_cvref_t<
      folly::invoke_result_t<detail::apply_indirection_fn, T>>;

 public:
  using Type = T;
  static constexpr protocol::TType thriftType() {
    return Cpp2Ops<S>::thriftType();
  }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return Cpp2Ops<S>::write(prot, &apply_indirection(*value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    return Cpp2Ops<S>::read(prot, &apply_indirection(*value));
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return Cpp2Ops<S>::serializedSize(prot, &apply_indirection(*value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return Cpp2Ops<S>::serializedSizeZC(prot, &apply_indirection(*value));
  }
};

} // namespace thrift
} // namespace apache
