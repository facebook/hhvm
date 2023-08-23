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
cpp_include "folly/container/F14Map.h"
cpp_include "folly/container/F14Set.h"

@thrift.Experimental
package "facebook.com/thrift/protocol/detail"

namespace cpp2 apache.thrift.protocol.detail
namespace py3 apache.thrift.protocol.detail
namespace php apache_thrift_protocol.detail
namespace java.swift com.facebook.thrift.protocol_detail_swift
namespace py.asyncio apache_thrift_asyncio.protocol_detail
namespace go thrift.lib.thrift.protocol_detail
namespace py thrift.lib.thrift.protocol_detail

// A dynamic struct/union/exception
@cpp.Adapter{
  name = "::apache::thrift::protocol::detail::ObjectAdapter",
  adaptedType = "::apache::thrift::protocol::detail::ObjectWrapper<::apache::thrift::protocol::detail::detail::Object>",
}
@cpp.UseOpEncode
struct Object {
  // The type of the object, if applicable.
  1: standard.Uri type;

  // The members of the object.
  // TODO(ytj): use schema.FieldId as key
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{template = "::folly::F14FastMap"}
  2: map<i16, Value> members;
} (cpp.virtual, thrift.uri = "facebook.com/thrift/protocol/Object", rust.ord)

// We need this to use float/double as set/map key in rust
typedef float Float (rust.newtype, rust.type = "OrderedFloat<f32>", rust.ord)
typedef double Double (rust.newtype, rust.type = "OrderedFloat<f64>", rust.ord)

// A dynamic value.
@cpp.Adapter{
  name = "::apache::thrift::protocol::detail::ValueAdapter",
  adaptedType = "::apache::thrift::protocol::detail::ValueWrapper<::apache::thrift::protocol::detail::detail::Value>",
}
@cpp.ScopedEnumAsUnionType
@cpp.UseOpEncode
union Value {
  // Integers.
  1: bool boolValue;
  2: byte byteValue;
  3: i16 i16Value;
  4: i32 i32Value;
  5: i64 i64Value;

  // Floats.
  6: Float floatValue;
  7: Double doubleValue;

  // Strings.
  8: string stringValue;

  9: standard.ByteBuffer binaryValue;

  // A dynamic object value.
  11: Object objectValue;

  // Containers of values.
  14: list<Value> listValue;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{template = "::folly::F14FastSet"}
  @hack.SkipCodegen{
    reason = "Set can only have integer/string/binary/enum values",
  }
  15: set<Value> setValue;

  // TODO(dokwon): Migrate to @thrift.Box after resolving incomplete type.
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{template = "::folly::F14FastMap"}
  @hack.SkipCodegen{reason = "Map keys can only be integer/string/binary/enum"}
  16: map<Value, Value> mapValue;
} (cpp.virtual, thrift.uri = "facebook.com/thrift/protocol/Value", rust.ord)
