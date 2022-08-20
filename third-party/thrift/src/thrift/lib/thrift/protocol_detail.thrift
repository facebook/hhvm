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

// TODO(ytj): merge this file into thrift/lib/thrift/type.thrift

include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/hack.thrift"
cpp_include "thrift/lib/thrift/detail/protocol.h"

@thrift.v1alpha
package "facebook.com/thrift/protocol/detail"

namespace cpp2 apache.thrift.protocol.detail
namespace py3 apache.thrift.protocol.detail
namespace php apache_thrift_protocol.detail
namespace java com.facebook.thrift.protocol.detail
namespace java.swift com.facebook.thrift.protocol_detail_swift
namespace py.asyncio apache_thrift_asyncio.protocol_detail
namespace go thrift.lib.thrift.protocol_detail
namespace py thrift.lib.thrift.protocol_detail

typedef ObjectStruct (
  cpp.type = "::apache::thrift::protocol::detail::ObjectWrapper<::apache::thrift::protocol::detail::ObjectStruct>",
  cpp.indirection,
) Object

typedef ValueUnion (
  cpp.type = "::apache::thrift::protocol::detail::ValueWrapper<::apache::thrift::protocol::detail::ValueUnion>",
  cpp.indirection,
) Value

// A dynamic struct/union/exception
struct ObjectStruct {
  // The type of the object, if applicable.
  1: standard.Uri type;

  // The members of the object.
  // TODO(ytj): use schema.FieldId as key
  2: map<i16, Value> members;
} (
  cpp.virtual, // TODO(ytj): add protected dtor for base class
  thrift.uri = "facebook.com/thrift/protocol/Object",
  cpp.detail.no_any,
)

// A dynamic value.
@cpp.ScopedEnumAsUnionType
union ValueUnion {
  // Integers.
  1: bool boolValue;
  2: byte byteValue;
  3: i16 i16Value;
  4: i32 i32Value;
  5: i64 i64Value;

  // Floats.
  6: float floatValue;
  7: double doubleValue;

  // Strings.
  8: string stringValue;

  9: standard.ByteBuffer binaryValue;

  // A dynamic object value.
  11: Object objectValue;

  // Containers of values.
  14: list<Value> listValue;
  @hack.SkipCodegen{
    reason = "Set can only have integer/string/binary/enum values",
  }
  15: set<Value> setValue;
  @hack.SkipCodegen{reason = "Map keys can only be integer/string/binary/enum"}
  16: map<Value, Value> mapValue;
} (
  cpp.virtual, // TODO(ytj): add protected dtor for base class
  thrift.uri = "facebook.com/thrift/protocol/Value",
  cpp.detail.no_any,
)
