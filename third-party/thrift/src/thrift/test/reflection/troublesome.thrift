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

// A set of defintions that require special care in the code gen.
namespace cpp2 apache.thrift.test.reflection

// An enum that has a value with the same name.
enum EnumAndValue {
  EnumAndValue = 1,
}

enum EnumAndUnion_Type {
  foo = 0,
}
union EnumAndUnion {
  1: i32 foo;
}

// Names used in fatal reflection code gen.
enum enums {
  Value = 1,
}
enum type {
  Value = 1,
}
struct name {
  1: i32 foo;
}
union member {
  1: i32 foo;
}
struct members {
  1: i32 foo;
}
union unions {
  1: i32 name;
  2: i32 type;
  3: bool member;
  4: bool members;
}
struct structs {
  1: i32 name;
  2: i32 type;
  3: bool member;
  4: bool members;
}
struct strings {
  1: i32 name;
  2: i32 type;
  3: bool member;
  4: bool members;
}
exception exceptions {
  1: i32 name;
  2: i32 type;
  3: bool member;
  4: bool members;
}
service services {
}
const i32 constants = 1;
const i32 troublesome__unique_strings_list = 2;
const i32 apache_thrift_test_troublesome__unique_strings_list = 2;
