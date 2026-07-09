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

package "test.dev/swift/tests/structs"

namespace swift FBThriftTestsStructs

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

struct MyDataItem {}

struct MyStruct {
  1: i64 my_int_field;
  2: string my_string_field;
  3: MyDataItem my_data_field;
  4: MyEnum my_enum;
  5: bool oneway;
  6: bool readonly;
  7: bool idempotent;
}

struct Containers {
  1: list<i32> i32_list;
  2: set<string> string_set;
  3: map<string, i64> string_to_i64_map;
}

// Typedef-to-container types (regression test: codegen must resolve through
// typedefs before accessing container element/key/value type properties).
typedef list<i32> IntList
typedef set<string> StringSet
typedef map<string, i64> StringToI64Map

struct TypedefContainerStruct {
  1: IntList int_list_field;
  2: StringSet string_set_field;
  3: StringToI64Map string_map_field;
}

struct OptionalFieldsStruct {
  1: optional string optional_string;
  2: optional MyDataItem optional_struct;
  3: optional list<i32> optional_list;
}

exception MyException {
  1: i64 my_int_field;
  2: string my_string_field;
}

// Fields with custom IDL defaults, so clear() (which resets to intrinsic zero
// values) can be distinguished from init() (which honors the IDL defaults).
struct StructWithDefaults {
  1: i64 int_with_default = 42;
  2: string string_with_default = "hello";
  3: list<i32> list_with_default = [1, 2, 3];
}
