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

namespace py3 ""

struct Old {
  1: string unqualified_to_unqualified;
  2: string unqualified_to_optional;
  3: string unqualified_to_required;
  4: string unqualified_old;
  5: string unqualified_removed;

  11: optional string optional_to_unqualified;
  12: optional string optional_to_optional;
  13: optional string optional_to_required;
  14: optional string optional_old;
  15: optional string optional_removed;

  21: required string required_to_unqualified;
  22: required string required_to_optional;
  23: required string required_to_required;
  24: required string required_old;
  25: required string required_removed;
}

struct New {
  1: string unqualified_to_unqualified;
  2: optional string unqualified_to_optional;
  3: required string unqualified_to_required;
  4: string unqualified_new;
  // 5: string unqualified_removed
  6: string unqualified_added;

  11: string optional_to_unqualified;
  12: optional string optional_to_optional;
  13: required string optional_to_required;
  14: optional string optional_new;
  // 15: optional string optional_removed
  16: optional string optional_added;

  21: string required_to_unqualified;
  22: optional string required_to_optional;
  23: required string required_to_required;
  24: required string required_new;
  // 25: required string required_removed
  26: required string required_added;
}

union MyUnion_V1 {
  1: i32 i32_field;
}

struct MyStruct_V1 {
  1: i32 i32_field;
  2: MyUnion_V1 union_field;
}

union MyUnion_V2 {
  1: i32 i32_field;
  2: string string_field;
}

struct MyStruct_V2 {
  1: i32 i32_field;
  2: MyUnion_V2 union_field;
}
