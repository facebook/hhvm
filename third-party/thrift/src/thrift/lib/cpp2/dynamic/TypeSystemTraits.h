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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>

namespace apache::thrift::type_system {

// Callable that converts a TypeRef (or one of its variants) into the wire-type
// for Binary and Compact protocols (TType)
struct ToTTypeFn {
  constexpr TType operator()(const TypeRef::Bool&) const noexcept {
    return TType::T_BOOL;
  }
  constexpr TType operator()(const TypeRef::Byte&) const noexcept {
    return TType::T_BYTE;
  }
  constexpr TType operator()(const TypeRef::I16&) const noexcept {
    return TType::T_I16;
  }
  constexpr TType operator()(const TypeRef::I32&) const noexcept {
    return TType::T_I32;
  }
  constexpr TType operator()(const TypeRef::I64&) const noexcept {
    return TType::T_I64;
  }
  constexpr TType operator()(const TypeRef::Float&) const noexcept {
    return TType::T_FLOAT;
  }
  constexpr TType operator()(const TypeRef::Double&) const noexcept {
    return TType::T_DOUBLE;
  }
  constexpr TType operator()(const TypeRef::String&) const noexcept {
    return TType::T_STRING;
  }
  constexpr TType operator()(const TypeRef::Binary&) const noexcept {
    return TType::T_STRING;
  }
  constexpr TType operator()(const StructNode&) const noexcept {
    return TType::T_STRUCT;
  }
  constexpr TType operator()(const UnionNode&) const noexcept {
    return TType::T_STRUCT;
  }
  constexpr TType operator()(const TypeRef::List&) const noexcept {
    return TType::T_LIST;
  }
  constexpr TType operator()(const TypeRef::Set&) const noexcept {
    return TType::T_SET;
  }
  constexpr TType operator()(const TypeRef::Map&) const noexcept {
    return TType::T_MAP;
  }
  constexpr TType operator()(const EnumNode&) const noexcept {
    return TType::T_I32;
  }
  constexpr TType operator()(const TypeRef::Any&) const noexcept {
    return TType::T_STRUCT;
  }
  constexpr TType operator()(const OpaqueAliasNode& alias) const noexcept {
    return (*this)(alias.targetType());
  }
  constexpr TType operator()(const TypeRef& tref) const noexcept {
    return tref.visit(*this);
  }
};

} // namespace apache::thrift::type_system
