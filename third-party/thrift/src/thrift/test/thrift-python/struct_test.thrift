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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/python.thrift"
include "thrift/test/thrift-python/included.thrift"

namespace py3 thrift.test.thrift_python

const bool bool_constant = true;
const byte byte_constant = -10; // byte is an 8-bit signed integer
const i16 i16_constant = 200;
const i32 i32_constant = 0xFA12EE;
const i64 i64_constant = 0xFFFFFFFFFF;
const float float_constant = 2.718281828459;
const double double_constant = 2.718281828459;
const string string_constant = "June 28, 2017";

const list<i32> list_constant = [2, 3, 5, 7];

const set<string> set_constant = ["foo", "bar", "baz"];

const map<string, i32> map_constant = {"foo": 1, "bar": 2};

enum TestEnum {
  ARM1 = 1,
  ARM2 = 2,
  ARM4 = 4,
}

struct TestStructConstant {
  1: i32 unqualified_i32;
  2: string unqualified_string;
  3: list<i32> unqualified_list_i32;
}

const TestStructConstant struct_constant = {
  "unqualified_i32": 42,
  "unqualified_string": "Hello world!",
  "unqualified_list_i32": [1, 2, 3],
};

struct TestStruct {
  1: string unqualified_string;
  2: optional string optional_string;
}

struct TestStructWithDefaultValues {
  1: i32 unqualified_integer = 42;

  2: optional i32 optional_integer = 43;

  3: TestStruct unqualified_struct = TestStruct{unqualified_string = "hello"};

  4: optional TestStruct optional_struct = TestStruct{
    unqualified_string = "world",
  };

  5: TestStruct unqualified_struct_intrinsic_default;

  6: optional TestStruct optional_struct_intrinsic_default;

  7: list<i32> unqualified_list_i32 = [1, 2, 3];

  8: TestEnum unqualified_enum = TestEnum.ARM1;

  9: optional TestEnum optional_enum = TestEnum.ARM2;
}

struct TestStructAllThriftPrimitiveTypes {
  1: bool unqualified_bool;
  2: optional bool optional_bool;

  3: byte unqualified_byte;
  4: optional byte optional_byte;

  5: i16 unqualified_i16;
  6: optional i16 optional_i16;

  7: i32 unqualified_i32;
  8: optional i32 optional_i32;

  9: i64 unqualified_i64;
  10: optional i64 optional_i64;

  11: float unqualified_float;
  12: optional float optional_float;

  13: double unqualified_double;
  14: optional double optional_double;

  15: string unqualified_string;
  16: optional string optional_string;
}

struct TestStructAllThriftPrimitiveTypesWithDefaultValues {
  1: bool unqualified_bool = true;

  2: byte unqualified_byte = 32;

  3: i16 unqualified_i16 = 512;

  4: i32 unqualified_i32 = 2048;

  5: i64 unqualified_i64 = 999;

  6: float unqualified_float = 1.0;

  7: double unqualified_double = 1.231;

  8: string unqualified_string = "thrift-python";
}

struct TestStructAllThriftContainerTypes {
  1: list<i32> unqualified_list_i32;
  2: optional list<i32> optional_list_i32;

  3: set<string> unqualified_set_string;
  4: optional set<string> optional_set_string;

  5: map<string, i32> unqualified_map_string_i32;
  6: optional map<string, i32> optional_map_string_i32;
}

struct TestStructAsListElement {
  1: string string_field;
  2: list<i32> list_int;
}

struct TestStructContainerAssignment {
  1: list<i32> list_int;
  2: list<i32> list_int_2;
  3: list<list<i32>> list_list_int;
  4: list<list<i32>> list_list_int_2;
  5: list<TestStructAsListElement> list_struct;

  6: set<string> set_string;
  7: set<string> set_string_2;

  8: map<i32, list<i32>> map_int_to_list_int;
}

exception TestExceptionAsListElement {
  1: string string_field;
  2: list<i32> list_int;
}

exception TestExceptionWithContainer {
  1: list<TestExceptionAsListElement> list_exception;
}

