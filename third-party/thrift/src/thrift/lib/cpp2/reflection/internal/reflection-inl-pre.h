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

#ifndef THRIFT_FATAL_REFLECTION_INL_PRE_H_
#define THRIFT_FATAL_REFLECTION_INL_PRE_H_ 1

#if !defined THRIFT_FATAL_REFLECTION_H_
#error "This file must be included from reflection.h"
#endif

namespace apache {
namespace thrift {
namespace detail {

template <typename Tag>
struct invoke_reffer;

struct thru_field_ref_fn {
  template <typename T>
  FOLLY_ERASE constexpr T operator()(field_ref<T> ref) const noexcept {
    return *ref;
  }
  template <typename T>
  FOLLY_ERASE constexpr T operator()(optional_field_ref<T> ref) const noexcept {
    return ref.value_unchecked();
  }
  template <typename T>
  FOLLY_ERASE constexpr T operator()(terse_field_ref<T> ref) const noexcept {
    return *ref;
  }
  template <typename T>
  FOLLY_ERASE constexpr optional_boxed_field_ref<T> operator()(
      optional_boxed_field_ref<T> ref) const noexcept {
    return ref;
  }
  template <typename T>
  FOLLY_ERASE constexpr T operator()(required_field_ref<T> ref) const noexcept {
    return *ref;
  }
  template <
      typename T,
      typename D = folly::remove_cvref_t<T>,
      std::enable_if_t<is_shared_or_unique_ptr_v<D>, int> = 0>
  FOLLY_ERASE constexpr T&& operator()(T&& ref) const noexcept {
    return static_cast<T&&>(ref);
  }
};
inline constexpr thru_field_ref_fn thru_field_ref{};

template <typename Tag>
struct invoke_reffer_thru {
  template <typename... A>
  FOLLY_ERASE constexpr auto operator()(A&&... a) noexcept(
      noexcept(access_field<Tag>(static_cast<A&&>(a)...)))
      -> decltype(thru_field_ref(access_field<Tag>(static_cast<A&&>(a)...))) {
    return thru_field_ref(access_field<Tag>(static_cast<A&&>(a)...));
  }
};

template <typename, typename, bool IsTry, typename Default = void>
struct reflect_module_tag_selector {
  using type = Default;
  static_assert(
      IsTry,
      "given type has no reflection metadata or is not a struct, enum or union");
};

template <typename>
struct reflect_module_tag_get;
template <typename, typename>
struct reflect_module_tag_try_get;
template <typename T>
struct reflect_type_class_of_thrift_class_impl;
template <typename T>
struct reflect_type_class_of_thrift_class_enum_impl;

struct reflection_metadata_tag {};
struct struct_traits_metadata_tag {};

namespace reflection_impl {

// FIXME: There is a bug that is_set(...) always return `true` for cpp.ref field
// We need to investigate how to fix this since it's breaking change.
struct is_set_fn {
  template <class T, class D>
  bool operator()(const std::unique_ptr<T, D>&) const {
    return true;
  }

  template <class T>
  bool operator()(const std::shared_ptr<T>&) const {
    return true;
  }

  template <class T>
  bool operator()(terse_field_ref<T>) const {
    return true;
  }

  template <class T>
  bool operator()(T ref) const {
    return ref.has_value();
  }
};

constexpr is_set_fn is_set;

struct mark_set_fn {
  template <class T>
  void operator()(required_field_ref<T>, bool) const {}

  template <class T>
  void operator()(terse_field_ref<T>, bool) const {}

  template <class T>
  void operator()(field_ref<T> ref, bool b) const {
    if (b) {
      ref.ensure();
    } else {
      ::apache::thrift::unset_unsafe_deprecated(ref);
    }
  }

  template <class T>
  void operator()(optional_field_ref<T> ref, bool b) const {
    if (b) {
      ::apache::thrift::ensure_isset_unsafe_deprecated(ref);
    } else {
      ::apache::thrift::unset_unsafe_deprecated(ref);
    }
  }

  template <class T>
  void operator()(optional_boxed_field_ref<T> ref, bool b) const {
    if (b) {
      ref.ensure();
    } else {
      ref.reset();
    }
  }

  template <class T>
  void operator()(boxed_value_ptr<T>&, bool) const {}

  template <class T, class D>
  void operator()(std::unique_ptr<T, D>&, bool) const {}

  template <class T>
  void operator()(std::shared_ptr<T>&, bool) const {}
};

constexpr mark_set_fn mark_set;

struct variant_member_name {
  template <typename Descriptor>
  using apply = typename Descriptor::metadata::name;
};

struct variant_member_field_id {
  template <typename Descriptor>
  using apply = typename Descriptor::metadata::id;
};

template <typename A>
using data_member_accessor = invoke_reffer_thru<A>;

template <typename... A>
struct chained_data_member_accessor;
template <>
struct chained_data_member_accessor<> {
  template <typename T>
  FOLLY_ERASE constexpr T&& operator()(T&& t) const noexcept {
    return static_cast<T&&>(t);
  }
};
template <typename V, typename... A>
struct chained_data_member_accessor<V, A...> {
  template <typename T>
  FOLLY_ERASE constexpr auto operator()(T&& t) const noexcept(
      noexcept(chained_data_member_accessor<A...>{}(V{}(static_cast<T&&>(t)))))
      -> decltype(
          chained_data_member_accessor<A...>{}(V{}(static_cast<T&&>(t)))) {
    return chained_data_member_accessor<A...>{}(V{}(static_cast<T&&>(t)));
  }
};

template <typename G>
struct getter_direct_getter {
  using type = G;
};
template <typename V, typename... A>
struct getter_direct_getter<chained_data_member_accessor<V, A...>> {
  using type = V;
};
template <typename G>
using getter_direct_getter_t = folly::_t<getter_direct_getter<G>>;

} // namespace reflection_impl
} // namespace detail

#define THRIFT_REGISTER_REFLECTION_METADATA(Tag, Traits) \
  FATAL_REGISTER_TYPE(                                   \
      ::apache::thrift::detail::reflection_metadata_tag, \
      Tag,                                               \
      ::apache::thrift::reflected_module<Traits>)

#define THRIFT_REGISTER_STRUCT_TRAITS(Struct, Traits)       \
  FATAL_REGISTER_TYPE(                                      \
      ::apache::thrift::detail::struct_traits_metadata_tag, \
      Struct,                                               \
      ::apache::thrift::reflected_struct<Traits>)

template <typename = void>
struct reflected_annotations;

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_REFLECTION_INL_PRE_H_
