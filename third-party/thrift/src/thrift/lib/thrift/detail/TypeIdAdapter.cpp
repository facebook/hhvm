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

#include <thrift/lib/thrift/detail/TypeIdAdapter.h>

#include <thrift/lib/thrift/gen-cpp2/type_id_types.h>

#include <fmt/core.h>

namespace apache::thrift::type_system::detail {

std::string toTypeidName(const TypeId& type) {
  return type.visit(
      [](TypeId::Bool) -> std::string { return "bool"; },
      [](TypeId::Byte) -> std::string { return "byte"; },
      [](TypeId::I16) -> std::string { return "i16"; },
      [](TypeId::I32) -> std::string { return "i32"; },
      [](TypeId::I64) -> std::string { return "i64"; },
      [](TypeId::Float) -> std::string { return "float"; },
      [](TypeId::Double) -> std::string { return "double"; },
      [](TypeId::String) -> std::string { return "string"; },
      [](TypeId::Binary) -> std::string { return "binary"; },
      [](TypeId::Any) -> std::string { return "any"; },
      [](const TypeId::Uri& uri) -> std::string { return uri; },
      [](const TypeId::List& list) -> std::string { return list.name(); },
      [](const TypeId::Set& set) -> std::string { return set.name(); },
      [](const TypeId::Map& map) -> std::string { return map.name(); });
}

std::string toTypeidName(const ListTypeId& listType) {
  return fmt::format("list<{}>", listType.elementType().name());
}

std::string toTypeidName(const SetTypeId& setType) {
  return fmt::format("set<{}>", setType.elementType().name());
}

std::string toTypeidName(const MapTypeId& mapType) {
  return fmt::format(
      "map<{}, {}>", mapType.keyType().name(), mapType.valueType().name());
}

[[noreturn]] void throwListBadElementType() {
  folly::throw_exception<std::runtime_error>(
      "TypeId::List does not contain an element type. This indicates invalid schema information.");
}

[[noreturn]] void throwSetBadElementType() {
  folly::throw_exception<std::runtime_error>(
      "TypeId::Set does not contain an element type. This indicates invalid schema information.");
}

[[noreturn]] void throwMapBadKeyType() {
  folly::throw_exception<std::runtime_error>(
      "TypeId::Map does not contain a key type. This indicates invalid schema information.");
}

[[noreturn]] void throwMapBadValueType() {
  folly::throw_exception<std::runtime_error>(
      "TypeId::Map does not contain a value type. This indicates invalid schema information.");
}

[[noreturn]] void throwTypeIdAccessInactiveKind(std::string_view actualKind) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "tried to access TypeId with inactive kind, actual kind was: {}",
      actualKind));
}

template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Bool) noexcept {
  this->data_.boolType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Byte) noexcept {
  this->data_.byteType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(I16) noexcept {
  this->data_.i16Type() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(I32) noexcept {
  this->data_.i32Type() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(I64) noexcept {
  this->data_.i64Type() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Float) noexcept {
  this->data_.floatType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Double) noexcept {
  this->data_.doubleType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(String) noexcept {
  this->data_.stringType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Binary) noexcept {
  this->data_.binaryType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Any) noexcept {
  this->data_.anyType() = {};
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Uri uri) noexcept {
  this->data_.userDefinedType() = std::move(uri);
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(List&& list) noexcept {
  this->data_.listType() = std::move(list);
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Set&& set) noexcept {
  this->data_.setType() = std::move(set);
}
template <>
TypeIdWrapper<TypeIdUnion>::TypeIdWrapper(Map&& map) noexcept {
  this->data_.mapType() = std::move(map);
}

} // namespace apache::thrift::type_system::detail

fmt::format_context::iterator
fmt::formatter<apache::thrift::type_system::detail::TypeId>::format(
    const apache::thrift::type_system::detail::TypeId& typeId,
    fmt::format_context& ctx) const {
  return fmt::format_to(ctx.out(), "{}", typeId.name());
}
