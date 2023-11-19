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

#include <stdexcept>
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/op/detail/Get.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {

/// Resolves to the number of definitions contained in Thrift class
template <typename T>
inline constexpr std::size_t size_v = detail::pa::__fbthrift_field_size_v<T>;

template <typename T, typename Id>
using get_ordinal =
    typename detail::GetOrdinalImpl<Id, type::infer_tag<T>>::type;

/// Gets the ordinal, for example:
///
/// * using Ord = get_ordinal_v<MyS, ident::foo>
///   // Resolves to ordinal at which the field "foo" was defined in MyS.
///
template <typename T, typename Id>
inline constexpr type::Ordinal get_ordinal_v = get_ordinal<T, Id>::value;

/// Calls the given function with ordinal<1> to ordinal<N>.
template <typename T, typename F>
void for_each_ordinal(F&& f) {
  detail::for_each_ordinal_impl(
      std::forward<F>(f), std::make_integer_sequence<size_t, size_v<T>>{});
}

/// Calls the given function with with ordinal<1> to ordinal<N>, returing the
/// first 'true' result produced.
template <typename T, typename F, std::enable_if_t<size_v<T> != 0>* = nullptr>
decltype(auto) find_by_ordinal(F&& f) {
  return detail::find_by_ordinal_impl(
      std::forward<F>(f), std::make_integer_sequence<size_t, size_v<T>>{});
}

template <typename T, typename F>
std::enable_if_t<size_v<T> == 0, bool> find_by_ordinal(F&&) {
  return false;
}

template <typename T, typename Id>
using get_field_id = folly::conditional_t<
    get_ordinal<T, Id>::value == type::Ordinal{},
    type::field_id<0>,
    detail::pa::field_id<T, get_ordinal<T, Id>>>;

/// Gets the field id, for example:
///
/// * using FieldId = get_field_id<MyS, ident::foo>
///   // Resolves to field id assigned to the field "foo" in MyS.
template <typename T, typename Id>
inline constexpr FieldId get_field_id_v = get_field_id<T, Id>::value;

/// Calls the given function with each field_id<{id}> in Thrift class.
template <typename T, typename F>
void for_each_field_id(F&& f) {
  for_each_ordinal<T>([&](auto ord) { f(get_field_id<T, decltype(ord)>{}); });
}

/// Calls the given function with with each field_id<{id}>, returning the
/// first 'true' result produced.
template <typename T, typename F>
decltype(auto) find_by_field_id(F&& f) {
  return find_by_ordinal<T>(
      [&](auto ord) { return f(get_field_id<T, decltype(ord)>{}); });
}

/// Gets the ident, for example:
///
///   // Resolves to thrift::ident::* type associated with field 7 in MyS.
///   using Ident = get_ident<MyS, field_id<7>>
///
template <typename T, typename Id>
using get_ident = detail::pa::ident<T, get_ordinal<T, Id>>;

/// It calls the given function with each folly::tag<thrift::ident::*>{} in
/// Thrift class.
template <typename T, typename F>
void for_each_ident(F&& f) {
  for_each_ordinal<T>(
      [&](auto ord) { f(folly::tag_t<get_ident<T, decltype(ord)>>{}); });
}

/// Gets the Thrift type tag, for example:
///
///   // Resolves to Thrift type tag for the field "foo" in MyS.
///   using Tag = get_type_tag<MyS, ident::foo>
///
template <typename T, typename Id>
using get_type_tag = detail::pa::type_tag<T, get_ordinal<T, Id>>;

template <typename T, typename Id>
using get_field_tag = typename std::conditional_t<
    get_ordinal<T, Id>::value == type::Ordinal{},
    void,
    type::field<
        get_type_tag<T, Id>,
        FieldContext<T, folly::to_underlying(get_field_id_v<T, Id>)>>>;

template <typename T, typename Id>
using get_native_type = type::native_type<get_field_tag<T, Id>>;

/// Gets the thrift field name, for example:
///
/// * op::get_name_v<MyStruct, field_id<7>>
///   // Returns the thrift field name associated with field 7 in MyStruct.
///
template <typename T, typename Id>
inline const folly::StringPiece get_name_v =
    detail::pa::__fbthrift_get_field_name<T, get_ordinal<T, Id>>();

/// Gets the thrift class name, for example:
///
/// * op::get_class_name_v<MyStruct> == "MyStruct"
///
template <typename T>
inline const folly::StringPiece get_class_name_v =
    detail::pa::__fbthrift_get_class_name<T>();

/// Gets the Thrift field, for example:
///
///   op::get<type::field_id<7>>(myStruct) = 4;
///
template <typename Id = void, typename T = void>
inline constexpr detail::Get<Id, T> get = {};

/// Returns pointer to the value from the given field.
/// Returns nullptr if it doesn't have a value.
/// For example:
/// * get_value_or_null(foo.field_ref())
///   // returns foo.field_ref().value()
/// * get_value_or_null(foo.smart_ptr_ref())
///   // returns *foo.smart_ptr_ref()
/// * get_value_or_null(foo.optional_ref())
///   // returns nullptr if optional field doesn't have a value.
inline constexpr detail::GetValueOrNull getValueOrNull;

/// Gets the field ref type of Thrift field, for example:
///
///     std::is_same_v<
///         get_field_ref<MyS, ident::foo>,
///         optional_field_ref<std::string&>>;
///
template <typename T, typename Id>
using get_field_ref =
    folly::remove_cvref_t<decltype(get<Id>(std::declval<T&>()))>;

// Implementation details.
namespace detail {
template <typename Id, typename Tag>
struct GetOrdinalImpl {
  // TODO(ytj): To reduce build time, only check whether Id is reflection
  // metadata if we couldn't find Id.
  static_assert(type::is_id_v<Id>, "");
  using type = detail::pa::ordinal<type::native_type<Tag>, Id>;
};

template <type::Ordinal Ord, typename Tag>
struct GetOrdinalImpl<std::integral_constant<type::Ordinal, Ord>, Tag> {
  static_assert(
      folly::to_underlying(Ord) <= size_v<type::native_type<Tag>>,
      "Ordinal cannot be larger than the number of definitions");

  // Id is an ordinal, return itself
  using type = type::ordinal_tag<Ord>;
};

template <typename TypeTag, typename Struct, int16_t Id, typename Tag>
struct GetOrdinalImpl<type::field<TypeTag, FieldContext<Struct, Id>>, Tag>
    : GetOrdinalImpl<field_id<Id>, Tag> {};

template <size_t... I, typename F>
void for_each_ordinal_impl(F&& f, std::index_sequence<I...>) {
  // This doesn't use fold expression (from C++17) as this file is used in
  // C++14 environment as well.
  int unused[] = {0, (f(type::detail::pos_to_ordinal<I>{}), 0)...};
  static_cast<void>(unused);
}

template <size_t... I, typename F>
ord_result_t<F> find_by_ordinal_impl(F&& f, std::index_sequence<I...>) {
  auto result = ord_result_t<F>();
  // TODO(afuller): Use a short circuting c++17 folding expression.
  for_each_ordinal_impl(
      [&](auto id) {
        auto found = f(id);
        if (static_cast<bool>(found)) {
          result = std::move(found);
        }
      },
      std::index_sequence<I...>{});
  return result;
}

template <typename Id, typename T, typename>
struct Get {
  template <typename U>
  constexpr decltype(auto) operator()(U&& obj) const {
    return access_field<get_ident<T, Id>>(std::forward<U>(obj));
  }
};
template <typename Id, typename Tag>
struct Get<Id, Tag, type::if_concrete<Tag>> {
  using T = type::native_type<Tag>;
  constexpr decltype(auto) operator()(T& obj) const {
    return op::get<Id, T>(obj);
  }
  constexpr decltype(auto) operator()(T&& obj) const {
    return op::get<Id, T>(std::move(obj));
  }
  constexpr decltype(auto) operator()(const T& obj) const {
    return op::get<Id, T>(obj);
  }
  constexpr decltype(auto) operator()(const T&& obj) const {
    return op::get<Id, T>(std::move(obj));
  }
};

template <typename Id>
struct Get<Id, void> {
  template <typename U>
  constexpr decltype(auto) operator()(U&& obj) const {
    return op::get<Id, folly::remove_cvref_t<U>>(std::forward<U>(obj));
  }
};
template <>
struct Get<void, void> {
  template <typename Id, typename U>
  constexpr decltype(auto) operator()(Id, U&& obj) const {
    return op::get<Id, folly::remove_cvref_t<U>>(std::forward<U>(obj));
  }
};

// Helper to get adapter type from Thrift type tag.
template <typename Tag>
struct get_adapter {
  static_assert(sizeof(Tag) == 0, "Not adapter.");
};
template <typename UTag, typename Context>
struct get_adapter<type::field<UTag, Context>> : get_adapter<UTag> {};
template <typename Adapter, typename UTag>
struct get_adapter<type::adapted<Adapter, UTag>> {
  using type = Adapter;
};
template <typename Tag>
using get_adapter_t = typename get_adapter<Tag>::type;

template <typename T, std::size_t pos = 0>
class InvokeByFieldId {
  static constexpr auto N = size_v<T>;

  // We use std::min to avoid index > N.
  template <std::size_t Ordinal>
  using OrdToFieldId = get_field_id<T, field_ordinal<std::min(Ordinal, N)>>;

 public:
  template <
      typename F,
      std::enable_if_t<sizeof(F) != 0 && pos != N, bool> = false>
  FOLLY_ALWAYS_INLINE constexpr decltype(auto) operator()(
      FieldId id, F&& f) const {
    // By default clang's maximum depth of recursive template instantiation is
    // 512. If we handle 8 cases at a time, it works with struct that has 4096
    // fields.
    if (id == OrdToFieldId<pos + 1>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 1>{});
    } else if (id == OrdToFieldId<pos + 2>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 2>{});
    } else if (id == OrdToFieldId<pos + 3>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 3>{});
    } else if (id == OrdToFieldId<pos + 4>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 4>{});
    } else if (id == OrdToFieldId<pos + 5>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 5>{});
    } else if (id == OrdToFieldId<pos + 6>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 6>{});
    } else if (id == OrdToFieldId<pos + 7>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 7>{});
    } else if (id == OrdToFieldId<pos + 8>::value) {
      return std::forward<F>(f)(OrdToFieldId<pos + 8>{});
    }

    return InvokeByFieldId<T, std::min(pos + 8, N)>{}(id, std::forward<F>(f));
  }

  template <
      typename F,
      std::enable_if_t<sizeof(F) != 0 && pos == N, bool> = false>
  FOLLY_ALWAYS_INLINE constexpr decltype(auto) operator()(
      FieldId, F&& f) const {
    // If not found, f() will be invoked.
    return std::forward<F>(f)();
  }

  template <typename... F>
  FOLLY_ALWAYS_INLINE constexpr decltype(auto) operator()(
      FieldId id, F&&... f) const {
    return operator()(id, folly::overload(std::forward<F>(f)...));
  }
};

} // namespace detail

/// Given a Thrift struct, callback with a runtime dynamic field id, convert it
/// to static compile-time field id and invoke the callback. For example,
///
/// `invoke_by_field_id<T>(FieldId{10}, f)` invokes `f(field_id<10>{})`.
///
/// If field id is not found in T, `f()` will be invoked.
///
/// In addition, `invoke_by_field_id<T>(id, f...)` is a syntactic sugar of
/// `invoke_by_field_id<T>(id, folly::overload(f...))`.
/// WARNING: inline expansion will always be applied to the call sites.
template <typename T>
inline constexpr detail::InvokeByFieldId<T> invoke_by_field_id{};

} // namespace op
} // namespace thrift
} // namespace apache
