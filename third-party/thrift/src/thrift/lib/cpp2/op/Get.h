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

#include <folly/Traits.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/op/detail/Get.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace apache {
namespace thrift {
namespace op {

// Resolves to the number of definitions contained in Thrift class.
template <typename S>
FOLLY_INLINE_VARIABLE constexpr std::size_t size_v =
    detail::pa::__fbthrift_field_size_v<S>;

// Gets the ordinal, for example:
//
//   // Resolves to ordinal at which the field "foo" was defined in MyS.
//   using Ord = get_ordinal<MyS, ident::foo>
//
template <class S, class Id>
using get_ordinal =
    typename detail::GetOrdinalImpl<type::infer_tag<S>, Id>::type;
template <class S, class Id>
FOLLY_INLINE_VARIABLE constexpr type::Ordinal get_ordinal_v =
    get_ordinal<S, Id>::value;

// It calls the given function with ordinal<1> to ordinal<N>.
template <typename S, typename F>
void for_each_ordinal(F&& f) {
  detail::for_each_ordinal_impl(
      std::forward<F>(f), std::make_integer_sequence<size_t, size_v<S>>{});
}

// Gets the field id, for example:
//
//   // Resolves to field id assigned to the field "foo" in MyS.
//   using FieldId = get_field_id<MyS, ident::foo>
//
template <class S, class Id>
using get_field_id = folly::conditional_t<
    get_ordinal<S, Id>::value == type::Ordinal{},
    field_id<0>,
    detail::pa::field_id<S, get_ordinal<S, Id>>>;
template <class S, class Id>
FOLLY_INLINE_VARIABLE constexpr FieldId get_field_id_v =
    get_field_id<S, Id>::value;

// It calls the given function with each field_id<{id}> in Thrift class.
template <typename S, typename F>
void for_each_field_id(F&& f) {
  for_each_ordinal<S>([&](auto ord) { f(get_field_id<S, decltype(ord)>{}); });
}

// Gets the ident, for example:
//
//   // Resolves to thrift::ident::* type associated with field 7 in MyS.
//   using Ident = get_field_id<MyS, field_id<7>>
//
template <class S, class Id>
using get_ident = detail::pa::ident<S, get_ordinal<S, Id>>;

// It calls the given function with each folly::tag<thrift::ident::*>{} in
// Thrift class.
template <typename S, typename F>
void for_each_ident(F&& f) {
  for_each_ordinal<S>(
      [&](auto ord) { f(folly::tag_t<get_ident<S, decltype(ord)>>{}); });
}

// Gets the Thrift type tag, for example:
//
//   // Resolves to Thrift type tag for the field "foo" in MyS.
//   using Tag = get_field_id<MyS, ident::foo>
//
template <typename S, typename Id>
using get_type_tag = detail::pa::type_tag<S, get_ordinal<S, Id>>;

template <class S, class Id>
using get_field_tag = typename std::conditional_t<
    get_ordinal<S, Id>::value == type::Ordinal{},
    void,
    type::field<
        get_type_tag<S, Id>,
        FieldContext<S, folly::to_underlying(get_field_id<S, Id>::value)>>>;

template <class S, class Id>
using get_native_type = type::native_type<get_field_tag<S, Id>>;

// Gets the thrift field name, for example:
//
//   // Returns the thrift field name associated with field 7 in MyStruct.
//   op::get_name_v<MyStruct, field_id<7>>
//
template <typename S, class Id>
FOLLY_INLINE_VARIABLE const folly::StringPiece get_name_v =
    detail::pa::__fbthrift_get_field_name<S, get_ordinal<S, Id>>();
// TODO(afuller): Migrate usage a remove.
template <typename S, class Id>
FOLLY_INLINE_VARIABLE const folly::StringPiece get_name =
    detail::pa::__fbthrift_get_field_name<S, get_ordinal<S, Id>>();

// Gets the Thrift field, for example:
//
//   op::get<MyStruct, type::field_id<7>>(myStruct) = 4;
template <typename S, class Id>
FOLLY_INLINE_VARIABLE constexpr auto get = access_field<get_ident<S, Id>>;

// Returns pointer to the value from the given field.
// Returns nullptr if it doesn't have a value.
// For example:
//   // returns foo.field_ref().value()
//   get_value_or_null(foo.field_ref())
//   // returns *foo.smart_ptr_ref()
//   get_value_or_null(foo.smart_ptr_ref())
//   // returns nullptr if optional field doesn't have a value.
//   get_value_or_null(foo.optional_ref())
FOLLY_INLINE_VARIABLE constexpr detail::GetValueOrNull getValueOrNull;

// Implementation details.
namespace detail {
template <class Tag, class Id>
struct GetOrdinalImpl {
  // TODO(ytj): To reduce build time, only check whether Id is reflection
  // metadata if we couldn't find Id.
  static_assert(type::is_id_v<Id>, "");
  using type = detail::pa::ordinal<type::native_type<Tag>, Id>;
};

template <class Tag, type::Ordinal Ord>
struct GetOrdinalImpl<Tag, std::integral_constant<type::Ordinal, Ord>> {
  static_assert(
      folly::to_underlying(Ord) <= size_v<type::native_type<Tag>>,
      "Ordinal cannot be larger than the number of definitions");

  // Id is an ordinal, return itself
  using type = type::ordinal_tag<Ord>;
};

template <class Tag, class TypeTag, class Struct, int16_t Id>
struct GetOrdinalImpl<Tag, type::field<TypeTag, FieldContext<Struct, Id>>>
    : GetOrdinalImpl<Tag, field_id<Id>> {};

template <size_t... I, typename F>
void for_each_ordinal_impl(F&& f, std::index_sequence<I...>) {
  // This doesn't use fold expression (from C++17) as this file is used in
  // C++14 environment as well.
  int unused[] = {(f(field_ordinal<I + 1>()), 0)...};
  static_cast<void>(unused);
}

} // namespace detail

} // namespace op
} // namespace thrift
} // namespace apache
