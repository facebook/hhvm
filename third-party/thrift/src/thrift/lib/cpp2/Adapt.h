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

#include <stddef.h>
#include <functional>
#include <type_traits>
#include <utility>

#include <folly/Traits.h>
#include <folly/lang/Ordering.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift::adapt_detail {

// Identical to std::declval<const T&>.
template <typename T>
const T& cr();

template <typename T>
using is_mutable_ref = folly::Conjunction<
    std::is_reference<T>,
    folly::Negation<std::is_const<std::remove_reference_t<T>>>>;

// The type returned by the adapter for the given Thrift type.
template <typename Adapter, typename ThriftT>
using adapted_t = decltype(Adapter::fromThrift(std::declval<ThriftT>()));

// Used to detect if Adapter has the fromThriftField function which takes an
// additional FieldContext argument.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
using FromThriftFieldIdType = decltype(Adapter::fromThriftField(
    std::declval<ThriftT>(), std::declval<FieldContext<Struct, FieldId>>()));
template <typename Adapter, typename ThriftT, typename Struct>
using FromThriftFieldType = decltype(Adapter::fromThriftField(
    std::declval<ThriftT>(), std::declval<FieldContext<Struct, 0>>()));
template <typename Adapter, typename ThriftT, typename Struct>
constexpr bool is_field_adapter_v =
    folly::is_detected_v<FromThriftFieldType, Adapter, ThriftT, Struct>;

template <
    typename Adapter,
    int16_t FieldId,
    typename ThriftT,
    typename Struct,
    typename R = FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>>
using if_field_adapter =
    std::enable_if_t<is_field_adapter_v<Adapter, ThriftT, Struct>, R>;
template <
    typename Adapter,
    typename ThriftT,
    typename Struct,
    typename R = adapted_t<Adapter, ThriftT>>
using if_not_field_adapter =
    std::enable_if_t<!is_field_adapter_v<Adapter, ThriftT, Struct>, R>;

// Used to detect if Adapter has a construct function override.
template <typename Adapter, typename AdaptedT, typename Context>
using ConstructType = decltype(Adapter::construct(
    std::declval<AdaptedT&>(), std::declval<Context>()));
template <typename Adapter, typename AdaptedT, typename Context>
constexpr bool is_ctor_adapter_v =
    folly::is_detected_v<ConstructType, Adapter, AdaptedT, Context>;
template <typename Adapter, typename AdaptedT, typename Context>
using if_ctor_adapter =
    std::enable_if_t<is_ctor_adapter_v<Adapter, AdaptedT, Context>>;
template <typename Adapter, typename AdaptedT, typename Context>
using if_not_ctor_adapter =
    std::enable_if_t<!is_ctor_adapter_v<Adapter, AdaptedT, Context>>;

// Used to detect if Adapter has an equal override.
template <typename Adapter, typename AdaptedT>
using EqualType = decltype(Adapter::equal(
    std::declval<const AdaptedT&>(), std::declval<const AdaptedT&>()));
template <typename Adapter, typename AdaptedT>
constexpr bool is_equal_adapter_v =
    folly::is_detected_v<EqualType, Adapter, AdaptedT>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_equal_adapter =
    std::enable_if_t<is_equal_adapter_v<Adapter, AdaptedT>, R>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_not_equal_adapter =
    std::enable_if_t<!is_equal_adapter_v<Adapter, AdaptedT>, R>;

// Used to detect if Adapter has a less override.
template <typename Adapter, typename AdaptedT>
using LessType = decltype(Adapter::less(
    std::declval<AdaptedT&>(), std::declval<const AdaptedT&>()));
template <typename Adapter, typename AdaptedT>
constexpr bool is_less_adapter_v =
    folly::is_detected_v<LessType, Adapter, AdaptedT>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_less_adapter =
    std::enable_if_t<is_less_adapter_v<Adapter, AdaptedT>, R>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_not_less_adapter =
    std::enable_if_t<!is_less_adapter_v<Adapter, AdaptedT>, R>;

// Used to detect if Adapter has a three-way comparison override.
template <typename Adapter, typename AdaptedT>
using CompareThreeWayType = decltype(Adapter::compareThreeWay(
    std::declval<const AdaptedT&>(), std::declval<const AdaptedT&>()));
