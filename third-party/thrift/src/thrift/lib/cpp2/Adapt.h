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
using is_mutable_ref = std::conjunction<
    std::is_reference<T>,
    std::negation<std::is_const<std::remove_reference_t<T>>>>;

// The type returned by the adapter for the given Thrift type.
template <typename Adapter, typename ThriftT>
using adapted_t = decltype(Adapter::fromThrift(std::declval<ThriftT>()));

template <typename Adapter, typename ThriftT>
concept TypeAdapter = requires {
  Adapter::fromThrift(std::declval<ThriftT>());
  Adapter::toThrift(std::declval<const adapted_t<Adapter, ThriftT>&>());
};

// Used to detect if Adapter has the fromThriftField function which takes an
// additional FieldContext argument.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
using FromThriftFieldIdType = decltype(Adapter::fromThriftField(
    std::declval<ThriftT>(), std::declval<FieldContext<Struct, FieldId>>()));
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
concept FieldAdapter = requires {
  typename FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>;
};
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
constexpr bool is_field_adapter_v =
    FieldAdapter<Adapter, FieldId, ThriftT, Struct>;

// Used to detect if Adapter has a construct function override.
template <typename Adapter, typename AdaptedT, typename Context>
concept ConstructAdapter = requires {
  Adapter::construct(std::declval<AdaptedT&>(), std::declval<Context>());
};
// Used to detect if Adapter has an equal override.
template <typename Adapter, typename AdaptedT>
concept EqualAdapter = requires {
  Adapter::equal(
      std::declval<const AdaptedT&>(), std::declval<const AdaptedT&>());
};
template <typename Adapter, typename AdaptedT>
concept EqualityComparableAdapter =
    requires(const AdaptedT& lhs, const AdaptedT& rhs) {
      { Adapter::equal(lhs, rhs) } -> std::convertible_to<bool>;
    } ||
    (!EqualAdapter<Adapter, AdaptedT> &&
     requires(const AdaptedT& lhs, const AdaptedT& rhs) {
       { lhs == rhs } -> std::convertible_to<bool>;
     }) ||
    (!EqualAdapter<Adapter, AdaptedT> &&
     !requires(const AdaptedT& lhs, const AdaptedT& rhs) { lhs == rhs; } &&
     requires(const AdaptedT& lhs, const AdaptedT& rhs) {
       { Adapter::toThrift(lhs) == Adapter::toThrift(rhs) } ->
           std::convertible_to<bool>;
     });
// Used to detect if Adapter has a less override.
template <typename Adapter, typename AdaptedT>
concept LessAdapter = requires {
  Adapter::less(
      std::declval<const AdaptedT&>(), std::declval<const AdaptedT&>());
};
// Used to detect if Adapter has a three-way comparison override.
template <typename Adapter, typename AdaptedT>
concept CompareThreeWayAdapter = requires {
  Adapter::compareThreeWay(
      std::declval<const AdaptedT&>(), std::declval<const AdaptedT&>());
};
// Used to detect if Adapter has a clear function override.
template <typename Adapter, typename AdaptedT>
concept ClearAdapter = requires { Adapter::clear(std::declval<AdaptedT&>()); };
// Used to detect if Adapter has an isEmpty function override.
template <typename Adapter, typename AdaptedT>
concept EmptyAdapter =
    requires { Adapter::isEmpty(std::declval<const AdaptedT&>()); };
// Used to detect if Adapter has a hash override.
template <typename Adapter, typename AdaptedT>
concept HashAdapter =
    requires { Adapter::hash(std::declval<const AdaptedT&>()); };
// Converts a Thrift field value into an adapted type via Adapter.
// This overload passes additional context containing the reference to the
// Thrift object containing the field and the field ID as a second argument
// to Adapter::fromThriftField.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
  requires FieldAdapter<Adapter, FieldId, ThriftT, Struct>
constexpr FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>
fromThriftField(ThriftT&& value, Struct& object) {
  return Adapter::fromThriftField(
      std::forward<ThriftT>(value), FieldContext<Struct, FieldId>{object});
}

// Converts a Thrift field value into an adapted type via Adapter.
// This overloads does the conversion via Adapter::fromThrift and is used when
// Adapter::fromThriftField is unavailable.
template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
  requires(!FieldAdapter<Adapter, FieldId, ThriftT, Struct>)
