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
include "thrift/annotation/compat.thrift"

package "test.dev/thrift/lib/java/test/thrift/any"

namespace java.swift com.facebook.thrift.test.thrift.any

@java.Adapter{
  adapterClassName = "com.facebook.thrift.any.AnyAdapter",
  typeClassName = "com.facebook.thrift.any.Any",
}
typedef any.Any Any

@java.Adapter{
  adapterClassName = "com.facebook.thrift.any.SemiAnyAdapter",
  typeClassName = "com.facebook.thrift.any.SemiAny",
}
typedef any.SemiAny SemiAny

struct TestStruct {
  1: i32 inf_field;
  2: bool bool_field;
  3: list<i32> list_field;
  10: Any any_field;
  11: SemiAny semiany_field;
}

safe permanent client exception TestException {
  1: i32 inf_field;
  2: bool bool_field;
  10: Any any_field;
  11: SemiAny semiany_field;
}

union TestUnion {
  1: i32 inf_field;
  2: bool bool_field;
  10: Any any_field;
  11: SemiAny semiany_field;
}

enum TestEnum {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}

@compat.Enums
enum TestOpenEnum {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}

struct TestShortUriStruct {
  1: i32 inf_field;
} (thrift.uri = "a.b/c/d")

struct Position {
  1: i32 x;
  2: i32 y;
}

struct Rectangle {
  1: i32 color;
  2: i32 len;
  3: i32 width;
  4: Position position;
  5: Any canvas;
}

struct Image {
  1: binary jpg;
}

struct SolidColor {
  1: i32 color;
}
