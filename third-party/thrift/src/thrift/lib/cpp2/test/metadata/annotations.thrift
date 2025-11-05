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

package "apache.org/thrift/test"

struct Bar {
  1: string baz;
}

struct Foo {
  1: Bar bar;
}

struct MyStruct {
  1: string stringField;
}

struct MyUnion {
  1: i32 i32Field;
  2: string stringField;
}

enum MyEnum {
  first = 1,
  second = 2,
}

typedef set<i32> SetOfI32

struct Annotation {
  1: bool boolField;
  2: i16 i16Field;
  3: i32 i32Field;
  4: float floatField;
  5: double doubleField;
  6: binary binaryField;
  7: MyStruct structField;
  8: MyUnion unionField;
  9: MyEnum enumField;
  10: list<i32> listField;
  11: SetOfI32 setField;
  12: map<i32, string> mapField;
}

typedef Foo FooTypedef

@Annotation{
  boolField = true,
  i16Field = 16,
  i32Field = 32,
  floatField = 10.0,
  doubleField = 20.0,
  binaryField = "binary",
  structField = MyStruct{stringField = "struct"},
  unionField = MyUnion{stringField = "union"},
  enumField = MyEnum.second,
  listField = [2, 1, 2],
  setField = [2, 1],
  mapField = {2: "20", 1: "10"},
}
@FooTypedef{bar = Bar{baz = "123"}}
enum TestEnum {
  foo = 1,
  bar = 2,
}

@Annotation{
  boolField = true,
  i16Field = 16,
  i32Field = 32,
  floatField = 10.0,
  doubleField = 20.0,
  binaryField = "binary",
  structField = MyStruct{stringField = "struct"},
  unionField = MyUnion{stringField = "union"},
  enumField = MyEnum.second,
  listField = [2, 1, 2],
  setField = [2, 1],
  mapField = {2: "20", 1: "10"},
}
@Foo{bar = Bar{baz = "123"}}
struct TestStruct {
  1: string field_1;

  @Annotation{
    boolField = true,
    i16Field = 16,
    i32Field = 32,
    floatField = 10.0,
    doubleField = 20.0,
    binaryField = "binary",
    structField = MyStruct{stringField = "struct"},
    unionField = MyUnion{stringField = "union"},
    enumField = MyEnum.second,
    listField = [2, 1, 2],
    setField = [2, 1],
    mapField = {2: "20", 1: "10"},
  }
  @Foo{bar = Bar{baz = "123"}}
  2: i32 field_2;
}

@Annotation{
  boolField = true,
  i16Field = 16,
  i32Field = 32,
  floatField = 10.0,
  doubleField = 20.0,
  binaryField = "binary",
  structField = MyStruct{stringField = "struct"},
  unionField = MyUnion{stringField = "union"},
  enumField = MyEnum.second,
  listField = [2, 1, 2],
  setField = [2, 1],
  mapField = {2: "20", 1: "10"},
}
@Foo{bar = Bar{baz = "123"}}
exception TestException {
  1: string field_1;
  2: i32 field_2;
}
