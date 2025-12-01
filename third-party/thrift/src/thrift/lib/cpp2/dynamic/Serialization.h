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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/dynamic/fwd.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/Traits.h>

#include <glog/logging.h>
#include <folly/Conv.h>
#include <folly/lang/Assume.h>
#include <folly/memory/not_null.h>

#include <memory_resource>

namespace apache::thrift::dynamic {

// ============================================================================
// serialize - converts Datum to wire format
// ============================================================================

// Serialize arithmetic types
template <typename T, typename ProtocolWriter>
  requires std::is_arithmetic_v<T>
void serialize(ProtocolWriter& writer, T value) {
  op::encode<type::infer_tag<T>>(writer, value);
}

// String
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const String&) {
  throw std::logic_error("Unimplemented: serialize(String)");
}

// Binary
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Binary&) {
  throw std::logic_error("Unimplemented: serialize(Binary)");
}

// Any
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Any&) {
  throw std::logic_error("Unimplemented: serialize(Any)");
}

// List
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const List&) {
  throw std::logic_error("Unimplemented: serialize(List)");
}

// Set
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Set&) {
  throw std::logic_error("Unimplemented: serialize(Set)");
}

// Map
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Map&) {
  throw std::logic_error("Unimplemented: serialize(Map)");
}

// Struct
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Struct&) {
  throw std::logic_error("Unimplemented: serialize(Struct)");
}

// Union
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, const Union&) {
  throw std::logic_error("Unimplemented: serialize(Union)");
}

// Null
template <typename ProtocolWriter>
void serialize(ProtocolWriter&, Null) {
  throw std::logic_error("Serializing Null is not possible.");
}

// ============================================================================
// deserialize - converts wire format to Datum
// ============================================================================

// Deserialize functions for TypeRef types
template <typename ProtocolReader>
bool deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Bool,
    std::pmr::memory_resource*) {
  bool ret;
  op::decode<type::bool_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int8_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Byte,
    std::pmr::memory_resource*) {
  int8_t ret;
  op::decode<type::byte_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int16_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I16,
    std::pmr::memory_resource*) {
  int16_t ret;
  op::decode<type::i16_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int32_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I32,
    std::pmr::memory_resource*) {
  int32_t ret;
  op::decode<type::i32_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int64_t deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::I64,
    std::pmr::memory_resource*) {
  int64_t ret;
  op::decode<type::i64_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
float deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Float,
    std::pmr::memory_resource*) {
  float ret;
  op::decode<type::float_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
double deserialize(
    ProtocolReader& reader,
    type_system::TypeRef::Double,
    std::pmr::memory_resource*) {
  double ret;
  op::decode<type::double_t>(reader, ret);
  return ret;
}

template <typename ProtocolReader>
int32_t deserialize(
    ProtocolReader& reader,
    const type_system::EnumNode&,
    std::pmr::memory_resource*) {
  int32_t ret;
  op::decode<type::i32_t>(reader, ret);
  return ret;
}

// String
template <typename ProtocolReader>
String deserialize(
    ProtocolReader&, type_system::TypeRef::String, std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::String)");
}

// Binary
template <typename ProtocolReader>
Binary deserialize(
    ProtocolReader&, type_system::TypeRef::Binary, std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Binary)");
}

// Any
template <typename ProtocolReader>
Any deserialize(
    ProtocolReader&, type_system::TypeRef::Any, std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Any)");
}

// List
template <typename ProtocolReader>
List deserialize(
    ProtocolReader&,
    const type_system::TypeRef::List&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::List)");
}

// Set
template <typename ProtocolReader>
Set deserialize(
    ProtocolReader&,
    const type_system::TypeRef::Set&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Set)");
}

// Map
template <typename ProtocolReader>
Map deserialize(
    ProtocolReader&,
    const type_system::TypeRef::Map&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(TypeRef::Map)");
}

// Struct
template <typename ProtocolReader>
Struct deserialize(
    ProtocolReader&,
    const type_system::StructNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(StructNode)");
}

// Union
template <typename ProtocolReader>
Union deserialize(
    ProtocolReader&,
    const type_system::UnionNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(UnionNode)");
}

// Unsupported types
template <typename ProtocolReader>
int deserialize(
    ProtocolReader&,
    const type_system::OpaqueAliasNode&,
    std::pmr::memory_resource*) {
  throw std::logic_error("Unimplemented: deserialize(OpaqueAliasNode)");
}

// Helper overload for not_null pointers
template <typename ProtocolReader, typename T>
auto deserialize(
    ProtocolReader& reader, folly::not_null<const T*> t, auto&&... args) {
  return deserialize(reader, *t, std::forward<decltype(args)>(args)...);
}

// ============================================================================
// serializeValue/deserializeValue - high-level serialization for DynamicValue
// ============================================================================

/**
 * Deserialize a DynamicValue from a protocol reader.
 */
template <typename ProtocolReader>
DynamicValue deserializeValue(
    ProtocolReader& prot,
    type_system::TypeRef type,
    std::pmr::memory_resource* mr) {
  return DynamicValue(type, type.visit([&](auto&& t) {
    return detail::Datum::make(deserialize(prot, t, mr));
  }));
}

/**
 * Serialize a DynamicConstRef to a protocol writer.
 */
template <typename ProtocolWriter>
void serializeValue(ProtocolWriter& prot, const DynamicConstRef& v) {
  using Kind = type_system::TypeRef::Kind;
  v.type().matchKind([&]<Kind k>(type_system::TypeRef::KindConstant<k>) {
    serialize(prot, v.as<k>());
  });
}

} // namespace apache::thrift::dynamic
