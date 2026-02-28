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

struct A {
  1: string a;
  2: map<string, string> map_of_string_to_string;
}

struct B {
  1: map<string, string> map_of_string_to_string;
  2: map<string, i32> map_of_string_to_i32;
  3: map<string, A> map_of_string_to_A;
  5: map<string, list<A>> map_of_string_to_list_of_A;
  6: map<string, map<string, i32>> map_of_string_to_map_of_string_to_i32;
  7: map<string, map<string, A>> map_of_string_to_map_of_string_to_A;

  8: list<string> list_of_string;
  9: list<map<string, A>> list_of_map_of_string_to_A;
  12: list<map<string, list<A>>> list_of_map_of_string_to_list_of_A;
}

struct C {
  1: optional map<string, string> map_of_string_to_string;
  2: optional map<string, i32> map_of_string_to_i32;
  3: optional map<string, A> map_of_string_to_A;
  5: optional map<string, list<A>> map_of_string_to_list_of_A;
  6: optional map<
    string,
    map<string, i32>
  > map_of_string_to_map_of_string_to_i32;
  7: optional map<string, map<string, A>> map_of_string_to_map_of_string_to_A;

  8: optional list<string> list_of_string;
  9: optional list<map<string, A>> list_of_map_of_string_to_A;
  12: optional list<map<string, list<A>>> list_of_map_of_string_to_list_of_A;
}
