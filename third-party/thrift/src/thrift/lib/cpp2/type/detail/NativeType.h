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

#include <map>
#include <set>
#include <string>
#include <vector>

#include <folly/Traits.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace type {
namespace detail {

// All the native_types for the given tag.
template <typename Tag>
struct native_types {
  // No types to declare for non concrete types.
  static_assert(is_abstract_v<Tag>, "");
};

// Resolves the concrete template type when paramaterizing the given template,
// with the standard types of the give Tags.
template <template <typename...> class TemplateT, typename... Tags>
using standard_template_t =
    TemplateT<typename native_types<Tags>::standard_type...>;

// Resolves the concrete template type when paramaterizing the given template,
// with the native types of the give Tags.
template <template <typename...> class TemplateT, typename... Tags>
using native_template_t =
    TemplateT<typename native_types<Tags>::native_type...>;
template <template <typename...> class TemplateT, typename... Tags>
struct native_template {
  using type = native_template_t<TemplateT, Tags...>;
};

// The (mixin) native_types all concrete types (_t suffix) define.
//
// All concrete types have an associated set of standard types and a native type
// (which by default is just the first standard type).
template <typename StandardType, typename NativeType = StandardType>
struct concrete_type {
  using standard_type = StandardType;
  using native_type = NativeType;
};
struct nonconcrete_type {};

template <template <typename...> class T, typename ValTag>
using parameterized_type = folly::conditional_t<
    is_concrete_v<ValTag>,
    concrete_type<standard_template_t<T, ValTag>, native_template_t<T, ValTag>>,
    nonconcrete_type>;

template <template <typename...> class T, typename KeyTag, typename ValTag>
using parameterized_kv_type = folly::conditional_t<
    is_concrete_v<KeyTag> && is_concrete_v<ValTag>,
    concrete_type<
        standard_template_t<T, KeyTag, ValTag>,
        native_template_t<T, KeyTag, ValTag>>,
    nonconcrete_type>;

template <>
struct native_types<void_t> : concrete_type<void> {};

// The native types for all primitive types.
template <>
struct native_types<bool_t> : concrete_type<bool> {};
template <>
struct native_types<byte_t> : concrete_type<int8_t> {};
template <>
struct native_types<i16_t> : concrete_type<int16_t> {};
template <>
struct native_types<i32_t> : concrete_type<int32_t> {};
template <>
struct native_types<i64_t> : concrete_type<int64_t> {};
template <>
struct native_types<float_t> : concrete_type<float> {};
template <>
struct native_types<double_t> : concrete_type<double> {};
template <>
struct native_types<string_t> : concrete_type<std::string> {};
template <>
struct native_types<binary_t> : concrete_type<std::string> {};

// Traits for enums.
template <typename E>
struct native_types<enum_t<E>> : concrete_type<E> {};

// Traits for structs.
template <typename T>
struct native_types<struct_t<T>> : concrete_type<T> {};

// Traits for unions.
template <typename T>
struct native_types<union_t<T>> : concrete_type<T> {};

// Traits for excpetions.
template <typename T>
struct native_types<exception_t<T>> : concrete_type<T> {};

// Traits for lists.
template <typename ValTag>
struct native_types<type::list<ValTag>>
    : parameterized_type<std::vector, ValTag> {};

// Traits for sets.
template <typename KeyTag>
struct native_types<set<KeyTag>> : parameterized_type<std::set, KeyTag> {};

// Traits for maps.
template <typename KeyTag, typename ValTag>
struct native_types<map<KeyTag, ValTag>>
    : parameterized_kv_type<std::map, KeyTag, ValTag> {};

// Traits for adapted types.
//
// Adapted types are concrete and have an adapted native_type.
template <typename Adapter, typename Tag>
struct native_types<adapted<Adapter, Tag>>
    : concrete_type<
          typename native_types<Tag>::standard_type,
          adapt_detail::
              adapted_t<Adapter, typename native_types<Tag>::native_type>> {};

// Traits for cpp_type types.
//
// cpp_type types are concrete and have special native_type.
template <typename T, typename Tag>
struct native_types<cpp_type<T, Tag>>
    : concrete_type<typename native_types<Tag>::standard_type, T> {};

// Traits for field type tag.
//
// TODO(dokwon): Remove this after field_t migration.
template <typename Tag, FieldId Id>
struct native_types<field_t<Id, Tag>> : native_types<Tag> {};

template <typename Tag, typename Context>
struct native_types<field<Tag, Context>> : native_types<Tag> {};

// Traits for field adapted types.
//
// Field adapted types are concrete and have an adapted native_type.
template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct native_types<field<adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>>
    : concrete_type<
          typename native_types<Tag>::standard_type,
          adapt_detail::adapted_field_t<
              Adapter,
              FieldId,
              typename native_types<Tag>::native_type,
              Struct>> {};

} // namespace detail
} // namespace type
} // namespace thrift
} // namespace apache
