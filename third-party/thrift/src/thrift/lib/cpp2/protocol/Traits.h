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

#include <cstddef>
#include <type_traits>

#include <folly/functional/Invoke.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/TypeClass.h>

namespace apache {
namespace thrift {

namespace detail {

template <typename... T>
constexpr protocol::TType protocol_type_ill_formed_(T&&...) {
  if (!sizeof...(T)) {
    folly::throw_exception<std::runtime_error>("ill-formed");
  }
  return protocol::TType::T_STOP;
}

template <typename TypeClass>
struct protocol_type_ {};

template <>
struct protocol_type_<type_class::integral> {
  // clang-format off
  template <typename Type>
  static constexpr protocol::TType apply =
      std::is_same<Type, bool>::value ? protocol::TType::T_BOOL :
      sizeof(Type) == 1 ? protocol::TType::T_BYTE :
      sizeof(Type) == 2 ? protocol::TType::T_I16 :
      sizeof(Type) == 4 ? protocol::TType::T_I32 :
      sizeof(Type) == 8 ? protocol::TType::T_I64 :
      protocol_type_ill_formed_();
  // clang-format on
};

template <>
struct protocol_type_<type_class::floating_point> {
  // clang-format off
  template <typename Type>
  static constexpr protocol::TType apply =
      sizeof(Type) == 4 ? protocol::TType::T_FLOAT :
      sizeof(Type) == 8 ? protocol::TType::T_DOUBLE :
      protocol_type_ill_formed_();
  // clang-format on
};

template <>
struct protocol_type_<type_class::binary> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_STRING;
};

template <>
struct protocol_type_<type_class::string> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_STRING;
};

template <>
struct protocol_type_<type_class::enumeration> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_I32;
};

template <>
struct protocol_type_<type_class::structure> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_STRUCT;
};

template <>
struct protocol_type_<type_class::variant> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_STRUCT;
};

template <typename ValueTypeClass>
struct protocol_type_<type_class::list<ValueTypeClass>> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_LIST;
};

template <typename ValueTypeClass>
struct protocol_type_<type_class::set<ValueTypeClass>> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_SET;
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct protocol_type_<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename>
  static constexpr protocol::TType apply = protocol::TType::T_MAP;
};

template <typename IndirectedTypeClass, typename Indirection>
struct protocol_type_<indirection_tag<IndirectedTypeClass, Indirection>> {
  template <typename Type>
  static constexpr protocol::TType apply =
      protocol_type_<IndirectedTypeClass>::template apply<
          std::remove_reference_t<folly::invoke_result_t<Indirection, Type&>>>;
};

} // namespace detail

//  protocol_type_v
//  protocol_type
//
//  A trait variable and type to map from type-class and type to values of type
//  enum TType.
template <typename TypeClass, typename Type>
inline constexpr protocol::TType protocol_type_v =
    ::apache::thrift::detail::protocol_type_<TypeClass>::template apply<Type>;
template <typename TypeClass, typename Type>
struct protocol_type //
    : std::integral_constant< //
          protocol::TType,
          protocol_type_v<TypeClass, Type>> {};

namespace detail {

template <typename TypeClass>
struct fixed_cost_skip_ {};

template <>
struct fixed_cost_skip_<type_class::integral> {
  template <typename, typename>
  static constexpr bool apply = true;
};

template <>
struct fixed_cost_skip_<type_class::floating_point> {
  template <typename, typename>
  static constexpr bool apply = true;
};

template <>
struct fixed_cost_skip_<type_class::binary> {
  template <typename Protocol, typename>
  static constexpr bool apply = !Protocol::kOmitsStringSizes();
};

template <>
struct fixed_cost_skip_<type_class::string> {
  template <typename Protocol, typename>
  static constexpr bool apply = !Protocol::kOmitsStringSizes();
};

template <>
struct fixed_cost_skip_<type_class::enumeration> {
  template <typename, typename>
  static constexpr bool apply = true;
};

template <>
struct fixed_cost_skip_<type_class::structure> {
  template <typename, typename>
  static constexpr bool apply = false;
};

template <>
struct fixed_cost_skip_<type_class::variant> {
  template <typename, typename>
  static constexpr bool apply = false;
};

template <typename ValueTypeClass>
struct fixed_cost_skip_<type_class::list<ValueTypeClass>> {
  template <typename Protocol, typename Type>
  static constexpr bool apply = !Protocol::kOmitsContainerSizes() &&
      Protocol::fixedSizeInContainer(
          protocol_type_v<ValueTypeClass, typename Type::value_type>) > 0;
};

template <typename ValueTypeClass>
struct fixed_cost_skip_<type_class::set<ValueTypeClass>> {
  template <typename Protocol, typename Type>
  static constexpr bool apply = !Protocol::kOmitsContainerSizes() &&
      Protocol::fixedSizeInContainer(
          protocol_type_v<ValueTypeClass, typename Type::value_type>) > 0;
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct fixed_cost_skip_<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename Protocol, typename Type>
  static constexpr bool apply = !Protocol::kOmitsContainerSizes() &&
      Protocol::fixedSizeInContainer(
          protocol_type_v<KeyTypeClass, typename Type::key_type>) > 0 &&
      Protocol::fixedSizeInContainer(
          protocol_type_v<MappedTypeClass, typename Type::mapped_type>) > 0;
};
} // namespace detail

//  fixed_cost_skip_v
//
//  A trait variable reflecting whether the type has fixed cost to skip.
//
//  For example, list<i32> has fixed cost to skip under Binary Protocol since
//  the list header includes the list length and since each list element has
//  fixed size. However, it is not cheap to skip under Compact Protocol since
//  list elements do not have fixed size. And it is not cheap to skip under
//  SimpleJson Protocol since the list header does not include the list length.
template <class Protocol, class TypeClass, class Type>
inline constexpr bool fixed_cost_skip_v =
    ::apache::thrift::detail::fixed_cost_skip_<
        TypeClass>::template apply<Protocol, Type>;

} // namespace thrift
} // namespace apache
