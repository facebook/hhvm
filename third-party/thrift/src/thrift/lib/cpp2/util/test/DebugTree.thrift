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

include "thrift/annotation/python.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/any.thrift"
include "thrift/annotation/thrift.thrift"

cpp_include "thrift/lib/cpp2/type/Any.h"

package "facebook.com/thrift/test"

namespace cpp2 apache.thrift.test

struct Def {
  1: i32 field;
}

@thrift.AllowLegacyTypedefUri
typedef Def MyDef

struct MyData {
  1: string data1;
  2: i32 data2;
  3: optional string data3;
}

struct MyStruct {
  1: bool boolVal;
  2: byte byteVal;
  3: i16 i16Val;
  4: i32 i32Val;
  5: i64 i64Val;
  6: float floatVal;
  7: double doubleVal;
  8: string stringVal;
  @cpp.Type{name = "folly::IOBuf"}
  9: binary binaryVal;

  36: list<i16> listVal;
  37: set<string> setVal;
  38: map<string, string> mapVal;

  11: MyData structVal;

  14: optional bool optBoolVal;

  26: optional list<i16> optListVal;
  27: optional set<string> optSetVal;
  28: optional map<string, string> optMapVal;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::InlineAdapter<::apache::thrift::type::AnyData>",
}
typedef any.Any AnyData

struct StructWithAny {
  1: any.Any any;
  2: map<i32, any.Any> any_map;
  @python.Py3Hidden
  3: AnyData anydata;
}

struct SharedPtr {
  @cpp.Ref{type = cpp.RefType.Shared}
  @python.Py3Hidden
  1: optional string shared_field;
  2: optional i32 field2;
  3: optional i32 field3;
}

struct StructWithTypedef {
  1: MyDef field;
  2: list<MyDef> list_field;
  3: set<MyDef> set_field;
  4: map<i32, MyDef> map_field;
}
