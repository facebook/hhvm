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

#include <iterator>
#include <stdexcept>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/op/detail/BaseOp.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/detail/Runtime.h>
#include <thrift/lib/cpp2/type/detail/TypeInfo.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <typename T, typename Tag = type::infer_tag<T>>
struct StructuredOp : BaseOp<Tag> {
  using Base = BaseOp<Tag>;
  using Base::check_found;
  using Base::ref;
  using Base::ret;
  using Base::unimplemented;
  template <typename Id>
  using FTag = op::get_field_tag<Id, T>;

  template <typename Id>
  static bool putIf(bool cond, T& self, const Dyn& val) {
    if (cond) {
      if (val.type().empty()) {
        op::clear_field<FTag<Id>>(op::get<Id>(self), self);
      } else {
        op::get<Id>(self) = val.as<FTag<Id>>();
      }
    }
    return cond;
  }

  static bool put(void* s, FieldId fid, const Dyn* n, const Dyn& val) {
    // TODO(afuller): Use a hash map lookups for these.
    if (n != nullptr) {
      const auto& name = n->as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return putIf<Id>(op::get_name_v<Id, T> == name, ref(s), val);
      }));
    } else {
      check_found(find_by_field_id<T>([&](auto id) {
        return putIf<decltype(id)>(id() == fid, ref(s), val);
      }));
    }
    return true;
  }

  template <typename Id>
  static bool getIf(bool cond, T& self, Ptr& result) {
    auto&& field = op::get<Id>(self);
    if (cond && !isAbsent(field)) {
      result = ret(op::get_type_tag<Id, T>{}, *field);
    }
    return cond;
  }

  static Ptr get(void* s, FieldId fid, size_t, const Dyn* n) {
    Ptr result;
    // TODO(afuller): Use a hash map for these lookups.
    if (n != nullptr) { // Get by name.
      const auto& name = n->as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return getIf<Id>(op::get_name_v<Id, T> == name, ref(s), result);
      }));
    } else { // Get by field id.
      check_found(find_by_field_id<T>([&](auto id) {
        return getIf<decltype(id)>(id() == fid, ref(s), result);
      }));
    }
    return result;
  }

  template <typename Id>
  static bool ensureIf(bool cond, T& self, const Dyn* val, Ptr& result) {
    if (cond) {
      auto&& field = op::get<Id>(self);
      if (isAbsent(field)) {
        if (val != nullptr) {
          *field = val->as<FTag<Id>>();
        } else {
          ensureValue(field);
        }
      }
      result = ret(get_type_tag<Id, T>{}, *field);
    }
    return cond;
  }

  static Ptr ensure(void* s, FieldId fid, const Dyn* n, const Dyn* val) {
    // TODO(afuller): Use a hash map for these lookups.
    Ptr result;
    if (n != nullptr) { // Ensure by name.
      const auto& name = n->as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return ensureIf<Id>(op::get_name_v<Id, T> == name, ref(s), val, result);
      }));
    } else { // Ensure by field id.
      check_found(find_by_field_id<T>([&](auto id) {
        return ensureIf<decltype(id)>(id() == fid, ref(s), val, result);
      }));
    }
    return result;
  }

  static size_t size(const void*) { return op::size_v<T>; }
};

template <typename T>
struct AnyOp<type::struct_t<T>> : StructuredOp<T> {};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
