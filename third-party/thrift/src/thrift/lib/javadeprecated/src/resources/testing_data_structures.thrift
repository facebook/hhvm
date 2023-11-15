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

namespace java com.facebook.thrift.java.test
namespace java.swift com.facebook.thrift.javaswift.test
namespace android com.facebook.thrift.android.test

include "thrift/annotation/java.thrift"

typedef map<i32, i64> (
  java.swift.type = "it.unimi.dsi.fastutil.ints.Int2LongArrayMap",
) FMap

struct MySimpleStruct {
  1: i64 id = 99;
  2: string name = "Batman";
}

struct MySensitiveStruct {
  1: i64 id;
  2: string password (java.sensitive);
}

@java.Mutable
struct MySimpleBean {
  1: i64 id;
  2: string name;
}

union MySimpleUnion {
  1: i64 caseOne;
  2: i64 caseTwo;
  3: string caseThree;
  4: MySimpleStruct caseFour;
  5: list<string> caseFive;
  6: SmallEnum smallEnum;
}

struct NestedStruct {
  1: map<i32, string> myMap;
  2: MySimpleStruct mySimpleStruct;
  3: set<i32> mySet;
  4: list<string> myList;
  5: MySimpleUnion myUnion;
}

struct ComplexNestedStruct {
  1: set<set<i32>> setOfSetOfInt;
  2: list<list<SmallEnum>> listOfListOfEnum;
  3: list<list<MySimpleStruct>> listOfListOfMyStruct;
  4: set<list<list<string>>> setOfListOfListOfString;
  5: map<i32, list<list<SmallEnum>>> mapKeyIntValListOfListOfEnum;
  6: map<map<i32, string>, map<string, string>> mapKeyMapValMap;
  7: MySimpleUnion myUnion;
}

struct SimpleStructTypes {
  1: string msg = "Bye Thrift Team";
  2: bool b = false;
  3: byte y = 97;
  4: i16 i = 1;
  5: i32 j = -9999;
  6: i64 k = 14444444444444;
  7: double d = 14;
}

struct SimpleCollectionStruct {
  1: list<double> lDouble;
  2: list<i16> lShort;
  3: map<i32, string> mIntegerString;
  4: map<string, string> mStringString;
  5: set<i64> sLong;
}

const i32 DEFAULT_PORT_NUM = 3456;

enum SmallEnum {
  UNKNOWN = 0, // default value
  RED = 1,
  BLUE = 2,
  GREEN = 3,
}

enum BigEnum {
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
  TWENTY = 20,
}

const MySimpleUnion constEnumUnion = {"smallEnum": SmallEnum.BLUE};

struct defaultValueStruct {
  1: i64 myLongDFset = 10; // default value is 10L
  2: i64 myLongDF; // default value is 0
  3: i32 portDFset = DEFAULT_PORT_NUM;
  4: i32 portNum;
  5: binary myBinaryDFset = "abc";
  6: binary myBinary;
  7: byte myByteDFSet = 17;
  8: byte myByte;
  9: double myDoubleDFset = 99.7678;
  10: double myDoubleDFZero = 0.0;
  11: double myDouble;
  12: map<i32, string> mIntegerString = {15: 'a_value', 2: 'b_value'};
  13: list<SmallEnum> myList = [SmallEnum.RED, SmallEnum.BLUE, SmallEnum.GREEN];
  14: set<string> mySet = ["house", "car", "dog"];
  15: MySimpleStruct simpleStruct = {"id": 40, "name": "John"};

  16: list<MySimpleStruct> listStructDFset = [
    {"id": 40, "name": "IronMan"},
    {"id": 999, "name": "Thanos"},
  ];
  17: MySimpleUnion myUnion = constEnumUnion;

