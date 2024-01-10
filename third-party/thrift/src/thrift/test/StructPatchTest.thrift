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
include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/patch.thrift"
include "thrift/lib/thrift/standard.thrift"
include "thrift/test/StructPatchTestInclude.thrift"

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
  1: bool boolVal;
  2: byte byteVal;
  3: i16 i16Val;
  4: i32 i32Val;
  5: i64 i64Val;
  6: float floatVal;
  7: double doubleVal;
  8: string stringVal;
  9: binary (cpp.type = "folly::IOBuf") binaryVal;
  10: MyEnum enumVal;
  11: StructPatchTestInclude.MyData structVal;
  12: StructPatchTestInclude.MyUnion unionVal;
  13: LateDefStruct lateStructVal;

  14: optional bool optBoolVal;
  15: optional byte optByteVal;
  16: optional i16 optI16Val;
  17: optional i32 optI32Val;
  18: optional i64 optI64Val;
  19: optional float optFloatVal;
  20: optional double optDoubleVal;
  21: optional string optStringVal;
  22: optional binary (cpp.type = "folly::IOBuf") optBinaryVal;
  23: optional MyEnum optEnumVal;
  24: optional StructPatchTestInclude.MyData optStructVal;
  25: optional LateDefStruct optLateStructVal;

  26: optional list<i16> optListVal;
  27: optional set<string> optSetVal;
  28: optional map<string, string> optMapVal;

  29: LongList longList;

  // These fields should not exist in field patch.
  @cpp.Ref{type = cpp.RefType.Shared}
  30: list<i32> sharedConst;
  @cpp.Ref{type = cpp.RefType.Shared}
  31: optional list<i32> optSharedConst;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  32: list<i32> sharedMutable;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  33: optional list<i32> optSharedMutable;
}

// Intentionally defined after MyStruct, so it's patch types are generated after MyStruct's.
struct LateDefStruct {
  // TODO: There is a bug that `apache::thrift::clear` won't unset field with empty struct
  // we should remove this after the bug is fixed.
  1: i32 field_do_not_use;
}

// AssignOnlyPatch annotation is required on struct level to avoid generating patchPrior
// and patch operation support. Because they will require FieldPatch types to be present,
// but they will be not defined yet.
@patch.AssignOnlyPatch
struct Bar {
  @patch.AssignOnlyPatch
  1: MapStruct extraCycle;
}

struct WithRequiredFields {
  1: required i64 required_int;
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
@cpp.UseOpEncode
struct IncludePatchStruct {
  1: StructPatchTestInclude.MyDataPatch patch (py3.hidden);
}

@cpp.Frozen2Exclude
@patch.AssignOnlyPatch
union IncludePatchUnion {
  1: StructPatchTestInclude.MyDataPatch patch (py3.hidden);
} (py3.hidden)

struct Strings {
  1: list<string> strings;
}

@patch.AssignOnlyPatch
struct AssignOnly {
  1: i32 field;
}

struct Def {
  1: i32 field;
}

struct Opt {
  1: optional i32 field;
}

union Union {
  1: i32 field;
}

struct Ter {
  @thrift.Experimental
  @thrift.TerseWrite
  1: i32 field;
}
