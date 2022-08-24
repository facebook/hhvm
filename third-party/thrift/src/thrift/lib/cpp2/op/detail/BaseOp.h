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

#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/detail/Runtime.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {
using FieldId = type::FieldId;
using Ptr = type::detail::Ptr;
using RuntimeBase = type::detail::RuntimeBase;
using RuntimeType = type::detail::RuntimeType;
using TypeInfo = type::detail::TypeInfo;

// Ops all Thrift types support.
template <typename Tag>
struct BaseAnyOp : type::detail::BaseErasedOp {
  // Blind convert the pointer to the native type.
  using T = type::native_type<Tag>;
  static T& ref(void* ptr) { return *static_cast<T*>(ptr); }
  static const T& ref(const void* ptr) { return cref(ptr); }
  static const T& cref(const void* ptr) { return *static_cast<const T*>(ptr); }

  static void delete_(void* ptr) { delete static_cast<T*>(ptr); }
  static void* make(void* ptr, bool consume) {
    if (ptr == nullptr) {
      return std::make_unique<T>().release();
    } else if (consume) {
      return std::make_unique<T>(std::move(ref(ptr))).release();
    } else {
      return std::make_unique<T>(cref(ptr)).release();
    }
  }
  static bool empty(const void* ptr) { return op::isEmpty<Tag>(ref(ptr)); }
  static void clear(void* ptr) { op::clear<Tag>(ref(ptr)); }
  static bool identical(const void* lhs, const RuntimeBase& rhs) {
    // Caller should have already checked the types match.
    assert(rhs.type() == Tag{});
    return op::identical<Tag>(ref(lhs), rhs.as<Tag>());
  }

  static folly::ordering compare(const void* lhs, const RuntimeBase& rhs) {
    if (const T* ptr = rhs.tryAs<Tag>()) {
      return cmp<Tag>(ref(lhs), *ptr);
    }
    // TODO(afuller): Throw bad_op() when all compatible type overloads are
    // implemented.
    unimplemented();
  }

 private:
  template <typename UTag>
  static if_comparable<UTag> cmp(const T& lhs, const T& rhs) {
    return op::compare<Tag>(lhs, rhs);
  }
  template <typename UTag>
  static if_not_comparable<UTag> cmp(const T& lhs, const T& rhs) {
    return op::equal<Tag>(lhs, rhs) ? folly::ordering::eq : folly::ordering::gt;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
