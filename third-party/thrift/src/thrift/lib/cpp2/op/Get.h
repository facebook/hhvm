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

#include <memory>
#include <optional>
#include <type_traits>

#include <folly/CPortability.h>
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

namespace apache::thrift::op {
namespace detail {

using pa = ::apache::thrift::detail::st::private_access;

template <typename Id, typename T>
struct Get;

// Similar to std::find, but returns ordinal.
template <class T>
FOLLY_CONSTEVAL type::Ordinal findOrdinal(
    const T* first, const T* last, const T& value) {
  for (const T* iter = first; iter != last; ++iter) {
    if (*iter == value) {
      return static_cast<type::Ordinal>(iter - first + 1);
    }
  }

  return static_cast<type::Ordinal>(0);
}

template <class T, class List>
class FindOrdinal {
  static_assert(folly::always_false<T>);
};

template <class T, class... Args>
class FindOrdinal<T, folly::tag_t<Args...>> {
 private:
  static constexpr bool matches[sizeof...(Args)] = {std::is_same_v<T, Args>...};

 public:
  static constexpr auto value = findOrdinal(matches, std::end(matches), true);
  static FOLLY_CONSTEVAL size_t count() {
    size_t count = 0;
    for (bool b : matches) {
      count += b;
    }
    return count;
  }
};

template <class T, class List>
inline constexpr type::Ordinal FindOrdinalInUniqueTypes =
    FindOrdinal<T, List>::value;

#if defined(__clang__) && \
    !defined(THRIFT_DISABLE_REFLECTION_MULTIWAY_LOOKUP_OPTIMIZATION)
// For now only enable for __clang__ due to bugs in MSVC/GCC

template <int>
struct IntTag {};

#define FBTHRIFT_LOOKUP_SIZE 63
#define FBTHRIFT_PARAMS_WITH_DEFAULT(Z, NUM, TEXT) \
  BOOST_PP_COMMA_IF(NUM) class A##NUM = IntTag<NUM>

//  The struct is defined like this
//
//    template<
//      class A0 = IntTag<0>,
//      class A1 = IntTag<1>,
//      ...
//      class AN = IntTag<N>>
//    struct MultiWayLookup {
//      template <class> struct value : std::integral_constant<int, 0> {};
//      template <> struct value<A0> : std::integral_constant<int, 0 + 1> {};
//      template <> struct value<A1> : std::integral_constant<int, 1 + 1> {};
//      ...
//      template <> struct value<AN> : std::integral_constant<int, N + 1> {};
//    }
//
// So that MultiWayLookup<Args...>::value<T> returns Ordinal of T in [Args...]
// list with O(1) lookup time (instead of traditionally scanning [Args...])
template <BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_PARAMS_WITH_DEFAULT, )>
struct MultiWayLookup {
  template <class>
  struct value : std::integral_constant<int, 0> {};

#define FBTHRIFT_DEFINE_VALUE_SPECIALIZATION(Z, NUM, TEXT) \
  template <>                                              \
  struct value<A##NUM> : std::integral_constant<int, NUM + 1> {};

  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_DEFINE_VALUE_SPECIALIZATION, )

#undef FBTHRIFT_DEFINE_VALUE_SPECIALIZATION
};
#undef FBTHRIFT_TEMPLATE_PARAMS_WITH_DEFAULT

// Find type T in list [Args...].
// FindOrdinalInUniqueTypesImpl::value will be the ordinal of T in [Args...]
// If not found, value will be Ordinal{0}
//
// Limitation: [Args...] can not have duplicated type, otherwise build failure.
template <class T, class... Args>
struct FindOrdinalInUniqueTypesImpl;

// If Found is not 0, we have found T in Args...
// In which case value = Found
template <int Found, class... Args>
struct FoundOrdinalOrCheckTheRest : field_ordinal<Found> {};

// Otherwise check the rest
// If found, value = FindOrdinalInUniqueTypesImpl<Args...>::value + SIZE
// If not found, value = 0
template <type::Ordinal ord>
using Rest = field_ordinal<
    ord == static_cast<type::Ordinal>(0)
        ? 0
        : folly::to_underlying(ord) + FBTHRIFT_LOOKUP_SIZE>;

