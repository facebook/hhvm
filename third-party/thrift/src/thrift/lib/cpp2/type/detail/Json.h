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

#include <string>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/op/Hash.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

namespace apache::thrift::type::detail {

template <typename JsonType, typename JsonValue>
class Json : public detail::EqWrap<Json<JsonType, JsonValue>, JsonValue> {
  using Base = detail::EqWrap<Json, JsonValue>;
  template <JsonType typ>
  using JType = std::integral_constant<JsonType, typ>;
  template <typename T, typename R = void>
  using if_not_self =
      std::enable_if_t<!std::is_same_v<folly::remove_cvref_t<T>, Json>, R>;

 public:
  static constexpr JsonType Null = JsonType::Null;
  static constexpr JsonType Boolean = JsonType::Boolean;
  static constexpr JsonType Number = JsonType::Number;
  static constexpr JsonType String = JsonType::String;
  static constexpr JsonType Array = JsonType::Array;
  static constexpr JsonType Object = JsonType::Object;

  Json() = default;
  Json(const Json&) = default;
  Json(Json&&) noexcept = default;
  explicit Json(const JsonValue& other) : Base(other) { normalize(); }
  explicit Json(JsonValue&& other) noexcept : Base(std::move(other)) {
    normalize();
  }
  Json& operator=(const Json&) = default;
  Json& operator=(Json&&) = default;

  JsonType type() const {
    return data_.getType() == JsonValue::floatValue
        ? JsonType::Number
        : static_cast<JsonType>(data_.getType());
  }

  bool isNull() const { return data_.getType() == JsonValue::__EMPTY__; }
  bool isBool() const { return data_.getType() == JsonValue::boolValue; }
  bool isInt() const { return data_.getType() == JsonValue::intValue; }
  bool isFloat() const { return data_.getType() == JsonValue::floatValue; }
  bool isNumber() const { return isInt() || isFloat(); }
  bool isString() const { return data_.getType() == JsonValue::stringValue; }
  bool isArray() const { return data_.getType() == JsonValue::arrayValue; }
  bool isObject() const { return data_.getType() == JsonValue::objectValue; }

  // Directly mutable fields.
  FBTHRIFT_WRAP_DATA_FIELD(boolean, boolValue_ref)
  FBTHRIFT_WRAP_DATA_FIELD(string, stringValue_ref)
  FBTHRIFT_WRAP_DATA_FIELD(array, arrayValue_ref)
  FBTHRIFT_WRAP_DATA_FIELD(object, objectValue_ref)

  // Numbers are mutated via `assign`.
  FBTHRIFT_WRAP_CDATA_FIELD(integer, intValue_ref)
  FBTHRIFT_WRAP_CDATA_FIELD(floating, floatValue_ref)

  Json& at(size_t pos) & { return array()->at(pos); }
  Json&& at(size_t pos) && { return std::move(array()->at(pos)); }
  const Json& at(size_t pos) const& { return array()->at(pos); }
  const Json&& at(size_t pos) const&& { return std::move(array()->at(pos)); }

  Json& at(Ordinal ord) & { return at(toPosition(ord)); }
  Json&& at(Ordinal ord) && { return at(toPosition(ord)); }
  const Json& at(Ordinal ord) const& { return at(toPosition(ord)); }
  const Json&& at(Ordinal ord) const&& { return at(toPosition(ord)); }

  // TODO(afuller): Switch to heterogenous lookup or string views.
  Json& at(const std::string& name) & { return object()->at(name); }
  Json&& at(const std::string& name) && {
    return std::move(object()->at(name));
  }
  const Json& at(const std::string& name) const& { return object()->at(name); }
  const Json&& at(const std::string& name) const&& {
    return std::move(object()->at(name));
  }

  void assign(std::string value) { data_.stringValue_ref() = std::move(value); }
  template <typename T>
  std::enable_if_t<std::is_same_v<T, bool>> assign(T value) {
    data_.boolValue_ref() = value;
  }
  template <typename T>
  std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>> assign(
      T value) {
    data_.intValue_ref() = value;
  }
  template <typename T>
  std::enable_if_t<std::is_floating_point_v<T>> assign(T value) {
    int64_t asInt = value;
    if (value == static_cast<T>(asInt)) {
      data_.intValue_ref() = value;
    } else {
      data_.floatValue_ref() = value;
    }
  }

  template <JsonType typ>
  decltype(auto) ensure() {
    return ensureImpl(JType<typ>{});
  }

  template <JsonType typ>
  bool ensureIfNull() {
    if (isNull()) {
      ensure<typ>();
      return true;
    }
    return type() == typ;
  }

  // TODO(afuller): Switch to heterogenous lookup or string views.
  decltype(auto) operator[](const std::string& name) & {
    ensureIfNull<JsonType::Object>();
    return object()->operator[](name);
  }
  decltype(auto) operator[](const std::string& name) && {
    ensureIfNull<JsonType::Object>();
    return std::move(object()->operator[](name));
  }
  decltype(auto) operator[](const std::string& name) const& {
    return object()->operator[](name);
  }
  decltype(auto) operator[](const std::string& name) const&& {
    return std::move(object()->operator[](name));
  }
  decltype(auto) operator[](Ordinal ord) & { return (adjust(ord), at(ord)); }
  decltype(auto) operator[](Ordinal ord) && { return (adjust(ord), at(ord)); }
  decltype(auto) operator[](Ordinal ord) const& { return at(ord); }
  decltype(auto) operator[](Ordinal ord) const&& { return at(ord); }
  decltype(auto) operator[](size_t pos) & { return (adjust(pos), at(pos)); }
  decltype(auto) operator[](size_t pos) && { return (adjust(pos), at(pos)); }
  decltype(auto) operator[](size_t pos) const& { return at(pos); }
  decltype(auto) operator[](size_t pos) const&& { return at(pos); }

  // TODO(afuller): Lots of other operators.
  template <typename T>
  if_not_self<T, Json&> operator=(T&& value) & {
    assign(std::forward<T>(value));
    return *this;
  }
  template <typename T>
  if_not_self<T, Json&&> operator=(T&& value) && {
    assign(std::forward<T>(value));
    return std::move(*this);
  }

 private:
  using Base::data_;

  // TODO(afuller): Lots of other operators.
  friend constexpr bool operator<(const Json& lhs, const Json& rhs) {
    return op::less<typename Base::underlying_type>(
        Base::data(lhs), Base::data(rhs));
  }

  void normalize() {
    if (isFloat()) {
      assign(*data_.floatValue_ref());
    }
  }

  void ensureImpl(JType<JsonType::Null>) { Base::clear(); }
  std::string& ensureImpl(JType<JsonType::String>) {
    return data_.stringValue_ref().ensure();
  }
  decltype(auto) ensureImpl(JType<JsonType::Array>) {
    return data_.arrayValue_ref().ensure();
  }
  decltype(auto) ensureImpl(JType<JsonType::Object>) {
    return data_.objectValue_ref().ensure();
  }

  void adjust(std::size_t pos) {
    if (ensureIfNull<JsonType::Array>() && pos >= array()->size()) {
      array()->resize(pos + 1);
    }
  }
  void adjust(Ordinal ord) { adjust(toPosition(ord)); }
};

template <typename JsonType>
using JsonAdapter = TemplateInlineAdapter<Json, JsonType>;

} // namespace apache::thrift::type::detail
