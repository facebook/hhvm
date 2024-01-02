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

#include <algorithm>
#include <bitset>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

#include <folly/Conv.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/View.h>
#include <folly/functional/Invoke.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderWireTypeInfo.h>
#include <thrift/lib/cpp2/protocol/Traits.h>

/**
 * Specializations of `protocol_methods` encapsulate a collection of
 * read/write/size/sizeZC methods that can be performed on Thrift
 * objects and primitives. TypeClass (see apache::thrift::type_class)
 * refers to the general type of data that Type is, and is passed around for
 * two reasons:
 *  - to provide support for generic containers which have a common interface
 *    for building collections (e.g. a `std::vector<int>` and `std::deque<int>`,
 *    which can back a Thrift list, and thus have
 *    `type_class::list<type_class::integral>`, or an
 *    `std::map<std::string, MyStruct>` would have
 *    `type_class::map<type_class::string, type_class::structure>``).
 *  - to differentiate between Thrift types that are represented with the
 *    same C++ type, e.g. both Thrift binaries and strings are represented
 *    with an `std::string`, TypeClass is used to decide which protocol
 *    method to call.
 *
 * Example:
 *
 * // MyModule.thrift:
 * struct MyStruct {
 *   1: list<set<string>> fieldA
 * }
 *
 * // C++
 *
 * using methods = protocol_methods<
 *    type_class::list<type_class::set<type_class::string>>,
 *    std::vector<std::set<std::string>>>
 *
 * MyStruct struct_instance;
 * CompactProtocolReader reader;
 * methods::read(struct_instance.fieldA, reader);
 */

