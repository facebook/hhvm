/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

struct Foo {
  1: i32 just_int;
  2: list<string> list_of_strings;
  3: set<i32> set_of_ints;
  4: map<string, list<string>> map_of_list_of_strings;
  5: map<string, set<string>> map_of_set_of_strings;
  6: map<string, map<string, i32>> map_of_strings_to_map_of_string_ints;
  7: optional map<i32, map<i32, set<string>>> optional_map_of_map_of_sets;
}
