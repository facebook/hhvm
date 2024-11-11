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

namespace go thrift.test.go.if.reflecttest

enum Colors {
  WHITE = 1,
  BLACK = 2,
}

struct ComparableStruct {
  1: i64 field1;
}

struct NonComparableStruct {
  // Go slices are not comparable
  // As per: https://go.dev/ref/spec#Comparison_operators
  1: list<string> field1;
}

// A struct to test various fields (types/optionality/etc)
struct VariousFieldsStruct {
  // Primitive fields
  1: byte field1;
  2: bool field2;
  3: i16 field3;
  4: i32 field4;
  5: i64 field5;
  6: float field6;
  7: double field7;
  8: binary field8;
  9: string field9;
  // Optional primitive fields
  11: optional byte field11;
  12: optional bool field12;
  13: optional i16 field13;
  14: optional i32 field14;
  15: optional i64 field15;
  16: optional float field16;
  17: optional double field17;
  18: optional binary field18;
  19: optional string field19;

  // Enum
  20: Colors field20;
  // Optional enum
  21: optional Colors field21;

  // Containers
  30: set<string> field30;
  31: list<string> field31;
  32: set<ComparableStruct> field32;
  33: list<ComparableStruct> field33;
  34: map<string, string> field34;
  // Optional containers
  41: optional set<string> field41;
  42: optional list<string> field42;
  43: optional map<string, string> field43;
  // Map edge cases
  51: map<ComparableStruct, string> field51;
  52: map<NonComparableStruct, string> field52;
  53: map<list<string>, string> field53;

  // Struct
  60: ComparableStruct field60;
  // Optional struct
  61: optional ComparableStruct field61;
}

const VariousFieldsStruct variousFieldsStructConst1 = {
  "field1": 12,
  "field2": true,
  "field3": 1500,
  "field4": 33000,
  "field5": 2247483648,
  "field6": 123.5,
  "field7": 1.7976931348623157e+308,
  "field8": "binary_test",
  "field9": "string_test",
  "field11": 12,
  "field12": true,
  "field13": 1500,
  "field14": 33000,
  "field15": 2247483648,
  "field16": 123.5,
  "field17": 1.7976931348623157e+308,
  "field18": "binary_test",
  "field19": "string_test",
  "field20": 1,
  "field21": 2,
  "field30": ["hello1", "hello2", "hello3"],
  "field31": ["hello1", "hello2", "hello3"],
  "field32": [{"field1": 123}, {"field1": 456}],
  "field33": [{"field1": 123}, {"field1": 456}],
  "field34": {"key1": "value1", "key2": "value2"},
  "field41": ["hello1", "hello2", "hello3"],
  "field42": ["hello1", "hello2", "hello3"],
  "field43": {"key1": "value1", "key2": "value2"},
  "field52": {{"field1": ["hello1", "hello2", "hello3"]}: "value1"},
  "field60": {"field1": 123},
  "field61": {"field1": 123},
};
