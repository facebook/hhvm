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
#include <thrift/lib/cpp2/op/Get.h>

namespace apache {
namespace thrift {
namespace detail {

template <typename Union, typename Type>
constexpr std::size_t countTypeInUnion() {
  std::size_t count = 0;
  op::for_each_ordinal<Union>([&](auto id) {
    count += std::is_same_v<op::get_native_type<Union, decltype(id)>, Type>;
  });
  return count;
}

template <typename V, typename T, typename F>
void findTypeInUnion(V& variant, F f) {
  using Union = std::remove_cvref_t<V>;
  using Type = std::remove_cvref_t<T>;

  static_assert(is_thrift_union_v<Union>);
  static_assert(countTypeInUnion<Union, Type>() == 1);

  op::for_each_ordinal<Union>([&](auto id) {
    using Id = decltype(id);
    if constexpr (std::is_same_v<op::get_native_type<Union, Id>, Type>) {
      f(op::get<Id>(variant));
    }
  });
}

} // namespace detail

/**
 * Gets a pointer to the Thrift union's member for the given type.
 *
 * If the desired field is not the one currently set in the union, returns
 * `nullptr`.
 *
 * Example:
 *
 *  // Foo.thrift
 *  union MyUnion {
 *    1: i32 a
 *    2: string b
 *    3: double c
 *  }
 *
 *  // foo.cpp
 *  #include "project_dir/gen-cpp2/Foo_types.h"
 *
 *  MyUnion u;
 *  u.a_ref() = 10;
 *
 *  // yields a pointer to field `a`
 *  std::cout << variant_try_get<std::int32_t>(u);
 *
 *  // yields `nullptr`
 *  std::cout << variant_try_get<double>(u);
 */
template <typename T, typename V>
folly::like_t<V, T>* variant_try_get(V& variant) {
  folly::like_t<V, T>* ret = nullptr;
  detail::findTypeInUnion<V, T>(variant, [&](auto p) {
    if (p) {
      ret = &*p;
    }
  });
  return ret;
}

/**
 * Sets the Thrift union's member for the given type with the provided value.
 *
 * Example:
 *
 *  // Foo.thrift
 *  union MyUnion {
 *    1: i32 a
 *    2: string b
 *    3: double c
 *  }
 *
 *  // foo.cpp
 *  #include "project_dir/gen-cpp2/Foo_types.h"
 *
 *  MyUnion u;
 *
 *  // sets the field `a` to the value `10`.
 *  variant_set(u, 10);
 *
 *  std::cout << *u.a_ref();
 */
template <typename V, typename T>
void variant_set(V& variant, T&& value) {
  detail::findTypeInUnion<V, T>(
      variant, [&](auto p) { p = std::forward<T>(value); });
}

} // namespace thrift
} // namespace apache
