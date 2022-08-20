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

@thrift.NoLegacy
package "facebook.com/testing/example"

namespace py3 testing
namespace cpp2 testing

typedef set<i32> SetI32

enum Color {
  red = 0,
  blue = 1,
  green = 2,
}
exception SimpleError {
  1: string errortext;
  2: i32 retcode;
}

exception ErrorWithEnum {
  1: Color color;
  2: i32 retcode;
}

exception ErrorWithMessageAnnotation {
  1: string errortext;
  2: i16 retcode;
} (message = "errortext")

struct TestStruct {
  1: string field1;
  2: optional string field2;
  3: Color field3;
  4: optional Color field4;
}

struct TestStructWithList {
  1: list<i32> numbers;
}

struct TestStructWithSet {
  1: SetI32 numbers;
}

struct TestNestedStruct {
  1: TestStructWithList nested;
}

struct TestStructSimple {
  1: string field1;
  2: i32 field2;
}

struct TestStructWithMixin {
  1: string field3;
  2: TestStructSimple field4 (cpp.mixin);
}

struct TestStructWithBoxAnnotation {
  @thrift.Box
  1: optional list<i32> data;
}

struct TestStructWithRefAnnotation {
  1: optional list<i32> data (cpp2.ref = "true");
}

union TestUnion {
  1: byte tiny;
  2: i16 small;
  3: i32 medium;
  4: i64 large;
  5: string strval;
  // Thrift-py3 does not support constructors with cpp ref field arguments
  // Use deserialization in order to create such a unions
  6: list<i32> dataptr (cpp2.ref_type = "shared");
}

service TestService {
  string getName();
  bool invert(1: bool value);
  list<i32> processCollection(
    1: list<i32> values,
    2: i32 addValue,
    3: bool doThrow,
  ) throws (1: ErrorWithEnum e);
  bool renamedMethod(1: bool ret) (cpp.name = "renamedMethodInCpp");
} (fun_times = "yes")
