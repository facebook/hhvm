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

typedef MyEnum MyEnumTypedef

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
  12: map<MyEnumTypedef, string> mapField;
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
  mapField = {MyEnum.second: "20", MyEnum.first: "10"},
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
  mapField = {MyEnum.second: "20", MyEnum.first: "10"},
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
    mapField = {MyEnum.second: "20", MyEnum.first: "10"},
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
  mapField = {MyEnum.second: "20", MyEnum.first: "10"},
}
@Foo{bar = Bar{baz = "123"}}
exception TestException {
  1: string field_1;
  2: i32 field_2;
}

@Bar{baz = "0"}
service TestService {
  @Bar{baz = "1"}
  TestStruct foo(
    @Bar{baz = "2"}
    1: i32 input,
  ) throws (
    @Bar{baz = "3"}
    1: TestException ex,
  );
}

@Annotation{floatField = 1.0}
struct TestFloat1 {}

// Delta < (1 / (2^26)), which can't be stored in float that only has 24 bits precision.
@Annotation{floatField = 1.00000001}
struct TestFloat2 {}

@Annotation{floatField = 1.0000001}
struct TestFloat3 {}
