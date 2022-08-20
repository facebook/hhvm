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

#include <cmath>

#include <folly/CPortability.h>
#include <folly/Overload.h>
#include <folly/Portability.h>
#include <folly/memory/SanitizeLeak.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// C++'s intrinsic default for the underlying native type, is the intrisitic
// default for for all unstructured types.
template <typename Tag>
struct GetIntrinsicDefault {
  static_assert(type::is_concrete_v<Tag>, "");
  using T = type::native_type<Tag>;

  template <typename TagT = Tag>
  constexpr type::if_is_a<TagT, type::string_c, T> operator()() const {
    return StringTraits<T>::fromStringLiteral("");
  }

  template <typename TagT = Tag>
  FOLLY_EXPORT type::if_is_a<TagT, type::structured_c, const T&> operator()()
      const {
    static const T& kDefault = *[]() {
      auto* value = new T{};
      // The default construct respects 'custom' defaults on fields, but
      // clearing any instance of a structured type, sets it to the
      // 'intrinsic' default.
      apache::thrift::clear(*value);
      return value;
    }();
    return kDefault;
  }

  // For rest of type tag, value initialize its native type.
  template <typename TagT = Tag>
  constexpr std::enable_if_t<
      !type::is_a_v<TagT, type::string_c> &&
          !type::is_a_v<TagT, type::structured_c>,
      T>
  operator()() const {
    return T{};
  }
};

template <typename Adapter, typename Tag>
struct GetIntrinsicDefault<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  using T = type::native_type<adapted_tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  FOLLY_EXPORT const T& operator()() const {
    static const T& kDefault = *new T(op::create<adapted_tag>());
    return kDefault;
  }
};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct GetIntrinsicDefault<type::field<Tag, Context>>
    : GetIntrinsicDefault<Tag> {};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct GetIntrinsicDefault<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  using T = type::native_type<field_adapted_tag>;
  static_assert(type::is_concrete_v<field_adapted_tag>, "");

  FOLLY_EXPORT const T& operator()() const {
    static const T& kDefault = *[]() {
      // Note, this is a separate leaky singleton instance from
      // 'op::getIntrinsicDefault<struct_t<Struct>>'.
      auto& obj = *new Struct{};
      folly::annotate_object_leaked(&obj);
      apache::thrift::clear(obj);
      auto* value = new T(op::create<field_adapted_tag>(obj));
      return value;
    }();
    return kDefault;
  }
};

template <typename Tag>
struct Clear {
  static_assert(type::is_concrete_v<Tag>, "");
  template <typename T>
  void operator()(T& value) const {
    folly::overload(
        [](auto& v, type::structured_c) { apache::thrift::clear(v); },
        [](auto& v, type::container_c) { v.clear(); },
        [](auto& v, type::all_c) {
          // All unstructured types can be cleared by assigning to the intrinsic
          // default.
          v = GetIntrinsicDefault<Tag>{}();
        })(value, Tag{});
  }
};

template <typename Adapter, typename Tag>
struct Clear<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  template <typename T>
  void operator()(T& value) const {
    ::apache::thrift::adapt_detail::clear<Adapter>(value);
  }
};

// TODO: support union
struct ClearOptionalField {
  template <typename T, typename Struct>
  void operator()(optional_boxed_field_ref<T> field, Struct&) const {
    field.reset();
  }
  template <typename T, typename Struct>
  void operator()(optional_field_ref<T> field, Struct&) const {
    field.reset();
  }
  template <typename T, typename Struct>
  void operator()(std::shared_ptr<T>& field, Struct&) const {
    field = nullptr;
  }
  template <typename T, typename Struct>
  void operator()(std::unique_ptr<T>& field, Struct&) const {
    field = nullptr;
  }
};

template <typename>
struct ClearField {};

template <typename Tag, typename Context>
struct ClearField<type::field<Tag, Context>> : ClearOptionalField {
  using ClearOptionalField::operator();

  template <typename T, typename Struct>
  void operator()(required_field_ref<T> field, Struct&) const {
    Clear<Tag>{}(*field);
  }

  template <typename T, typename Struct>
  void operator()(terse_field_ref<T> field, Struct&) const {
    Clear<Tag>{}(*field);
  }

  template <typename T, typename Struct>
  void operator()(field_ref<T> field, Struct&) const {
    Clear<Tag>{}(*field);
  }
};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct ClearField<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>>
    : ClearOptionalField {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  static_assert(type::is_concrete_v<field_adapted_tag>, "");

  using ClearOptionalField::operator();

  template <typename T>
  void operator()(required_field_ref<T> field, Struct& s) const {
    ::apache::thrift::adapt_detail::clear<Adapter, FieldId>(*field, s);
  }

  template <typename T>
  void operator()(terse_field_ref<T> field, Struct& s) const {
    ::apache::thrift::adapt_detail::clear<Adapter, FieldId>(*field, s);
  }

  template <typename T>
  void operator()(field_ref<T> field, Struct& s) const {
    ::apache::thrift::adapt_detail::clear<Adapter, FieldId>(*field, s);
  }
};

template <typename Tag>
struct Empty {
  static_assert(type::is_concrete_v<Tag>, "");
  template <typename T = type::native_type<Tag>>
  constexpr bool operator()(const T& value) const {
    return folly::overload(
        [](const auto& v, type::string_c) {
          return StringTraits<T>::isEmpty(v);
        },
        [](const auto& v, type::container_c) { return v.empty(); },
        [](const auto& v, type::structured_c) {
          return apache::thrift::empty(v);
        },
        [](const auto& v, type::all_c) {
          // All unstructured values are 'empty' if they are identical to their
          // intrinsic default.
          return op::identical<Tag>(v, GetIntrinsicDefault<Tag>{}());
        })(value, Tag{});
  }
};

template <typename Adapter, typename Tag>
struct Empty<type::adapted<Adapter, Tag>> {
  using adapted_tag = type::adapted<Adapter, Tag>;
  static_assert(type::is_concrete_v<adapted_tag>, "");
  template <typename T>
  constexpr bool operator()(const T& value) const {
    return op::identical<adapted_tag>(
        value, GetIntrinsicDefault<adapted_tag>{}());
  }
};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct Empty<type::field<Tag, Context>> : Empty<Tag> {};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct Empty<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  static_assert(type::is_concrete_v<field_adapted_tag>, "");
  template <typename T>
  constexpr bool operator()(const T& value) const {
    return op::identical<field_adapted_tag>(
        value, GetIntrinsicDefault<field_adapted_tag>{}());
  }
};
} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
