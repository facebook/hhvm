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

#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace type {
namespace detail {
template <typename U>
struct StructuredTagImpl {
  using T = folly::remove_cvref_t<U>;
  static_assert(is_thrift_class_v<T>, "");
  using tag = folly::conditional_t<
      is_thrift_union_v<T>,
      type::union_t<T>,
      folly::conditional_t<
          is_thrift_exception_v<T>,
          type::exception_t<T>,
          type::struct_t<T>>>;
};
} // namespace detail

// Generate structured type tag from a thrift class.
template <typename T>
using structured_tag = typename detail::StructuredTagImpl<T>::tag;

} // namespace type
} // namespace thrift
} // namespace apache