struct TestStructAdaptedTypes {
  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  1: i32 unqualified_adapted_i32_to_datetime;

  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  2: optional i32 optional_adapted_i32_to_datetime;

  @python.Adapter{
    name = "thrift.python.test.adapters.atoi.AtoiAdapter",
    typeHint = "int",
  }
  3: string unqualified_adapted_string_to_i32 = "123";
}

struct TestStructEmpty {}
typedef TestStructEmpty TestStructEmptyAlias

struct TestStructWithTypedefField {
  1: i32 n;
  2: TestStructEmpty empty_struct;
  3: TestStructEmptyAlias empty_struct_alias;
}

struct TestStructNested_2 {
  1: i32 i32_field;
}

struct TestStructNested_1 {
  1: i32 i32_field;
  2: TestStructNested_2 nested_2;
}

struct TestStructNested_0 {
  1: i32 i32_field;
  2: TestStructNested_1 nested_1;
}

union TestUnion {
  1: i32 i32_field;
  2: string string_field;
}

struct TestStructWithUnionField {
  1: i32 i32_field;
  2: TestUnion union_field;
  3: included.TestUnion union_field_from_included;
}

exception TestExceptionAllThriftPrimitiveTypes {
  1: bool unqualified_bool;
  2: optional bool optional_bool;

  3: byte unqualified_byte;
  4: optional byte optional_byte;

  5: i16 unqualified_i16;
  6: optional i16 optional_i16;

  7: i32 unqualified_i32;
  8: optional i32 optional_i32;

  9: i64 unqualified_i64;
  10: optional i64 optional_i64;

  11: float unqualified_float;
  12: optional float optional_float;

  13: double unqualified_double;
  14: optional double optional_double;

  15: string unqualified_string;
  16: optional string optional_string;
}

struct TestStructWithExceptionField {
  1: i32 i32_field;
  2: TestExceptionAllThriftPrimitiveTypes exception_field;
}

struct TestStructCopy {
  1: i32 unqualified_i32;
  2: optional i32 optional_i32;

  3: string unqualified_string;
  4: optional string optional_string;

  5: list<i32> unqualified_list_i32;
  6: set<string> unqualified_set_string;
  7: map<string, i32> unqualified_map_string_i32;

  8: optional TestStructCopy recursive_struct;

  @cpp.Type{name = "folly::IOBuf"}
  9: binary unqualified_binary;
}

exception TestExceptionCopy {
  1: i32 unqualified_i32;
  2: optional i32 optional_i32;

  3: string unqualified_string;
  4: optional string optional_string;

  5: list<i32> unqualified_list_i32;
  6: optional list<i32> optional_list_i32;

  7: set<string> unqualified_set_string;
  8: optional set<string> optional_set_string;

  9: map<string, i32> unqualified_map_string_i32;
  10: optional TestExceptionCopy recursive_exception;
}

struct TestStructWithInvariantField {
  1: i32 unqualified_i32;
  2: string unqualified_string;
  3: map<TestStruct, i32> unqualified_map_struct_i32;
}

struct TestStructWithNestedContainers {
  1: list<list<i32>> list_list_i32;
  2: list<set<i32>> list_set_i32;
  3: list<map<string, i32>> list_map_string_i32;
  4: list<map<string, list<i32>>> list_map_string_list_i32;
  5: list<map<string, set<i32>>> list_map_string_set_i32;
  6: set<list<i32>> set_list_i32;
  7: set<set<i32>> set_set_i32;
  8: set<map<string, i32>> set_map_string_i32;
  9: set<map<string, list<i32>>> set_map_string_list_i32;
  10: set<map<string, set<i32>>> set_map_string_set_i32;
  11: map<i32, list<i32>> map_i32_list_i32;
  12: map<i32, set<i32>> map_i32_set_i32;
  13: map<i32, map<string, i32>> map_i32_map_string_i32;
  14: map<i32, map<string, list<i32>>> map_i32_map_string_list_i32;
  15: map<i32, map<string, set<i32>>> map_i32_map_string_set_i32;
  16: list<list<map<i32, list<map<string, list<set<i32>>>>>>> many_nested;
}
