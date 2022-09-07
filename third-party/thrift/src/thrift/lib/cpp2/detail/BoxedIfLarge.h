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
#include <type_traits>
#include <utility>

#include <folly/Portability.h>

namespace apache {
namespace thrift {
namespace detail {

/**
 * @brief Autoboxing the value when it's too large.
 *
 * If sizeof(T) > kThreshold (100KB by default), boxing T, otherwise inlining T.
 *
 * @tparam T the type of the object managed by this BoxedIfLarge
 * @tparam kThreshold Maximum allowed unboxed size
 */
template <class T, size_t kThreshold = 100 * 1024, class = void>
class BoxedIfLarge {
 public:
  template <class... Args>
  explicit BoxedIfLarge(Args&&... args)
      : ptr_(std::make_unique<T>(std::forward<Args>(args)...)) {}

  T& operator*() & noexcept { return *ptr_; }
  T&& operator*() && noexcept { return *ptr_; }
  const T& operator*() const& noexcept { return *ptr_; }
  const T&& operator*() const&& noexcept { return *ptr_; }

  T* operator->() noexcept { return ptr_.get(); }
  const T* operator->() const noexcept { return ptr_.get(); }

 private:
  std::unique_ptr<T> ptr_;
};

template <class T, size_t kThreshold>
class BoxedIfLarge<T, kThreshold, std::enable_if_t<sizeof(T) <= kThreshold>> {
 public:
  template <class... Args>
  explicit BoxedIfLarge(Args&&... args) : data_(std::forward<Args>(args)...) {}

  T& operator*() & noexcept { return data_; }
  T&& operator*() && noexcept { return data_; }
  const T& operator*() const& noexcept { return data_; }
  const T&& operator*() const&& noexcept { return data_; }

  T* operator->() noexcept { return &data_; }
  const T* operator->() const noexcept { return &data_; }

 private:
  T data_;
};

} // namespace detail
} // namespace thrift
} // namespace apache
