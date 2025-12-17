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

package "facebook.com/thrift/test/fixtures/basic_enum"

namespace java.swift test.fixtures.swift.enumstrict
namespace java test.fixtures.enumstrict
namespace cpp2 test.fixtures.enumstrict
namespace py3 test.fixtures.enumstrict
include "thrift/annotation/java.thrift"

enum EmptyEnum {
}

enum MyEnum {
  ONE = 1,
  TWO = 2,
}

@java.UseIntrinsicDefault
enum MyUseIntrinsicDefaultEnum {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}

enum MyBigEnum {
  UNKNOWN = 0,
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  SIX = 6,
  SEVEN = 7,
  EIGHT = 8,
  NINE = 9,
  TEN = 10,
  ELEVEN = 11,
  TWELVE = 12,
  THIRTEEN = 13,
  FOURTEEN = 14,
  FIFTEEN = 15,
  SIXTEEN = 16,
  SEVENTEEN = 17,
  EIGHTEEN = 18,
  NINETEEN = 19,
}

const MyEnum kOne = MyEnum.ONE;

const map<MyEnum, string> enumNames = {MyEnum.ONE: "one", MyEnum.TWO: "two"};

struct MyStruct {
  1: MyEnum myEnum;
  2: MyBigEnum myBigEnum = MyBigEnum.ONE;
}
