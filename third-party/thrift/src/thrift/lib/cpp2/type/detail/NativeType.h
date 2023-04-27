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
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include <folly/Traits.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {

template <class T>
struct InlineAdapter;

namespace type {
namespace detail {

template <typename T, typename Tag>
class Wrap;

// All the NativeTypes for the given tag.
template <typename Tag, typename = void>
struct NativeTypes {
  // No types to declare for non concrete types.
  static_assert(is_abstract_v<Tag>, "");
};

// Infers an appropriate type tag for given non-adapted native type.
template <typename T, typename = void>
struct InferTag;

// Resolves the concrete template type when paramaterizing the given template,
// with the standard types of the give Tags.
template <template <typename...> class TemplateT, typename... Tags>
using standard_template_t =
    TemplateT<typename NativeTypes<Tags>::standard_type...>;

// Resolves the concrete template type when paramaterizing the given template,
// with the native types of the give Tags.
template <template <typename...> class TemplateT, typename... Tags>
using native_template_t = TemplateT<typename NativeTypes<Tags>::native_type...>;
template <template <typename...> class TemplateT, typename... Tags>
struct native_template {
  using type = native_template_t<TemplateT, Tags...>;
};

// The types all concrete types (_t suffix) define.
//
// All concrete types have an associated set of standard types and a native type
// (which by default is just the first standard type).
template <typename StandardType, typename NativeType = StandardType>
struct ConcreteType {
  using standard_type = StandardType;
  using native_type = NativeType;
};
struct AbstractType {};
template <typename Tag>
struct StandardTag {
  using type = Tag;
};
template <typename T, typename Tag>
struct CppTag {
  using type = cpp_type<T, Tag>;
};

template <template <typename...> class T, typename VTag>
using parameterized_type = folly::conditional_t<
    is_concrete_v<VTag>,
    ConcreteType<standard_template_t<T, VTag>, native_template_t<T, VTag>>,
    AbstractType>;

template <template <typename...> class T, typename KTag, typename VTag>
using parameterized_kv_type = folly::conditional_t<
    is_concrete_v<KTag> && is_concrete_v<VTag>,
    ConcreteType<
        standard_template_t<T, KTag, VTag>,
        native_template_t<T, KTag, VTag>>,
    AbstractType>;

template <>
struct NativeTypes<void_t> : ConcreteType<void> {};
template <>
struct InferTag<void> : StandardTag<void_t> {};
template <> // TODO: Consider also std::null_opt
struct InferTag<std::nullptr_t> : StandardTag<void_t> {};
template <typename T, size_t I = sizeof(T)>
struct IntegerTag;
template <typename T>
struct InferTag<T, std::enable_if_t<folly::is_integral_v<T>>> : IntegerTag<T> {
};

// The native types for all primitive types.
template <>
struct NativeTypes<bool_t> : ConcreteType<bool> {};
template <>
struct IntegerTag<bool> : StandardTag<bool_t> {};
template <>
struct NativeTypes<byte_t> : ConcreteType<int8_t> {};
template <>
struct IntegerTag<int8_t> : StandardTag<byte_t> {};
template <typename T>
struct IntegerTag<T, sizeof(int8_t)> : CppTag<T, byte_t> {};
template <>
struct NativeTypes<i16_t> : ConcreteType<int16_t> {};
template <>
struct IntegerTag<int16_t> : StandardTag<i16_t> {};
template <typename T>
struct IntegerTag<T, sizeof(int16_t)> : CppTag<T, i16_t> {};
template <>
struct NativeTypes<i32_t> : ConcreteType<int32_t> {};
template <>
struct IntegerTag<int32_t> : StandardTag<i32_t> {};
template <typename T>
struct IntegerTag<T, sizeof(int32_t)> : CppTag<T, i32_t> {};
template <>
struct NativeTypes<i64_t> : ConcreteType<int64_t> {};
template <>
struct IntegerTag<int64_t> : StandardTag<i64_t> {};
template <typename T>
struct IntegerTag<T, sizeof(int64_t)> : CppTag<T, i64_t> {};
template <>
struct NativeTypes<float_t> : ConcreteType<float> {};
template <>
struct InferTag<float> : StandardTag<float_t> {};
template <>
struct NativeTypes<double_t> : ConcreteType<double> {};
template <>
struct InferTag<double> : StandardTag<double_t> {};
template <>
struct NativeTypes<string_t> : ConcreteType<std::string> {};
template <>
struct NativeTypes<binary_t> : ConcreteType<std::string> {};

// Traits for enums.
template <typename E>
struct NativeTypes<enum_t<E>> : ConcreteType<E> {};

// Traits for structs.
template <typename T>
struct NativeTypes<struct_t<T>> : ConcreteType<T> {};

// Traits for unions.
template <typename T>
struct NativeTypes<union_t<T>> : ConcreteType<T> {};

// Traits for excpetions.
template <typename T>
struct NativeTypes<exception_t<T>> : ConcreteType<T> {};

// Traits for lists.
template <typename VTag>
struct NativeTypes<type::list<VTag>> : parameterized_type<std::vector, VTag> {};

template <typename T>
struct InferTag<std::vector<T>> {
  using type = type::list<typename InferTag<T>::type>;
};

// Traits for sets.
template <typename KTag>
struct NativeTypes<set<KTag>> : parameterized_type<std::set, KTag> {};
template <typename T>
struct InferTag<std::set<T>> {
  using type = type::set<typename InferTag<T>::type>;
};

// Traits for maps.
template <typename KTag, typename VTag>
struct NativeTypes<map<KTag, VTag>>
    : parameterized_kv_type<std::map, KTag, VTag> {};
template <typename K, typename V>
struct InferTag<std::map<K, V>> {
  using type =
      type::map<typename InferTag<K>::type, typename InferTag<V>::type>;
};

// Traits for adapted types.
//
// Adapted types are concrete and have an adapted native_type.
template <typename Adapter, typename Tag>
struct NativeTypes<adapted<Adapter, Tag>>
    : ConcreteType<
          typename NativeTypes<Tag>::standard_type,
          adapt_detail::
              adapted_t<Adapter, typename NativeTypes<Tag>::native_type>> {};

// Traits for cpp_type types.
//
// cpp_type types have a special native_type.
template <typename T, typename Tag>
struct NativeTypes<cpp_type<T, Tag>> : NativeTypes<Tag> {
  using native_type = T;
};

// Traits for field type tag.
//
// TODO(dokwon): Remove this after field_t migration.
template <typename Tag, FieldId Id>
struct NativeTypes<field_t<Id, Tag>> : NativeTypes<Tag> {};

template <typename Tag, typename Context>
struct NativeTypes<field<Tag, Context>> : NativeTypes<Tag> {};

// Traits for field adapted types.
//
// Field adapted types are concrete and have an adapted native_type.
template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct NativeTypes<field<adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>>
    : ConcreteType<
          typename NativeTypes<Tag>::standard_type,
          adapt_detail::adapted_field_t<
              Adapter,
              FieldId,
              typename NativeTypes<Tag>::native_type,
              Struct>> {};

// All generated structured types.
template <typename T>
struct InferTag<T, std::enable_if_t<is_thrift_union_v<T>>> {
  using type = type::union_t<T>;
};
template <typename T>
struct InferTag<T, std::enable_if_t<is_thrift_exception_v<T>>> {
  using type = type::exception_t<T>;
};
template <typename T>
struct InferTag<T, std::enable_if_t<is_thrift_struct_v<T>>> {
  using type = type::struct_t<T>;
};

// Pass through type tags.
template <typename Tag>
struct InferTag<Tag, if_thrift_type_tag<Tag>> {
  using type = Tag;
};

// For Wrap based adapted type, the adapter is InlineAdapter.
template <typename T>
struct InferTag<
    T,
    std::enable_if_t<std::is_base_of<
        Wrap<typename T::underlying_type, typename T::underlying_tag>,
        T>::value>> {
  using type = adapted<InlineAdapter<T>, typename T::underlying_tag>;
};

} // namespace detail
} // namespace type
} // namespace thrift
} // namespace apache
