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

enum TestEnum {
  UNKNOWN = 0,
  NEW_VALUE = 1,
}

union TestUnion {
  1: i32 int_value;
  2: string str_value;
  3: double double_value;
  4: list<string> list_of_strings;
  5: map<string, i32> map_of_string_to_ints;
  6: Foo struct_foo;
}

struct Foo {
  1: list<string> a;
  2: optional map<string, list<set<i32>>> b;
  3: i64 c = 7;
  4: optional bool d = 0;
  5: string str_value = "hello";
}

struct TestStruct {
  1: Foo foo_struct = {"c": 99};
  2: TestUnion union_value;
  3: TestStruct struct_of_self;
  4: list<Foo> list_of_struct_foo;
  5: map<string, Foo> map_of_string_to_struct_foo;
  6: list<TestStruct> list_of_struct_self;
}

exception Baz {
  1: string message;
  2: Foo some_field;
  3: set<string> some_container;
  4: i32 code;
}

exception OptBaz {
  1: optional string message;
}

service Bar {
  string baz(
    1: set<i32> a,
    2: list<map<i32, set<string>>> b,
    3: i64 c,
    4: Foo d,
    5: i64 e = 4,
  );
}
