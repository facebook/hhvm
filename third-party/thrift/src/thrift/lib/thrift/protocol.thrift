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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/id.thrift"
include "thrift/lib/thrift/protocol_detail.thrift"
include "thrift/lib/thrift/type.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/annotation/python.thrift"

cpp_include "folly/container/F14Map.h"
cpp_include "thrift/lib/thrift/detail/id.h"

@thrift.Experimental
@thrift.TerseWrite
package "facebook.com/thrift/protocol"

namespace cpp2 apache.thrift.protocol
namespace py3 apache.thrift.protocol
namespace php apache_thrift_protocol
namespace java.swift com.facebook.thrift.protocol_swift
namespace py.asyncio apache_thrift_asyncio.protocol
namespace go thrift.lib.thrift.protocol
namespace py thrift.lib.thrift.protocol

typedef protocol_detail.Object Object (thrift.uri = "")
typedef protocol_detail.Value Value (thrift.uri = "")

// Represents serialized data of unmasked fields.
@python.Py3Hidden
union MaskedData {
  1: id.ValueId full;
  // TODO(dokwon): Migrate to @thrift.Box after resolving incomplete type.
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{template = "folly::F14VectorMap"}
  2: map<id.FieldId, MaskedData> fields;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.Type{template = "folly::F14VectorMap"}
  3: map<id.ValueId, MaskedData> values;
}

@python.Py3Hidden
struct EncodedValue {
  1: type.BaseType wireType;
  2: standard.ByteBuffer data;
}

// MaskedData uses ValueId to get encodedValues and map keys from the lists.
@python.Py3Hidden
@cpp.UseOpEncode
struct MaskedProtocolData {
  1: type.Protocol protocol;
  2: MaskedData data;
  3: list<EncodedValue> values;
  4: list<Value> keys;
}
