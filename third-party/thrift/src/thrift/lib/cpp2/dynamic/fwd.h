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

#include <cstdint>

namespace apache::thrift::dynamic {

// Forward declarations for all dynamic types

// Base types
struct Null;

namespace detail {
class Datum;

// Datum::Kind enum - defined here so it can be used in type traits
enum class DatumKind : uint8_t {
  Null = 0,
  Bool = 1,
  Byte = 2,
  I16 = 3,
  I32 = 4,
  I64 = 5,
  Float = 6,
  Double = 7,
  String = 8,
  Binary = 9,
  Any = 10,
  List = 11,
  Set = 12,
  Map = 13,
  Struct = 14,
  Union = 15,
};

// Container implementation types
class IList;
template <typename T>
class ConcreteList;
class ISet;
template <typename T>
class ConcreteSet;
class IMap;
template <typename K, typename V>
class ConcreteMap;
struct DatumHash;
struct DatumEqual;

class IStruct;

} // namespace detail

class String;
class Binary;
class Any;
class List;
class Set;
class Map;
class Struct;
class Union;

// Value wrappers
class DynamicValue;
class DynamicConstRef;
class DynamicRef;

// Forward declarations for functions defined in Serialization.h
template <typename ProtocolReader>
DynamicValue deserializeValue(
    ProtocolReader& prot,
    type_system::TypeRef type,
    std::pmr::memory_resource* mr = nullptr);

namespace detail {

// Type traits for mapping Datum::Kind to C++ types
// These are defined here to avoid circular dependencies

/**
 * Maps Datum::Kind to the actual C++ type stored in the variant
 */
template <DatumKind k>
struct DatumKindToType;

template <>
struct DatumKindToType<DatumKind::Null> {
  using type = Null;
};
template <>
struct DatumKindToType<DatumKind::Bool> {
  using type = bool;
};
template <>
struct DatumKindToType<DatumKind::Byte> {
  using type = int8_t;
};
template <>
struct DatumKindToType<DatumKind::I16> {
  using type = int16_t;
};
template <>
struct DatumKindToType<DatumKind::I32> {
  using type = int32_t;
};
template <>
struct DatumKindToType<DatumKind::I64> {
  using type = int64_t;
};
template <>
struct DatumKindToType<DatumKind::Float> {
  using type = float;
};
template <>
struct DatumKindToType<DatumKind::Double> {
  using type = double;
};
template <>
struct DatumKindToType<DatumKind::String> {
  using type = String;
};
template <>
struct DatumKindToType<DatumKind::Binary> {
  using type = Binary;
};
template <>
struct DatumKindToType<DatumKind::Any> {
  using type = Any;
};
template <>
struct DatumKindToType<DatumKind::List> {
  using type = List;
};
template <>
struct DatumKindToType<DatumKind::Set> {
  using type = Set;
};
template <>
struct DatumKindToType<DatumKind::Map> {
  using type = Map;
};
template <>
struct DatumKindToType<DatumKind::Struct> {
  using type = Struct;
};
template <>
struct DatumKindToType<DatumKind::Union> {
  using type = Union;
};

template <DatumKind k>
using datum_kind_to_type = typename DatumKindToType<k>::type;

/**
 * Maps TypeRef::Kind to Datum::Kind and the C++ type
 */
template <type_system::TypeRef::Kind k>
struct TypeRefKindToDatum;

#define THRIFT_DEFINE_TYPE_REF_TO_DATUM(typeKind, datumKind) \
  template <>                                                \
  struct TypeRefKindToDatum<typeKind> {                      \
    static constexpr DatumKind kind = DatumKind::datumKind;  \
    using type = datum_kind_to_type<DatumKind::datumKind>;   \
  }

THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::BOOL, Bool);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::BYTE, Byte);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::I16, I16);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::I32, I32);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::I64, I64);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::FLOAT, Float);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::DOUBLE, Double);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::STRING, String);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::BINARY, Binary);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::ANY, Any);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::LIST, List);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::SET, Set);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::MAP, Map);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::STRUCT, Struct);
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::UNION, Union);
// Enum types are always represented as int32_t
THRIFT_DEFINE_TYPE_REF_TO_DATUM(type_system::TypeRef::Kind::ENUM, I32);

#undef THRIFT_DEFINE_TYPE_REF_TO_DATUM

/**
 * Convenience alias: maps TypeRef::Kind to the C++ type
 */
template <type_system::TypeRef::Kind k>
using type_of_type_kind = typename TypeRefKindToDatum<k>::type;

/**
 * Convenience alias: maps TypeRef::Kind to Datum::Kind
 */
template <type_system::TypeRef::Kind k>
inline constexpr DatumKind kind_of_type_kind = TypeRefKindToDatum<k>::kind;

struct FreeDeleter {
  template <typename T>
  void operator()(T* ptr) const {
    ptr->free();
  }
};

} // namespace detail

} // namespace apache::thrift::dynamic
