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

enum Enum {
  ENUM = 1,
}

struct A {
  1: Enum just_an_enum;
  2: string a;
}

struct B {
  1: map<string, string> map_of_string_to_string;
  2: map<string, i32> map_of_string_to_i32;
  3: map<string, A> map_of_string_to_A;
  5: map<string, list<A>> map_of_string_to_list_of_A;
  6: map<string, map<string, i32>> map_of_string_to_map_of_string_to_i32;
  7: map<string, map<string, A>> map_of_string_to_map_of_string_to_A;
  8: map<string, list<i32>> map_of_string_to_list_of_i32;
  9: map<string, set<i32>> map_of_string_to_set_of_i32;

  10: list<string> list_of_string;
  11: list<i32> list_of_i32;
  14: list<set<i32>> list_of_set_of_i32;
  15: list<map<string, list<A>>> list_of_map_of_string_to_list_of_A;
  16: list<map<string, A>> list_of_map_of_string_to_A;

  17: set<i32> set_of_i32;
  18: set<string> set_of_string;
}

struct C {
  1: optional A just_an_A;
  2: optional Enum just_an_enum;
  4: optional map<string, string> map_of_string_to_string;
  5: optional map<string, i32> map_of_string_to_i32;
  6: optional map<string, A> map_of_string_to_A;
  7: optional map<string, B> map_of_string_to_self;
  8: optional map<string, list<A>> map_of_string_to_list_of_A;
  9: optional map<
    string,
    map<string, i32>
  > map_of_string_to_map_of_string_to_i32;
  10: optional map<string, map<string, A>> map_of_string_to_map_of_string_to_A;
  11: optional map<string, list<i32>> map_of_string_to_list_of_i32;
  12: optional map<string, set<i32>> map_of_string_to_set_of_i32;

  13: optional list<string> list_of_string;
  14: optional list<map<string, A>> list_of_map_of_string_to_A;
  17: optional list<map<string, list<A>>> list_of_map_of_string_to_list_of_A;
  18: optional list<i32> list_of_i32;
  19: optional map<string, list<string>> map_of_string_to_list_of_string;

  20: optional set<i32> set_of_i32;
  21: optional set<string> set_of_string;
}
