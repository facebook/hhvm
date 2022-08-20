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
include "thrift/test/StructPatchTestInclude.thrift"

@patch.GeneratePatch
package "facebook.com/thrift/test/patch"

namespace cpp2 apache.thrift.test.patch

struct MyStruct {
  1: bool boolVal;
  2: byte byteVal;
  3: i16 i16Val;
  4: i32 i32Val;
  5: i64 i64Val;
  6: float floatVal;
  7: double doubleVal;
  8: string stringVal;
  9: binary (cpp.type = "::folly::IOBuf") binaryVal;
  10: StructPatchTestInclude.MyData structVal;

  11: optional bool optBoolVal;
  12: optional byte optByteVal;
  13: optional i16 optI16Val;
  14: optional i32 optI32Val;
  15: optional i64 optI64Val;
  16: optional float optFloatVal;
  17: optional double optDoubleVal;
  18: optional string optStringVal;
  19: optional binary (cpp.type = "::folly::IOBuf") optBinaryVal;
  20: optional StructPatchTestInclude.MyData optStructVal;

  21: optional list<i16> optListVal;
  22: optional set<string> optSetVal;
  23: optional map<string, string> optMapVal;

  30: StructPatchTestInclude.MyUnion unionVal;
  31: LateDefStruct lateStructVal;
}

// Intentionally defined after MyStruct, so it's patch types are generated after MyStruct's.
struct LateDefStruct {}
