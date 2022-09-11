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

// Compile-time and type-erased Thrift operator implementations.
template <typename Tag, typename = void>
struct AnyOp : BaseAnyOp<Tag> {
  static_assert(type::is_concrete_v<Tag>, "");
  using Base = BaseAnyOp<Tag>;

  // TODO(afuller): Implement all Tags and remove runtime throwing fallback.
  using Base::unimplemented;
  [[noreturn]] static void append(void*, const Dyn&) { unimplemented(); }
  [[noreturn]] static bool add(void*, const Dyn&) { unimplemented(); }
  [[noreturn]] static bool put(void*, FieldId, const Dyn*, const Dyn&) {
    unimplemented();
  }
  [[noreturn]] static Ptr ensure(void*, FieldId, const Dyn*, const Dyn*) {
    unimplemented();
  }
  [[noreturn]] static Ptr get(void*, FieldId, size_t, const Dyn*) {
    unimplemented();
  }
  [[noreturn]] static size_t size(const void*) { unimplemented(); }
};

template <typename Tag>
struct NumericOp : BaseAnyOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = BaseAnyOp<Tag>;
  using Base::ref;

  static bool add(T& self, const T& val) { return (self += val, true); }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<Tag>()); }
};

template <>
struct AnyOp<type::bool_t> : NumericOp<type::bool_t> {};
template <>
struct AnyOp<type::byte_t> : NumericOp<type::byte_t> {};
template <>
struct AnyOp<type::i16_t> : NumericOp<type::i16_t> {};
template <>
struct AnyOp<type::i32_t> : NumericOp<type::i32_t> {};
template <>
struct AnyOp<type::i64_t> : NumericOp<type::i64_t> {};
template <>
struct AnyOp<type::float_t> : NumericOp<type::float_t> {};
template <>
struct AnyOp<type::double_t> : NumericOp<type::double_t> {};

template <typename Tag>
struct ContainerOp : BaseAnyOp<Tag> {
  using Base = BaseAnyOp<Tag>;
  using Base::ref;

  static size_t size(const void* s) { return ref(s).size(); }

  // TODO(afuller): This is O(n) for non-random access iterators, expose
  // some form of type-erased iterators directly as well.
  static auto find(void* s, size_t pos) {
    if (pos >= ref(s).size()) {
      // TODO(afuller): Consider returning 'end' instead.
      folly::throw_exception<std::out_of_range>("out of range");
    }
    auto itr = ref(s).begin();
    std::advance(itr, pos);
    return itr;
  }
};

template <typename VTag, typename Tag = type::list<VTag>>
struct ListOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = ContainerOp<Tag>;
  using Base::check_op;
  using Base::find;
  using Base::ref;
  using Base::ret;
  using Base::unimplemented;

  template <typename V = type::native_type<VTag>>
  static void append(T& self, V&& val) {
    self.push_back(std::forward<V>(val));
  }
  static void append(void* s, const Dyn& v) { append(ref(s), v.as<VTag>()); }

  template <typename V = type::native_type<VTag>>
  [[noreturn]] static bool add(T&, V&&) {
    unimplemented(); // TODO(afuller): Add if not already present.
  }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<VTag>()); }

  template <typename U>
  static decltype(auto) get(U&& self, size_t pos) {
    return folly::forward_like<U>(self.at(pos));
  }

  static Ptr get(void* s, FieldId, size_t pos, const Dyn*) {
    check_op(pos != std::string::npos);
    return ret(VTag{}, *find(s, pos));
  }
};

template <typename VTag>
struct AnyOp<type::list<VTag>> : ListOp<VTag> {};
template <typename T, typename VTag>
struct AnyOp<type::cpp_type<T, type::list<VTag>>>
    : ListOp<VTag, type::cpp_type<T, type::list<VTag>>> {};

template <typename KTag, typename Tag = type::set<KTag>>
struct SetOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = ContainerOp<Tag>;
  using Base::find;
  using Base::ref;
  using Base::ret;
  using Base::unimplemented;

  template <typename K = type::native_type<KTag>>
  static bool add(T& self, K&& key) {
    return self.insert(std::forward<K>(key)).second;
  }
  static bool add(void* s, const Dyn& k) { return add(ref(s), k.as<KTag>()); }

  template <typename K = type::native_type<KTag>>
  static bool contains(const T& self, K&& key) {
    return self.find(std::forward<K>(key)) != self.end();
  }
  static Ptr get(void* s, FieldId, size_t pos, const Dyn*) {
    if (pos != std::string::npos) {
      return ret(KTag{}, *find(s, pos));
    }
    unimplemented(); // TODO(afuller): Get by key (aka contains).
  }
};

template <typename KTag>
struct AnyOp<type::set<KTag>> : SetOp<KTag> {};

template <typename KTag, typename VTag, typename Tag = type::map<KTag, VTag>>
struct MapOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using K = type::native_type<KTag>;
  using V = type::native_type<VTag>;
  using Base = ContainerOp<Tag>;
  using Base::bad_op;
  using Base::check_op;
  using Base::ref;
  using Base::ret;
  using Base::unimplemented;

  template <typename J = K, typename U = V>
  static bool put(T& self, J&& key, U&& val) {
    auto itr = self.find(key);
    if (itr == self.end()) {
      self.emplace(std::forward<J>(key), std::forward<U>(val));
      return false; // new entry.`
    }
    itr->second = std::forward<U>(val);
    return true; // replacing existing.
  }

  template <typename J = K, typename U = V>
  static V& ensure(T& self, J&& key, U&& val) {
    auto itr = self.find(key);
    if (itr == self.end()) {
      itr = self.emplace(std::forward<J>(key), std::forward<U>(val)).first;
    }
    return itr->second;
  }

  static bool put(void* s, FieldId, const Dyn* k, const Dyn& v) {
    check_op(k != nullptr);
    return put(ref(s), k->as<KTag>(), v.as<VTag>());
  }

  static Ptr ensure(void* s, FieldId, const Dyn* k, const Dyn* v) {
    check_op(k != nullptr);
    if (v == nullptr) {
      return ret(VTag{}, ensure(ref(s), k->as<KTag>(), V{}));
    } else {
      return ret(VTag{}, ensure(ref(s), k->as<KTag>(), v->as<VTag>()));
    }
  }

  static Ptr get(void* s, FieldId, size_t pos, const Dyn* k) {
    if (k != nullptr) {
      return ret(VTag{}, ref(s).at(k->as<KTag>()));
    } else if (pos != std::string::npos) {
      return ret(KTag{}, Base::find(s, pos)->first);
    }
    bad_op();
  }
};
template <typename KTag, typename VTag>
struct AnyOp<type::map<KTag, VTag>> : MapOp<KTag, VTag> {};
template <typename T, typename KTag, typename VTag>
struct AnyOp<type::cpp_type<T, type::map<KTag, VTag>>>
    : MapOp<KTag, VTag, type::cpp_type<T, type::map<KTag, VTag>>> {};

template <typename T, typename Tag = type::infer_tag<T>>
struct StructuredOp : BaseAnyOp<Tag> {
  using Base = BaseAnyOp<Tag>;
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

  [[noreturn]] static Ptr get(void*, FieldId, size_t, const Dyn*) {
    unimplemented();
  }
  static size_t size(const void*) { return op::size_v<T>; }
};

template <typename T>
struct AnyOp<type::struct_t<T>> : StructuredOp<T> {};

// Create a AnyOp-based Thrift type info.
template <typename Tag>
const TypeInfo& getAnyTypeInfo() {
  return type::detail::getTypeInfo<AnyOp<Tag>, Tag>();
}

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
