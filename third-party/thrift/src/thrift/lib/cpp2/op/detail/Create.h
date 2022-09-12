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

#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// Helpers for detecting compatible optional types.
template <typename T>
struct is_optional_type : std::false_type {};
template <typename T>
struct is_optional_type<optional_field_ref<T>> : std::true_type {};
template <typename T>
struct is_optional_type<optional_boxed_field_ref<T>> : std::true_type {};
#ifdef THRIFT_HAS_OPTIONAL
template <typename T>
struct is_optional_type<std::optional<T>> : std::true_type {};
#endif

template <typename T, typename R = void>
using if_opt_type = std::enable_if_t<is_optional_type<T>::value, R>;
template <typename T, typename R = void>
using if_not_opt_type = std::enable_if_t<!is_optional_type<T>::value, R>;

template <typename T>
if_opt_type<T, bool> hasValue(const T& opt) {
  return opt.has_value();
}
template <typename T>
bool hasValue(field_ref<T> val) {
  return !thrift::empty(*val);
}
template <typename T>
bool hasValue(terse_field_ref<T> val) {
  return !thrift::empty(*val);
}

// If the given field is absent/unset/void.
template <typename T>
if_opt_type<T, bool> isAbsent(const T& opt) {
  return !opt.has_value();
}
template <typename T>
constexpr bool isAbsent(union_field_ref<T> opt) {
  return opt.has_value();
}
template <typename T>
constexpr bool isAbsent(field_ref<T>) {
  return false;
}
template <typename T>
constexpr bool isAbsent(terse_field_ref<T>) {
  return false;
}

template <typename T>
if_opt_type<folly::remove_cvref_t<T>> resetValue(T&& opt) {
  opt.reset();
}
template <typename T>
constexpr if_not_opt_type<T> resetValue(T&&) {}

// TODO: use op::clear and op::ensure to avoid duplication
template <typename T>
if_opt_type<folly::remove_cvref_t<T>> clearValue(T&& opt) {
  opt.reset();
}
template <typename T>
if_not_opt_type<T> clearValue(T& unn) {
  thrift::clear(unn);
}

template <typename T>
if_opt_type<T> ensureValue(T& opt) {
  if (!opt.has_value()) {
    opt.emplace();
  }
}
template <typename T>
void ensureValue(union_field_ref<T> val) {
  val.ensure();
}
template <typename T>
void ensureValue(field_ref<T> val) {
  val.ensure();
}
template <typename T>
void ensureValue(terse_field_ref<T>) {
  // A terse field doesn't have a set or unset state, so ensure is a noop.
}

template <typename Tag>
struct Create {
  static_assert(type::is_concrete_v<Tag>, "");

  template <typename T = type::native_type<Tag>>
  constexpr T operator()() const {
    return T{};
  }
};

template <typename>
struct Ensure {};

template <typename Adapter, typename Tag>
struct Create<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");

  template <typename T = type::native_type<adapted_tag>>
  constexpr T operator()() const {
    // Note, we use op::create to create the intrinsic default of underlying
    // type since the underlying type might be another adapted type is not
    // default constructible.
    return Adapter::fromThrift(Create<Tag>{}());
  }
};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct Create<type::field<Tag, Context>> : Create<Tag> {};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct Create<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  static_assert(type::is_concrete_v<field_adapted_tag>, "");

  template <typename AdapterT = Adapter>
  constexpr adapt_detail::
      if_not_field_adapter<AdapterT, type::native_type<Tag>, Struct>
      operator()(Struct&) const {
    return AdapterT::fromThrift(Create<Tag>{}());
  }

  template <typename AdapterT = Adapter>
  constexpr adapt_detail::
      FromThriftFieldIdType<AdapterT, FieldId, type::native_type<Tag>, Struct>
      operator()(Struct& object) const {
    auto obj = AdapterT::fromThriftField(
        Create<Tag>{}(), FieldContext<Struct, FieldId>{object});
    adapt_detail::construct<Adapter, FieldId>(obj, object);
    return obj;
  }
};

// TODO: support adapted field, smart pointers with custom allocators, and union
// TODO: return T& to get value from ensure method and accept a custom default
template <typename Tag, typename Context>
struct Ensure<type::field<Tag, Context>> {
  using field_tag = type::field<Tag, Context>;
  static_assert(type::is_concrete_v<field_tag>, "");
  template <typename T, typename Struct>
  void operator()(field_ref<T&> field_ref, Struct&) const {
    field_ref.ensure();
  }
  template <typename T, typename Struct>
  void operator()(optional_field_ref<T&> field_ref, Struct&) const {
    field_ref.ensure();
  }
  template <typename T, typename Struct>
  void operator()(optional_boxed_field_ref<T&> field_ref, Struct&) const {
    field_ref.ensure();
  }
  template <typename T, typename Struct>
  void operator()(terse_field_ref<T&>, Struct&) const {
    // A terse field doesn't have a set or unset state, so ensure is a noop.
  }
  template <typename T, typename Struct>
  if_opt_type<T> operator()(T& opt, Struct&) const {
    if (!opt.has_value()) {
      opt.emplace();
    }
  }

  template <typename T, typename Struct>
  void operator()(std::unique_ptr<T>& ptr, Struct&) const {
    if (!ptr) {
      ptr = std::make_unique<T>();
    }
  }
  template <typename T, typename Struct>
  void operator()(std::shared_ptr<T>& ptr, Struct&) const {
    if (!ptr) {
      ptr = std::make_shared<T>();
    }
  }
};
} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