namespace apache {
namespace thrift {

namespace type {
namespace detail {
template <typename, typename>
class Wrap;
}
} // namespace type

namespace detail {
namespace pm {

template <typename C, typename... A>
using detect_reserve = decltype(FOLLY_DECLVAL(C).reserve(FOLLY_DECLVAL(A)...));

template <typename Container, typename Size>
auto reserve_if_possible(Container* t, Size size) {
  if constexpr (folly::is_detected_v<detect_reserve, Container&, Size>) {
    t->reserve(size);
    return std::true_type{};
  } else {
    return std::false_type{};
  }
}

template <typename Container>
typename Container::reference emplace_back_default(Container& c) {
  return c.emplace_back(detail::default_set_element(c));
}

template <typename Container, typename Map>
typename Container::reference emplace_back_default_map(Container& c, Map& m) {
  return c.emplace_back(
      detail::default_map_key(m), detail::default_map_value(m));
}

template <typename Map, typename KeyDeserializer, typename MappedDeserializer>
std::enable_if_t<detail::alloc_should_propagate_map<Map>>
deserialize_key_val_into_map(
    Map& m, const KeyDeserializer& kr, const MappedDeserializer& mr) {
  typename Map::key_type key = detail::default_map_key(m);
  typename Map::mapped_type value = detail::default_map_value(m);
  kr(key);
  mr(value);
  m.emplace(std::move(key), std::move(value));
}

template <typename Map, typename KeyDeserializer, typename MappedDeserializer>
std::enable_if_t<!detail::alloc_should_propagate_map<Map>>
deserialize_key_val_into_map(
    Map& m, const KeyDeserializer& kr, const MappedDeserializer& mr) {
  typename Map::key_type key; // Create key/val without allocator awareness.
  kr(key);
  mr(m[std::move(key)]);
}

template <typename Void, typename T>
inline constexpr bool sorted_unique_constructible_ = false;
template <typename T>
inline constexpr bool sorted_unique_constructible_<
    folly::void_t<
        decltype(T(folly::sorted_unique, typename T::container_type())),
        decltype(T(typename T::container_type()))>,
    T> = true;
template <typename T>
inline constexpr bool sorted_unique_constructible_v =
    sorted_unique_constructible_<void, T>;

FOLLY_CREATE_MEMBER_INVOKER(emplace_hint_invoker, emplace_hint);

template <typename T>
using detect_key_compare = typename T::key_compare;

template <typename T>
constexpr bool map_emplace_hint_is_invocable_v = folly::is_invocable_v<
    emplace_hint_invoker,
    T,
    typename T::iterator,
    typename T::key_type,
    typename T::mapped_type>;

template <typename T>
constexpr bool set_emplace_hint_is_invocable_v = folly::is_invocable_v<
    emplace_hint_invoker,
    T,
    typename T::iterator,
    typename T::value_type>;

template <typename Map, typename KeyDeserializer, typename MappedDeserializer>
typename std::enable_if<sorted_unique_constructible_v<Map>>::type
deserialize_known_length_map(
    Map& map,
    std::uint32_t map_size,
    const KeyDeserializer& kr,
    const MappedDeserializer& mr) {
  if (map_size == 0) {
    return;
  }

  bool sorted = true;
  typename Map::container_type tmp(map.get_allocator());
  reserve_if_possible(&tmp, map_size);
  {
    decltype(auto) elem0 = emplace_back_default_map(tmp, map);
    kr(elem0.first);
    mr(elem0.second);
  }
  for (size_t i = 1; i < map_size; ++i) {
    decltype(auto) elem = emplace_back_default_map(tmp, map);
    kr(elem.first);
    mr(elem.second);
    sorted = sorted && map.key_comp()(tmp[i - 1].first, elem.first);
  }

  using folly::sorted_unique;
  map = sorted ? Map(sorted_unique, std::move(tmp)) : Map(std::move(tmp));
}

template <typename Map, typename KeyDeserializer, typename MappedDeserializer>
typename std::enable_if<
    !sorted_unique_constructible_v<Map> &&
    map_emplace_hint_is_invocable_v<Map>>::type
deserialize_known_length_map(
    Map& map,
    std::uint32_t map_size,
    const KeyDeserializer& kr,
    const MappedDeserializer& mr) {
  reserve_if_possible(&map, map_size);

  for (auto i = map_size; i--;) {
    typename Map::key_type key = detail::default_map_key(map);
    typename Map::mapped_type value = detail::default_map_value(map);
    kr(key);
    mr(value);
    map.emplace_hint(map.end(), std::move(key), std::move(value));
  }
}

template <typename Map, typename KeyDeserializer, typename MappedDeserializer>
typename std::enable_if<
    !sorted_unique_constructible_v<Map> &&
    !map_emplace_hint_is_invocable_v<Map>>::type
deserialize_known_length_map(
    Map& map,
    std::uint32_t map_size,
    const KeyDeserializer& kr,
    const MappedDeserializer& mr) {
  reserve_if_possible(&map, map_size);

  for (auto i = map_size; i--;) {
    deserialize_key_val_into_map(map, kr, mr);
  }
}

template <typename Set, typename ValDeserializer>
typename std::enable_if<sorted_unique_constructible_v<Set>>::type
deserialize_known_length_set(
    Set& set, std::uint32_t set_size, const ValDeserializer& vr) {
  if (set_size == 0) {
    return;
  }

  bool sorted = true;
  typename Set::container_type tmp(set.get_allocator());
  reserve_if_possible(&tmp, set_size);
  {
    auto& elem0 = emplace_back_default(tmp);
    vr(elem0);
  }
  for (size_t i = 1; i < set_size; ++i) {
    auto& elem = emplace_back_default(tmp);
    vr(elem);
    sorted = sorted && set.key_comp()(tmp[i - 1], elem);
  }

  using folly::sorted_unique;
  set = sorted ? Set(sorted_unique, std::move(tmp)) : Set(std::move(tmp));
}

template <typename Set, typename ValDeserializer>
typename std::enable_if<
    !sorted_unique_constructible_v<Set> &&
    set_emplace_hint_is_invocable_v<Set>>::type
deserialize_known_length_set(
    Set& set, std::uint32_t set_size, const ValDeserializer& vr) {
  reserve_if_possible(&set, set_size);

  for (auto i = set_size; i--;) {
    typename Set::value_type value = detail::default_set_element(set);
    vr(value);
    set.emplace_hint(set.end(), std::move(value));
  }
}

template <typename Set, typename ValDeserializer>
typename std::enable_if<
    !sorted_unique_constructible_v<Set> &&
    !set_emplace_hint_is_invocable_v<Set>>::type
deserialize_known_length_set(
    Set& set, std::uint32_t set_size, const ValDeserializer& vr) {
  reserve_if_possible(&set, set_size);

  for (auto i = set_size; i--;) {
    typename Set::value_type value = detail::default_set_element(set);
    vr(value);
    set.insert(std::move(value));
  }
}

inline uint32_t checked_container_size(size_t size) {
  const size_t limit = std::numeric_limits<int32_t>::max();
  if (size > limit) {
    TProtocolException::throwExceededSizeLimit(size, limit);
  }
  return static_cast<uint32_t>(size);
}

/*
 * Primitive Types Specialization
 */
template <typename TypeClass, typename Type, typename Enable = void>
struct protocol_methods;

#define THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON(Class, Type, Method)      \
  template <typename Protocol>                                               \
  static void read(Protocol& protocol, Type& out) {                          \
    protocol.read##Method(out);                                              \
  }                                                                          \
  template <typename Protocol, typename Context>                             \
  static void readWithContext(Protocol& protocol, Type& out, Context& ctx) { \
    if constexpr (Context::kAcceptsContext) {                                \
      protocol.read##Method##WithContext(out, ctx);                          \
    } else {                                                                 \
      protocol.read##Method(out);                                            \
    }                                                                        \
  }                                                                          \
  template <typename Protocol>                                               \
  static std::size_t write(Protocol& protocol, const Type& in) {             \
    if (std::is_same<type_class::Class, type_class::binary>::value ||        \
        std::is_same<type_class::Class, type_class::string>::value) {        \
      return checked_container_size(protocol.write##Method(in));             \
    } else {                                                                 \
      return protocol.write##Method(in);                                     \
    }                                                                        \
  }

#define THRIFT_PROTOCOL_METHODS_REGISTER_SS_COMMON(Class, Type, Method)   \
  template <bool, typename Protocol>                                      \
  static std::size_t serializedSize(Protocol& protocol, const Type& in) { \
    return protocol.serializedSize##Method(in);                           \
  }

// stamp out specializations for primitive types
#define THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(Class, Type, Method) \
  template <>                                                          \
  struct protocol_methods<type_class::Class, Type> {                   \
    THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON(Class, Type, Method)    \
    THRIFT_PROTOCOL_METHODS_REGISTER_SS_COMMON(Class, Type, Method)    \
  }

THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(integral, std::int8_t, Byte);
THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(integral, std::int16_t, I16);
THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(integral, std::int32_t, I32);
THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(integral, std::int64_t, I64);

// Macros for defining protocol_methods for unsigned integers
// Need special macros due to the casts needed
#define THRIFT_PROTOCOL_METHODS_REGISTER_RW_UI(Class, Type, Method)          \
  using SignedType = std::make_signed_t<Type>;                               \
  template <typename Protocol>                                               \
  static void read(Protocol& protocol, Type& out) {                          \
    SignedType tmp;                                                          \
    protocol.read##Method(tmp);                                              \
    out = folly::to_unsigned(tmp);                                           \
  }                                                                          \
  template <typename Protocol, typename Context>                             \
  static void readWithContext(Protocol& protocol, Type& out, Context& ctx) { \
    SignedType tmp;                                                          \
    if constexpr (Context::kAcceptsContext) {                                \
      protocol.read##Method##WithContext(tmp, ctx);                          \
    } else {                                                                 \
      protocol.read##Method(tmp);                                            \
    }                                                                        \
    out = folly::to_unsigned(tmp);                                           \
  }                                                                          \
  template <typename Protocol>                                               \
  static std::size_t write(Protocol& protocol, const Type& in) {             \
    return protocol.write##Method(folly::to_signed(in));                     \
  }

#define THRIFT_PROTOCOL_METHODS_REGISTER_SS_UI(Class, Type, Method)       \
  template <bool, typename Protocol>                                      \
  static std::size_t serializedSize(Protocol& protocol, const Type& in) { \
    return protocol.serializedSize##Method(folly::to_signed(in));         \
  }

// stamp out specializations for unsigned integer primitive types
#define THRIFT_PROTOCOL_METHODS_REGISTER_UI(Class, Type, Method) \
  template <>                                                    \
  struct protocol_methods<type_class::Class, Type> {             \
    THRIFT_PROTOCOL_METHODS_REGISTER_RW_UI(Class, Type, Method)  \
    THRIFT_PROTOCOL_METHODS_REGISTER_SS_UI(Class, Type, Method)  \
  }

THRIFT_PROTOCOL_METHODS_REGISTER_UI(integral, std::uint8_t, Byte);
THRIFT_PROTOCOL_METHODS_REGISTER_UI(integral, std::uint16_t, I16);
THRIFT_PROTOCOL_METHODS_REGISTER_UI(integral, std::uint32_t, I32);
THRIFT_PROTOCOL_METHODS_REGISTER_UI(integral, std::uint64_t, I64);

#undef THRIFT_PROTOCOL_METHODS_REGISTER_UI
#undef THRIFT_PROTOCOL_METHODS_REGISTER_RW_UI
#undef THRIFT_PROTOCOL_METHODS_REGISTER_SS_UI

// std::vector<bool> isn't actually a container, so
// define a special overload which takes its specialized
// proxy type
template <>
struct protocol_methods<type_class::integral, bool> {
  THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON(integral, bool, Bool)
  THRIFT_PROTOCOL_METHODS_REGISTER_SS_COMMON(integral, bool, Bool)

  template <typename Protocol>
  static void read(Protocol& protocol, std::vector<bool>::reference out) {
    bool tmp;
    read(protocol, tmp);
    out = tmp;
  }
};

THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(floating_point, double, Double);
THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD(floating_point, float, Float);

#undef THRIFT_PROTOCOL_METHODS_REGISTER_OVERLOAD

template <typename Type>
struct protocol_methods<type_class::string, Type> {
  THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON(string, Type, String)
  THRIFT_PROTOCOL_METHODS_REGISTER_SS_COMMON(string, Type, String)
};

template <typename Type>
struct protocol_methods<type_class::binary, Type> {
  THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON(binary, Type, Binary)

  template <bool ZeroCopy, typename Protocol>
  static typename std::enable_if<ZeroCopy, std::size_t>::type serializedSize(
      Protocol& protocol, const Type& in) {
    return protocol.serializedSizeZCBinary(in);
  }
  template <bool ZeroCopy, typename Protocol>
  static typename std::enable_if<!ZeroCopy, std::size_t>::type serializedSize(
      Protocol& protocol, const Type& in) {
    return protocol.serializedSizeBinary(in);
  }
};

#undef THRIFT_PROTOCOL_METHODS_REGISTER_SS_COMMON
#undef THRIFT_PROTOCOL_METHODS_REGISTER_RW_COMMON

/*
 * Enum Specialization
 */

template <typename Type, typename int_type = std::underlying_type_t<Type>>
struct enum_protocol_methods {
  static_assert(std::is_enum<Type>::value, "must be enum");
  using int_methods = protocol_methods<type_class::integral, int_type>;

  template <typename Protocol>
  static void read(Protocol& protocol, Type& out) {
    int_type tmp;
    int_methods::read(protocol, tmp);
    out = static_cast<Type>(tmp);
  }

  template <typename Protocol, typename Context>
  static void readWithContext(Protocol& protocol, Type& out, Context& ctx) {
    int_type tmp;
    int_methods::readWithContext(protocol, tmp, ctx);
    out = static_cast<Type>(tmp);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const Type& in) {
    int_type tmp = static_cast<int_type>(in);
    return int_methods::template write<Protocol>(protocol, tmp);
  }

  template <bool ZeroCopy, typename Protocol>
  static std::size_t serializedSize(Protocol& protocol, const Type& in) {
    int_type tmp = static_cast<int_type>(in);
    return int_methods::template serializedSize<ZeroCopy>(protocol, tmp);
  }
};

// Thrift enums are always read as int32_t
template <typename Type>
struct protocol_methods<type_class::enumeration, Type>
    : enum_protocol_methods<Type, std::int32_t> {};

// Strong integral types keep their precision.
template <typename Type>
struct protocol_methods<
    type_class::integral,
    Type,
    std::enable_if_t<std::is_enum<Type>::value>> : enum_protocol_methods<Type> {
};

/*
 * List Specialization
 */
template <typename ElemClass, typename Type>
struct protocol_methods<type_class::list<ElemClass>, Type> {
  static_assert(
      !std::is_same<ElemClass, type_class::unknown>(),
      "Unable to serialize unknown list element");

  using elem_type = folly::remove_cvref_t<typename Type::value_type>;
  using elem_methods = protocol_methods<ElemClass, elem_type>;
  using elem_ttype = protocol_type<ElemClass, elem_type>;

 private:
  template <typename Protocol>
  FOLLY_ERASE static void read_one(Protocol& protocol, Type& out) {
    if constexpr ( //
        std::is_const_v<std::remove_reference_t<typename Type::reference>>) {
      out.emplace_back(folly::invocable_to([&] {
        elem_type elem;
        elem_methods::read(protocol, elem);
        return elem;
      }));
    } else {
      elem_methods::read(protocol, emplace_back_default(out));
    }
  }

 public:
  template <typename Protocol>
  static void read(Protocol& protocol, Type& out) {
    std::uint32_t list_size = -1;
    using WireTypeInfo = ProtocolReaderWireTypeInfo<Protocol>;
    using WireType = typename WireTypeInfo::WireType;

    WireType reported_type = WireTypeInfo::defaultValue();

    protocol.readListBegin(reported_type, list_size);
    if (protocol.kOmitsContainerSizes()) {
      // list size unknown, SimpleJSON protocol won't know type, either
      // so let's just hope that it spits out something that makes sense
      while (protocol.peekList()) {
        read_one(protocol, out);
      }
    } else {
      if (reported_type != WireTypeInfo::fromTType(elem_ttype::value)) {
        apache::thrift::skip_n(protocol, list_size, {reported_type});
      } else {
        if (!canReadNElements(protocol, list_size, {reported_type})) {
          protocol::TProtocolException::throwTruncatedData();
        }

        reserve_if_possible(&out, list_size);
        while (list_size--) {
          read_one(protocol, out);
        }
      }
    }
    protocol.readListEnd();
  }

  template <typename Protocol, typename Context>
  static void readWithContext(Protocol& protocol, Type& out, Context&) {
    read(protocol, out);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const Type& out) {
    std::size_t xfer = 0;

    xfer += protocol.writeListBegin(
        elem_ttype::value, checked_container_size(out.size()));

    for (const auto& elem : out) {
      xfer += elem_methods::write(protocol, elem);
    }
    xfer += protocol.writeListEnd();
    return xfer;
  }

  template <bool ZeroCopy, typename Protocol>
  static std::size_t serializedSize(Protocol& protocol, const Type& out) {
    std::size_t xfer = 0;

    xfer += protocol.serializedSizeListBegin(
        elem_ttype::value, folly::to_narrow(folly::to_unsigned(out.size())));
    for (const auto& elem : out) {
      xfer += elem_methods::template serializedSize<ZeroCopy>(protocol, elem);
    }
    xfer += protocol.serializedSizeListEnd();
    return xfer;
  }
};

/*
 * Set Specialization
 */
template <typename ElemClass, typename Type>
struct protocol_methods<type_class::set<ElemClass>, Type> {
  static_assert(
      !std::is_same<ElemClass, type_class::unknown>(),
      "Unable to serialize unknown type");

  using elem_type = typename Type::value_type;
  using elem_methods = protocol_methods<ElemClass, elem_type>;
  using elem_ttype = protocol_type<ElemClass, elem_type>;

 private:
  template <typename Protocol>
  static void consume_elem(Protocol& protocol, Type& out) {
    elem_type tmp;
    elem_methods::read(protocol, tmp);
    out.insert(std::move(tmp));
  }

 public:
  template <typename Protocol>
  static void read(Protocol& protocol, Type& out) {
    std::uint32_t set_size = -1;

    using WireTypeInfo = ProtocolReaderWireTypeInfo<Protocol>;
    using WireType = typename WireTypeInfo::WireType;

    WireType reported_type = WireTypeInfo::defaultValue();

    protocol.readSetBegin(reported_type, set_size);
    if (protocol.kOmitsContainerSizes()) {
      while (protocol.peekSet()) {
        consume_elem(protocol, out);
      }
    } else {
      if (reported_type != WireTypeInfo::fromTType(elem_ttype::value)) {
        apache::thrift::skip_n(protocol, set_size, {reported_type});
      } else {
        if (!canReadNElements(protocol, set_size, {reported_type})) {
          protocol::TProtocolException::throwTruncatedData();
        }
        const auto vreader = [&protocol](auto& value) {
          elem_methods::read(protocol, value);
        };
        deserialize_known_length_set(out, set_size, vreader);
      }
    }
    protocol.readSetEnd();
  }

  template <typename Protocol, typename Context>
  static void readWithContext(Protocol& protocol, Type& out, Context&) {
    read(protocol, out);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const Type& out) {
    std::size_t xfer = 0;

    xfer += protocol.writeSetBegin(
        elem_ttype::value, checked_container_size(out.size()));

    if (!folly::is_detected_v<detect_key_compare, Type> &&
        protocol.kSortKeys()) {
      std::vector<typename Type::const_iterator> iters;
      iters.reserve(out.size());
      for (auto it = out.begin(); it != out.end(); ++it) {
        iters.push_back(it);
      }
      std::sort(
          iters.begin(), iters.end(), [](auto a, auto b) { return *a < *b; });
      for (auto it : iters) {
        xfer += elem_methods::write(protocol, *it);
      }
    } else {
      // Support containers with defined but non-FIFO iteration order.
      auto get_view = folly::order_preserving_reinsertion_view_or_default;
      for (const auto& elem : get_view(out)) {
        xfer += elem_methods::write(protocol, elem);
      }
    }
    xfer += protocol.writeSetEnd();
    return xfer;
  }

  template <bool ZeroCopy, typename Protocol>
  static std::size_t serializedSize(Protocol& protocol, const Type& out) {
    std::size_t xfer = 0;

    xfer += protocol.serializedSizeSetBegin(
        elem_ttype::value, folly::to_narrow(folly::to_unsigned(out.size())));
    for (const auto& elem : out) {
      xfer += elem_methods::template serializedSize<ZeroCopy>(protocol, elem);
    }
    xfer += protocol.serializedSizeSetEnd();
    return xfer;
  }
};

/*
 * Map Specialization
 */
template <typename KeyClass, typename MappedClass, typename Type>
struct protocol_methods<type_class::map<KeyClass, MappedClass>, Type> {
  static_assert(
      !std::is_same<KeyClass, type_class::unknown>(),
      "Unable to serialize unknown key type in map");
  static_assert(
      !std::is_same<MappedClass, type_class::unknown>(),
      "Unable to serialize unknown mapped type in map");

  using key_type = typename Type::key_type;
  using mapped_type = typename Type::mapped_type;
  using key_methods = protocol_methods<KeyClass, key_type>;
  using mapped_methods = protocol_methods<MappedClass, mapped_type>;
  using key_ttype = protocol_type<KeyClass, key_type>;
  using mapped_ttype = protocol_type<MappedClass, mapped_type>;

 protected:
  template <typename Protocol, typename U>
  static void consume_elem(Protocol& protocol, U& out) {
    key_type key_tmp;
    key_methods::read(protocol, key_tmp);
    mapped_methods::read(protocol, out[std::move(key_tmp)]);
  }

 public:
  template <typename Protocol, typename U>
  static void read(Protocol& protocol, U& out) {
    std::uint32_t map_size = -1;
    using WireTypeInfo = ProtocolReaderWireTypeInfo<Protocol>;
    using WireType = typename WireTypeInfo::WireType;

    WireType rpt_key_type = WireTypeInfo::defaultValue(),
             rpt_mapped_type = WireTypeInfo::defaultValue();

    protocol.readMapBegin(rpt_key_type, rpt_mapped_type, map_size);
    if (protocol.kOmitsContainerSizes()) {
      while (protocol.peekMap()) {
        consume_elem(protocol, out);
      }
    } else {
      // CompactProtocol does not transmit key/mapped types if
      // the map is empty
      if (map_size > 0 &&
          (WireTypeInfo::fromTType(key_ttype::value) != rpt_key_type ||
           WireTypeInfo::fromTType(mapped_ttype::value) != rpt_mapped_type)) {
        apache::thrift::skip_n(
            protocol, map_size, {rpt_key_type, rpt_mapped_type});
      } else {
        if (!canReadNElements(
                protocol, map_size, {rpt_key_type, rpt_mapped_type})) {
          protocol::TProtocolException::throwTruncatedData();
        }
        const auto kreader = [&protocol](auto& key) {
          key_methods::read(protocol, key);
        };
        const auto vreader = [&protocol](auto& value) {
          mapped_methods::read(protocol, value);
        };
        deserialize_known_length_map(out, map_size, kreader, vreader);
      }
    }
    protocol.readMapEnd();
  }

  template <typename Protocol, typename Context, typename U>
  static void readWithContext(Protocol& protocol, U& out, Context&) {
    read(protocol, out);
  }

  template <typename Protocol, typename U>
  static std::size_t write(Protocol& protocol, const U& out) {
    std::size_t xfer = 0;

    xfer += protocol.writeMapBegin(
        key_ttype::value,
        mapped_ttype::value,
        checked_container_size(out.size()));

    if (!folly::is_detected_v<detect_key_compare, Type> &&
        protocol.kSortKeys()) {
      std::vector<typename U::const_iterator> iters;
      iters.reserve(out.size());
      for (auto it = out.begin(); it != out.end(); ++it) {
        iters.push_back(it);
      }
      std::sort(iters.begin(), iters.end(), [](auto a, auto b) {
        return a->first < b->first;
      });
      for (auto it : iters) {
        xfer += writeMapValueBegin(protocol);
        xfer += key_methods::write(protocol, it->first);
        xfer += mapped_methods::write(protocol, it->second);
        xfer += writeMapValueEnd(protocol);
      }
    } else {
      // Support containers with defined but non-FIFO iteration order.
      auto get_view = folly::order_preserving_reinsertion_view_or_default;
      for (const auto& elem_pair : get_view(out)) {
        xfer += writeMapValueBegin(protocol);
        xfer += key_methods::write(protocol, elem_pair.first);
        xfer += mapped_methods::write(protocol, elem_pair.second);
        xfer += writeMapValueEnd(protocol);
      }
    }
    xfer += protocol.writeMapEnd();
    return xfer;
  }

  template <bool ZeroCopy, typename Protocol, typename U>
  static std::size_t serializedSize(Protocol& protocol, const U& out) {
    std::size_t xfer = protocol.serializedSizeMapBegin(
        key_ttype::value,
        mapped_ttype::value,
        folly::to_narrow(folly::to_unsigned(out.size())));
    for (const auto& elem_pair : out) {
      xfer += key_methods::template serializedSize<ZeroCopy>(
          protocol, elem_pair.first);
      xfer += mapped_methods::template serializedSize<ZeroCopy>(
          protocol, elem_pair.second);
    }
    xfer += protocol.serializedSizeMapEnd();
    return xfer;
  }

 private:
  template <typename Protocol>
  using map_value_begin_t =
      decltype(std::declval<Protocol>().writeMapValueBegin());

  template <typename Protocol>
  using map_value_end_t = decltype(std::declval<Protocol>().writeMapValueEnd());

  template <typename Protocol>
  static constexpr bool map_value_api_v =
      folly::is_detected_v<map_value_begin_t, Protocol>&&
          folly::is_detected_v<map_value_end_t, Protocol>;

  template <typename Protocol>
  static std::size_t writeMapValueBegin(Protocol& protocol) {
    const auto writeMapValueBeginFunc =
        std::get<map_value_api_v<Protocol&>>(std::make_pair(
            [](auto&) { return 0u; },
            [](auto& protocolWithMapValueApi) {
              return protocolWithMapValueApi.writeMapValueBegin();
            }));
    return writeMapValueBeginFunc(protocol);
  }

  template <typename Protocol>
  static std::size_t writeMapValueEnd(Protocol& protocol) {
    const auto writeMapValueEndFunc =
        std::get<map_value_api_v<Protocol&>>(std::make_pair(
            [](auto&) { return 0u; },
            [](auto& protocolWithMapValueApi) {
              return protocolWithMapValueApi.writeMapValueEnd();
            }));
    return writeMapValueEndFunc(protocol);
  }
};

/*
 * Struct with Indirection Specialization
 */
template <typename ElemClass, typename Indirection, typename Type>
struct protocol_methods<indirection_tag<ElemClass, Indirection>, Type> {
  using indirection = Indirection;
  using elem_type =
      std::remove_reference_t<folly::invoke_result_t<indirection, Type&>>;
  using elem_methods = protocol_methods<ElemClass, elem_type>;

  template <typename Protocol>
  static void read(Protocol& protocol, Type& out) {
    elem_methods::read(protocol, indirection{}(out));
  }

  template <typename Protocol, typename Context>
  static void readWithContext(Protocol& protocol, Type& out, Context& ctx) {
    elem_methods::readWithContext(protocol, indirection{}(out), ctx);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const Type& in) {
    return elem_methods::write(protocol, indirection{}(in));
  }
  template <bool ZeroCopy, typename Protocol>
  static std::size_t serializedSize(Protocol& protocol, const Type& in) {
    return elem_methods::template serializedSize<ZeroCopy>(
        protocol, indirection{}(in));
  }
};

/*
 * Struct Specialization
 * Forwards to Cpp2Ops wrapper around member read/write/etc.
 */
template <typename Type>
struct protocol_methods<type_class::structure, Type> {
  template <typename Tag>
  using Wrap = type::detail::Wrap<Type, Tag>;
  static Type& unwrap(Type& inst) { return inst; }
  static const Type& unwrap(const Type& inst) { return inst; }
  template <typename Tag>
  static Type& unwrap(Wrap<Tag>& inst) {
    return inst.toThrift();
  }
  template <typename Tag>
  static const Type& unwrap(const Wrap<Tag>& inst) {
    return inst.toThrift();
  }

  template <typename Protocol, typename U>
  static void read(Protocol& protocol, U& out) {
    Cpp2Ops<Type>::read(&protocol, &unwrap(out));
  }
  template <typename Protocol, typename Context, typename U>
  static void readWithContext(Protocol& protocol, U& out, Context&) {
    read(protocol, out);
  }
  template <typename Protocol, typename U>
  static std::size_t write(Protocol& protocol, const U& in) {
    return Cpp2Ops<Type>::write(&protocol, &unwrap(in));
  }
  template <bool ZeroCopy, typename Protocol, typename U>
  static std::size_t serializedSize(Protocol& protocol, const U& in) {
    if (ZeroCopy) {
      return Cpp2Ops<Type>::serializedSizeZC(&protocol, &unwrap(in));
    } else {
      return Cpp2Ops<Type>::serializedSize(&protocol, &unwrap(in));
    }
  }
};

/*
 * Union Specialization
 * Forwards to Cpp2Ops wrapper around member read/write/etc.
 */
template <typename Type>
struct protocol_methods<type_class::variant, Type>
    : protocol_methods<type_class::structure, Type> {};

} // namespace pm
} // namespace detail
} // namespace thrift
} // namespace apache