template <class... Args>
struct FoundOrdinalOrCheckTheRest<0, Args...>
    : Rest<FindOrdinalInUniqueTypesImpl<Args...>::value> {};

#define FBTHRIFT_TEMPLATE_PARAMS(Z, NUM, TEXT) \
  BOOST_PP_COMMA_IF(NUM) TEXT A##NUM

// This macro will be expanded to "A0, A1, A2, ..., AN"
#define FBTHRIFT_PARAMS \
  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_TEMPLATE_PARAMS, )

// This macro will be expanded to "class A0, class A1, class A2, ..., class AN"
#define FBTHRIFT_PARAMS_WITH_CLASS \
  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_TEMPLATE_PARAMS, class)

// If size of (Args...) < FBTHRIFT_LOOKUP_SIZE: just do a multiway lookup
template <class T, class... Args>
struct FindOrdinalInUniqueTypesImpl
    : field_ordinal<static_cast<int>(
          typename MultiWayLookup<Args...>::template value<T>())> {};

// If size of (Args...) > FBTHRIFT_LOOKUP_SIZE: used batched multiway lookup
template <class T, FBTHRIFT_PARAMS_WITH_CLASS, class... Args>
struct FindOrdinalInUniqueTypesImpl<T, FBTHRIFT_PARAMS, Args...>
    : FoundOrdinalOrCheckTheRest<
          static_cast<int>(
              typename MultiWayLookup<FBTHRIFT_PARAMS>::template value<T>()),
          T,
          Args...> {};

#undef FBTHRIFT_TEMPLATE_PARAMS
#undef FBTHRIFT_PARAMS
#undef FBTHRIFT_PARAMS_WITH_CLASS
#undef FBTHRIFT_LOOKUP_SIZE

template <class T, class... Args>
inline constexpr type::Ordinal
    FindOrdinalInUniqueTypes<T, folly::tag_t<Args...>> =
        FindOrdinalInUniqueTypesImpl<T, Args...>::value;
#endif

template <class Id, class Idents, class TypeTags>
FOLLY_CONSTEVAL std::enable_if_t<std::is_same_v<Id, void>, FieldOrdinal>
getFieldOrdinal(const int16_t*, size_t) {
  return static_cast<FieldOrdinal>(0);
}

template <class Id, class Idents, class TypeTags>
FOLLY_CONSTEVAL std::enable_if_t<type::is_field_id_v<Id>, FieldOrdinal>
getFieldOrdinal(const int16_t* ids, size_t numFields) {
  return findOrdinal(
      ids + 1, ids + numFields + 1, folly::to_underlying(Id::value));
}

template <class Id, class Idents, class TypeTags>
FOLLY_CONSTEVAL std::enable_if_t<type::is_ident_v<Id>, FieldOrdinal>
getFieldOrdinal(const int16_t*, size_t) {
  return FindOrdinalInUniqueTypes<Id, Idents>;
}

template <class Id, class Idents, class TypeTags>
FOLLY_CONSTEVAL std::enable_if_t<type::detail::is_type_tag_v<Id>, FieldOrdinal>
getFieldOrdinal(const int16_t*, size_t) {
  static_assert(
      FindOrdinal<Id, TypeTags>::count() <= 1, "Type Tag is not unique");
  return FindOrdinal<Id, TypeTags>::value;
}

template <typename Id, typename Tag>
struct get_ordinal_impl {
  using native_type = type::native_type<Tag>;

  // TODO(ytj): To reduce build time, only check whether Id is reflection
  // metadata if we couldn't find Id.
  static_assert(type::is_id_v<Id>);

  using type = type::ordinal_tag<getFieldOrdinal<
      Id,
      decltype(pa::idents<native_type>()),
      decltype(pa::type_tags<native_type>())>(
      pa::field_ids<native_type>(), pa::num_fields<native_type>)>;
};

template <type::Ordinal Ord, typename Tag>
struct get_ordinal_impl<std::integral_constant<type::Ordinal, Ord>, Tag> {
  static_assert(
      folly::to_underlying(Ord) <= pa::num_fields<type::native_type<Tag>>,
      "Ordinal cannot be larger than the number of fields");

  // Id is an ordinal, return itself.
  using type = type::ordinal_tag<Ord>;
};

