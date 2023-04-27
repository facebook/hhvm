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

namespace hack test.fixtures.jsenum

include "thrift/annotation/hack.thrift"

enum MyThriftEnum {
  foo = 1,
  bar = 2,
  baz = 3,
} (hack.attributes = "ApiEnum, JSEnum")

struct MyThriftStruct {
  1: string foo (hack.attributes = "FieldAttribute");
  2: string bar;
  3: string baz;
} (hack.attributes = "ClassAttribute")

struct MySecondThriftStruct {
  1: MyThriftEnum foo;
  2: MyThriftStruct bar;
  3: i64 baz;
}

@hack.Attributes{attributes = ["ApiEnum", "JSEnum"]}
struct MyThirdThriftStruct {
  @hack.Attributes{attributes = ["FieldAttribute"]}
  1: i32 foo;
}

union UnionTesting {
  1: string foo;
  3: i64 bar;
} (hack.union_enum_attributes = "EnumAttributes")

@hack.UnionEnumAttributes{attributes = ["EnumAttributes", "EnumAttributes2"]}
union UnionTestingStructured {
  1: string foo;
  3: i64 bar;
}
