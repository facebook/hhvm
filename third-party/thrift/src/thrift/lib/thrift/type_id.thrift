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

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
package "facebook.com/thrift/type_system"

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"

cpp_include "thrift/lib/thrift/detail/TypeIdAdapter.h"

namespace cpp2 apache.thrift.type_system

@cpp.Adapter{
  underlyingName = "ListTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::ListTypeId",
  name = "::apache::thrift::type_system::detail::ListTypeIdAdapter",
}
struct ListTypeId {
  @thrift.Box
  1: optional TypeId elementType;
}

@cpp.Adapter{
  underlyingName = "SetTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::SetTypeId",
  name = "::apache::thrift::type_system::detail::SetTypeIdAdapter",
}
struct SetTypeId {
  @thrift.Box
  1: optional TypeId elementType;
}

@cpp.Adapter{
  underlyingName = "MapTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::MapTypeId",
  name = "::apache::thrift::type_system::detail::MapTypeIdAdapter",
}
struct MapTypeId {
  @thrift.Box
  1: optional TypeId keyType;
  @thrift.Box
  2: optional TypeId valueType;
}

typedef string Uri

// These adapters are necessary because the other adapted types (e.g. TypeId)
// are much easier to implement with complete types in C++. Since the adapters
// are in a cpp_include'd file, the Thrift-generated structs are not available.
@cpp.Adapter{
  underlyingName = "BoolTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::BoolTypeId",
  name = "::apache::thrift::type_system::detail::BoolTypeIdAdapter",
}
struct BoolTypeId {}

@cpp.Adapter{
  underlyingName = "ByteTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::ByteTypeId",
  name = "::apache::thrift::type_system::detail::ByteTypeIdAdapter",
}
struct ByteTypeId {}

@cpp.Adapter{
  underlyingName = "I16TypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::I16TypeId",
  name = "::apache::thrift::type_system::detail::I16TypeIdAdapter",
}
struct I16TypeId {}

@cpp.Adapter{
  underlyingName = "I32TypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::I32TypeId",
  name = "::apache::thrift::type_system::detail::I32TypeIdAdapter",
}
struct I32TypeId {}

@cpp.Adapter{
  underlyingName = "I64TypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::I64TypeId",
  name = "::apache::thrift::type_system::detail::I64TypeIdAdapter",
}
struct I64TypeId {}

@cpp.Adapter{
  underlyingName = "FloatTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::FloatTypeId",
  name = "::apache::thrift::type_system::detail::FloatTypeIdAdapter",
}
struct FloatTypeId {}

@cpp.Adapter{
  underlyingName = "DoubleTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::DoubleTypeId",
  name = "::apache::thrift::type_system::detail::DoubleTypeIdAdapter",
}
struct DoubleTypeId {}

@cpp.Adapter{
  underlyingName = "StringTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::StringTypeId",
  name = "::apache::thrift::type_system::detail::StringTypeIdAdapter",
}
struct StringTypeId {}

@cpp.Adapter{
  underlyingName = "BinaryTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::BinaryTypeId",
  name = "::apache::thrift::type_system::detail::BinaryTypeIdAdapter",
}
struct BinaryTypeId {}

@cpp.Adapter{
  underlyingName = "AnyTypeIdStruct",
  adaptedType = "::apache::thrift::type_system::detail::AnyTypeId",
  name = "::apache::thrift::type_system::detail::AnyTypeIdAdapter",
}
struct AnyTypeId {}

/**
 * A unique identifier for a Thrift type within a type system, colloquially
 * called its "typeid". The primary purpose of typeids are to associate types
 * across different type systems.
 *
 * Primitive types have a well-known identity across all type systems.
 *
 * User-defined types are identified by their URIs. The "kind" of definition
 * (e.g. struct vs union) is not part of the type's identity.
 *
 * Container types are composed of other type identities.
 */
@cpp.Adapter{
  underlyingName = "TypeIdUnion",
  adaptedType = "::apache::thrift::type_system::detail::TypeId",
  name = "::apache::thrift::type_system::detail::TypeIdAdapter",
}
union TypeId {
  // Primitive types
  1: BoolTypeId boolType;
  2: ByteTypeId byteType;
  3: I16TypeId i16Type;
  4: I32TypeId i32Type;
  5: I64TypeId i64Type;
  6: FloatTypeId floatType;
  7: DoubleTypeId doubleType;
  8: StringTypeId stringType;
  9: BinaryTypeId binaryType;
  10: AnyTypeId anyType;

  /**
   * All (and only) user-defined types are identified with a URI.
   *
   * Disambigutation of the actual definition (e.g. struct vs union) can only
   * be achieved in the context of a type system.
   */
  11: Uri userDefinedType;

  // Container types
  12: ListTypeId listType;
  13: SetTypeId setType;
  14: MapTypeId mapType;
}
