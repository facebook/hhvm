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

#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Clear.h>

namespace apache {
namespace thrift {
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

} // namespace thrift
} // namespace apache
