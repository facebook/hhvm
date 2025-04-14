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

namespace apache::thrift::protocol::experimental {

// ---- NativeList ---- //

template <typename T>
NativeList::NativeList(ListOf<T>&& l) : kind_(std::move(l)) {}
template <typename T>
NativeList::NativeList(const ListOf<T>& l) : kind_(l) {}
template <typename T>
bool NativeList::has_element() const noexcept {
  return std::holds_alternative<
      ::apache::thrift::protocol::experimental::detail::list_t<T>>(kind_);
}
template <typename T>
bool NativeList::is_type() const noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  using ElemTy = std::remove_cv_t<typename T::value_type>;
  return std::holds_alternative<detail::list_t<ElemTy>>(kind_);
}
template <typename T>
const NativeList::Specialized<T>& NativeList::as_type() const {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get<Specialized<T>>(kind_);
}
template <typename T>
NativeList::Specialized<T>& NativeList::as_type() {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get<Specialized<T>>(kind_);
}
template <typename T>
const NativeList::Specialized<T>* NativeList::if_type() const noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get_if<Specialized<T>>(&kind_);
}
template <typename T>
NativeList::Specialized<T>* NativeList::if_type() noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get_if<Specialized<T>>(&kind_);
}

// ---- Object ---- //

template <typename... Args>
Value& Object::emplace(FieldId id, Args... args) {
  auto it = fields.emplace(id, std::forward<Args...>(args)...).first;
  return it->second;
}

// ---- ValueAPI ---- //

template <typename T>
TType ValueAccess<T>::get_ttype() const {
  return folly::variant_match(
      as_value().inner(),
      [](const NativeList&) { return T_LIST; },
      [](const NativeSet&) { return T_SET; },
      [](const NativeMap&) { return T_MAP; },
      [](const Object&) { return T_STRUCT; },
      [](const auto& primitive) {
        using V = std::remove_cvref_t<decltype(primitive)>;
        static_assert(detail::is_primitive_v<V>);
        using Tag = detail::native_value_type<V, true>::tag;
        return op::typeTagToTType<Tag>;
      });
}

template <typename T>
ValueAccess<T>::operator Value&() noexcept {
  return as_value();
}

template <typename T>
ValueAccess<T>::operator const Value&() const noexcept {
  return as_const_value();
}

template <typename T>
template <typename Ty>
bool ValueAccess<T>::is_type() const noexcept {
  return std::holds_alternative<detail::native_value_type_t<Ty>>(
      as_value().inner());
}

template <typename T>
template <typename Ty>
const detail::native_value_type_t<Ty>& ValueAccess<T>::as_type() const {
  return std::get<detail::native_value_type_t<Ty>>(as_value().inner());
}

template <typename T>
template <typename Ty>
detail::native_value_type_t<Ty>& ValueAccess<T>::as_type() {
  return std::get<detail::native_value_type_t<Ty>>(as_value().inner());
}

template <typename T>
template <typename Ty>
const detail::native_value_type_t<Ty>* ValueAccess<T>::if_type()
    const noexcept {
  return std::get_if<detail::native_value_type_t<Ty>>(&as_value().inner());
}

template <typename T>
template <typename Ty>
detail::native_value_type_t<Ty>* ValueAccess<T>::if_type() noexcept {
  return std::get_if<detail::native_value_type_t<Ty>>(&as_value().inner());
}

template <typename T>
bool ValueAccess<T>::operator==(const Value& other) const {
  return as_value().inner() == other.inner();
}

template <typename T>
bool ValueAccess<T>::operator!=(const Value& other) const {
  return as_value().inner() != other.inner();
}

#define FB_THRIFT_VALUE_ACCESS_IMPL(TYPE, NAME)                   \
  template <typename T>                                           \
  bool ValueAccess<T>::is_##NAME() const {                        \
    return std::holds_alternative<TYPE>(as_value().inner());      \
  }                                                               \
  template <typename T>                                           \
  const TYPE& ValueAccess<T>::as_##NAME() const {                 \
    return std::get<TYPE>(as_const_value().inner());              \
  }                                                               \
  template <typename T>                                           \
  TYPE& ValueAccess<T>::as_##NAME() {                             \
    return std::get<TYPE>(as_value().inner());                    \
  }                                                               \
  template <typename T>                                           \
  const TYPE* ValueAccess<T>::if_##NAME() const {                 \
    return std::get_if<TYPE>(&as_const_value().inner());          \
  }                                                               \
  template <typename T>                                           \
  TYPE* ValueAccess<T>::if_##NAME() {                             \
    return std::get_if<TYPE>(&as_value().inner());                \
  }                                                               \
  template <typename T>                                           \
  decltype(auto) ValueAccess<T>::ensure_##NAME() {                \
    if (!std::holds_alternative<TYPE>(as_value().inner())) {      \
      return as_value().inner().template emplace<TYPE>();         \
    }                                                             \
    return std::get<TYPE>(as_value().inner());                    \
  }                                                               \
  template <typename T>                                           \
  template <typename... Args>                                     \
  decltype(auto) ValueAccess<T>::emplace_##NAME(Args&&... args) { \
    return as_value().inner().template emplace<TYPE>(             \
        std::forward<Args>(args)...);                             \
  }

FB_THRIFT_VALUE_ACCESS_IMPL(Bool, bool)
FB_THRIFT_VALUE_ACCESS_IMPL(I8, byte)
FB_THRIFT_VALUE_ACCESS_IMPL(I16, i16)
FB_THRIFT_VALUE_ACCESS_IMPL(I32, i32)
FB_THRIFT_VALUE_ACCESS_IMPL(I64, i64)
FB_THRIFT_VALUE_ACCESS_IMPL(Float, float)
FB_THRIFT_VALUE_ACCESS_IMPL(Double, double)
FB_THRIFT_VALUE_ACCESS_IMPL(Bytes, bytes)
FB_THRIFT_VALUE_ACCESS_IMPL(String, string)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeList, list)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeSet, set)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeMap, map)
FB_THRIFT_VALUE_ACCESS_IMPL(Object, object)

} // namespace apache::thrift::protocol::experimental
