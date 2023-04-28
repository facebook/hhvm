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
include "thrift/test/StructPatchTestInclude.thrift"

@thrift.Testing
@patch.GeneratePatch
package "facebook.com/thrift/test/patch"

namespace cpp2 apache.thrift.test.patch
namespace py3 thrift.test

enum MyEnum {
  MyValue0 = 0,
  MyValue9 = 9,
}

typedef list<i64> (cpp.template = "std::deque") LongList

struct MyStruct {
  bool boolVal;
  byte byteVal;
  i16 i16Val;
  i32 i32Val;
  i64 i64Val;
  float floatVal;
  double doubleVal;
  string stringVal;
  binary (cpp.type = "folly::IOBuf") binaryVal;
  MyEnum enumVal;
  StructPatchTestInclude.MyData structVal;
  StructPatchTestInclude.MyUnion unionVal;
  LateDefStruct lateStructVal;

  optional bool optBoolVal;
  optional byte optByteVal;
  optional i16 optI16Val;
  optional i32 optI32Val;
  optional i64 optI64Val;
  optional float optFloatVal;
  optional double optDoubleVal;
  optional string optStringVal;
  optional binary (cpp.type = "folly::IOBuf") optBinaryVal;
  optional MyEnum optEnumVal;
  optional StructPatchTestInclude.MyData optStructVal;
  optional LateDefStruct optLateStructVal;

  optional list<i16> optListVal;
  optional set<string> optSetVal;
  optional map<string, string> optMapVal;

  LongList longList;
}

// Intentionally defined after MyStruct, so it's patch types are generated after MyStruct's.
struct LateDefStruct {
  // TODO: There is a bug that `apache::thrift::clear` won't unset field with empty struct
  // we should remove this after the bug is fixed.
  i32 field_do_not_use;
}

// AssignOnlyPatch annotation is required on struct level to avoid generating patchPrior
// and patch operation support. Because they will require FieldPatch types to be present,
// but they will be not defined yet.
@patch.AssignOnlyPatch
struct Bar {
  @patch.AssignOnlyPatch
  MapStruct extraCycle;
}

struct WithRequiredFields {
  required i64 requrired_int;
}

@cpp.Frozen2Exclude
struct MapStruct {
  @patch.AssignOnlyPatch
  1: map<i32, MapStruct> rabbitHole;
  @thrift.Box
  2: optional Bar bar;
}

struct InvalidMapMaskKeyStruct {
  1: map<double, i32> field1;
}

@cpp.Frozen2Exclude
@patch.AssignOnlyPatch
struct IncludePatchStruct {
  1: StructPatchTestInclude.MyDataPatch patch (py3.hidden);
}

@cpp.Frozen2Exclude
@patch.AssignOnlyPatch
union IncludePatchUnion {
  1: StructPatchTestInclude.MyDataPatch patch (py3.hidden);
} (py3.hidden)

struct Strings {
  list<string> strings;
}
