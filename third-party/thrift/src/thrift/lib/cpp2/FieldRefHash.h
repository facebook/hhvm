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

#include <functional>

#include <folly/Hash.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/FieldRef.h>

namespace apache {
namespace thrift {
namespace detail {
const size_t kHashValueForNonExistsOptionalField = -3333;
} // namespace detail
} // namespace thrift
} // namespace apache

namespace folly {
template <class T>
struct hasher<apache::thrift::field_ref<T>>
    : private hasher<folly::remove_cvref_t<T>> {
  using hasher<folly::remove_cvref_t<T>>::hasher;
  size_t operator()(apache::thrift::field_ref<T> obj) const {
    return hasher<folly::remove_cvref_t<T>>::operator()(*obj);
  }
};

template <class T>
struct hasher<apache::thrift::optional_field_ref<T>>
    : private hasher<folly::remove_cvref_t<T>> {
  using hasher<folly::remove_cvref_t<T>>::hasher;
  size_t operator()(apache::thrift::optional_field_ref<T> obj) const {
    return obj ? hasher<folly::remove_cvref_t<T>>::operator()(*obj)
               : apache::thrift::detail::kHashValueForNonExistsOptionalField;
  }
};
} // namespace folly

namespace std {
template <class T>
struct hash<apache::thrift::field_ref<T>>
    : private hash<folly::remove_cvref_t<T>> {
  using hash<folly::remove_cvref_t<T>>::hash;
  size_t operator()(apache::thrift::field_ref<T> obj) const {
    return hash<folly::remove_cvref_t<T>>::operator()(*obj);
  }
};

template <class T>
struct hash<apache::thrift::optional_field_ref<T>>
    : private hash<folly::remove_cvref_t<T>> {
  using hash<folly::remove_cvref_t<T>>::hash;
  size_t operator()(apache::thrift::optional_field_ref<T> obj) const {
    return obj ? hash<folly::remove_cvref_t<T>>::operator()(*obj)
               : apache::thrift::detail::kHashValueForNonExistsOptionalField;
  }
};
} // namespace std
