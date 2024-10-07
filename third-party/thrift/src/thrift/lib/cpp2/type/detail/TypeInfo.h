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

#include <any>
#include <stdexcept>
#include <typeinfo>

#include <folly/CPortability.h>
#include <folly/lang/Exception.h>
#include <folly/lang/Ordering.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Type.h>

namespace apache::thrift::type::detail {
class Dyn;
class Ptr;

enum class IterType { Default = 0, Key, Value };

// Runtime type information for a Thrift type.
struct TypeInfo {
  const Type thriftType;
  const std::type_info& cppType;

  // Type-erased ~v-table.
  // TODO(afuller): Consider merging some of these functions to reduce size.
  void (*delete_)(void*);
  void* (*make)(void*, bool);
  bool (*empty)(const void*);
  bool (*identical)(const void*, const Dyn&);
  folly::partial_ordering (*compare_)(const void*, const Dyn&);
  void (*clear)(void*);
  void (*assign)(void*, const Dyn&);
  void (*append)(void*, const Dyn&);
  bool (*add)(void*, const Dyn&);
  void (*prepend)(void*, const Dyn&);
  bool (*put_)(void*, FieldId, size_t, const Dyn&, const Dyn&);
  Ptr (*ensure_)(void*, FieldId, const Dyn&, const Dyn&);
  Ptr (*get_)(void*, FieldId, size_t, const Dyn&);
  Ptr (*next)(void*, IterType, std::any&);
  size_t (*size)(const void*);

  bool equal(const void* lhs, const Dyn& rhs) const {
    return op::detail::is_eq(compare_(lhs, rhs));
  }

  folly::ordering compare(const void* lhs, const Dyn& rhs) const {
    return folly::ordering(compare_(lhs, rhs));
  }

  bool less(const void* lhs, const Dyn& rhs) const {
    return op::detail::is_lt(compare(lhs, rhs));
  }

  Ptr get(void* ptr, FieldId id) const;
  Ptr get(void* ptr, size_t pos) const;
  Ptr get(void* ptr, const Dyn& val) const;

  bool put(void* ptr, FieldId id, const Dyn& val) const;
  bool put(void* ptr, const Dyn& key, const Dyn& val) const;
  void insert(void* ptr, size_t pos, const Dyn& val) const;

  bool remove(void* ptr, size_t pos) const;
  bool remove(void* ptr, const Dyn& key) const;
  bool remove(void* ptr, FieldId id) const;

  Ptr ensure(void* ptr, FieldId id) const;
  Ptr ensure(void* ptr, FieldId id, const Dyn& defVal) const;
  Ptr ensure(void* ptr, const Dyn& key) const;
  Ptr ensure(void* ptr, const Dyn& key, const Dyn& defVal) const;

  // Type-safe, const-preserving casting functions.
  template <typename T>
  constexpr T* tryAs(void* ptr) const noexcept {
    return cppType == typeid(T) ? static_cast<T*>(ptr) : nullptr;
  }
  template <typename T>
  const T* tryAs(const void* ptr) const noexcept {
    return cppType == typeid(T) ? static_cast<const T*>(ptr) : nullptr;
  }
  template <typename T, typename V = void>
  decltype(auto) as(V* ptr) const {
    if (auto* tptr = tryAs<T>(ptr)) {
      return *tptr;
    }
    folly::throw_exception<std::bad_any_cast>();
  }
};

// Returns the singleton TypeInfo.
template <typename Op, typename Tag, typename T = native_type<Tag>>
FOLLY_EXPORT const TypeInfo& getTypeInfo() {
  static const auto& kValue = *new TypeInfo{
      Tag{},
      typeid(T),
      &Op::delete_,
      &Op::make,
      &Op::empty,
      &Op::identical,
      &Op::compare,
      &Op::clear,
      &Op::assign,
      &Op::append,
      &Op::add,
      &Op::prepend,
      &Op::put,
      &Op::ensure,
      &Op::get,
      &Op::next,
      &Op::size,
  };
  return kValue;
}

} // namespace apache::thrift::type::detail
