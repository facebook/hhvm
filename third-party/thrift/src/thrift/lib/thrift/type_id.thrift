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
package "facebook.com/thrift/dynamic"

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"

cpp_include "thrift/lib/thrift/detail/TypeIdAdapter.h"

namespace cpp2 apache.thrift.dynamic

@cpp.Adapter{
  underlyingName = "ListTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::ListTypeId",
  name = "::apache::thrift::dynamic::detail::ListTypeIdAdapter",
}
struct ListTypeId {
  @thrift.Box
  1: optional TypeId elementType;
}

@cpp.Adapter{
  underlyingName = "SetTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::SetTypeId",
  name = "::apache::thrift::dynamic::detail::SetTypeIdAdapter",
}
struct SetTypeId {
  @thrift.Box
  1: optional TypeId elementType;
}

@cpp.Adapter{
  underlyingName = "MapTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::MapTypeId",
  name = "::apache::thrift::dynamic::detail::MapTypeIdAdapter",
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
  adaptedType = "::apache::thrift::dynamic::detail::BoolTypeId",
  name = "::apache::thrift::dynamic::detail::BoolTypeIdAdapter",
}
struct BoolTypeId {}

@cpp.Adapter{
  underlyingName = "ByteTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::ByteTypeId",
  name = "::apache::thrift::dynamic::detail::ByteTypeIdAdapter",
}
struct ByteTypeId {}

@cpp.Adapter{
  underlyingName = "I16TypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::I16TypeId",
  name = "::apache::thrift::dynamic::detail::I16TypeIdAdapter",
}
struct I16TypeId {}

@cpp.Adapter{
  underlyingName = "I32TypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::I32TypeId",
  name = "::apache::thrift::dynamic::detail::I32TypeIdAdapter",
}
struct I32TypeId {}

@cpp.Adapter{
  underlyingName = "I64TypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::I64TypeId",
  name = "::apache::thrift::dynamic::detail::I64TypeIdAdapter",
}
struct I64TypeId {}

@cpp.Adapter{
  underlyingName = "FloatTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::FloatTypeId",
  name = "::apache::thrift::dynamic::detail::FloatTypeIdAdapter",
}
struct FloatTypeId {}

@cpp.Adapter{
  underlyingName = "DoubleTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::DoubleTypeId",
  name = "::apache::thrift::dynamic::detail::DoubleTypeIdAdapter",
}
struct DoubleTypeId {}

@cpp.Adapter{
  underlyingName = "StringTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::StringTypeId",
  name = "::apache::thrift::dynamic::detail::StringTypeIdAdapter",
}
struct StringTypeId {}

@cpp.Adapter{
  underlyingName = "BinaryTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::BinaryTypeId",
  name = "::apache::thrift::dynamic::detail::BinaryTypeIdAdapter",
}
struct BinaryTypeId {}

@cpp.Adapter{
  underlyingName = "AnyTypeIdStruct",
  adaptedType = "::apache::thrift::dynamic::detail::AnyTypeId",
  name = "::apache::thrift::dynamic::detail::AnyTypeIdAdapter",
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
  adaptedType = "::apache::thrift::dynamic::detail::TypeId",
  name = "::apache::thrift::dynamic::detail::TypeIdAdapter",
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