constexpr adapted_t<Adapter, ThriftT> fromThriftField(
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

template <typename Adapter, typename AdaptedT>
concept InplaceAdapter =
    requires { requires is_mutable_ref<thrift_t<Adapter, AdaptedT>>::value; };

// If the adapter exposes access to the standard thrift value
// from the toThrift method.
template <typename Adapter, typename AdaptedT>
using has_inplace_toThrift =
    std::bool_constant<InplaceAdapter<Adapter, AdaptedT>>;

template <typename Adapter, typename AdaptedT, typename ThriftT>
void fromThrift(AdaptedT& adapted, ThriftT&& value) {
  adapted = Adapter::fromThrift(std::forward<ThriftT>(value));
}

// Called during the construction of a Thrift object to perform any additional
// initialization of an adapted type. This overload passes a context containing
// the reference to the Thrift object containing the field and the field ID as
// a second argument to Adapter::construct.
template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr void construct(AdaptedT& field, Struct& object) {
  if constexpr (ConstructAdapter<
                    Adapter,
                    AdaptedT,
                    FieldContext<Struct, FieldId>>) {
    Adapter::construct(field, FieldContext<Struct, FieldId>{object});
  }
}

// Clear op based on the adapter, with a fallback to calling the default
// constructor and Adapter::construct for context population.
template <typename Adapter, typename AdaptedT>
constexpr void clear(AdaptedT& field) {
  if constexpr (ClearAdapter<Adapter, AdaptedT>) {
    Adapter::clear(field);
  } else {
    field = AdaptedT();
  }
}

// Clear op based on the field adapter, with a fallback to calling the default
// constructor and Adapter::construct for context population.
template <typename Adapter, int16_t FieldId, typename AdaptedT, typename Struct>
constexpr void clear(AdaptedT& field, Struct& object) {
  if constexpr (ClearAdapter<Adapter, AdaptedT>) {
    Adapter::clear(field);
  } else {
    field = AdaptedT();
    construct<Adapter, FieldId>(field, object);
  }
}

// Equal op based on the thrift types.
template <typename Adapter, typename AdaptedT>
struct thrift_equal {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return Adapter::toThrift(lhs) == Adapter::toThrift(rhs);
  }
};

// Equal op based on the adapted types, with a fallback on thrift_equal.
template <typename Adapter, typename AdaptedT>
struct adapted_equal : thrift_equal<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires requires(const AdaptedT& lhs, const AdaptedT& rhs) { lhs == rhs; }
struct adapted_equal<Adapter, AdaptedT> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return lhs == rhs;
  }
};

// Equal op based on the adapter, with a fallback on adapted_equal.
template <typename Adapter, typename AdaptedT>
struct adapter_equal : adapted_equal<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires EqualAdapter<Adapter, AdaptedT>
struct adapter_equal<Adapter, AdaptedT> {
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
template <typename Adapter, typename AdaptedT>
struct adapted_less : thrift_less<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires requires(const AdaptedT& lhs, const AdaptedT& rhs) { lhs < rhs; }
struct adapted_less<Adapter, AdaptedT> {
  constexpr bool operator()(const AdaptedT& lhs, const AdaptedT& rhs) const {
    return lhs < rhs;
  }
};

// Less op based on the adapter, with a fallback on adapted_less.
template <typename Adapter, typename AdaptedT>
struct adapter_less : adapted_less<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires LessAdapter<Adapter, AdaptedT>
struct adapter_less<Adapter, AdaptedT> {
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
template <typename Adapter, typename AdaptedT>
struct adapted_compare_three_way : thrift_compare_three_way<Adapter, AdaptedT> {
};
template <typename Adapter, typename AdaptedT>
  requires requires(const AdaptedT& lhs, const AdaptedT& rhs) { lhs < rhs; }
struct adapted_compare_three_way<Adapter, AdaptedT> {
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
template <typename Adapter, typename AdaptedT>
struct adapter_compare_three_way
    : adapted_compare_three_way<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires CompareThreeWayAdapter<Adapter, AdaptedT>
struct adapter_compare_three_way<Adapter, AdaptedT> {
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
template <typename Adapter, typename AdaptedT>
struct adapted_hash : thrift_hash<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires requires { std::hash<std::decay_t<AdaptedT>>{}(cr<AdaptedT>()); }
struct adapted_hash<Adapter, AdaptedT> : std::hash<std::decay_t<AdaptedT>> {};

// Hash based on the adapter, with a fallback on adapted_hash.
template <typename Adapter, typename AdaptedT>
struct adapter_hash : adapted_hash<Adapter, AdaptedT> {};
template <typename Adapter, typename AdaptedT>
  requires HashAdapter<Adapter, AdaptedT>
struct adapter_hash<Adapter, AdaptedT> {
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

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF>
struct adapter_serialized_size {
  uint32_t operator()(
      Protocol& /*unused*/, const AdaptedT& /*unused*/, FallbackF f) {
    return f();
  }
};

template <
    typename Adapter,
    bool ZeroCopy,
    typename Tag,
    typename AdaptedT,
    typename Protocol>
concept SerializedSizeAdapter = requires {
  Adapter::template serializedSize<ZeroCopy, Tag>(
      std::declval<Protocol&>(), std::declval<const AdaptedT&>());
};

template <typename Adapter, typename Tag, typename AdaptedT, typename Protocol>
concept EncodeAdapter = requires {
  Adapter::template encode<Tag>(
      std::declval<Protocol&>(), std::declval<const AdaptedT&>());
};

template <typename Adapter, typename Tag, typename AdaptedT, typename Protocol>
concept DecodeAdapter = requires {
  Adapter::template decode<Tag>(
      std::declval<Protocol&>(), std::declval<AdaptedT&>());
};

template <
    bool ZeroCopy,
    typename Tag,
    typename Adapter,
    typename AdaptedT,
    typename Protocol,
    typename FallbackF>
  requires SerializedSizeAdapter<Adapter, ZeroCopy, Tag, AdaptedT, Protocol>
struct adapter_serialized_size<
    ZeroCopy,
    Tag,
    Adapter,
    AdaptedT,
    Protocol,
    FallbackF> {
  uint32_t operator()(
      Protocol& prot, const AdaptedT& val, FallbackF /*unused*/) {
    return Adapter::template serializedSize<ZeroCopy, Tag>(prot, val);
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
