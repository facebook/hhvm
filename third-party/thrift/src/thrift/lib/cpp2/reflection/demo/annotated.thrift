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

namespace cpp2 static_reflection.demo

include "thrift/annotation/thrift.thrift"

@thrift.DeprecatedUnvalidatedAnnotations{
  items = {
    "description": "example of an annotated enum",
    "purpose": "toy example of enum annotations",
  },
}
enum annotated_enum {
  field0 = 0,
  field1 = 1,
  field2 = 2,
}

@thrift.DeprecatedUnvalidatedAnnotations{
  items = {
    "description": "example of an annotated struct",
    "purpose": "toy example of struct annotations",
  },
}
struct annotated_struct {
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"description": "example of an annotated struct member"},
  }
  1: i32 i32_data;
  2: i16 i16_data;
  3: double double_data;
  @thrift.DeprecatedUnvalidatedAnnotations{
    items = {"description": "example of a fourth annotated struct member"},
  }
  4: string string_data;
}
