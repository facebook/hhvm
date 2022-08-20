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
struct AnyOp : BaseAnyOp<Tag> {
  static_assert(type::is_concrete_v<Tag>, "");
  using Base = BaseAnyOp<Tag>;

  // TODO(afuller): Implement all Tags and remove runtime throwing fallback.
  using Base::unimplemented;
  [[noreturn]] static void append(void*, const RuntimeBase&) {
    unimplemented();
  }
  [[noreturn]] static bool add(void*, const RuntimeBase&) { unimplemented(); }
  [[noreturn]] static bool put(
      void*, FieldId, const RuntimeBase*, const RuntimeBase&) {
    unimplemented();
  }
  [[noreturn]] static Ptr get(void*, FieldId, size_t, const RuntimeBase*) {
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
  static bool add(void* s, const RuntimeBase& v) {
    return add(ref(s), v.as<Tag>());
  }
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

template <typename ValTag, typename Tag = type::list<ValTag>>
struct ListOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = ContainerOp<Tag>;
  using Base::bad_op;
  using Base::find;
  using Base::ref;
  using Base::unimplemented;

  template <typename V = type::native_type<ValTag>>
  static void append(T& self, V&& val) {
    self.push_back(std::forward<V>(val));
  }
  static void append(void* s, const RuntimeBase& v) {
    append(ref(s), v.as<ValTag>());
  }

  template <typename V = type::native_type<ValTag>>
  [[noreturn]] static bool add(T&, V&&) {
    unimplemented(); // TODO(afuller): Add if not already present.
  }
  static bool add(void* s, const RuntimeBase& v) {
    return add(ref(s), v.as<ValTag>());
  }

  template <typename U>
  static decltype(auto) get(U&& self, size_t pos) {
    return folly::forward_like<U>(self.at(pos));
  }

  static Ptr get(void* s, FieldId, size_t pos, const RuntimeBase*) {
    if (pos != std::string::npos) {
      return {getAnyType<ValTag>(), &*find(s, pos)};
    }
    bad_op();
  }
};

template <typename ValTag>
struct AnyOp<type::list<ValTag>> : ListOp<ValTag> {};
template <typename T, typename ValTag>
struct AnyOp<type::cpp_type<T, type::list<ValTag>>>
    : ListOp<ValTag, type::cpp_type<T, type::list<ValTag>>> {};

template <typename KeyTag, typename Tag = type::set<KeyTag>>
struct SetOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = ContainerOp<Tag>;
  using Base::find;
  using Base::ref;
  using Base::unimplemented;

  template <typename K = type::native_type<KeyTag>>
  static bool add(T& self, K&& key) {
    return self.insert(std::forward<K>(key)).second;
  }
  static bool add(void* s, const RuntimeBase& k) {
    return add(ref(s), k.as<KeyTag>());
  }

  template <typename K = type::native_type<KeyTag>>
  static bool contains(const T& self, K&& key) {
    return self.find(std::forward<K>(key)) != self.end();
  }
  static Ptr get(void* s, FieldId, size_t pos, const RuntimeBase*) {
    if (pos != std::string::npos) {
      return {getAnyType<KeyTag>(), &*find(s, pos)};
    }
    unimplemented(); // TODO(afuller): Get by key (aka contains).
  }
};

template <typename KeyTag>
struct AnyOp<type::set<KeyTag>> : SetOp<KeyTag> {};

template <
    typename KeyTag,
    typename ValTag,
    typename Tag = type::map<KeyTag, ValTag>>
struct MapOp : ContainerOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = ContainerOp<Tag>;
  using Base::bad_op;
  using Base::ref;
  using Base::unimplemented;

  template <
      typename K = type::native_type<KeyTag>,
      typename V = type::native_type<ValTag>>
  static bool put(T& self, K&& key, V&& val) {
    auto itr = self.find(key);
    if (itr == self.end()) {
      self.emplace(std::forward<K>(key), std::forward<V>(val));
      return false; // new entry.
    }
    itr->second = std::forward<V>(val);
    return true; // replacing existing.
  }

  static bool put(
      void* s, FieldId, const RuntimeBase* k, const RuntimeBase& v) {
    if (k == nullptr) {
      bad_op();
    }
    return put(ref(s), k->as<KeyTag>(), v.as<ValTag>());
  }

  [[noreturn]] static bool add(void*, const RuntimeBase&) {
    // TODO(afuller): Add key/value pair, if key not already present.
    unimplemented();
  }

  static Ptr get(void* s, FieldId, size_t pos, const RuntimeBase* k) {
    if (k != nullptr) {
      return {getAnyType<ValTag>(), &ref(s).at(k->as<KeyTag>())};
    }
    if (pos != std::string::npos) {
      return {getAnyType<KeyTag>(), &Base::find(s, pos)->first};
    }
    bad_op();
  }
};

template <typename KeyTag, typename ValTag>
struct AnyOp<type::map<KeyTag, ValTag>> : MapOp<KeyTag, ValTag> {};

template <typename T, typename KeyTag, typename ValTag>
struct AnyOp<type::cpp_type<T, type::map<KeyTag, ValTag>>>
    : MapOp<KeyTag, ValTag, type::cpp_type<T, type::map<KeyTag, ValTag>>> {};

// Create a AnyOp-based Thrift type info.
template <typename Tag>
const TypeInfo& getAnyTypeInfo() {
  return type::detail::getTypeInfo<AnyOp<Tag>, Tag>();
}

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
