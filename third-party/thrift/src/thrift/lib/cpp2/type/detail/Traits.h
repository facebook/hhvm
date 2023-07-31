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

#include <map>
#include <set>
#include <string>
#include <vector>

#include <fatal/type/slice.h>
#include <fatal/type/sort.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace type {
namespace detail {

template <typename... Tags>
struct types {
  static constexpr bool contains(BaseType baseType) {
    return (... || (base_type_v<Tags> == baseType));
  }

  template <typename Tag>
  static constexpr bool contains() {
    return contains(base_type_v<Tag>);
  }

  // The Ith type.
  template <size_t I>
  using at = typename fatal::at<types, I>;

  // Converts the type list to a type list of the given types.
  template <template <typename...> class T>
  using as = T<Tags...>;

  template <typename F>
  using filter = typename fatal::filter<types, F>;

  template <typename CTag>
  using of = filter<type::bound::is_a<CTag>>;
};

template <typename Ts, typename Tag, typename R = void>
using if_contains = std::enable_if_t<Ts::template contains<Tag>(), R>;

} // namespace detail
} // namespace type
} // namespace thrift
} // namespace apache
