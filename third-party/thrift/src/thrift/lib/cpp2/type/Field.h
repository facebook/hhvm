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

namespace apache {
namespace thrift {
namespace type {

// Helpers for detecting compatible optional types.
template <typename T>
struct is_optional_type : std::false_type {};
template <typename T>
struct is_optional_type<optional_field_ref<T>> : std::true_type {};
template <typename T>
struct is_optional_type<optional_boxed_field_ref<T>> : std::true_type {};

template <typename U, typename R = void>
using if_opt_type =
    std::enable_if_t<is_optional_type<folly::remove_cvref_t<U>>::value, R>;
template <typename U, typename R = void>
using if_not_opt_type =
    std::enable_if_t<!is_optional_type<folly::remove_cvref_t<U>>::value, R>;

} // namespace type
} // namespace thrift
} // namespace apache
