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

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 benchmark

struct ThriftTestDataSmall {
  1: i32 intField1;
  2: i64 longField1;
  3: string shortString1;
  4: list<i32> intList1;
  5: map<string, i32> stringMap1;
  6: bool boolField1;
  7: double doubleField1;
}

struct ThriftTestDataMedium {
  1: i32 intField1;
  2: i64 longField1;
  3: string shortString1;
  4: list<i32> intList1;
  5: map<string, i32> stringMap1;
  6: bool boolField1;
  7: double doubleField1;
  8: i32 intField2;
  9: i64 longField2;
  10: string shortString2;
  11: list<i32> intList2;
  12: map<string, i32> stringMap2;
  13: bool boolField2;
  14: double doubleField2;
  15: i32 intField3;
  16: i64 longField3;
  17: string shortString3;
  18: list<i32> intList3;
  19: map<string, i32> stringMap3;
  20: bool boolField3;
  21: double doubleField3;
}

struct ThriftTestDataLarge {
  1: i32 intField1;
  2: i64 longField1;
  3: string shortString1;
  4: list<i32> intList1;
  5: map<string, i32> stringMap1;
  6: bool boolField1;
  7: double doubleField1;
  8: i32 intField2;
  9: i64 longField2;
  10: string shortString2;
  11: list<i32> intList2;
  12: map<string, i32> stringMap2;
  13: bool boolField2;
  14: double doubleField2;
  15: i32 intField3;
  16: i64 longField3;
  17: string shortString3;
  18: list<i32> intList3;
  19: map<string, i32> stringMap3;
  20: bool boolField3;
  21: double doubleField3;
  22: i32 intField4;
  23: i64 longField4;
  24: string shortString4;
  25: list<i32> intList4;
  26: map<string, i32> stringMap4;
  27: bool boolField4;
  28: double doubleField4;
  29: i32 intField5;
  30: i64 longField5;
  31: string shortString5;
  32: list<i32> intList5;
  33: map<string, i32> stringMap5;
  34: bool boolField5;
  35: double doubleField5;
  36: i32 intField6;
  37: i64 longField6;
  38: string shortString6;
  39: list<i32> intList6;
  40: map<string, i32> stringMap6;
  41: bool boolField6;
  42: double doubleField6;
  43: i32 intField7;
  44: i64 longField7;
  45: string shortString7;
  46: list<i32> intList7;
  47: map<string, i32> stringMap7;
  48: bool boolField7;
  49: double doubleField7;
  50: i32 intField8;
  51: i64 longField8;
  52: string shortString8;
  53: list<i32> intList8;
  54: map<string, i32> stringMap8;
  55: bool boolField8;
  56: double doubleField8;
  57: i32 intField9;
  58: i64 longField9;
  59: string shortString9;
  60: list<i32> intList9;
  61: map<string, i32> stringMap9;
  62: bool boolField9;
  63: double doubleField9;
}
