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

include "thrift/annotation/thrift.thrift"
include "thrift/lib/thrift/patch.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/annotation/cpp.thrift"

@thrift.Experimental
@thrift.TerseWrite
@patch.GeneratePatch
package "test.dev/fixtures/patch"

namespace android test.fixtures.patch
namespace java test.fixtures.patch
namespace java.swift test.fixtures.patch

struct MyData {
  1: string data1;
  2: i32 data2;
}

struct MyDataWithCustomDefault {
  1: string data1 = "1";
  2: i32 data2 = 2;
}

union InnerUnion {
  1: binary innerOption;
}

union MyUnion {
  1: string option1;
  2: i32 option2;
  3: InnerUnion option3;
}

enum MyEnum {
  MyValue0 = 0,
}

struct MyStruct {
  bool boolVal;
  byte byteVal;
  i16 i16Val;
  i32 i32Val;
  i64 i64Val;
  float floatVal;
  double doubleVal;
  string stringVal;
  @cpp.Type{name = "folly::IOBuf"}
  binary binaryVal;
  MyEnum enumVal;
  MyData structVal;
  MyUnion unionVal;
  LateDefStruct lateStructVal;

  optional bool optBoolVal;
  optional byte optByteVal;
  optional i16 optI16Val;
  optional i32 optI32Val;
  optional i64 optI64Val;
  optional float optFloatVal;
  optional double optDoubleVal;
  optional string optStringVal;
  @cpp.Type{name = "folly::IOBuf"}
  optional binary optBinaryVal;
  optional MyEnum optEnumVal;
  optional MyData optStructVal;
  optional LateDefStruct optLateStructVal;

  optional list<i16> optListVal;
  optional set<string> optSetVal;
  optional map<string, string> optMapVal;

  list<map<string, i32>> listMap;
  map<string, map<string, i32>> mapMap;

  i32 i32WithCustomDefault = 1;
  MyDataWithCustomDefault structWithCustomDefault;
  1: MyData structWithFieldCustomDefault = {"data1": "1", "data2": 2};
}

// Intentionally defined after MyStruct, so it's patch types are generated after MyStruct's.
struct LateDefStruct {}

struct Recursive {
  @patch.AssignOnlyPatch
  map<string, Recursive> nodes;
}

struct Bar {
  @cpp.Ref{type = cpp.RefType.Unique}
  Loop loop;
}

@patch.AssignOnlyPatch
struct Loop {
  @patch.AssignOnlyPatch
  Bar bar;
}

struct RefFields {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: list<i32> unique;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: list<i32> shared_const;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: list<i32> shared_mustable;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: optional list<i32> opt_unique;
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional list<i32> opt_shared_const;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  6: optional list<i32> opt_shared_mustable;
  @thrift.Box
  7: optional list<i32> opt_box;
}
