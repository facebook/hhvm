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

#include <thrift/lib/cpp/protocol/TType.h>
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

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
