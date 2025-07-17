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
#include <folly/container/F14Map-fwd.h>
#include <folly/container/F14Set-fwd.h>
#include <folly/json/dynamic.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

namespace apache::thrift::protocol::detail {

namespace detail {
class Object;
class Value;
} // namespace detail

template <class Base = detail::Object>
class ObjectWrapper;
template <class Base = detail::Value>
class ValueWrapper;

using Object = ObjectWrapper<detail::Object>;
using Value = ValueWrapper<detail::Value>;
} // namespace apache::thrift::protocol::detail

namespace apache::thrift::protocol {
folly::dynamic toDynamic(const ::apache::thrift::protocol::detail::Object& obj);
folly::dynamic toDynamic(const ::apache::thrift::protocol::detail::Value& val);
} // namespace apache::thrift::protocol

namespace apache::thrift::syntax_graph {
class Annotation;
}

namespace apache::thrift::protocol::detail {
template <class Base>
class ObjectWrapper : public ::apache::thrift::type::detail::Wrap<Base> {
 private:
  using Wrap = ::apache::thrift::type::detail::Wrap<Base>;

  static_assert(std::is_same_v<Base, detail::Object>);
  friend struct ::apache::thrift::detail::st::struct_private_access;
  static const char* __fbthrift_thrift_uri();
  bool __fbthrift_is_empty() const;

 public:
  using Tag = typename Wrap::underlying_tag;
  using Wrap::toThrift;
  using Wrap::Wrap;

  decltype(auto) type() & { return toThrift().type(); }
  decltype(auto) type() const& { return toThrift().type(); }
  decltype(auto) type() && { return toThrift().type(); }
  decltype(auto) type() const&& { return toThrift().type(); }
  decltype(auto) members() & { return toThrift().members(); }
  decltype(auto) members() const& { return toThrift().members(); }
  decltype(auto) members() && { return toThrift().members(); }
  decltype(auto) members() const&& { return toThrift().members(); }
  [[deprecated("Prefer members()")]] decltype(auto) members_ref() & {
    return toThrift().members_ref();
  }
  [[deprecated("Prefer members()")]] decltype(auto) members_ref() const& {
    return toThrift().members_ref();
  }

  template <typename Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const {
    return toThrift().serializedSize(prot_);
  }

  Value& operator[](FieldId i) { return (*members())[folly::to_underlying(i)]; }

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

 private:
  folly::dynamic toDynamicImpl() const;
  friend folly::dynamic apache::thrift::protocol::toDynamic(
      const apache::thrift::protocol::detail::Object&);

  friend bool operator==(
      const ObjectWrapper& lhs, const ObjectWrapper& rhs) noexcept {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(
      const ObjectWrapper& lhs, const ObjectWrapper& rhs) noexcept {
    return lhs.data_ != rhs.data_;
  }
  friend bool operator<(
      const ObjectWrapper& lhs, const ObjectWrapper& rhs) noexcept {
    return lhs.data_ < rhs.data_;
  }
};

template <class Base>
class ValueWrapper : public ::apache::thrift::type::detail::Wrap<Base> {
 private:
  using Wrap = ::apache::thrift::type::detail::Wrap<Base>;
  static_assert(std::is_same_v<Base, detail::Value>);

 public:
  using ThriftValue = Base;
  using Type = typename Wrap::underlying_type::Type;
  using Wrap::toThrift;

  static const char* __fbthrift_thrift_uri();
  bool __fbthrift_is_empty() const;

  static ValueWrapper fromThrift(const Base& base) {
    return ValueWrapper<>{base};
  }
  static ValueWrapper fromThrift(Base&& base) { return ValueWrapper{base}; }

  ValueWrapper(ValueWrapper&& other) noexcept = default;
  ValueWrapper(const ValueWrapper& other) = default;
  ValueWrapper& operator=(ValueWrapper&& other) noexcept = default;
  ValueWrapper& operator=(const ValueWrapper& other) = default;
  ~ValueWrapper() = default;

