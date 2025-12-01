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

#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

#include <fmt/core.h>
#include <folly/lang/Exception.h>

namespace apache::thrift::dynamic {

// Placeholder equality operators for unimplemented types
bool operator==(const Any&, const Any&) noexcept {
  folly::terminate_with<std::logic_error>("Any comparison not yet implemented");
}

} // namespace apache::thrift::dynamic

namespace apache::thrift::dynamic::detail {

std::string_view Datum::nameOf(Kind k) noexcept {
  switch (k) {
    case Kind::Null:
      return "Null";
    case Kind::Bool:
      return "Bool";
    case Kind::Byte:
      return "Byte";
    case Kind::I16:
      return "I16";
    case Kind::I32:
      return "I32";
    case Kind::I64:
      return "I64";
    case Kind::Float:
      return "Float";
    case Kind::Double:
      return "Double";
    case Kind::String:
      return "String";
    case Kind::Binary:
      return "Binary";
    case Kind::Any:
      return "Any";
    case Kind::List:
      return "List";
    case Kind::Set:
      return "Set";
    case Kind::Map:
      return "Map";
    case Kind::Struct:
      return "Struct";
    case Kind::Union:
      return "Union";
  }
  return "<invalid>";
}

void Datum::throwIfNot(Kind k) const {
  if (kind() != k) {
    throw std::runtime_error(
        fmt::format(
            "Expected Datum kind {} but got {}", nameOf(k), nameOf(kind())));
  }
}

void Datum::throwIfNull() const {
  if (kind() == Kind::Null) {
    throw std::runtime_error("Datum is null");
  }
}

bool Datum::operator==(const Datum& other) const {
  return alternative_ == other.alternative_;
}

} // namespace apache::thrift::dynamic::detail

namespace apache::thrift::dynamic {

detail::Datum fromRecord(
    const type_system::SerializableRecord& r,
    const type_system::TypeRef& type,
    std::pmr::memory_resource* alloc) {
  return type.visit(
      [&](const auto& ty) {
        return detail::Datum::make(fromRecord(r, ty, alloc));
      },
      [](const type_system::OpaqueAliasNode&) -> detail::Datum {
        throw std::logic_error("OpaqueAlias not supported");
      });
}

namespace detail {

void expectType(
    const type_system::TypeRef& expected, const type_system::TypeRef& actual) {
  auto debug = [](type_system::TypeRef t) {
    return debugString(t.id().toThrift());
  };
  if (!expected.isEqualIdentityTo(actual)) {
    throw std::runtime_error(
        fmt::format(
            "Type mismatch: expected {}, but was {}",
            debug(expected),
            debug(actual)));
  }
}

[[noreturn]] void throwInvalidDatumKindError(
    Datum::Kind expected, Datum::Kind actual) {
  throw std::runtime_error(
      fmt::format(
          "Invalid Datum kind access: expected {}, but was {}",
          Datum::nameOf(expected),
          Datum::nameOf(actual)));
}

} // namespace detail
} // namespace apache::thrift::dynamic
