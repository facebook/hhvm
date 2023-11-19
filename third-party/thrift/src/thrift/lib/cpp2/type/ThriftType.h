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

#include <folly/Portability.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/type/Tag.h>

// Helpers for working with thrift type tags.
namespace apache {
namespace thrift {
namespace type {

// If a given type tag refers to concrete type and not a class of types.
//
// For example:
//     is_concrete_v<byte_t> -> true
//     is_concrete_v<list_c> -> false
//     is_concrete_v<list<byte_t>> -> true
//     is_concrete_v<list<struct_c>> -> false
template <typename Tag>
struct is_concrete : std::false_type {};

template <typename Tag>
constexpr bool is_concrete_v = is_concrete<Tag>::value;

// If a given Thrift type tag is wellformed.
//
// For example:
//     is_thrift_type_tag_v<int> -> false
//     is_thrift_type_tag_v<byte_t> -> true
//     is_thrift_type_tag_v<list_c> -> true
//     is_thrift_type_tag_v<list<Foo>> -> false
//     is_thrift_type_tag_v<list<byte_t>> -> true
//     is_thrift_type_tag_v<list<struct_c>> -> true
template <typename Tag>
struct is_thrift_type_tag : is_concrete<Tag> {};

template <typename Tag>
constexpr bool is_thrift_type_tag_v = is_thrift_type_tag<Tag>::value;

// If a given Thrift type tag is not concrete.
//
// For example:
//     is_abstract_v<int> -> false
//     is_abstract_v<byte_t> -> false
//     is_abstract_v<list_c> -> true
//     is_abstract_v<list<Foo>> -> false
//     is_abstract_v<list<byte_t>> -> false
//     is_abstract_v<list<struct_c>> -> true
template <typename Tag>
struct is_abstract : std::false_type {};

template <typename Tag>
constexpr bool is_abstract_v =
    is_thrift_type_tag<Tag>::value && !is_concrete<Tag>::value;

namespace detail {
template <typename... Tags>
constexpr void checkTags() {
  static_assert((is_thrift_type_tag_v<Tags> && ...));
}
} // namespace detail

// Is `true` iff `Tag` is in the type class, `CTag`.
//
// For example:
//     is_a_v<void_t, integral_c> -> false
//     is_a_v<i32_t, integral_c> -> true
//     is_a_v<i32_t, i32_t> -> true
//     is_a_v<i32_t, i64_t> -> false
//     is_a_v<list<i64_t>, list<i64_t>> -> true
//     is_a_v<list<integral_c>, list_c> -> true
//     is_a_v<list<i64_t>, list_c> -> true
//     is_a_v<list<i64_t>, list<integral_c>> -> true
//     is_a_v<list<i64_t>, list<i64_t>> -> true
template <typename Tag, typename CTag>
constexpr bool is_a_v =
    (detail::checkTags<Tag, CTag>(), std::is_base_of_v<CTag, Tag>);

// Helpers to enable/disable declarations based on if a type tag matches
// a constraint.
template <typename Tag, typename R = void, typename...>
using if_concrete = std::enable_if_t<is_concrete_v<Tag>, R>;
template <typename Tag, typename R = void, typename...>
using if_abstract = std::enable_if_t<is_abstract_v<Tag>, R>;
template <typename Tag, typename R = void, typename...>
using if_not_concrete = if_abstract<Tag, R>;
template <typename CTag, typename Tag, typename R = void, typename...>
using if_is_a = std::enable_if_t<is_a_v<CTag, Tag>, R>;
template <typename Tag, typename R = void, typename...>
using if_thrift_type_tag = std::enable_if_t<is_thrift_type_tag_v<Tag>, R>;
template <typename Tag, typename R = void, typename...>
using if_not_thrift_type_tag = std::enable_if_t<!is_thrift_type_tag_v<Tag>, R>;

// Helpers for applying the conditions.
namespace bound {
struct is_concrete {
  template <typename Tag>
  using apply = type::is_concrete<Tag>;
};
struct is_thrift_type_tag {
  template <typename Tag>
  using apply = type::is_thrift_type_tag<Tag>;
};
struct is_abstract {
  template <typename Tag>
  using apply = type::is_abstract<Tag>;
};
template <typename CTag>
struct is_a {
  template <typename Tag>
  using apply = folly::bool_constant<type::is_a_v<Tag, CTag>>;
};
} // namespace bound

////
// Implemnation details

template <>
struct is_concrete<void_t> : std::true_type {};
template <>
struct is_concrete<bool_t> : std::true_type {};
template <>
struct is_concrete<byte_t> : std::true_type {};
template <>
struct is_concrete<i16_t> : std::true_type {};
template <>
struct is_concrete<i32_t> : std::true_type {};
template <>
struct is_concrete<i64_t> : std::true_type {};
template <>
struct is_concrete<float_t> : std::true_type {};
template <>
struct is_concrete<double_t> : std::true_type {};
template <>
struct is_concrete<string_t> : std::true_type {};
template <>
struct is_concrete<binary_t> : std::true_type {};

template <typename T>
struct is_concrete<enum_t<T>> : std::true_type {};
template <typename T>
struct is_concrete<struct_t<T>> : std::true_type {};
template <typename T>
struct is_concrete<union_t<T>> : std::true_type {};
template <typename T>
struct is_concrete<exception_t<T>> : std::true_type {};
template <typename T>
struct is_concrete<service_t<T>> : std::true_type {};

template <typename VTag>
struct is_concrete<list<VTag>> : folly::bool_constant<is_concrete_v<VTag>> {};
template <typename KTag>
struct is_concrete<set<KTag>> : folly::bool_constant<is_concrete_v<KTag>> {};
template <typename KTag, typename VTag>
struct is_concrete<map<KTag, VTag>>
    : folly::bool_constant<is_concrete_v<KTag> && is_concrete_v<VTag>> {};

template <typename Adapter, typename Tag>
struct is_concrete<adapted<Adapter, Tag>>
    : folly::bool_constant<is_concrete_v<Tag>> {};
template <typename T, typename Tag>
struct is_concrete<cpp_type<T, Tag>>
    : folly::bool_constant<is_concrete_v<Tag>> {};

template <typename Tag, typename Context>
struct is_concrete<field<Tag, Context>>
    : folly::bool_constant<is_concrete_v<Tag>> {};
template <typename Adapter, typename Tag, typename Context>
struct is_concrete<field<adapted<Adapter, Tag>, Context>>
    : folly::bool_constant<is_concrete_v<Tag>> {};

template <>
struct is_thrift_type_tag<all_c> : std::true_type {};
template <>
struct is_thrift_type_tag<primitive_c> : std::true_type {};
template <>
struct is_thrift_type_tag<number_c> : std::true_type {};
template <>
struct is_thrift_type_tag<integral_c> : std::true_type {};
template <>
struct is_thrift_type_tag<floating_point_c> : std::true_type {};
template <>
struct is_thrift_type_tag<enum_c> : std::true_type {};
template <>
struct is_thrift_type_tag<string_c> : std::true_type {};
template <>
struct is_thrift_type_tag<structured_c> : std::true_type {};
template <>
struct is_thrift_type_tag<struct_except_c> : std::true_type {};
template <>
struct is_thrift_type_tag<struct_c> : std::true_type {};
template <>
struct is_thrift_type_tag<union_c> : std::true_type {};
template <>
struct is_thrift_type_tag<exception_c> : std::true_type {};
template <>
struct is_thrift_type_tag<container_c> : std::true_type {};
template <>
struct is_thrift_type_tag<list_c> : std::true_type {};
template <>
struct is_thrift_type_tag<set_c> : std::true_type {};
template <>
struct is_thrift_type_tag<map_c> : std::true_type {};
template <>
struct is_thrift_type_tag<service_c> : std::true_type {};

template <typename VTag>
struct is_thrift_type_tag<list<VTag>>
    : folly::bool_constant<is_thrift_type_tag_v<VTag>> {};
template <typename KTag>
struct is_thrift_type_tag<set<KTag>>
    : folly::bool_constant<is_thrift_type_tag_v<KTag>> {};
template <typename KTag, typename VTag>
struct is_thrift_type_tag<map<KTag, VTag>>
    : folly::bool_constant<
          is_thrift_type_tag_v<KTag> && is_thrift_type_tag_v<VTag>> {};

template <typename Adapter, typename Tag>
struct is_thrift_type_tag<adapted<Adapter, Tag>>
    : folly::bool_constant<is_thrift_type_tag_v<Tag>> {};
template <typename T, typename Tag>
struct is_thrift_type_tag<cpp_type<T, Tag>>
    : folly::bool_constant<is_thrift_type_tag_v<Tag>> {};

template <typename Tag, typename Context>
struct is_thrift_type_tag<field<Tag, Context>> : std::true_type {};

template <typename V1, typename V2>
inline constexpr bool is_a_v<list<V1>, list<V2>> = is_a_v<V1, V2>;
template <typename K1, typename K2>
inline constexpr bool is_a_v<set<K1>, set<K2>> = is_a_v<K1, K2>;

template <typename K1, typename V1, typename K2, typename V2>
inline constexpr bool is_a_v<map<K1, V1>, map<K2, V2>> =
    is_a_v<K1, K2>&& is_a_v<V1, V2>;

template <typename A, typename Tag, typename CTag>
inline constexpr bool is_a_v<adapted<A, Tag>, CTag> = is_a_v<Tag, CTag>;
template <typename A, typename Tag, typename CTag>
inline constexpr bool is_a_v<adapted<A, Tag>, adapted<A, CTag>> =
    is_a_v<Tag, CTag>;

template <typename T, typename Tag, typename CTag>
inline constexpr bool is_a_v<cpp_type<T, Tag>, CTag> = is_a_v<Tag, CTag>;
template <typename T, typename Tag, typename CTag>
inline constexpr bool is_a_v<cpp_type<T, Tag>, cpp_type<T, CTag>> =
    is_a_v<Tag, CTag>;

} // namespace type
} // namespace thrift
} // namespace apache
