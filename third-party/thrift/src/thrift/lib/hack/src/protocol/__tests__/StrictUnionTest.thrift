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

package "meta.com/thrift/core/protocol/_tests_/strict_union_test"

namespace hack ""

union SerializerTestStrictUnion {
  1: i32 int_value;
  2: string str_value;
  3: list<string> list_of_strings;
  4: set<string> set_of_strings;
  5: map<i32, string> map_of_int_to_strings;
}
