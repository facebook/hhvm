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

namespace detail {
template <class T>
struct is_field_ref : std::false_type {};
template <class T>
struct is_field_ref<field_ref<T>> : std::true_type {};
template <class T>
struct is_required_field_ref : std::false_type {};
template <class T>
struct is_required_field_ref<required_field_ref<T>> : std::true_type {};
template <class T>
struct is_optional_field_ref : std::false_type {};
template <class T>
struct is_optional_field_ref<optional_field_ref<T>> : std::true_type {};
template <class T>
struct is_optional_boxed_field_ref : std::false_type {};
template <class T>
struct is_optional_boxed_field_ref<optional_boxed_field_ref<T>>
    : std::true_type {};
template <class T>
struct is_union_field_ref : std::false_type {};
template <class T>
struct is_union_field_ref<union_field_ref<T>> : std::true_type {};
template <class T>
struct is_unique_ptr : std::false_type {};
template <class T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};
template <class T>
struct is_shared_ptr : std::false_type {};
template <class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <class T>
struct is_shared_or_unique_ptr
    : folly::bool_constant<is_unique_ptr<T>::value || is_shared_ptr<T>::value> {
};

template <class T>
constexpr bool is_field_ref_v = is_field_ref<T>::value;
template <class T>
constexpr bool is_optional_field_ref_v = is_optional_field_ref<T>::value;
template <class T>
constexpr bool is_required_field_ref_v = is_required_field_ref<T>::value;
template <class T>
constexpr bool is_optional_boxed_field_ref_v =
    is_optional_boxed_field_ref<T>::value;
template <class T>
constexpr bool is_union_field_ref_v = is_union_field_ref<T>::value;
template <class T>
constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;
template <class T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;
template <class T>
constexpr bool is_shared_or_unique_ptr_v = is_shared_or_unique_ptr<T>::value;
} // namespace detail
} // namespace thrift
} // namespace apache
