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

#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>
#include <thrift/lib/cpp2/type/Any.h>

namespace apache::thrift::type_system {
namespace {
struct ResolveType {
  const TypeSystem& ts;

  TypeRef operator()(const type::TypeUri& type) const {
    using S = type::TypeUri::Type;
    switch (type.getType()) {
      case S::uri: {
        const auto& uri = type.uri().value();
        if (uri == thrift::uri<type::AnyStruct>()) {
          return TypeRef{TypeRef::Any{}};
        }

        return TypeRef(ts.UserDefined(uri));
      }
      default:
        throw std::invalid_argument("resolveType received invalid type uri");
    }
  }

  TypeRef operator()(const type::TypeStruct& type) const {
    using S = type::TypeName::Type;
    switch (type.name()->getType()) {
      case type::TypeName::Type::__EMPTY__:
        throw std::invalid_argument("resolveType received empty type name");
      case S::boolType:
        return TypeRef{TypeRef::Bool{}};
      case S::byteType:
        return TypeRef{TypeRef::Byte{}};
      case S::i16Type:
        return TypeRef{TypeRef::I16{}};
      case S::i32Type:
        return TypeRef{TypeRef::I32{}};
      case S::i64Type:
        return TypeRef{TypeRef::I64{}};
      case S::floatType:
        return TypeRef{TypeRef::Float{}};
      case S::doubleType:
        return TypeRef{TypeRef::Double{}};
      case S::stringType:
        return TypeRef{TypeRef::String{}};
      case S::binaryType:
        return TypeRef{TypeRef::Binary{}};
      case S::enumType:
        return (*this)(type.name()->enumType().value());
      case S::structType:
        return (*this)(type.name()->structType().value());
      case S::unionType:
        return (*this)(type.name()->unionType().value());
      case S::exceptionType:
        // Exceptions will be resolved as structs
        return (*this)(type.name()->exceptionType().value());
      case S::typedefType:
        return (*this)(type.name()->typedefType().value());
      case S::listType:
        return ts.ListOf((*this)(type.params()->at(0)));
      case S::setType:
        return ts.SetOf((*this)(type.params()->at(0)));
      case S::mapType:
        return ts.MapOf(
            (*this)(type.params()->at(0)), (*this)(type.params()->at(1)));
    }
  }
};

struct ToAnyType {
  type::Type operator()(const TypeRef& type) const { return type.visit(*this); }

  type::Type operator()(const TypeRef::Bool&) const {
    return type::Type::create<type::bool_t>();
  }

  type::Type operator()(const TypeRef::Byte&) const {
    return type::Type::create<type::byte_t>();
  }

  type::Type operator()(const TypeRef::I16&) const {
    return type::Type::create<type::i16_t>();
  }

  type::Type operator()(const TypeRef::I32&) const {
    return type::Type::create<type::i32_t>();
  }

  type::Type operator()(const TypeRef::I64&) const {
    return type::Type::create<type::i64_t>();
  }

  type::Type operator()(const TypeRef::Float&) const {
    return type::Type::create<type::float_t>();
  }

  type::Type operator()(const TypeRef::Double&) const {
    return type::Type::create<type::double_t>();
  }

  type::Type operator()(const TypeRef::String&) const {
    return type::Type::create<type::string_t>();
  }

  type::Type operator()(const TypeRef::Binary&) const {
    return type::Type::create<type::binary_t>();
  }

  type::Type operator()(const TypeRef::Any&) const {
    return type::Type::create<type::struct_t<type::AnyStruct>>();
  }

  type::Type operator()(const TypeRef::List& type) const {
    return type::Type(type::list_c{}, (*this)(type.elementType()));
  }

  type::Type operator()(const TypeRef::Set& type) const {
    return type::Type(type::set_c{}, (*this)(type.elementType()));
  }

  type::Type operator()(const TypeRef::Map& type) const {
    return type::Type(
        type::map_c{}, (*this)(type.keyType()), (*this)(type.valueType()));
  }

  type::Type operator()(const StructNode& type) const {
    return type::Type(type::struct_c{}, type.uri());
  }

  type::Type operator()(const UnionNode& type) const {
    return type::Type(type::union_c{}, type.uri());
  }

  type::Type operator()(const EnumNode& type) const {
    return type::Type(type::enum_c{}, type.uri());
  }

  type::Type operator()(const OpaqueAliasNode&) const {
    throw std::invalid_argument(
        "Opaque aliases are not supported in toAnyType conversion");
  }
};
} // namespace

TypeRef resolveAnyType(const TypeSystem& ts, const type::Type& type) {
  if (!type.isValid()) {
    throw std::invalid_argument(
        "resolveType received abstract/incomplete type as argument");
  }
  return ResolveType{ts}(type.toThrift());
}

type::Type toAnyType(const TypeRef& ref) {
  return ToAnyType{}(ref);
}

} // namespace apache::thrift::type_system
