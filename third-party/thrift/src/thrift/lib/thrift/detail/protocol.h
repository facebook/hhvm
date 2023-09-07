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

#include <folly/Utility.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Hash.h>

namespace apache::thrift::protocol::detail {

namespace detail {
class Object;
class Value;
} // namespace detail

struct ObjectAdapter;

template <class Base = detail::Object>
class ObjectWrapper;
template <class Base = detail::Value>
class ValueWrapper;

using Object = ObjectWrapper<detail::Object>;
using Value = ValueWrapper<detail::Value>;

template <class Base>
class ObjectWrapper : public Base {
 private:
  static_assert(std::is_same_v<Base, detail::Object>);
  friend struct ::apache::thrift::detail::st::struct_private_access;
  static const char* __fbthrift_thrift_uri();

 public:
  using Base::Base;
  using Base::members;
  using Tag = type::adapted<ObjectAdapter, type::struct_t<detail::Object>>;

  explicit ObjectWrapper(const Base& base) : Base(base) {}
  explicit ObjectWrapper(Base&& base) : Base(std::move(base)) {}

  // TODO(ytj): Provide boost.json.value like APIs
  // www.boost.org/doc/libs/release/libs/json/doc/html/json/ref/boost__json__object.html

  Value& operator[](FieldId i) { return (*members())[folly::to_underlying(i)]; }

  const Value& operator[](FieldId i) const {
    return (*members())[folly::to_underlying(i)];
  }

  Value& at(FieldId i) { return members()->at(folly::to_underlying(i)); }
  const Value& at(FieldId i) const {
    return members()->at(folly::to_underlying(i));
  }

  Value* if_contains(FieldId i) {
    auto iter = members()->find(folly::to_underlying(i));
    return iter == members()->end() ? nullptr : &iter->second;
  }

  const Value* if_contains(FieldId i) const {
    auto iter = members()->find(folly::to_underlying(i));
    return iter == members()->end() ? nullptr : &iter->second;
  }

  bool contains(FieldId i) const {
    return members()->find(folly::to_underlying(i)) != members()->end();
  }

  std::size_t erase(FieldId i) {
    return members()->erase(folly::to_underlying(i));
  }

  [[nodiscard]] auto begin() { return members()->begin(); }
  [[nodiscard]] auto begin() const { return members()->begin(); }
  [[nodiscard]] auto end() { return members()->end(); }
  [[nodiscard]] auto end() const { return members()->end(); }
  [[nodiscard]] size_t size() const { return members()->size(); }
  [[nodiscard]] bool empty() const { return members()->empty(); }
};

struct ObjectAdapter {
  template <class Object>
  static auto fromThrift(Object&& obj) {
    return ObjectWrapper<Object>{std::forward<Object>(obj)};
  }

  template <class Object>
  static Object& toThrift(ObjectWrapper<Object>& obj) {
    return static_cast<Object&>(obj);
  }
  template <class Object>
  static const Object& toThrift(const ObjectWrapper<Object>& obj) {
    return static_cast<const Object&>(obj);
  }
};

template <class Base>
class ValueWrapper : public Base {
 private:
  static_assert(std::is_same_v<Base, detail::Value>);
  friend struct ::apache::thrift::detail::st::struct_private_access;
  static const char* __fbthrift_thrift_uri();

 public:
  using Base::Base;
  explicit ValueWrapper(const Base& base) : Base(base) {}
  explicit ValueWrapper(Base&& base) : Base(std::move(base)) {}

  // TODO(ytj): Provide boost.json.value like APIs
  // www.boost.org/doc/libs/release/libs/json/doc/html/json/ref/boost__json__value.html

#define FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(TYPE)                  \
  decltype(auto) as_##TYPE() { return *Base::TYPE##Value_ref(); }         \
  decltype(auto) as_##TYPE() const { return *Base::TYPE##Value_ref(); }   \
  bool is_##TYPE() const { return Base::TYPE##Value_ref().has_value(); }  \
  [[deprecated]] decltype(auto) ensure_##TYPE() {                         \
    return Base::TYPE##Value_ref().ensure();                              \
  }                                                                       \
  template <typename... Args>                                             \
  decltype(auto) emplace_##TYPE(Args&&... args) {                         \
    return Base::TYPE##Value_ref().emplace(static_cast<Args&&>(args)...); \
  }                                                                       \
  decltype(auto) if_##TYPE() {                                            \
    return is_##TYPE() ? &*Base::TYPE##Value_ref() : nullptr;             \
  }                                                                       \
  decltype(auto) if_##TYPE() const {                                      \
    return is_##TYPE() ? &*Base::TYPE##Value_ref() : nullptr;             \
  }                                                                       \
  /* enforce semicolon after macro */ static_assert(true, "")

  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(bool);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(byte);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(i16);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(i32);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(i64);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(float);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(double);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(string);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(binary);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(object);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(list);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(set);
  FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(map);

#undef FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE
};

struct ValueAdapter {
  template <class Value>
  static auto fromThrift(Value&& obj) {
    return ValueWrapper<Value>{std::forward<Value>(obj)};
  }

  template <class Value>
  static Value& toThrift(ValueWrapper<Value>& obj) {
    return static_cast<Value&>(obj);
  }
  template <class Value>
  static const Value& toThrift(const ValueWrapper<Value>& obj) {
    return static_cast<const Value&>(obj);
  }
};

} // namespace apache::thrift::protocol::detail

template <>
struct std::hash<apache::thrift::protocol::detail::Value> {
  std::size_t operator()(
      const apache::thrift::protocol::detail::Value& s) const noexcept {
    // TODO(dokwon): Remove specifying op::StdHasher and use default op::hash
    // after op::StdHasherDeprecated migration.
    auto accumulator = apache::thrift::op::makeDeterministicAccumulator<
        apache::thrift::op::StdHasher>();
    apache::thrift::op::hash<apache::thrift::type::union_t<
        apache::thrift::protocol::detail::detail::Value>>(

        apache::thrift::protocol::detail::ValueAdapter::toThrift(s),
        accumulator);
    return std::move(accumulator.result()).getResult();
  }
};