template <typename TypeTag, typename Struct, int16_t Id, typename Tag>
struct get_ordinal_impl<type::field<TypeTag, FieldContext<Struct, Id>>, Tag>
    : get_ordinal_impl<field_id<Id>, Tag> {};

template <size_t... I, typename F>
constexpr void for_each_ordinal_impl(F&& f, std::index_sequence<I...>);

template <typename F, size_t I = 0>
using ord_result_t =
    decltype(std::declval<F>()(type::detail::pos_to_ordinal<I>{}));

template <size_t... I, typename F>
ord_result_t<F> find_by_ordinal_impl(F&& f, std::index_sequence<I...>);

struct GetValueOrNull {
  template <typename T>
  auto* operator()(field_ref<T&> ref) const {
    return &ref.value();
  }
  template <typename T>
  auto* operator()(required_field_ref<T&> ref) const {
    return &ref.value();
  }
  template <typename T>
  auto* operator()(optional_field_ref<T&> ref) const {
    return ref.has_value() ? &ref.value() : nullptr;
  }
  template <typename T>
  auto* operator()(optional_boxed_field_ref<T&> ref) const {
    return ref.has_value() ? &ref.value() : nullptr;
  }
  template <typename T>
  auto* operator()(terse_field_ref<T&> ref) const {
    return &ref.value();
  }
  template <typename T>
  auto* operator()(union_field_ref<T&> ref) const {
    return ref.has_value() ? &ref.value() : nullptr;
  }
  template <typename T>
  auto* operator()(terse_intern_boxed_field_ref<T&> ref) const {
    return &ref.value();
  }
  template <typename T>
  auto* operator()(intern_boxed_field_ref<T&> ref) const {
    return &ref.value();
  }

  template <typename T>
  T* operator()(std::optional<T>& opt) const {
    return opt ? &opt.value() : nullptr;
  }
  template <typename T>
  const T* operator()(const std::optional<T>& opt) const {
    return opt ? &opt.value() : nullptr;
  }

  template <typename T, typename Deleter>
  T* operator()(const std::unique_ptr<T, Deleter>&& ptr) const = delete;
  template <typename T, typename Deleter>
  T* operator()(const std::unique_ptr<T, Deleter>& ptr) const {
    return ptr ? ptr.get() : nullptr;
  }
  template <typename T>
  T* operator()(const std::shared_ptr<T>&& ptr) const = delete;
  template <typename T>
  T* operator()(const std::shared_ptr<T>& ptr) const {
    return ptr ? ptr.get() : nullptr;
  }
};

} // namespace detail

/// Gives the number of fields in a Thrift struct, union or exception.
template <typename T>
inline constexpr std::size_t num_fields = detail::pa::num_fields<T>;

template <typename T, typename Id>
using get_ordinal =
    typename detail::get_ordinal_impl<Id, type::infer_tag<T>>::type;

/// Gets the ordinal, for example:
///
/// * using Ord = get_ordinal_v<MyS, ident::foo>
///   // Resolves to ordinal at which the field "foo" was defined in MyS.
///
template <typename T, typename Id>
inline constexpr type::Ordinal get_ordinal_v = get_ordinal<T, Id>::value;

/// Calls the given function with ordinal<1> to ordinal<N>.
template <typename T, typename F>
constexpr void for_each_ordinal(F&& f) {
  detail::for_each_ordinal_impl(
      std::forward<F>(f), std::make_integer_sequence<size_t, num_fields<T>>{});
}

/// Calls the given function with with ordinal<1> to ordinal<N>, returing the
/// first 'true' result produced.
template <
    typename T,
    typename F,
    std::enable_if_t<num_fields<T> != 0>* = nullptr>
decltype(auto) find_by_ordinal(F&& f) {
  return detail::find_by_ordinal_impl(
      std::forward<F>(f), std::make_integer_sequence<size_t, num_fields<T>>{});
}

template <typename T, typename F>
std::enable_if_t<num_fields<T> == 0, bool> find_by_ordinal(F&&) {
  return false;
}

template <typename T, typename Id>
using get_field_id = type::field_id<
    get_ordinal<T, Id>::value == type::Ordinal()
        ? 0
        : detail::pa::field_ids<T>()[static_cast<size_t>(
              get_ordinal<T, Id>::value)]>;

