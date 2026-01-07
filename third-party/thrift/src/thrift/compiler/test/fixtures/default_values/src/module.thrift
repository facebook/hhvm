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

package "facebook.com/thrift/compiler/test/fixtures/default_values"

include "thrift/annotation/thrift.thrift"

struct TrivialStruct {
  1: i32 int_value;
}

struct TrivialStructWithDefault {
  1: i32 int_value_1;
  2: i32 int_value_2 = 42;
}

struct StructWithNoCustomDefaultValues {
  1: i32 unqualified_integer;
  2: optional i32 optional_integer;
  3: required i32 required_integer;

  4: TrivialStruct unqualified_struct;
  5: optional TrivialStruct optional_struct;
  6: required TrivialStruct required_struct;
}

struct StructWithCustomDefaultValues {
  1: i32 unqualified_integer = 42;
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  2: optional i32 optional_integer = 43;
  3: required i32 required_integer = 44;

  4: TrivialStruct unqualified_struct = TrivialStruct{int_value = 123};
  @thrift.AllowUnsafeOptionalCustomDefaultValue
  5: optional TrivialStruct optional_struct = TrivialStruct{int_value = 456};
  6: required TrivialStruct required_struct = TrivialStruct{int_value = 789};
  7: TrivialStructWithDefault struct_with_default_unspecified = TrivialStructWithDefault{};
  8: TrivialStructWithDefault struct_with_default_specified = TrivialStructWithDefault{int_value_1 = 123};
}

struct StructWithCollectionDefaultValues {
  1: list<i32> list_with_list_value = [];
  2: set<i32> set_with_list_value = [];
  3: map<i32, i32> map_with_map_value = {};

  // Mixing empty list/map is a compiler warning, but allowed
  4: list<i32> list_with_map_value = {};
  5: set<i32> set_with_map_value = {};
  6: map<i32, i32> map_with_list_value = [];
}
