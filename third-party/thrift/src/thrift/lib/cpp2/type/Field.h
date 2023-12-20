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

#include <type_traits>

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Get.h>

namespace apache {
namespace thrift {
namespace type {
namespace detail {

template <typename T>
struct is_optional_field_ref : std::false_type {};
template <typename T>
struct is_optional_field_ref<optional_field_ref<T>> : std::true_type {};
template <typename T>
struct is_optional_field_ref<optional_boxed_field_ref<T>> : std::true_type {};
template <typename T>
struct is_optional_field_ref<union_field_ref<T>> : std::true_type {};

} // namespace detail

// Helpers for detecting compatible optional field.
template <typename T, typename Id>
inline constexpr bool is_optional_field_v =
    detail::is_optional_field_ref<op::get_field_ref<T, Id>>::value ||
    ::apache::thrift::detail::qualifier::
        is_cpp_ref_field_optional<T, op::get_field_id<T, Id>>::value;

template <typename U, typename R = void>
using if_optional_field = std::enable_if_t<
    detail::is_optional_field_ref<folly::remove_cvref_t<U>>::value,
    R>;
template <typename U, typename R = void>
using if_not_optional_field = std::enable_if_t<
    !detail::is_optional_field_ref<folly::remove_cvref_t<U>>::value,
    R>;

} // namespace type
} // namespace thrift
} // namespace apache