template <typename Adapter, typename AdaptedT>
constexpr bool is_compare_three_way_adapter_v =
    folly::is_detected_v<CompareThreeWayType, Adapter, AdaptedT>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_compare_three_way_adapter =
    std::enable_if_t<is_compare_three_way_adapter_v<Adapter, AdaptedT>, R>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_not_compare_three_way_adapter =
    std::enable_if_t<!is_compare_three_way_adapter_v<Adapter, AdaptedT>, R>;

// Used to detect if Adapter has a clear function override.
template <typename Adapter, typename AdaptedT>
using ClearType = decltype(Adapter::clear(std::declval<AdaptedT&>()));
template <typename Adapter, typename AdaptedT>
constexpr bool is_clear_adapter_v =
    folly::is_detected_v<ClearType, Adapter, AdaptedT>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_clear_adapter =
    std::enable_if_t<is_clear_adapter_v<Adapter, AdaptedT>, R>;
template <typename Adapter, typename AdaptedT, typename R = void>
using if_not_clear_adapter =
    std::enable_if_t<!is_clear_adapter_v<Adapter, AdaptedT>, R>;

// Used to detect if Adapter has an isEmpty function override.
template <typename Adapter, typename AdaptedT>
using IsEmptyType = decltype(Adapter::isEmpty(std::declval<const AdaptedT&>()));
template <typename Adapter, typename AdaptedT>
constexpr bool is_empty_adapter_v =
    folly::is_detected_v<IsEmptyType, Adapter, AdaptedT>;
template <typename Adapter, typename AdaptedT>
using if_is_empty_adapter =
    std::enable_if_t<is_empty_adapter_v<Adapter, AdaptedT>, bool>;
template <typename Adapter, typename AdaptedT>
using if_not_is_empty_adapter =
    std::enable_if_t<!is_empty_adapter_v<Adapter, AdaptedT>, bool>;

// Converts a Thrift field value into an adapted type via Adapter.
// This overload passes additional context containing the reference to the
// Thrift object containing the field and the field ID as a second argument
// to Adapter::fromThriftField.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
constexpr if_field_adapter<Adapter, FieldId, ThriftT, Struct> fromThriftField(
    ThriftT&& value, Struct& object) {
  return Adapter::fromThriftField(
      std::forward<ThriftT>(value), FieldContext<Struct, FieldId>{object});
}

// Converts a Thrift field value into an adapted type via Adapter.
// This overloads does the conversion via Adapter::fromThrift and is used when
// Adapter::fromThriftField is unavailable.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
constexpr if_not_field_adapter<Adapter, ThriftT, Struct> fromThriftField(
    ThriftT&& value, Struct& /*unused*/) {
  return Adapter::fromThrift(std::forward<ThriftT>(value));
}

// The type returned by the adapter for the given thrift type of a struct field.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
using adapted_field_t =
    decltype(fromThriftField<Adapter, FieldId, ThriftT, Struct>(
        std::declval<ThriftT>(), std::declval<Struct&>()));

// The type returned by the adapter for the given adapted type.
template <typename Adapter, typename AdaptedT>
using thrift_t = decltype(Adapter::toThrift(std::declval<AdaptedT&>()));

// If the adapter exposes access to the standard thrift value
// from the toThrift method.
template <typename Adapter, typename AdaptedT, typename = void>
using has_inplace_toThrift =
    is_mutable_ref<folly::detected_t<thrift_t, Adapter, AdaptedT>>;

template <typename Adapter, typename AdaptedT, typename ThriftT>
void fromThrift(AdaptedT& adapted, ThriftT&& value) {
  adapted = Adapter::fromThrift(std::forward<ThriftT>(value));
}

// Called during the construction of a Thrift object to perform any additional
// initialization of an adapted type. This overload passes a context containing
// the reference to the Thrift object containing the field and the field ID as
// a second argument to Adapter::construct.
template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr if_ctor_adapter<Adapter, AdaptedT, FieldContext<Struct, FieldId>>
construct(AdaptedT& field, Struct& object) {
  Adapter::construct(field, FieldContext<Struct, FieldId>{object});
}
template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr if_not_ctor_adapter<Adapter, AdaptedT, FieldContext<Struct, FieldId>>
construct(AdaptedT& /*unused*/, Struct& /*unused*/) {}

