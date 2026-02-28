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
#include <thrift/lib/thrift/gen-cpp2/type_id_types_custom_protocol.h>

#include <iosfwd>

#include <string_view>

namespace apache::thrift::type {
class AnyStruct;
} // namespace apache::thrift::type

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

  // Thrift's URI is a folly::cstring_view, so convert it to a Uri
  template <typename T>
    requires std::constructible_from<std::string, T>
  static TypeId uri(T&& uri) {
    return Uri{std::forward<T>(uri)};
  }

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

// Convert a type-tag into a TypeId
template <typename Tag>
TypeId tagToTypeId(Tag);

namespace detail {
struct TagToTypeId {
  TypeId operator()(type::byte_t) const noexcept { return TypeIds::Byte; }
  TypeId operator()(type::bool_t) const noexcept { return TypeIds::Bool; }
  TypeId operator()(type::i16_t) const noexcept { return TypeIds::I16; }
  TypeId operator()(type::i32_t) const noexcept { return TypeIds::I32; }
  TypeId operator()(type::i64_t) const noexcept { return TypeIds::I64; }
  TypeId operator()(type::float_t) const noexcept { return TypeIds::Float; }
  TypeId operator()(type::double_t) const noexcept { return TypeIds::Double; }
  TypeId operator()(type::string_t) const noexcept { return TypeIds::String; }
  TypeId operator()(type::binary_t) const noexcept { return TypeIds::Binary; }

  template <typename E>
  TypeId operator()(type::list<E>) const {
    return TypeIds::list((*this)(E{}));
  }

  template <typename E>
  TypeId operator()(type::set<E>) const {
    return TypeIds::set((*this)(E{}));
  }

  template <typename K, typename V>
  TypeId operator()(type::map<K, V>) const {
    return TypeIds::map((*this)(K{}), (*this)(V{}));
  }

  template <typename T>
  TypeId operator()(type::struct_t<T>) const {
    return TypeIds::uri(uri<T>());
  }

  template <typename T>
  TypeId operator()(type::union_t<T>) const {
    return TypeIds::uri(uri<T>());
  }

  template <typename E>
  TypeId operator()(type::enum_t<E>) const {
    return TypeIds::uri(uri<E>());
  }

  TypeId operator()(type::struct_t<type::AnyStruct>) const noexcept {
    return TypeIds::Any;
  }

  // TypeId doesn't change for adapted types.
  template <typename Adapter, typename Tag>
  TypeId operator()(type::adapted<Adapter, Tag>) const noexcept {
    return (*this)(Tag{});
  }

  // Catch All for invalid tags (e.g. type::struct_c)
  template <typename Tag>
  void operator()(Tag) const {
    static_assert(
        (int)sizeof(Tag) == -1,
        "Tag resolution not supported for abstract tags");
  }
};
} // namespace detail

template <typename Tag>
TypeId tagToTypeId(Tag) {
  return detail::TagToTypeId{}(Tag{});
}

} // namespace apache::thrift::type_system

// Make TypeId (and subtypes) hashable
// DISCLAIMER: THIS IS NOT A DETERMINISTIC HASH! DO NOT PERSIST HASH VALUES!
FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type_system::ListTypeId)
FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type_system::SetTypeId)
FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type_system::MapTypeId)
FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type_system::TypeId)
