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

namespace apache::thrift::op::detail {
using FieldId = type::FieldId;
using Ptr = type::detail::Ptr;
using Dyn = type::detail::Dyn;
using RuntimeType = type::detail::RuntimeType;
using TypeInfo = type::detail::TypeInfo;
using IterType = type::detail::IterType;

template <typename Tag>
const TypeInfo& getAnyTypeInfo();

// Create a AnyOp-based Thrift runtime type.
template <typename Tag, typename T = type::native_type<Tag>>
RuntimeType getAnyType() {
  static_assert(
      std::is_same<folly::remove_cvref_t<T>, type::native_type<Tag>>::value,
      "type missmatch");
  return RuntimeType::create<T>(getAnyTypeInfo<Tag>());
}

// Compile-time and type-erased Thrift operator implementations.
template <typename Tag, typename = void>
struct AnyOp;

// Ops all Thrift types support.
template <typename Tag>
struct BaseOp : type::detail::BaseErasedOp {
  // Blind convert the pointer to the native type.
  using T = type::native_type<Tag>;
  static T& ref(void* ptr) { return *static_cast<T*>(ptr); }
  static const T& ref(const void* ptr) { return cref(ptr); }
  static const T& cref(const void* ptr) { return *static_cast<const T*>(ptr); }
  template <typename RTag, typename T>
  static Ptr ret(T&& val) {
    return {getAnyType<RTag, T>(), &val};
  }
  template <typename RTag, typename T>
  static Ptr ret(RTag, T&& val) {
    return ret<RTag>(std::forward<T>(val));
  }

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
  static void assign(void* ptr, const Dyn& val) { ref(ptr) = val.as<Tag>(); }
  static bool identical(const void* lhs, const Dyn& rhs) {
    // Caller should have already checked the types match.
    assert(rhs.type() == Tag{});
    return op::identical<Tag>(ref(lhs), rhs.as<Tag>());
  }

  static folly::partial_ordering compare(const void* lhs, const Dyn& rhs) {
    if (const T* ptr = rhs.tryAs<Tag>()) {
      return partialCmp<Tag>(ref(lhs), *ptr);
    }
    // TODO(afuller): Throw bad_op() when all compatible type overloads are
    // implemented.
    unimplemented();
  }

 private:
  template <typename UTag>
  static if_comparable<UTag> partialCmp(const T& lhs, const T& rhs) {
    return op::compare<Tag>(lhs, rhs);
  }
  template <typename UTag>
  static if_not_comparable<UTag> partialCmp(const T& lhs, const T& rhs) {
    return op::equal<Tag>(lhs, rhs) ? folly::partial_ordering::equivalent
                                    : folly::partial_ordering::unordered;
  }
};

} // namespace apache::thrift::op::detail
