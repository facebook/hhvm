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

namespace java.swift test.fixtures.shapes
namespace hack test.fixtures

enum Enum {
  ENUM = 1,
}

union Union {
  1: i64 intValue;
  5: string stringValue;
}

struct A {
  1: string a;
}

struct B {
  1: A just_an_A;
  2: set<i32> set_of_i32;
  3: list<i32> list_of_i32;
  4: list<string> list_of_string;
  5: map<string, i32> map_of_string_to_i32;
  6: map<string, A> map_of_string_to_A;
  7: map<string, list<i32>> map_of_string_to_list_of_i32;
  8: map<string, list<A>> map_of_string_to_list_of_A;
  9: map<string, set<i32>> map_of_string_to_set_of_i32;
  10: map<string, map<string, i32>> map_of_string_to_map_of_string_to_i32;
  11: map<string, map<string, A>> map_of_string_to_map_of_string_to_A;
  12: list<set<i32>> list_of_set_of_i32;
  13: list<map<string, list<A>>> list_of_map_of_string_to_list_of_A;
  14: list<map<string, A>> list_of_map_of_string_to_A;
  17: Enum just_an_enum;
  18: Union just_a_union;

  51: optional A optional_just_an_A;
  52: optional set<i32> optional_set_of_i32;
  53: optional list<i32> optional_list_of_i32;
  54: optional list<string> optional_list_of_string;
  55: optional map<string, i32> optional_map_of_string_to_i32;
  56: optional map<string, A> optional_map_of_string_to_A;
  57: optional map<string, list<i32>> optional_map_of_string_to_list_of_i32;
  58: optional map<string, list<A>> optional_map_of_string_to_list_of_A;
  59: optional map<string, set<i32>> optional_map_of_string_to_set_of_i32;
  60: optional Enum optional_enum;

  70: required Enum required_enum_with_default = ENUM;

  80: string string_with_default_value = "de_fault_in_our_stars";
  81: i32 i32_with_default_value = 1729;
  82: double double_with_default_value = 3.1415926;
  83: Enum enum_with_default_value = ENUM;
  84: A A_with_default_value = {"a": "bc"};
  85: set<i32> set_of_i32_with_default_value = [3, 7, 31, 127];
  86: map<i32, string> map_of_i32_to_string_with_default_value = {
    1: "fish",
    2: "fish",
  };
  87: list<string> list_of_string_with_default_value = [
    "red fish",
    "blue fish",
  ];
  88: map<string, list<i32>> map_of_string_to_list_of_i32_with_default_value = {
    "Jenny": [8, 6, 7, 5, 3, 0, 9],
  };
}
