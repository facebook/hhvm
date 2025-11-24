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
include "included_enum.thrift"

package "test.dev/fixtures/enums"

namespace android test.fixtures.enums
namespace java test.fixtures.enums
namespace java.swift test.fixtures.enums

enum Metasyntactic {
  FOO = 1,
  BAR = 2,
  BAZ = 3,
  BAX = 4,
}

struct SomeStruct {
  1: Metasyntactic reasonable = FOO;
  2: Metasyntactic fine = 2;
  3: Metasyntactic questionable = -1;
  4: set<i32> tags = [];
}

enum MyEnum1 {
  ME1_0 = 0,
  ME1_1 = 1,
  ME1_2 = 2,
  ME1_3 = 3,
  ME1_5 = 5,
  ME1_6 = 6,
}

enum MyEnum2 {
  ME2_0 = 0,
  ME2_1 = 1,
  ME2_2 = 2,
}

enum MyEnum3 {
  ME3_0 = 0,
  ME3_1 = 1,
  ME3_N2 = -2,
  ME3_N1 = -1,
  ME3_9 = 9,
  ME3_10 = 10,
}

enum MyEnum4 {
  ME4_A = 0x7ffffffd,
  ME4_B = 0x7ffffffe,
  ME4_C = 0x7fffffff,
  ME4_D = -2147483648,
}

struct MyStruct {
  1: MyEnum2 me2_3 = 3;
  2: MyEnum3 me3_n3 = -3;
  4: MyEnum1 me1_t1 = 1;
  6: MyEnum1 me1_t2 = MyEnum1.ME1_1;
}

@thrift.BitmaskEnum
enum MyBitmaskEnum1 {
  ONE = 1,
  TWO = 2,
  FOUR = 4,
}

@thrift.BitmaskEnum
enum MyBitmaskEnum2 {
  ONE = 1,
  TWO = 2,
  FOUR = 4,
}

typedef included_enum.IncludedEnum IncludedEnumAlias