// Clear op based on the adapter, with a fallback to calling the default
// constructor and Adapter::construct for context population.
template <typename Adapter, typename AdaptedT>
constexpr if_clear_adapter<Adapter, AdaptedT> clear(AdaptedT& field) {
  Adapter::clear(field);
}

template <typename Adapter, typename AdaptedT>
constexpr if_not_clear_adapter<Adapter, AdaptedT> clear(AdaptedT& field) {
  field = AdaptedT();
}

// Clear op based on the field adapter, with a fallback to calling the default
// constructor and Adapter::construct for context population.
template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr if_clear_adapter<Adapter, AdaptedT> clear(
    AdaptedT& field, Struct& /*unused*/) {
  Adapter::clear(field);
}

template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr if_not_clear_adapter<Adapter, AdaptedT> clear(
    AdaptedT& field, Struct& object) {
  field = AdaptedT();
  construct<Adapter, FieldId>(field, object);
}

// Equal op based on the thrift types.
template <typename Adapter, typename AdaptedT>
struct thrift_equal {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::toThrift(lhs) == Adapter::toThrift(rhs);
  }
};

// Equal op based on the adapted types, with a fallback on thrift_equal.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapted_equal : thrift_equal<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapted_equal<
    Adapter,
    AdaptedT,
    folly::void_t<decltype(cr<AdaptedT>() == cr<AdaptedT>())>> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return lhs == rhs;
  }
};

// Equal op based on the adapter, with a fallback on adapted_equal.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapter_equal : adapted_equal<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapter_equal<
    Adapter,
    AdaptedT,
    folly::void_t<EqualType<Adapter, AdaptedT>>> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::equal(lhs, rhs);
  }
};

// Less op based on the thrift types.
template <typename Adapter, typename AdaptedT>
struct thrift_less {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::toThrift(lhs) < Adapter::toThrift(rhs);
  }
};

// Less op based on the adapted types, with a fallback on thrift_less.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapted_less : thrift_less<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapted_less<
    Adapter,
    AdaptedT,
    folly::void_t<decltype(cr<AdaptedT>() < cr<AdaptedT>())>> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return lhs < rhs;
  }
};

// Less op based on the adapter, with a fallback on adapted_less.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapter_less : adapted_less<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapter_less<
    Adapter,
    AdaptedT,
    folly::void_t<LessType<Adapter, AdaptedT>>> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::less(lhs, rhs);
  }
};

// CompareThreeWay op based on the thrift types.
template <typename Adapter, typename AdaptedT>
struct thrift_compare_three_way {
  constexpr folly::ordering operator()(
      const AdaptedT& lhs, const AdaptedT& rhs) const {
    if (Adapter::toThrift(lhs) == Adapter::toThrift(rhs)) {
      return folly::ordering::eq;
    } else if (Adapter::toThrift(lhs) < Adapter::toThrift(rhs)) {
      return folly::ordering::lt;
    }
    return folly::ordering::gt;
  }
};

// CompareThreeWay op based on the adapted types, with a fallback on
// thrift_compare_three_way.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapted_compare_three_way : thrift_compare_three_way<Adapter, AdaptedT> {
};
template <typename Adapter, typename AdaptedT>
struct adapted_compare_three_way<
    Adapter,
    AdaptedT,
    folly::void_t<decltype(cr<AdaptedT>() < cr<AdaptedT>())>> {
  constexpr folly::ordering operator()(
      const AdaptedT& lhs, const AdaptedT& rhs) const {
    if (lhs == rhs) {
      return folly::ordering::eq;
    } else if (lhs < rhs) {
      return folly::ordering::lt;
    }
    return folly::ordering::gt;
  }
};

// CompareThreeWay op based on the adapter, with a fallback on
// adapted_compare_three_way.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapter_compare_three_way
    : adapted_compare_three_way<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapter_compare_three_way<
    Adapter,
    AdaptedT,
    folly::void_t<CompareThreeWayType<Adapter, AdaptedT>>> {
  constexpr folly::ordering operator()(
      const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::compareThreeWay(lhs, rhs);
  }
};

