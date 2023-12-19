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
#include <type_traits>

#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/Field.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename T>
type::if_optional_field<T, bool> hasValue(const T& opt) {
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
template <typename T>
bool hasValue(terse_intern_boxed_field_ref<T&> val) {
  // TODO: check whether val has default address first as short-cut
  return !thrift::empty(*as_const_intern_box(val));
}

// If the given field is absent/unset/void.
template <typename T>
type::if_optional_field<T, bool> isAbsent(const T& opt) {
  return !opt.has_value();
}
template <typename T>
constexpr bool isAbsent(union_field_ref<T> opt) {
  return !opt.has_value();
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
constexpr bool isAbsent(terse_intern_boxed_field_ref<T>) {
  return false;
}
template <typename T>
[[deprecated]] constexpr bool isAbsent(required_field_ref<T>) {
  return false;
}
template <typename T>
constexpr bool isAbsent(std::unique_ptr<T>& ptr) {
  return ptr == nullptr;
}
template <typename T>
constexpr bool isAbsent(std::shared_ptr<T>& ptr) {
  return ptr == nullptr;
}

template <typename T, typename = type::if_optional_field<T>>
auto ensureValue(T&& opt) -> decltype(opt.value()) {
  if (isAbsent(opt)) {
    opt.emplace();
  }
  return opt.value();
}

template <typename T>
decltype(auto) ensureValue(union_field_ref<T> val) {
  return val.ensure();
}
template <typename T>
decltype(auto) ensureValue(field_ref<T> val) {
  return val.ensure();
}
template <typename T>
decltype(auto) ensureValue(terse_field_ref<T> val) {
  // A terse field doesn't have a set or unset state, so ensure is a noop.
  return *val;
}
template <typename T>
decltype(auto) ensureValue(terse_intern_boxed_field_ref<T> val) {
  // A terse field doesn't have a set or unset state, so ensure is a noop.
  return *val;
}
template <typename T>
[[deprecated]] decltype(auto) ensureValue(required_field_ref<T> val) {
  // A required field doesn't have a set or unset state, so ensure is a noop.
  return *val;
}
template <typename T>
T& ensureValue(std::unique_ptr<T>& ptr) {
  if (ptr == nullptr) {
    ptr = std::make_unique<T>();
  }
  return *ptr;
}
template <typename T>
T& ensureValue(std::shared_ptr<T>& ptr) {
  if (ptr == nullptr) {
    ptr = std::make_shared<T>();
  }
  return *ptr;
}

template <typename Tag>
struct Create {
  static_assert(type::is_concrete_v<Tag>, "");

  template <typename T = type::native_type<Tag>>
  constexpr T operator()() const {
    return T{};
  }
};

// TODO: Support adapted field, smart pointers with custom allocators.
// TODO: Optionally accept a custom default.
template <typename Id = void, typename Tag = void, typename = void>
struct Ensure {
  template <typename T>
  constexpr decltype(auto) operator()(T& obj) const {
    return ensureValue(op::get<Id, Tag>(obj));
  }
};
template <typename Id>
struct Ensure<Id, void> {
  template <typename T>
  constexpr decltype(auto) operator()(T& obj) const {
    return Ensure<Id, type::infer_tag<T>>{}(obj);
  }
};
template <>
struct Ensure<void, void> {
  template <typename Id, typename T>
  constexpr decltype(auto) operator()(Id, T& obj) const {
    return Ensure<Id>{}(obj);
  }
};

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
      if_field_adapter<AdapterT, FieldId, type::native_type<Tag>, Struct>
      operator()(Struct& object) const {
    auto obj = AdapterT::fromThriftField(
        Create<Tag>{}(), FieldContext<Struct, FieldId>{object});
    adapt_detail::construct<Adapter, FieldId>(obj, object);
    return obj;
  }
};

// TODO(afuller): Migrate all usage and remove.
template <typename Tag, typename Context>
struct Ensure<type::field<Tag, Context>> {
  using field_tag = type::field<Tag, Context>;
  static_assert(type::is_concrete_v<field_tag>, "");
  template <typename T, typename Struct>
  decltype(auto) operator()(T&& val, Struct&) const {
    return ensureValue(std::forward<T>(val));
  }
};
} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