  auto getType() const { return toThrift().getType(); }

#define FBTHRIFT_THRIFT_VALUE_GEN_METHOD_FROM_TYPE(TYPE)                       \
  decltype(auto) as_##TYPE() { return *toThrift().TYPE##Value_ref(); }         \
  decltype(auto) as_##TYPE() const { return *toThrift().TYPE##Value_ref(); }   \
  bool is_##TYPE() const { return toThrift().TYPE##Value_ref().has_value(); }  \
  decltype(auto) ensure_##TYPE() {                                             \
    return toThrift().TYPE##Value_ref().ensure();                              \
  }                                                                            \
  template <typename... Args>                                                  \
  decltype(auto) emplace_##TYPE(Args&&... args) {                              \
    return toThrift().TYPE##Value_ref().emplace(static_cast<Args&&>(args)...); \
  }                                                                            \
  decltype(auto) if_##TYPE() {                                                 \
    return is_##TYPE() ? &*toThrift().TYPE##Value_ref() : nullptr;             \
  }                                                                            \
  decltype(auto) if_##TYPE() const {                                           \
    return is_##TYPE() ? &*toThrift().TYPE##Value_ref() : nullptr;             \
  }                                                                            \
  decltype(auto) move_##TYPE() { return toThrift().move_##TYPE##Value(); }     \
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

#define FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(TYPE)                      \
  [[deprecated("Prefer as_XYZ()")]] decltype(auto) get_##TYPE##Value() const { \
    return toThrift().get_##TYPE##Value();                                     \
  }                                                                            \
  [[deprecated("Prefer as_XYZ()")]] decltype(auto) TYPE##Value_ref() {         \
    return toThrift().TYPE##Value_ref();                                       \
  }                                                                            \
  [[deprecated("Prefer as_XYZ()")]] decltype(auto) TYPE##Value_ref() const {   \
    return toThrift().TYPE##Value_ref();                                       \
  }                                                                            \
  template <typename... Args>                                                  \
  [[deprecated("Prefer emplace_XYZ()")]] decltype(auto) set_##TYPE##Value(     \
      Args&&... args) {                                                        \
    return toThrift().set_##TYPE##Value(std::forward<Args>(args)...);          \
  }                                                                            \
  [[deprecated("Prefer move_XYZ()")]] decltype(auto) move_##TYPE##Value() {    \
    return toThrift().move_##TYPE##Value();                                    \
  }                                                                            \
  [[deprecated("Prefer as_XYZ()")]] decltype(auto) mutable_##TYPE##Value() {   \
    return toThrift().mutable_##TYPE##Value();                                 \
  }                                                                            \
  /* enforce semicolon after macro */ static_assert(true, "")

  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(bool);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(byte);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(i16);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(i32);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(i64);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(float);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(double);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(string);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(binary);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(object);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(set);
  FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE(map);

