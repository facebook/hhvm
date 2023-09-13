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
#include <folly/Traits.h>
namespace apache {
namespace thrift {

template <class T>
class field_ref;
template <class T>
class optional_field_ref;
template <class T>
class required_field_ref;
template <class T>
class optional_boxed_field_ref;
template <class T>
class union_field_ref;
template <class T>
class intern_boxed_field_ref;
template <class T>
class terse_intern_boxed_field_ref;
template <class T>
class terse_field_ref;

namespace detail {
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_field_ref_v<field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_optional_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool
    is_optional_field_ref_v<optional_field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_required_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool
    is_required_field_ref_v<required_field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_optional_boxed_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool
    is_optional_boxed_field_ref_v<optional_boxed_field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_union_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_union_field_ref_v<union_field_ref<T>> =
    true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_intern_boxed_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool
    is_intern_boxed_field_ref_v<intern_boxed_field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_terse_intern_boxed_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool
    is_terse_intern_boxed_field_ref_v<terse_intern_boxed_field_ref<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_terse_field_ref_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_terse_field_ref_v<terse_field_ref<T>> =
    true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_unique_ptr_v = false;
template <class T, class D>
FOLLY_INLINE_VARIABLE constexpr bool is_unique_ptr_v<std::unique_ptr<T, D>> =
    true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_shared_ptr_v = false;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_shared_ptr_v<std::shared_ptr<T>> = true;
template <class T>
FOLLY_INLINE_VARIABLE constexpr bool is_shared_or_unique_ptr_v =
    is_unique_ptr_v<T> || is_shared_ptr_v<T>;

} // namespace detail
} // namespace thrift
} // namespace apache