  18: list<MySimpleUnion> listUnionDFset = [
    {"smallEnum": SmallEnum.BLUE},
    {"caseTwo": 123},
  ];
  19: map<i32, list<MySimpleStruct>> mapNestlistStructDfSet = {
    1: [{"id": 40, "name": "IronMan"}, {"id": 999, "name": "Thanos"}],
    2: [{"id": 28, "name": "BatMan"}, {"id": 12, "name": "Robin"}],
    5: [{"id": 12, "name": "RatMan"}, {"id": 6, "name": "Catman"}],
  };
  20: map<i64, string> mapJavaTypeDFset = {15: 'a_value', 2: 'b_value'};
  21: map<i64, i32> emptyMap = {};
  22: map<string, map<i32, SmallEnum>> enumMapDFset = {
    "SANDY BRIDGE": {16: SmallEnum.RED, 144: SmallEnum.RED},
    "IVY BRIDGE": {32: SmallEnum.GREEN, 144: SmallEnum.BLUE},
    "HASWELL": {32: SmallEnum.RED, 128: SmallEnum.BLUE, 256: SmallEnum.GREEN},
  };
}

struct MyListStruct {
  1: list<i64> ids;
}

struct MySetStruct {
  1: set<i64> ids;
}

struct MyMapStruct {
  1: map<i64, string> mapping;
}

struct MyStringStruct {
  1: string aLongString;
}

typedef i64 PersonID

struct MyOptioalStruct {
  1: PersonID id;
  2: string name;
  3: optional i16 ageShortOptional;
  4: i16 ageShort;
  5: optional i64 ageLongOptional;
  6: i64 ageLong;
  7: optional MySimpleStruct mySimpleStructOptional;
  8: MySimpleStruct mySimpleStruct;
  9: optional map<i32, string> mIntegerStringOptional;
  10: map<i32, string> mIntegerString;
  11: optional SmallEnum smallEnumOptional;
  12: SmallEnum mySmallEnum;
}

struct TypeRemapped {
  1: map_i64_string_6611 lsMap;
  2: map_i32_FMap_3815 ioMap;
  4: binary_9731 byteBufferForBinary;
  5: binary byteArrayForBinary;
  6: list<FMap> myListOfFMaps;
}

struct StructWithOptional {
  1: i64 id;
  2: string name;
  3: optional i32 id2;
  4: optional string name2;
}

struct StructWithAllTypes {
  1: optional bool bb;
  2: byte b;
  3: i16 s;
  4: i32 i;
  5: optional i64 l;
  6: float f;
  7: double d;
  8: string myString;
  9: binary bin;
  10: list<i32> intList;
  11: set<i32> intSet;
  12: map<i32, i32> intMap;
  13: SmallEnum myEnum;
  14: MySimpleStruct myStruct;
  15: MySimpleUnion myUnion;
}

@java.Mutable
struct StructMutable {
  1: i16 myInt16 = 42;
  2: i32 myInt32 = 422;
  3: i64 myInt64 = 422222222;
  4: string myString = "foo";
  5: bool myBool = true;
  6: double myDouble = 42.42;
  7: set<string> mySet = ["foo", "bar", "baz"];
}

struct StructWithMaps {
  1: map<string, string> stringstrings;
  2: map<bool, string> boolstrings;
  3: map<string, bool> stringbools;
  4: map<i32, string> intstrings;
  5: map<string, i32> stringints;
}

struct EveryLayout {
  1: bool aBool;
  2: i32 aInt;
  3: i64 aLong;
  5: string aString;
  6: double aDouble;
  7: float aFloat;
  8: i16 aShort;
  9: byte aByte;
  10: list<string> aList;
  11: set<string> aSet;
  12: map<i32, string> aMap;
  13: list<list<string>> aListOfLists;
  14: set<set<string>> aSetOfSets;
  15: map<i32, list<i32>> aMapOfLists;
  16: list<set<map<bool, string>>> listOfSetsOfMap;
  17: binary blob;
} (android.generate_builder = "true")

struct Nesting {
  1: EveryLayout every;
  2: map<string, bool> boolMap;
  3: map<string, EveryLayout> structMap;
}

struct BoolStruct {
  1: bool aBool;
}

struct StringAndList {
  1: string myString;
  2: list<i32> myInts;
}

// The following were automatically generated and may benefit from renaming.
typedef binary (java.swift.type = "java.nio.ByteBuffer") binary_9731
typedef map<i32, FMap> (
  java.swift.type = "it.unimi.dsi.fastutil.ints.Int2ObjectArrayMap<it.unimi.dsi.fastutil.ints.Int2LongArrayMap>",
) map_i32_FMap_3815
typedef map<i64, string> (
  java.swift.type = "it.unimi.dsi.fastutil.longs.Long2ObjectArrayMap<String>",
) map_i64_string_6611

