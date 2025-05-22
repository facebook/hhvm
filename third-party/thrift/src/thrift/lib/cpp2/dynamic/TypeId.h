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

#include <thrift/lib/thrift/gen-cpp2/type_id_types.h>

#include <iosfwd>

#include <string_view>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
namespace apache::thrift::type_system {

using UriView = std::string_view;

namespace detail {
struct UriHeterogeneousHash {
  using is_transparent = void;

  std::size_t operator()(const Uri& uri) const noexcept {
    return std::hash<UriView>{}(uri);
  }
  std::size_t operator()(const UriView& uri) const noexcept {
    return std::hash<UriView>{}(uri);
  }
};
} // namespace detail

/**
 * A class containing helpers to author TypeId objects.
 */
class TypeIds final {
 public:
  TypeIds() = delete;

  static inline const TypeId Bool = TypeId::Bool();
  static inline const TypeId Byte = TypeId::Byte();
  static inline const TypeId I16 = TypeId::I16();
  static inline const TypeId I32 = TypeId::I32();
  static inline const TypeId I64 = TypeId::I64();
  static inline const TypeId Float = TypeId::Float();
  static inline const TypeId Double = TypeId::Double();
  static inline const TypeId String = TypeId::String();
  static inline const TypeId Binary = TypeId::Binary();
  static inline const TypeId Any = TypeId::Any();

  static TypeId uri(Uri value) { return std::move(value); }

  static TypeId list(TypeId elementType) {
    return TypeId::List(std::move(elementType));
  }
  static TypeId set(TypeId elementType) {
    return TypeId::Set(std::move(elementType));
  }
  static TypeId map(TypeId keyType, TypeId valueType) {
    return TypeId::Map(std::move(keyType), std::move(valueType));
  }
};

std::ostream& operator<<(std::ostream&, const TypeId&);

} // namespace apache::thrift::type_system