/// Gets the field id, for example:
///
///   using FieldId = get_field_id<MyStruct, apache::thrift::ident::foo>;
///   // Resolves to the id of the field `foo` in `MyStruct`.
template <typename T, typename Id>
inline constexpr FieldId get_field_id_v = get_field_id<T, Id>::value;

/// Calls the given function with each field_id<{id}> in a Thrift struct.
template <typename T, typename F>
constexpr void for_each_field_id(F&& f) {
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
using get_ident = apache::thrift::detail::
    at<decltype(detail::pa::idents<T>()), get_ordinal<T, Id>::value>;

/// Calls the given function with each folly::tag<apache::thrift::ident::*> in a
/// Thrift structured type.
template <typename T, typename F>
constexpr void for_each_ident(F&& f) {
  for_each_ordinal<T>(
      [&](auto ord) { f(folly::tag_t<get_ident<T, decltype(ord)>>{}); });
}

/// Gets the Thrift type tag, for example:
///
///   // Resolves to Thrift type tag for the field `foo` in `MyStruct`.
///   using Tag = get_type_tag<MyStruct, apache::thriftident::foo>;
///
template <typename T, typename Id>
using get_type_tag = apache::thrift::detail::
    at<decltype(detail::pa::type_tags<T>()), get_ordinal<T, Id>::value>;

template <typename T, typename Id>
using get_field_tag = typename std::conditional_t<
    get_ordinal<T, Id>::value == type::Ordinal{},
    void,
    type::field<
        get_type_tag<T, Id>,
        FieldContext<T, folly::to_underlying(get_field_id_v<T, Id>)>>>;

template <typename T, typename Id>
using get_native_type = type::native_type<get_field_tag<T, Id>>;

FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wglobal-constructors")

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

FOLLY_POP_WARNING

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
inline constexpr detail::GetValueOrNull get_value_or_null;

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

template <size_t... I, typename F>
constexpr void for_each_ordinal_impl(F&& f, std::index_sequence<I...>) {
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

template <typename Id, typename T>
struct Get {
  template <typename U>
  constexpr decltype(auto) operator()(U&& obj) const {
    return access_field<get_ident<T, Id>>(std::forward<U>(obj));
  }
};
template <typename Id, type::ConcreteThriftTypeTag Tag>
struct Get<Id, Tag> {
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
  static_assert(folly::always_false<Tag>, "Not adapter.");
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
  static constexpr auto N = num_fields<T>;

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

/// Applies the callable to active member of thrift union. Example:
///
///   T thriftUnion;
///
///   op::visit_union_with_tag(
///     thriftUnion,
///     [](folly::tag_t<ident::int_field>, int& f) {
///       LOG(INFO) << "Int value: " << f;
///     },
///     [](folly::tag_t<ident::string_field>, std::string& f) {
///       LOG(INFO) << "String value: " << f;
///     },
///     []() { LOG(INFO) << "No active field"; });
///
///   op::visit_union_with_tag(
///     thriftUnion,
///     []<typename ident>(folly::tag_t<ident>, auto& value) {
///       LOG(INFO) << op::get_name_v<T, ident> << " --> " << value;
///     },
///      []() { LOG(INFO) << "Empty union"; });
///
/// If union is empty, callable will be called with no arguments.
///
/// @param t thrift union
/// @param f... one or more callables that accepts all member types from union

template <typename T, typename F>
constexpr decltype(auto) visit_union_with_tag(T&& t, F&& f) {
  using Type = folly::remove_cvref_t<T>;
  static_assert(is_thrift_union_v<Type>, "T must be a thrift union");

  return invoke_by_field_id<Type>(
      static_cast<FieldId>(t.getType()),
      [&](auto id) -> decltype(auto) {
        using Ident = get_ident<Type, decltype(id)>;
        return std::forward<F>(f)(
            folly::tag_t<Ident>{}, *get<Ident>(std::forward<T>(t)));
      },
      [&]() -> decltype(auto) { return std::forward<F>(f)(); });
}
template <typename T, typename... F>
FOLLY_ALWAYS_INLINE constexpr decltype(auto) visit_union_with_tag(
    T&& t, F&&... f) {
  return visit_union_with_tag(
      std::forward<T>(t), folly::overload(std::forward<F>(f)...));
}

} // namespace apache::thrift::op
