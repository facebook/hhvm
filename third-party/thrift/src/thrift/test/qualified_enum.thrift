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
include "thrift/annotation/cpp.thrift"
include "thrift/test/enum.thrift"

@thrift.AllowLegacyMissingUris
package;

enum MyQualifiedEnum {
  FOO = 0,
  BAR = 1,
}
const MyQualifiedEnum FOO = BAR;

struct MyQualifiedStruct {
  1: MyQualifiedEnum field1 = FOO;
  2: MyQualifiedEnum field2 = MyQualifiedEnum.FOO;
  3: enum.MyEnum1 field3 = enum.ME1_1;
  4: enum.MyEnum1 field4 = enum.MyEnum1.ME1_1;
  5: enum.MyEnum4 field5 = enum.c_me4_a;
}

@thrift.BitmaskEnum
enum MyBitMaskEnum {
  Nil = 0,
  Foo = 1,
  Bar = 2,
  Baz = 4,
}

@cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
@thrift.BitmaskEnum
enum MyBitMaskEnumShort {
  Nil = 0,
  Foo = 1,
  Bar = 2,
  Baz = 4,
}
