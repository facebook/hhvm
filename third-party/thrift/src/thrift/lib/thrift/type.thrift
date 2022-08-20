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

include "thrift/lib/thrift/type_rep.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "<thrift/lib/cpp2/type/BaseType.h>"
cpp_include "<thrift/lib/cpp2/type/Protocol.h>"
cpp_include "<thrift/lib/cpp2/type/Type.h>"
cpp_include "<thrift/lib/cpp2/type/UniversalHashAlgorithm.h>"

/** Canonical representations for well-known Thrift types. */
@thrift.v1alpha
package "facebook.com/thrift/type"

namespace cpp2 apache.thrift.type
namespace py3 apache.thrift.type
namespace php apache_thrift_type
namespace java com.facebook.thrift.type
namespace java.swift com.facebook.thrift.type_swift
namespace py.asyncio apache_thrift_asyncio.type
namespace go thrift.lib.thrift.type
namespace py thrift.lib.thrift.type

/**
 * An enumeration of all base types in thrift.
 *
 * Base types are not parameterized. For example, the base type of
 * map<int, string> is BaseType::Map and the base type of all thrift
 * structs is BaseType::Struct.
 *
 * Similar to lib/cpp/protocol/TType.h, but IDL concepts instead of protocol
 * concepts.
 */
@cpp.Adapter{
  name = "::apache::thrift::StaticCastAdapter<::apache::thrift::type::BaseType, ::apache::thrift::type::BaseTypeEnum>",
}
typedef BaseTypeEnum BaseType
enum BaseTypeEnum {
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
  Map = 16,
}

/** The hash algorithms that can be used with type names. */
@cpp.Adapter{
  name = "::apache::thrift::StaticCastAdapter<::apache::thrift::type::UniversalHashAlgorithm, ::apache::thrift::type::UniversalHashAlgorithmEnum>",
}
typedef UniversalHashAlgorithmEnum UniversalHashAlgorithm
enum UniversalHashAlgorithmEnum {
  Sha2_256 = 2, // = getFieldId(TypeUri::typeHashPrefixSha2_256).
}

/**
 * A 'normal' Duration.
 *
 * This representation is always safe to 'normalize' or 'saturate' at
 * +/-infinite time, instead of overflowing.
 */
// TODO(afuller): Provide const definitions for +/-infinite time.
@thrift.Experimental // TODO(afuller): Adapt!
typedef type_rep.DurationStruct Duration (thrift.uri = "")

/**
 * A 'normal' Time.
 *
 * This representation is always safe to 'normalize' or 'saturate' at
 * the infinite future/past, instead of overflowing.
 */
// TODO(afuller): Provide const definitions for infinite future/past.
@thrift.Experimental // TODO(afuller): Adapt!
typedef type_rep.TimeStruct Time (thrift.uri = "")

/**
 * An 'internet timestamp' as described in [RFC 3339](https://www.ietf.org/rfc/rfc3339.txt).
 *
 * Similar to `Time`, but can only represent values in the range
 * 0001-01-01T00:00:00Z to 9999-12-31T23:59:59Z inclusive, for compatibility
 * with the 'date string' format. Thus `seconds` must be in the range
 * -62'135'769'600 to 253'402'300'799 inclusive, when `normal`.
 *
 * This representation is always safe to 'normalize' or 'saturate' at the
 * min/max allowed 'date string' values, instead of overflowing.
 */
@thrift.Experimental // TODO(afuller): Adapt!
typedef type_rep.TimeStruct Timestamp

/**
 * The minimum timestamp allowed by [RFC 3339](https://www.ietf.org/rfc/rfc3339.txt).
 */
// TODO(afuller): Ignore single quote (') in number literals.
// TODO(afuller): Allow unquoted identifiers as field names/string map keys.
const Timestamp minTimestamp = {"seconds": -62135769600};

/**
 * The maximum timestamp allowed by [RFC 3339](https://www.ietf.org/rfc/rfc3339.txt).
 */
const Timestamp maxTimestamp = {"seconds": 253402300799, "nanos": 999999999};

/**
 * A 'normal' Fraction.
 *
 * This representation is always safe to 'normalize'.
 */
@thrift.Experimental // TODO(afuller): Adapt!
typedef type_rep.FractionStruct Fraction (thrift.uri = "")

/**
 * A 'simple' Fraction.
 *
 * This representation is always safe to 'simplify'.
 */
@thrift.Experimental // TODO(afuller): Adapt!
typedef type_rep.FractionStruct SimpleFraction

@cpp.Adapter{
  name = "::apache::thrift::InlineAdapter<::apache::thrift::type::Protocol>",
}
typedef type_rep.ProtocolUnion Protocol (thrift.uri = "")

@cpp.Adapter{
  name = "::apache::thrift::InlineAdapter<::apache::thrift::type::Type>",
}
typedef type_rep.TypeStruct Type (thrift.uri = "")

/** A list of Types, accessible by `TypeId`. */
@thrift.Experimental // TODO(afuller): Adapt! and use Type directly when supported.
typedef list<type_rep.TypeStruct> TypeList
