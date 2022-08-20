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

#ifndef THRIFT_FATAL_VARIANT_H_
#define THRIFT_FATAL_VARIANT_H_ 1

#include <cassert>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <fatal/type/search.h>
#include <fatal/type/transform.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

namespace apache {
namespace thrift {
namespace detail {
template <typename T, typename V>
using variant_helper =
    typename reflect_variant<folly::remove_cvref_t<V>>::traits;
} // namespace detail

/**
 * READ ME FIRST: this header enhances Thrift unions with variant-style
 * functionality.
 *
 * Please refer to the top of `thrift/lib/cpp2/reflection/reflection.h` on how
 * to enable compile-time reflection for unions. The present header relies on it
 * for its functionality.
 *
 * TROUBLESHOOTING:
 *  - make sure you've followed the instructions on `reflection.h` to enable
 *    generation of compile-time reflection;
 *  - make sure you've included the union metadata for your Thrift union, as
 *    specified in `reflection.h`;
 *  - make sure exactly one of the Thrift union's field map to a specific type.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */

/**
 * Gets a reference to the Thrift union's member for the given type.
 *
 * NOTE: this is an unchecked version of the variant getter. If the desired
 * field is not the one currently set in the union, the behavior of this call
 * is undefined. For a safer alternative, use either `variant_checked_get` or
 * `variant_try_get`.
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
 *  #include "project_dir/gen-cpp2/Foo_fatal_union.h"
 *  #include <thrift/lib/cpp2/reflection/variant.h>
 *
 *  MyUnion u;
 *  u.set_a(10);
 *
 *  // yields a reference to field `a`
 *  std::cout << variant_get<std::int32_t>(u);
 *
 *  // undefined behavior
 *  std::cout << variant_get<double>(u);
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename V>
auto variant_get(V&& variant)
    -> decltype(apache::thrift::detail::variant_helper<T, V>::by_type ::
                    template get<T>(std::forward<V>(variant))) {
  using traits = apache::thrift::detail::variant_helper<T, V>;
  assert(traits::get_id(variant) == traits::by_type::template id<T>::value);
  return traits::by_type::template get<T>(std::forward<V>(variant));
}

/**
 * Gets a reference to the Thrift union's member for the given type.
 *
 * If the desired field is not the one currently set in the union, throws an
 * `std::invalid_argument` exception.
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
 *  #include "project_dir/gen-cpp2/Foo_fatal_union.h"
 *  #include <thrift/lib/cpp2/reflection/variant.h>
 *
 *  MyUnion u;
 *  u.set_a(10);
 *
 *  // yields a reference to field `a`
 *  std::cout << variant_checked_get<std::int32_t>(u);
 *
 *  // throws `std::invalid_argument`
 *  std::cout << variant_checked_get<double>(u);
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename V>
auto variant_checked_get(V&& variant)
    -> decltype(apache::thrift::detail::variant_helper<T, V>::by_type ::
                    template get<T>(std::forward<V>(variant))) {
  using traits = apache::thrift::detail::variant_helper<T, V>;

  if (traits::get_id(variant) != traits::by_type::template id<T>::value) {
    throw std::invalid_argument(
        "type requested to variant_checked_get() is not the one stored in the"
        " variant");
  }

  return traits::by_type::template get<T>(std::forward<V>(variant));
}

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
 *  #include "project_dir/gen-cpp2/Foo_fatal_union.h"
 *  #include <thrift/lib/cpp2/reflection/variant.h>
 *
 *  MyUnion u;
 *  u.set_a(10);
 *
 *  // yields a pointer to field `a`
 *  std::cout << variant_try_get<std::int32_t>(u);
 *
 *  // yields `nullptr`
 *  std::cout << variant_try_get<double>(u);
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename V>
auto variant_try_get(V& variant) -> decltype(std::addressof(
    apache::thrift::detail::variant_helper<T, V>::by_type::template get<T>(
        variant))) {
  using traits = apache::thrift::detail::variant_helper<T, V>;

  if (traits::get_id(variant) != traits::by_type::template id<T>::value) {
    return nullptr;
  }

  return std::addressof(traits::by_type::template get<T>(variant));
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
 *  #include "project_dir/gen-cpp2/Foo_fatal_union.h"
 *  #include <thrift/lib/cpp2/reflection/variant.h>
 *
 *  MyUnion u;
 *
 *  // sets the field `a` to the value `10`.
 *  variant_set(u, 10);
 *
 *  std::cout << u.get_a();
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename V, typename T>
folly::remove_cvref_t<T>& variant_set(V& variant, T&& value) {
  using type = folly::remove_cvref_t<T>;
  using by_type =
      typename apache::thrift::detail::variant_helper<type, V>::by_type;

  by_type::template set<type>(variant, std::forward<T>(value));
  return by_type::template get<type>(variant);
}

/**
 * Sets the Thrift union's member for the given type by perfectly forwarding the
 * given arguments to its constructor.
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
 *  #include "project_dir/gen-cpp2/Foo_fatal_union.h"
 *  #include <thrift/lib/cpp2/reflection/variant.h>
 *
 *  MyUnion u;
 *
 *  // sets the field `b` to the string "hello, world".
 *  variant_emplace<std::string>(u, "hello, world");
 *
 *  std::cout << u.get_b();
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename T, typename V, typename... Args>
T& variant_emplace(V& variant, Args&&... args) {
  using by_type =
      typename apache::thrift::detail::variant_helper<T, V>::by_type;

  by_type::template set<T>(variant, std::forward<Args>(args)...);
  return by_type::template get<T>(variant);
}

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_VARIANT_H_
