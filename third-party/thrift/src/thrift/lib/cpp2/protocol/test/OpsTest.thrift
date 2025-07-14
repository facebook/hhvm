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

include "thrift/lib/thrift/any.thrift"

package "facebook.com/thrift/test"

struct NestedStruct {
  1: i32 int_field;
  2: string string_field;
}

struct Struct {
  1: bool bool_field;
  2: i32 int_field;
  3: NestedStruct struct_field;
  4: map<i32, NestedStruct> int_map_field;
  5: map<string, bool> string_map_field;
  6: any.Any any_field;
}
