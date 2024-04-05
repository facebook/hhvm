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

namespace py3 thrift.test.thrift_python

struct TestStruct {
  1: string unqualified_string;
  2: optional string optional_string;
}

struct TestStructWithDefaultValues {
  1: i32 unqualified_integer = 42;

  2: optional i32 optional_integer = 43;

  3: TestStruct unqualified_struct = TestStruct{unqualified_string = "hello"};

  4: optional TestStruct optional_struct = TestStruct{
    unqualified_string = "world",
  };

  5: TestStruct unqualified_struct_intrinsic_default;

  6: optional TestStruct optional_struct_intrinsic_default;
}
