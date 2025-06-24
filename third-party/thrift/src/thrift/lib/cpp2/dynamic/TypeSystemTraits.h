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
struct ToTTypeFn;

// Resolves a concrete cpp2 Tag-type into a TypeRef.
// NOTE: This API will statically reject abstract tags (e.g. type::struct_c>)
template <typename Tag>
TypeRef resolveTag(const TypeSystem& ts, Tag);

///
// API IMPLEMENTATION FOLLOWS
///
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

namespace detail {
struct TagResolver {
  const TypeSystem& ts;

  auto operator()(const type::bool_t&) const noexcept {
    return TypeRef::Bool{};
  }

  auto operator()(const type::byte_t&) const noexcept {
    return TypeRef::Byte{};
  }

  auto operator()(const type::i16_t&) const noexcept { return TypeRef::I16{}; }

  auto operator()(const type::i32_t&) const noexcept { return TypeRef::I32{}; }

  auto operator()(const type::i64_t&) const noexcept { return TypeRef::I64{}; }

  auto operator()(const type::float_t&) const noexcept {
    return TypeRef::Float{};
  }

  auto operator()(const type::double_t&) const noexcept {
    return TypeRef::Double{};
  }

  auto operator()(const type::string_t&) const noexcept {
    return TypeRef::String{};
  }

  auto operator()(const type::binary_t&) const noexcept {
    return TypeRef::Binary{};
  }

  template <typename Tag>
  auto operator()(const type::list<Tag>&) const {
    return TypeRef::List(resolveTag(ts, Tag{}));
  }

  template <typename Tag>
  auto operator()(const type::set<Tag>&) const {
    return TypeRef::Set(resolveTag(ts, Tag{}));
  }

  template <typename KTag, typename VTag>
  auto operator()(const type::map<KTag, VTag>&) const {
    return TypeRef::Map(resolveTag(ts, KTag{}), resolveTag(ts, VTag{}));
  }

  template <typename T>
  auto operator()(const type::struct_t<T>&) const {
    return resolveUri(uri<T>());
  }

  template <typename T>
  auto operator()(const type::union_t<T>&) const {
    return resolveUri(uri<T>());
  }

  template <typename T>
  auto operator()(const type::enum_t<T>&) const {
    return resolveUri(uri<T>());
  }

  auto operator()(type::struct_t<type::AnyStruct>) const noexcept {
    return TypeRef::Any{};
  }

  // Adapter doesn't change the resolution of the underlying type.
  template <typename Adapter, typename Tag>
  auto operator()(type::adapted<Adapter, Tag>) const {
    return (*this)(Tag{});
  }

  // Catch All for invalid tags (e.g. type::struct_c)
  template <typename Tag>
  void operator()(Tag) const {
    static_assert(
        (int)sizeof(Tag) == -1,
        "Tag resolution not supported for abstract tags");
  }

 private:
  TypeRef resolveUri(std::string_view uri) const {
    CHECK(!uri.empty());
    return TypeRef::fromDefinition(ts.getUserDefinedTypeOrThrow(uri));
  }
};

} // namespace detail

template <typename Tag>
TypeRef resolveTag(const TypeSystem& ts, Tag) {
  return TypeRef(detail::TagResolver{ts}(Tag{}));
}
} // namespace apache::thrift::type_system
