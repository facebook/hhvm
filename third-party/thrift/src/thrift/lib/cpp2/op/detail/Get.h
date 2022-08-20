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
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <class Tag, class Id>
struct GetOrdinalImpl;
template <size_t... I, typename F>
void for_each_ordinal_impl(F&& f, std::index_sequence<I...>);

// TODO: support adapted field and smart pointers with custom allocators
struct GetValueOrNull {
  template <typename T>
  auto* operator()(field_ref<T&> field_ref) const {
    return &field_ref.value();
  }
  template <typename T>
  auto* operator()(required_field_ref<T&> field_ref) const {
    return &field_ref.value();
  }
  template <typename T>
  auto* operator()(optional_field_ref<T&> field_ref) const {
    return field_ref.has_value() ? &field_ref.value() : nullptr;
  }
  template <typename T>
  auto* operator()(optional_boxed_field_ref<T&> field_ref) const {
    return field_ref.has_value() ? &field_ref.value() : nullptr;
  }
  template <typename T>
  auto* operator()(terse_field_ref<T&> field_ref) const {
    return &field_ref.value();
  }
  template <typename T>
  auto* operator()(union_field_ref<T&> field_ref) const {
    return field_ref.has_value() ? &field_ref.value() : nullptr;
  }

#ifdef THRIFT_HAS_OPTIONAL
  template <typename T>
  T* operator()(std::optional<T>& opt) const {
    return bool(opt) ? &opt.value() : nullptr;
  }
  template <typename T>
  const T* operator()(const std::optional<T>& opt) const {
    return bool(opt) ? &opt.value() : nullptr;
  }
#endif

  template <typename T>
  T* operator()(const std::unique_ptr<T>&& ptr) const = delete;
  template <typename T>
  T* operator()(const std::unique_ptr<T>& ptr) const {
    return ptr ? ptr.get() : nullptr;
  }
  template <typename T>
  T* operator()(const std::shared_ptr<T>&& ptr) const = delete;
  template <typename T>
  T* operator()(const std::shared_ptr<T>& ptr) const {
    return ptr ? ptr.get() : nullptr;
  }
};
} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
