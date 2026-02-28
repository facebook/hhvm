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
#include <tuple>
#include <utility>

namespace whisker {

/**
 * A managed_ptr<T> represents one of:
 *   1. A "static"-ally managed object. See manage_as_static(...).
 *   2. An owned instance of T. See manage_owned(...).
 *   3. A derived instance of T, that is a sub-object of another managed object.
 *      See manage_derived(...) and manage_derived_ref(...).
 *
 * All three cases are represented by shared_ptr. See `folly::MaybeManagedPtr`.
 * (1) and (3) use the aliasing constructor of shared_ptr.
 */
template <typename T>
using managed_ptr = std::shared_ptr<const T>;

/**
 * Returns a pointer which is an unmanaged reference to the provided
 * object.
 *
 * The caller must guarantee that the provided object is kept alive as long as
 * the returned pointer is used.
 */
template <typename T>
managed_ptr<T> manage_as_static(const T& o) {
  // Alias an empty shared_ptr to create a non-owning shared_ptr that doesn't
  // allocate a control block (unlike using a noop deleter).
  return managed_ptr<T>(managed_ptr<void>(), std::addressof(o));
}

/**
 * Returns a pointer which directly owns an instance of T.
 */
template <typename T, typename... Args>
managed_ptr<T> manage_owned(Args&&... args) {
  return std::make_shared<const T>(std::forward<Args>(args)...);
}

/**
 * Returns a pointer where the lifetime of `value` depends on one or more other
 * objects. This means that `from` must outlive `value`.
 *
 * The caller must guarantee that the provided object is kept alive as long as
 * the returned pointer is used.
 */
template <typename T, typename U>
managed_ptr<T> manage_derived_ref(const managed_ptr<U>& from, const T& value) {
  return managed_ptr<T>(from, std::addressof(value));
}
// Same as above except derived from multiple objects.
template <typename T, typename... U>
managed_ptr<T> manage_derived_ref(
    std::tuple<managed_ptr<U>...> froms, const T& value) {
  return managed_ptr<T>(
      std::make_shared<std::tuple<managed_ptr<U>...>>(std::move(froms)),
      std::addressof(value));
}

/**
 * Returns a pointer where the lifetime of `value` depends on one or more other
 * objects. This means that `from` must outlive `value`.
 */
template <typename T, typename U>
managed_ptr<T> manage_derived(
    const managed_ptr<U>& from, managed_ptr<T> value_ptr) {
  const T& value = *value_ptr;
  return manage_derived_ref(
      std::tuple<managed_ptr<U>, managed_ptr<T>>(from, std::move(value_ptr)),
      value);
}
// Same as above except derived from multiple objects.
template <typename T, typename... U>
managed_ptr<T> manage_derived(
    std::tuple<managed_ptr<U>...> froms, managed_ptr<T> value_ptr) {
  const T& value = *value_ptr;
  return manage_derived_ref(
      std::tuple_cat(std::tuple(std::move(value_ptr)), std::move(froms)),
      value);
}

} // namespace whisker
