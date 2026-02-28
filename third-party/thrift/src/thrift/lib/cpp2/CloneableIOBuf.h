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

#ifndef THRIFT_CLONEABLE_IOBUF_H_
#define THRIFT_CLONEABLE_IOBUF_H_

#include <memory>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/Thrift.h>

namespace apache::thrift {

namespace detail {

// Implementation of a subset of unique_ptr functionality that also supports
// copy constructor
template <typename T, typename Deleter = std::default_delete<T>>
class CloneableUniquePtr : public std::unique_ptr<T, Deleter> {
 public:
  using Base = std::unique_ptr<T, Deleter>;
  CloneableUniquePtr() : std::unique_ptr<T, Deleter>() {}
  explicit CloneableUniquePtr(std::nullptr_t)
      : std::unique_ptr<T, Deleter>(nullptr) {}
  explicit CloneableUniquePtr(typename Base::pointer p)
      : std::unique_ptr<T, Deleter>(p) {}
  template <typename D>
  CloneableUniquePtr(typename Base::pointer p, D&& d)
      : std::unique_ptr<T, Deleter>(p, std::forward<D>(d)) {}
  explicit CloneableUniquePtr(Base&& other)
      : std::unique_ptr<T, Deleter>(std::move(other)) {}
  explicit CloneableUniquePtr(CloneableUniquePtr&& other) noexcept
      : std::unique_ptr<T, Deleter>(std::move(other)) {}

  explicit CloneableUniquePtr(const Base& other)
      : std::unique_ptr<T, Deleter>(other ? other->clone() : nullptr) {}
  CloneableUniquePtr(const CloneableUniquePtr<T, Deleter>& other)
      : std::unique_ptr<T, Deleter>(other ? other->clone() : nullptr) {}

  CloneableUniquePtr& operator=(CloneableUniquePtr&& other) {
    Base::operator=(std::move(other));
    return *this;
  }
  CloneableUniquePtr& operator=(Base&& other) {
    Base::operator=(std::move(other));
    return *this;
  }
  CloneableUniquePtr& operator=(std::nullptr_t) {
    Base::operator=(nullptr);
    return *this;
  }

  CloneableUniquePtr& operator=(const CloneableUniquePtr& other) {
    Base::operator=(other ? other->clone() : nullptr);
    return *this;
  }
  CloneableUniquePtr& operator=(const Base& other) {
    Base::operator=(other ? other->clone() : nullptr);
    return *this;
  }
};

template <class T, class Deleter>
void swap(
    CloneableUniquePtr<T, Deleter>& lhs, CloneableUniquePtr<T, Deleter>& rhs) {
  using base = std::unique_ptr<T, Deleter>;
  return swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
}

} // namespace detail

using CloneableIOBuf = apache::thrift::detail::CloneableUniquePtr<folly::IOBuf>;

} // namespace apache::thrift

#endif // #ifndef THRIFT_CLONEABLE_IOBUF_H_
