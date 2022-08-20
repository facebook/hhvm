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

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename>
struct Ensure {};

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
