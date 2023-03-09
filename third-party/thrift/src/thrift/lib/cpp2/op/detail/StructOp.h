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
  using NameList = std::array<std::string, op::size_v<T>>;
  using size_type = typename NameList::size_type;
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

  static bool put(void* s, FieldId fid, size_t, const Dyn& n, const Dyn& val) {
    // TODO(afuller): Use a hash map lookups for these.
    if (n != nullptr) {
      const auto& name = n.as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return putIf<Id>(op::get_name_v<T, Id> == name, ref(s), val);
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
      result = ret(op::get_type_tag<T, Id>{}, *field);
    }
    return cond;
  }

  static Ptr get(void* s, FieldId fid, size_t, const Dyn& n) {
    Ptr result;
    // TODO(afuller): Use a hash map for these lookups.
    if (n != nullptr) { // Get by name.
      const auto& name = n.as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return getIf<Id>(op::get_name_v<T, Id> == name, ref(s), result);
      }));
    } else { // Get by field id.
      check_found(find_by_field_id<T>([&](auto id) {
        return getIf<decltype(id)>(id() == fid, ref(s), result);
      }));
    }
    return result;
  }

  static Ptr getByName(void* s, const std::string& name) {
    return get(s, FieldId{0}, 0, ret(type::string_t{}, name));
  }

  static size_type& pos(std::any& i) {
    if (!i.has_value()) {
      i = size_type{};
    }
    return std::any_cast<size_type&>(i);
  }

  static Ptr next(T& self, IterType type, size_type& itr) {
    if (itr == op::size_v<T>) {
      return {};
    }

    static const NameList& kNames = *([]() {
      auto result = std::make_unique<NameList>();
      op::for_each_ordinal<T>([&](auto ord) {
        (*result)[type::toPosition(ord)] = op::get_name_v<T, decltype(ord)>;
      });
      return result.release();
    })();

    const std::string& name = kNames[itr++];
    switch (type) {
      case IterType::Key:
        return ret(type::string_t{}, name);
      case IterType::Value:
        return getByName(&self, name);
      case IterType::Default:
        unimplemented();
    }
  }

  static Ptr next(void* s, IterType type, std::any& i) {
    if (type == IterType::Default) {
      unimplemented(); // TODO(afuller): Key-value pair?
    }
    return next(ref(s), type, pos(i));
  }

  template <typename Id>
  static bool ensureIf(bool cond, T& self, const Dyn& val, Ptr& result) {
    if (cond) {
      auto&& field = op::get<Id>(self);
      if (isAbsent(field)) {
        if (val != nullptr) {
          *field = val.as<FTag<Id>>();
        } else {
          ensureValue(field);
        }
      }
      result = ret(get_type_tag<T, Id>{}, *field);
    }
    return cond;
  }

  static Ptr ensure(void* s, FieldId fid, const Dyn& n, const Dyn& val) {
    // TODO(afuller): Use a hash map for these lookups.
    Ptr result;
    if (n != nullptr) { // Ensure by name.
      const auto& name = n.as<type::string_t>();
      check_found(find_by_field_id<T>([&](auto id) {
        using Id = decltype(id);
        return ensureIf<Id>(op::get_name_v<T, Id> == name, ref(s), val, result);
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
