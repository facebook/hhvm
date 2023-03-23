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
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {

template <typename AdaptedT>
struct IndirectionAdapter;

namespace type_class {

template <class Tag>
struct from_type_tag;

template <class... T>
using from_type_tag_t = typename from_type_tag<T...>::type;

template <>
struct from_type_tag<type::bool_t> {
  using type = integral;
};
template <>
struct from_type_tag<type::byte_t> {
  using type = integral;
};
template <>
struct from_type_tag<type::i16_t> {
  using type = integral;
};
template <>
struct from_type_tag<type::i32_t> {
  using type = integral;
};
template <>
struct from_type_tag<type::i64_t> {
  using type = integral;
};
template <>
struct from_type_tag<type::float_t> {
  using type = floating_point;
};
template <>
struct from_type_tag<type::double_t> {
  using type = floating_point;
};
template <>
struct from_type_tag<type::string_t> {
  using type = string;
};
template <>
struct from_type_tag<type::binary_t> {
  using type = binary;
};
template <class T>
struct from_type_tag<type::enum_t<T>> {
  using type = enumeration;
};
template <class T>
struct from_type_tag<type::struct_t<T>> {
  using type = structure;
};
template <class T>
struct from_type_tag<type::union_t<T>> {
  using type = variant;
};
template <class Tag>
struct from_type_tag<type::list<Tag>> {
  using type = list<from_type_tag_t<Tag>>;
};
template <class Tag>
struct from_type_tag<type::set<Tag>> {
  using type = set<from_type_tag_t<Tag>>;
};
template <class Key, class Value>
struct from_type_tag<type::map<Key, Value>> {
  using type = map<from_type_tag_t<Key>, from_type_tag_t<Value>>;
};
template <class T, class Tag>
struct from_type_tag<type::cpp_type<T, Tag>> : from_type_tag<Tag> {};
template <class T, class Tag>
struct from_type_tag<
    type::adapted<::apache::thrift::IndirectionAdapter<T>, Tag>> {
  using type = detail::indirection_tag<from_type_tag_t<Tag>, T>;
};
template <class Adapter, class Tag>
struct from_type_tag<type::adapted<Adapter, Tag>> {
  using type = from_type_tag<Tag>;
};

} // namespace type_class
} // namespace thrift
} // namespace apache