#undef FBTHRIFT_THRIFT_DEPRECATED_METHOD_FROM_TYPE

  using ValueList = ::std::vector<::apache::thrift::protocol::detail::Value>;
  using BoxedValueList = ::std::unique_ptr<ValueList>;

  // Specializations for list, set, and map, as they are commonly used with
  // deduced argument types, e.g. `X.set_listValue({1, 2, 3})`
  ValueList& emplace_list(BoxedValueList t);
  ValueList& emplace_list(const ValueList& t);
  ValueList& emplace_list(ValueList&& t);

  using ValueSet =
      ::folly::F14VectorSet<::apache::thrift::protocol::detail::Value>;
  using BoxedValueSet = ::std::unique_ptr<ValueSet>;

  [[deprecated("Prefer emplace_set()")]] BoxedValueSet& set_setValue(
      BoxedValueSet t);
  [[deprecated("Prefer emplace_set()")]] BoxedValueSet& set_setValue(
      const ValueSet& t);
  [[deprecated("Prefer emplace_set()")]] BoxedValueSet& set_setValue(
      ValueSet&& t);

  using ValueMap = ::folly::F14FastMap<
      ::apache::thrift::protocol::detail::Value,
      ::apache::thrift::protocol::detail::Value>;
  using BoxedValueMap = ::std::unique_ptr<ValueMap>;

  [[deprecated("Prefer emplace_map()")]] BoxedValueMap& set_mapValue(
      BoxedValueMap t);
  [[deprecated("Prefer emplace_map()")]] BoxedValueMap& set_mapValue(
      const ValueMap& t);
  [[deprecated("Prefer emplace_map()")]] BoxedValueMap& set_mapValue(
      ValueMap&& t);

  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const {
    return toThrift().write(prot_);
  }

  friend bool operator==(
      const ValueWrapper& lhs, const ValueWrapper& rhs) noexcept {
    return lhs.toThrift() == rhs.toThrift();
  }
  friend bool operator!=(
      const ValueWrapper& lhs, const ValueWrapper& rhs) noexcept {
    return lhs.toThrift() != rhs.toThrift();
  }
  friend bool operator<(
      const ValueWrapper& lhs, const ValueWrapper& rhs) noexcept {
    return lhs.toThrift() < rhs.toThrift();
  }

 private:
  folly::dynamic toDynamicImpl() const;

  template <typename T>
  friend class ObjectWrapper;
  friend folly::dynamic apache::thrift::protocol::toDynamic(
      const apache::thrift::protocol::detail::Value&);
  friend class apache::thrift::syntax_graph::Annotation;

  using Wrap::Wrap;
};

size_t hash_value(const Value& s);

const detail::Value* into_inner_value(const Value* v);
const detail::Object* into_inner_object(const Object* o);

} // namespace apache::thrift::protocol::detail

template <class Base>
struct std::hash<apache::thrift::protocol::detail::ValueWrapper<Base>> {
  std::size_t operator()(
      const apache::thrift::protocol::detail::ValueWrapper<Base>& s)
      const noexcept {
    return apache::thrift::protocol::detail::hash_value(s);
  }
};

// TODO(sadroeck) - Remove Cpp2Ops specialization for Protocol.Object/Value
// T219294380
template <class Base>
class apache::thrift::Cpp2Ops<
    apache::thrift::protocol::detail::ObjectWrapper<Base>> {
 public:
  using Type = apache::thrift::protocol::detail::ObjectWrapper<Base>;

  static constexpr apache::thrift::protocol::TType thriftType() {
    return apache::thrift::protocol::TType::T_STRUCT;
  }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return Cpp2Ops<Base>::write(
        prot, ::apache::thrift::protocol::detail::into_inner_object(value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    return Cpp2Ops::read(
        prot, ::apache::thrift::protocol::detail::into_inner_object(value));
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return Cpp2Ops::serializedSize(
        prot, ::apache::thrift::protocol::detail::into_inner_object(value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return Cpp2Ops::serializedSizeZC(
        prot, ::apache::thrift::protocol::detail::into_inner_object(value));
  }
};

template <class Base>
class apache::thrift::Cpp2Ops<
    apache::thrift::protocol::detail::ValueWrapper<Base>> {
 public:
  using Type = apache::thrift::protocol::detail::ValueWrapper<Base>;

  static constexpr protocol::TType thriftType() {
    return protocol::TType::T_STRUCT;
  }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Type* value) {
    return Cpp2Ops<Base>::write(
        prot, ::apache::thrift::protocol::detail::into_inner_value(value));
  }
  template <class Protocol>
  static void read(Protocol* prot, Type* value) {
    return Cpp2Ops<Base>::read(
        prot, ::apache::thrift::protocol::detail::into_inner_value(value));
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Type* value) {
    return Cpp2Ops<Base>::serializedSize(
        prot, ::apache::thrift::protocol::detail::into_inner_value(value));
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Type* value) {
    return Cpp2Ops<Base>::serializedSizeZC(
        prot, ::apache::thrift::protocol::detail::into_inner_value(value));
  }
};
