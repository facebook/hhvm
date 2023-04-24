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

@thrift.TerseWrite
@thrift.Experimental
package "test.dev/thrift/lib/java/test/terse"

namespace java.swift com.facebook.thrift.test.terse

enum PackageLevelEnum {
  ME0 = 0,
  ME1 = 1,
}

struct PackageLevelTerseData {
  1: string data1;
  2: i32 data2;
}

union PackageLevelInnerUnion {
  1: binary innerOption;
}

union PackageLevelTerseUnion {
  1: string option1;
  2: i32 option2;
  3: PackageLevelInnerUnion option3;
}

struct PackageLevelTerseStruct {
  1: bool boolVal;
  2: byte byteVal;
  3: i16 i16Val;
  4: i32 i32Val;
  5: i64 i64Val;
  6: float floatVal;
  7: double doubleVal;
  8: string stringVal;
  @cpp.Type{name = "::folly::IOBuf"}
  9: binary binaryVal;
  10: PackageLevelTerseData structVal;

  11: optional bool optBoolVal;
  12: optional byte optByteVal;
  13: optional i16 optI16Val;
  14: optional i32 optI32Val;
  15: optional i64 optI64Val;
  16: optional float optFloatVal;
  17: optional double optDoubleVal;
  18: optional string optStringVal;
  @cpp.Type{name = "::folly::IOBuf"}
  19: optional binary optBinaryVal;
  20: optional PackageLevelTerseData optStructVal;

  21: optional list<i16> optListVal;
  22: optional set<string> optSetVal;
  23: optional map<string, string> optMapVal;

  30: PackageLevelTerseUnion unionVal;

  40: PackageLevelEnum enumVal;
  41: list<i16> listVal;
  42: set<i16> setVal;
  43: map<i16, i16> mapVal;
}
