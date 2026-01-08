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

// Various value representations for tests.

namespace cpp2 apache.thrift.conformance
namespace php apache_thrift
namespace py thrift.conformance.test_value
namespace py.asyncio thrift_asyncio.conformance.test_value
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance
namespace go thrift.conformance.test_value

include "thrift/lib/thrift/type.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

// A value defined via calls to a protocol implementation.
struct EncodeValue {
  // The unique uri of the type represented by this encoding.
  1: string type;
  // The sequence of writes to call to create the value.
  2: list<WriteOp> writes;
}

// A single call to a protocol write function.
union WriteOp {
  1: bool writeBool;
  2: byte writeByte;
  3: i16 writeI16;
  4: i32 writeI32;
  5: i64 writeI64;
  6: float writeFloat;
  7: double writeDouble;
  8: string writeString;
  @cpp.Type{name = "folly::IOBuf"}
  9: binary writeBinary;

  10: WriteToken writeToken;
  11: WriteStructBegin writeStructBegin;
  12: WriteFieldBegin writeFieldBegin;
  13: WriteMapBegin writeMapBegin;
  14: WriteListBegin writeListBegin;
  15: WriteSetBegin writeSetBegin;
}

// Write functions that accept no argurments.
enum WriteToken {
  StructEnd = 2,
  FieldEnd = 3,
  FieldStop = 4,
  MapEnd = 5,
  ListEnd = 6,
  SetEnd = 7,
}

struct WriteStructBegin {
  1: string name;
}

struct WriteFieldBegin {
  1: string name;
  2: type.BaseType type;
  3: i16 id;
}

struct WriteMapBegin {
  1: type.BaseType keyType;
  2: type.BaseType valueType;
  3: i32 size;
}

struct WriteListBegin {
  1: type.BaseType elemType;
  2: i32 size;
}

struct WriteSetBegin {
  1: type.BaseType elemType;
  2: i32 size;
}
