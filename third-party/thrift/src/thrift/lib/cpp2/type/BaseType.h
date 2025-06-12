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

#include <stdexcept>
#include <string>
#include <type_traits>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::type {

enum class BaseType {
  Void = 0,

  // Integer types.
  Bool = 1,
  Byte = 2,
  I16 = 3,
  I32 = 4,
  I64 = 5,

  // Floating point types.
  Float = 6,
  Double = 7,

  // String types.
  String = 8,
  Binary = 9,

  // Enum type class.
  Enum = 10,

  // Structured type classes.
  Struct = 11,
  Union = 12,
  Exception = 13,

  // Container type classes.
  List = 14,
  Set = 15,
  Map = 16
};

std::string_view getBaseTypeName(BaseType type) noexcept;

namespace detail {
constexpr BaseType getBaseType(void_t) {
  return BaseType::Void;
}
constexpr BaseType getBaseType(bool_t) {
  return BaseType::Bool;
}
constexpr BaseType getBaseType(byte_t) {
  return BaseType::Byte;
}
constexpr BaseType getBaseType(i16_t) {
  return BaseType::I16;
}
constexpr BaseType getBaseType(i32_t) {
  return BaseType::I32;
}
constexpr BaseType getBaseType(i64_t) {
  return BaseType::I64;
}
constexpr BaseType getBaseType(float_t) {
  return BaseType::Float;
}
constexpr BaseType getBaseType(double_t) {
  return BaseType::Double;
}
constexpr BaseType getBaseType(string_t) {
  return BaseType::String;
}
constexpr BaseType getBaseType(binary_t) {
  return BaseType::Binary;
}
constexpr BaseType getBaseType(enum_c) {
  return BaseType::Enum;
}
constexpr BaseType getBaseType(struct_c) {
  return BaseType::Struct;
}
constexpr BaseType getBaseType(union_c) {
  return BaseType::Union;
}
constexpr BaseType getBaseType(exception_c) {
  return BaseType::Exception;
}
constexpr BaseType getBaseType(list_c) {
  return BaseType::List;
}
constexpr BaseType getBaseType(set_c) {
  return BaseType::Set;
}
constexpr BaseType getBaseType(map_c) {
  return BaseType::Map;
}
} // namespace detail

// The BaseType for the given ThriftType.
template <typename Tag>
constexpr BaseType base_type_v = detail::getBaseType(Tag{});

// Only defined if T has the BaseType B.
template <typename Tag, BaseType B, typename R = void>
using if_base_type = std::enable_if_t<B == base_type_v<Tag>, R>;

constexpr protocol::TType toTType(BaseType type) {
  using protocol::TType;
  switch (type) {
    case BaseType::Void:
      return TType::T_VOID;
    case BaseType::Bool:
      return TType::T_BOOL;
    case BaseType::Byte:
      return TType::T_BYTE;
    case BaseType::I16:
      return TType::T_I16;
    case BaseType::Enum:
    case BaseType::I32:
      return TType::T_I32;
    case BaseType::I64:
      return TType::T_I64;
    case BaseType::Double:
      return TType::T_DOUBLE;
    case BaseType::Float:
      return TType::T_FLOAT;
    case BaseType::String:
      return TType::T_UTF8;
    case BaseType::Binary:
      return TType::T_STRING;

    case BaseType::List:
      return TType::T_LIST;
    case BaseType::Set:
      return TType::T_SET;
    case BaseType::Map:
      return TType::T_MAP;

    case BaseType::Struct:
      return TType::T_STRUCT;
    case BaseType::Union:
      return TType::T_STRUCT;
    case BaseType::Exception:
      return TType::T_STRUCT;
    default:
      folly::throw_exception<std::invalid_argument>(
          "Unsupported conversion from: " + std::to_string((int)type));
  }
}

constexpr BaseType toBaseType(protocol::TType type) {
  using protocol::TType;
  switch (type) {
    case TType::T_BOOL:
      return BaseType::Bool;
    case TType::T_BYTE:
      return BaseType::Byte;
    case TType::T_I16:
      return BaseType::I16;
    case TType::T_I32:
      return BaseType::I32;
    case TType::T_I64:
      return BaseType::I64;
    case TType::T_DOUBLE:
      return BaseType::Double;
    case TType::T_FLOAT:
      return BaseType::Float;
    case TType::T_LIST:
      return BaseType::List;
    case TType::T_MAP:
      return BaseType::Map;
    case TType::T_SET:
      return BaseType::Set;
    case TType::T_STRING:
      return BaseType::Binary;
    case TType::T_STRUCT:
      return BaseType::Struct;
    case TType::T_UTF8:
      return BaseType::String;
    case TType::T_VOID:
      return BaseType::Void;
    case TType::T_STOP:
    case TType::T_U64:
      // TODO(dokwon): Consider handling as `BaseType::I64`.
    case TType::T_UTF16:
    case TType::T_STREAM:
    default:
      folly::throw_exception<std::invalid_argument>(
          "Unsupported conversion from: " + std::to_string(type));
  }
}

} // namespace apache::thrift::type