// Hash based on the thrift type.
template <typename Adapter, typename AdaptedT>
struct thrift_hash {
  constexpr size_t operator()(const AdaptedT& value) const {
    auto&& tvalue = Adapter::toThrift(value);
    return std::hash<folly::remove_cvref_t<decltype(tvalue)>>()(tvalue);
  }
};

// Hash based on the adapted types, with a fallback on thrift_hash.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapted_hash : thrift_hash<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapted_hash<
    Adapter,
    AdaptedT,
    folly::void_t<decltype(std::hash<std::decay_t<AdaptedT>>())>>
    : std::hash<std::decay_t<AdaptedT>> {};

// Hash based on the adapter, with a fallback on adapted_hash.
template <typename Adapter, typename AdaptedT, typename = void>
struct adapter_hash : adapted_hash<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
struct adapter_hash<
    Adapter,
    AdaptedT,
    folly::void_t<decltype(Adapter::hash(cr<AdaptedT>()))>> {
  constexpr size_t operator()(const AdaptedT& value) const {
    return Adapter::hash(value);
  }
};

template <typename Adapter, typename AdaptedT>
constexpr bool equal(const AdaptedT& lhs, const AdaptedT& rhs) {
  return adapter_equal<Adapter, AdaptedT>()(lhs, rhs);
}

// Helper for optional fields.
template <typename Adapter, typename FieldRefT>
constexpr bool equal_opt(const FieldRefT& lhs, const FieldRefT& rhs) {
  using AdaptedT = decltype(lhs.value());
  return lhs.has_value() == rhs.has_value() &&
      (!lhs.has_value() || equal<Adapter, AdaptedT>(lhs.value(), rhs.value()));
}

template <typename Adapter, typename AdaptedT>
constexpr bool not_equal(const AdaptedT& lhs, const AdaptedT& rhs) {
  return !adapter_equal<Adapter, AdaptedT>()(lhs, rhs);
}

// Helper for optional fields.
template <typename Adapter, typename FieldRefT>
constexpr bool not_equal_opt(const FieldRefT& lhs, const FieldRefT& rhs) {
  return !equal_opt<Adapter, FieldRefT>(lhs, rhs);
}

template <typename Adapter, typename AdaptedT>
constexpr bool less(const AdaptedT& lhs, const AdaptedT& rhs) {
  return adapter_less<Adapter, AdaptedT>()(lhs, rhs);
}

// A less comparision when the values are already known to be not equal.
// Helper for optional fields.
template <typename Adapter, typename FieldRefT>
constexpr bool neq_less_opt(const FieldRefT& lhs, const FieldRefT& rhs) {
  using AdaptedT = decltype(lhs.value());
  return !lhs.has_value() ||
      (rhs.has_value() &&
       adapter_less<Adapter, AdaptedT>()(lhs.value(), rhs.value()));
}

template <typename Adapter, typename AdaptedT>
constexpr size_t hash(const AdaptedT& value) {
  return adapter_hash<Adapter, AdaptedT>()(value);
}

// Helpers replace less, hash, equal_to functions
// for a set, with the appropriate adapted versions.
template <
    typename Adapter,
    template <typename, typename, typename> class SetT,
    typename Key,
    typename Less,
    typename Allocator>
SetT<Key, adapt_detail::adapted_less<Adapter, Key>, Allocator>
resolveSetForAdapated(const SetT<Key, Less, Allocator>&);
template <
    typename Adapter,
    template <typename, typename, typename, typename> class SetT,
    typename Key,
    typename Hash,
    typename KeyEqual,
    typename Allocator>
SetT<
    Key,
    adapt_detail::adapted_hash<Adapter, Key>,
    adapt_detail::adapted_equal<Adapter, Key>,
    Allocator>
resolveSetForAdapated(const SetT<Key, Hash, KeyEqual, Allocator>&);
template <typename KeyAdapter, typename StandardSet>
using adapt_set_key_t =
    decltype(resolveSetForAdapated<KeyAdapter>(std::declval<StandardSet>()));

// Helpers to set the appropriate less, hash, equal_to functions
// for a map with an adapted key type.
template <
    typename Adapter,
    template <typename, typename, typename, typename> class MapT,
    typename Key,
    typename Value,
    typename Less,
    typename Allocator>
