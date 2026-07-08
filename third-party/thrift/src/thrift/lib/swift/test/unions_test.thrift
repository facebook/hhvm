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

package "test.dev/swift/tests/unions"

namespace swift FBThriftTestsUnions

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

struct MyDataItem {}

union MyUnion {
  1: MyEnum my_enum;
  2: MyDataItem my_data_item;
}

union SingleFieldUnion {
  1: i32 reserved_field;
}

// Typedef-to-container types.
typedef list<i32> IntList
typedef map<string, i32> StringIntMap

union TypedefContainerUnion {
  1: IntList int_list_field;
  2: StringIntMap string_int_map_field;
}
