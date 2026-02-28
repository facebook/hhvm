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

namespace java.swift test.fixtures.basic_swift_bean

include "thrift/annotation/java.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
  DOMAIN = 2,
}

struct MyStruct1 {
  1: i64 MyIntField;
  2: string MyStringField;
  # use the type before it is defined. Thrift should be able to handle this
  3: MyDataItem MyDataField;
  # glibc has macros with this name, Thrift should be able to prevent collisions
  @cpp.Name{value = "majorVer"}
  4: i64 major;
}

@java.Mutable
struct MyStruct2 {
  1: MyStruct1 myStruct1;
  2: string myString;
}

@java.Mutable
struct MyStruct3 {
  1: i16 myInt16 = 42;
  2: i32 myInt32 = 422;
  3: i64 myInt64 = 422222222;
  4: string myString = "foo";
  5: bool myBool = true;
  6: double myDouble = 42.42;
  7: set<string> mySet = ["foo", "bar", "baz"];
  8: MyDataItem MyDataItem = {"field1": 29, "field2": 30};
  9: list<MyDataItem> myList = [
    {"field1": 29, "field2": 30},
    {"field1": 31, "field2": 32},
  ];
  10: map<i32, list<MyDataItem>> myMapList = {
    1: [{"field1": 29, "field2": 30}, {"field1": 31, "field2": 32}],
    2: [{"field1": 33, "field2": 34}, {"field1": 35, "field2": 36}],
  };
  11: list<MyDataItem> myEmptyList = [];
  12: map<i32, list<MyDataItem>> myEmptyMapList = {};
}

@java.Mutable
struct MyDataItem {
  1: i32 field1;
  2: i32 field2;
}

@java.Mutable
struct LegacyStruct {
  1: i32 normal;
  -1: i32 bad;
}

const MyStruct1 ms = {
  "MyIntField": 42,
  "MyStringField": "Meaning_of_life",
  "MyDataField": {"field1": 1, "field2": 2},
  "major": 32,
};

service legacy_service {
  map<string, list<i32>> getPoints(1: set<string> key, -1: i64 legacyStuff);
}
