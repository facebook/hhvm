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

#include <concepts>

#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/StringTypeAdapter.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Clear.h>

namespace apache::thrift {

template <typename Adapter, typename ThriftT>
concept ThriftConstAdapter =
    requires { typename adapt_detail::adapted_t<Adapter, ThriftT>; };

// The disjunctions below preserve support for non-semiregular Thrift types,
// notably std::unique_ptr<folly::IOBuf>.
template <typename Adapter, typename ThriftT>
concept ThriftTypeAdapter = adapt_detail::TypeAdapter<Adapter, ThriftT> &&
    (!std::semiregular<ThriftT> ||
     std::semiregular<adapt_detail::adapted_t<Adapter, ThriftT>>) &&
    (adapt_detail::EqualityComparableAdapter<
        Adapter,
        adapt_detail::adapted_t<Adapter, ThriftT>>);

// Field adapters can omit fromThrift because fromThriftField determines their
// adapted type from the containing struct and field id.
template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept ThriftFieldAdapter =
    adapt_detail::FieldAdapter<Adapter, FieldId, ThriftT, Struct> &&
    requires {
      Adapter::toThrift(
          std::declval<
              const adapt_detail::
                  FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>&>());
    } &&
    (!std::semiregular<ThriftT> ||
     std::semiregular<
         adapt_detail::
             FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>>) &&
    adapt_detail::EqualityComparableAdapter<
        Adapter,
        adapt_detail::FromThriftFieldIdType<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept ThriftAdapter = ThriftTypeAdapter<Adapter, ThriftT> ||
    ThriftFieldAdapter<Adapter, ThriftT, Struct, FieldId>;

namespace adapt_detail {

template <typename Adapter, typename ThriftT>
void validateAdapter() {
  static_assert(
      ThriftTypeAdapter<Adapter, ThriftT>,
      "@cpp.Adapter on a type must satisfy ThriftTypeAdapter");
}

template <typename Adapter, int16_t FieldId, typename ThriftT, typename Struct>
void validateFieldAdapter() {
  static_assert(
      ThriftAdapter<Adapter, ThriftT, Struct, FieldId>,
      "@cpp.Adapter on a field must satisfy ThriftAdapter");
}

} // namespace adapt_detail

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithConstruct =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::ConstructAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>,
        FieldContext<Struct, FieldId>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithClear =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::ClearAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithIsEmpty =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::EmptyAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithEqual =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::EqualAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithLess =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::LessAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithCompareThreeWay =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::CompareThreeWayAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept AdapterWithHash =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::HashAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

template <
    typename Adapter,
    typename ThriftT,
    typename Struct,
    int16_t FieldId,
    bool ZeroCopy,
    typename Tag,
    typename Protocol>
concept AdapterWithSerializedSize =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::SerializedSizeAdapter<
        Adapter,
        ZeroCopy,
        Tag,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>,
        Protocol>;

template <
    typename Adapter,
    typename ThriftT,
    typename Struct,
    int16_t FieldId,
    typename Tag,
    typename Protocol>
concept AdapterWithEncode =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::EncodeAdapter<
        Adapter,
        Tag,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>,
        Protocol>;

template <
    typename Adapter,
    typename ThriftT,
    typename Struct,
    int16_t FieldId,
    typename Tag,
    typename Protocol>
concept AdapterWithDecode =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::DecodeAdapter<
        Adapter,
        Tag,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>,
        Protocol>;

template <typename Adapter, typename ThriftT, typename Struct, int16_t FieldId>
concept InplaceThriftAdapter =
    ThriftAdapter<Adapter, ThriftT, Struct, FieldId> &&
    adapt_detail::InplaceAdapter<
        Adapter,
        adapt_detail::adapted_field_t<Adapter, FieldId, ThriftT, Struct>>;

namespace adapt_detail {

// Used to detect if an adapted type has a reset method.
template <typename AdaptedT>
using HasResetType = decltype(std::declval<AdaptedT>().reset());
template <typename AdaptedT>
constexpr bool has_reset_v = folly::is_detected_v<HasResetType, AdaptedT>;
template <typename AdaptedT, typename R = void>
using if_has_reset = std::enable_if_t<has_reset_v<AdaptedT>, R>;
template <typename AdaptedT, typename R = void>
using if_has_no_reset = std::enable_if_t<!has_reset_v<AdaptedT>, R>;

} // namespace adapt_detail

template <typename AdaptedT>
struct IndirectionAdapter {
  template <typename ThriftT>
  static constexpr AdaptedT fromThrift(ThriftT&& value) {
    AdaptedT adapted;
    toThrift(adapted) = std::forward<ThriftT>(value);
    return adapted;
  }
  FOLLY_ERASE static constexpr decltype(auto)
  toThrift(AdaptedT& adapted) noexcept(
      noexcept(::apache::thrift::apply_indirection(adapted))) {
    return ::apache::thrift::apply_indirection(adapted);
  }
  FOLLY_ERASE static constexpr decltype(auto)
  toThrift(const AdaptedT& adapted) noexcept(
      noexcept(::apache::thrift::apply_indirection(adapted))) {
    return ::apache::thrift::apply_indirection(adapted);
  }
};

namespace type {
template <typename Type, typename Tag>
using indirected = adapted<
    ::apache::thrift::IndirectionAdapter<Type>,
    cpp_type<
        folly::remove_cvref_t<::apache::thrift::adapt_detail::thrift_t<
            ::apache::thrift::IndirectionAdapter<Type>,
            folly::remove_cvref_t<Type>>>,
        Tag>>;
}

template <typename AdaptedT, typename ThriftT>
struct StaticCastAdapter {
  template <typename T>
  static constexpr decltype(auto) fromThrift(T&& value) {
    return static_cast<AdaptedT>(std::forward<T>(value));
  }
  template <typename T>
  static constexpr decltype(auto) toThrift(T&& value) {
    return static_cast<ThriftT>(std::forward<T>(value));
  }
};

template <class T>
struct InlineAdapter {
  template <typename U>
  static decltype(auto) toThrift(U&& value) {
    return std::forward<U>(value).toThrift();
  }

  // If an adapted type (e.g. type::detail::Wrap) has a reset method, use it.
  template <typename U>
  static adapt_detail::if_has_reset<U> clear(U& value) {
    value.reset();
  }

  template <typename Tag, typename Protocol, typename U>
  static auto encode(Protocol& prot_, const U& u) -> decltype(u.encode(prot_)) {
    return u.encode(prot_);
  }

  template <typename Tag, typename Protocol, typename U>
  static auto decode(Protocol& prot_, U& u) -> decltype(u.decode(prot_)) {
    u.decode(prot_);
  }

  template <typename U>
  static adapt_detail::if_has_no_reset<U> clear(U& value) {
    static_assert(
        adapt_detail::is_mutable_ref<decltype(value.toThrift())>::value,
        "not a mutable reference");
    apache::thrift::op::clear<>(value.toThrift());
  }

  template <typename U>
  static T fromThrift(U&& value) {
    return T{std::forward<U>(value)};
  }

  template <typename U>
  static bool isEmpty(const U& value) {
    return value.empty();
  }
};

} // namespace apache::thrift