MapT<Key, Value, adapt_detail::adapted_less<Adapter, Key>, Allocator>
resolveMapForAdapated(const MapT<Key, Value, Less, Allocator>&);
template <
    typename Adapter,
    template <typename, typename, typename, typename, typename> class MapT,
    typename Key,
    typename Value,
    typename Hash,
    typename KeyEqual,
    typename Allocator>
MapT<
    Key,
    Value,
    adapt_detail::adapted_hash<Adapter, Key>,
    adapt_detail::adapted_equal<Adapter, Key>,
    Allocator>
resolveMapForAdapated(const MapT<Key, Value, Hash, KeyEqual, Allocator>&);
template <typename KeyAdapter, typename StandardMap>
using adapt_map_key_t =
    decltype(resolveMapForAdapated<KeyAdapter>(std::declval<StandardMap>()));

// Validates an adapter.
// Checking decltype(equal<Adapter>(...)) is not sufficient for validation.
template <typename Adapter, typename AdaptedT>
void validate() {
  const auto adapted = AdaptedT();
  equal<Adapter>(adapted, adapted);
  not_equal<Adapter>(adapted, adapted);
  // less and hash are not validated because not all adapters provide it.
}

template <typename Adapter, typename ThriftT>
void validateAdapter() {
  validate<Adapter, adapted_t<Adapter, ThriftT>>();
}

template <typename Adapter, int16_t FieldID, typename ThriftT, typename Struct>
void validateFieldAdapter() {
  validate<Adapter, adapted_field_t<Adapter, FieldID, ThriftT, Struct>>();
}

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF,
    typename = void>
struct adapter_serialized_size {
  uint32_t operator()(
      Protocol& /*unused*/, const AdaptedT& /*unused*/, FallbackF f) {
    return f();
  }
};

template <typename Tag, typename Adapter, typename AdaptedT, typename Protocol>
using serialized_size_type =
    decltype(Adapter::template serializedSize<false, Tag>(
        std::declval<Protocol&>(), std::declval<AdaptedT&>()));

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF>
struct adapter_serialized_size<
    ZeroCopy,
    Tag,
    Adapter,
    AdaptedT,
    Protocol,
    FallbackF,
    folly::void_t<serialized_size_type<Tag, Adapter, AdaptedT, Protocol>>> {
  uint32_t operator()(
      Protocol& prot, const AdaptedT& val, FallbackF /*unused*/) {
    return Adapter::template serializedSize<ZeroCopy, Tag>(prot, val);
  }
};

template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, bool /*unused*/) {
  return protocol.serializedSizeBool();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, int8_t /*unused*/) {
  return protocol.serializedSizeByte();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, int16_t /*unused*/) {
  return protocol.serializedSizeI16();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, int32_t /*unused*/) {
  return protocol.serializedSizeI32();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, int64_t /*unused*/) {
  return protocol.serializedSizeI64();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, double /*unused*/) {
  return protocol.serializedSizeDouble();
}
template <typename Protocol>
uint32_t serializedSizeFixed(Protocol& protocol, float /*unused*/) {
  return protocol.serializedSizeFloat();
}

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF>
struct adapter_serialized_size<
    ZeroCopy,
    Tag,
    Adapter,
    AdaptedT,
    Protocol,
    FallbackF,
    std::enable_if_t<
        !folly::is_detected_v<
            serialized_size_type,
            Tag,
            Adapter,
            AdaptedT,
            Protocol> &&
        std::is_arithmetic_v<decltype(Adapter::toThrift(
            std::declval<AdaptedT&>()))>>> {
  uint32_t operator()(
      Protocol& prot, const AdaptedT& /*unused*/, FallbackF /*unused*/) {
    return serializedSizeFixed(
        prot, decltype(Adapter::toThrift(std::declval<AdaptedT&>()))(0));
  }
};

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF>
uint32_t serializedSize(Protocol& prot, const AdaptedT& val, FallbackF f) {
  return adapter_serialized_size<
      ZeroCopy,
      Tag,
      Adapter,
      AdaptedT,
      Protocol,
      FallbackF>()(prot, val, f);
}

} // namespace apache::thrift::adapt_detail
