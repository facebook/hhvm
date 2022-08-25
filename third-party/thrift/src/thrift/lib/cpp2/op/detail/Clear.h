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

// Helper that creates a leaky singleton.
template <typename F>
FOLLY_EXPORT const auto& staticDefault(F&& make) {
  static const auto& kVal = *make().release();
  return kVal;
}

// C++'s intrinsic default for the underlying native type, is the intrisitic
// default for for all unstructured types.
template <typename Tag, typename = void>
struct GetIntrinsicDefault {
  constexpr auto operator()() const { return type::native_type<Tag>{}; }
};

// Gets the intrinsic default via `thrift::clear`
//
// Useful for any type that supports custom defaults.
template <typename Tag>
struct ThriftClearDefault {
  const auto& operator()() const {
    return staticDefault([] {
      auto value = std::make_unique<type::native_type<Tag>>();
      apache::thrift::clear(*value);
      return value;
    });
  }
};

// Gets the intrinsic default via `op::create`
template <typename Tag>
struct CreateDefault {
  const auto& operator()() const {
    return staticDefault([] {
      return std::make_unique<type::native_type<Tag>>(op::create<Tag>());
    });
  }
};

// Cache the cleared defaults for structured types.
template <typename T>
struct GetIntrinsicDefault<type::struct_t<T>>
    : ThriftClearDefault<type::struct_t<T>> {};
template <typename T>
struct GetIntrinsicDefault<type::exception_t<T>>
    : ThriftClearDefault<type::exception_t<T>> {};
// TODO(afuller): Is this actually needed for union?
template <typename T>
struct GetIntrinsicDefault<type::union_t<T>>
    : ThriftClearDefault<type::union_t<T>> {};

// Cache the result of op::create for adapters.
template <typename Adapter, typename Tag>
struct GetIntrinsicDefault<type::adapted<Adapter, Tag>>
    : CreateDefault<type::adapted<Adapter, Tag>> {};

// TODO(dokwon): Support field_ref types.
template <typename Tag, typename Context>
struct GetIntrinsicDefault<type::field<Tag, Context>>
    : GetIntrinsicDefault<Tag> {};

template <typename Adapter, typename UTag, typename Struct, int16_t FieldId>
struct GetIntrinsicDefault<
    type::field<type::adapted<Adapter, UTag>, FieldContext<Struct, FieldId>>> {
  using Tag =
      type::field<type::adapted<Adapter, UTag>, FieldContext<Struct, FieldId>>;
  const auto& operator()() const {
    return staticDefault([] {
      // Note, this is a separate leaky singleton instance used by
      // staticDefault.
      auto& obj = *new Struct{};
      folly::annotate_object_leaked(&obj);
      apache::thrift::clear(obj);
      return std::make_unique<type::native_type<Tag>>(op::create<Tag>(obj));
    });
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
