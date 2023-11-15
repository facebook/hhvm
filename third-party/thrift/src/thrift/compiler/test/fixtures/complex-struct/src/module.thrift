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

namespace java.swift test.fixtures.complex_struct

typedef string stringTypedef
typedef i64 longTypeDef
typedef map<i16, string> mapTypedef
typedef list<double> listTypedef
typedef float floatTypedef
typedef map<i32, i64> (
  java.swift.type = "it.unimi.dsi.fastutil.ints.Int2LongArrayMap",
) FMap

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
  MyValue3 = 3,
  MyValue4 = 4,
  MyValue5 = 5,
}

struct MyStructFloatFieldThrowExp {
  1: i64 myLongField;
  2: byte MyByteField;
  3: string myStringField;
  4: float myFloatField;
}

struct MyStructMapFloatThrowExp {
  1: i64 myLongField;
  2: map<i32, list<list<floatTypedef>>> mapListOfFloats;
}
struct MyStruct {
  1: i64 MyIntField;
  2: string MyStringField;
  3: MyDataItem MyDataField;
  4: MyEnum myEnum;
  5: bool MyBoolField;
  6: byte MyByteField;
  7: i16 MyShortField;
  8: i64 MyLongField;
  9: double MyDoubleField;
  10: list<double> lDouble;
  11: list<i16> lShort;
  12: list<i32> lInteger;
  13: list<i64> lLong;
  14: list<string> lString;
  15: list<bool> lBool;
  16: list<byte> lByte;
  17: map<i16, string> mShortString;
  18: map<i32, string> mIntegerString;
  19: map<string, MyStruct> mStringMyStruct;
  20: map<string, bool> mStringBool;
  21: map<i32, i32> mIntegerInteger;
  22: map<i32, bool> mIntegerBool;
  23: set<i16> sShort;
  24: set<MyStruct> sMyStruct;
  25: set<i64> sLong;
  26: set<string> sString;
  27: set<byte> sByte;
  28: map<list<i32>, list<i32>> mListList;
}

struct SimpleStruct {
  1: i64 age = 60;
  2: string name = "Batman";
}

const i32 DEFAULT_PORT_NUM = 3456;

const MyUnion constEnumUnion = {"myEnum": MyEnum.MyValue2};

struct defaultStruct {
  1: i64 myLongDFset = 10; // default value is 10
  2: i64 myLongDF; // default value is 0
  3: i32 portDFset = DEFAULT_PORT_NUM;
  4: i32 portNum;
  5: binary myBinaryDFset = "abc";
  6: binary myBinary;
  7: byte myByteDFSet = 17;
  8: byte myByte;
  9: double myDoubleDFset = 99.7678;
  10: double myDoubleDFZero = 0.0;
  12: double myDouble;
  13: map<i32, string> field3 = {15: 'a_value', 2: 'b_value'};
  14: list<MyEnum> myList = [MyEnum.MyValue1, MyEnum.MyValue1, MyEnum.MyValue2];
  15: set<string> mySet = ["house", "car", "dog"];
  16: SimpleStruct simpleStruct = {"age": 40, "name": "John"};
  17: list<SimpleStruct> listStructDFset = [
    {"age": 40, "name": "IronMan"},
    {"age": 999, "name": "Thanos"},
  ];
  18: MyUnion myUnion = constEnumUnion;
  19: list<MyUnion> listUnionDFset = [
    {"myEnum": MyEnum.MyValue2},
    {"intValue": 123},
  ];
  20: map<i32, list<SimpleStruct>> mapNestlistStructDfSet = {
    1: [{"age": 40, "name": "IronMan"}, {"age": 999, "name": "Thanos"}],
    2: [{"age": 28, "name": "BatMan"}, {"age": 12, "name": "Robin"}],
    5: [{"age": 12, "name": "RatMan"}, {"age": 6, "name": "Catman"}],
  };
  21: map_i64_string_5732 mapJavaTypeDFset = {15: 'a_value', 2: 'b_value'};
  22: map<i64, i32> emptyMap = {};
  23: map<string, map<i32, MyEnum>> enumMapDFset = {
    "SANDY BRIDGE": {16: MyEnum.MyValue1, 144: MyEnum.MyValue1},
    "IVY BRIDGE": {32: MyEnum.MyValue2, 144: MyEnum.MyValue2},
    "HASWELL": {32: MyEnum.MyValue3,128: MyEnum.MyValue3,256: MyEnum.MyValue3,},
  };
}

struct MyStructTypeDef {
  1: i64 myLongField;
  2: longTypeDef myLongTypeDef;
  3: string myStringField;
  4: stringTypedef myStringTypedef;
  5: map<i16, string> myMapField;
  6: mapTypedef myMapTypedef;
  7: list<double> myListField;
  8: listTypedef myListTypedef;
  9: map<i16, list<listTypedef>> myMapListOfTypeDef;
}

struct MyDataItem {}

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
  4: ComplexNestedStruct complexNestedStruct;
  5: i64 longValue;
  6: i32 intValue;
}

union MyUnionFloatFieldThrowExp {
  1: MyEnum myEnum;
  2: list<list<float>> setFloat;
  3: MyDataItem myDataItem;
  4: ComplexNestedStruct complexNestedStruct;
}
struct ComplexNestedStruct {
  // collections of collections
  1: set<set<i32>> setOfSetOfInt;
  2: list<list<list<list<MyEnum>>>> listofListOfListOfListOfEnum;
  3: list<list<MyStruct>> listOfListOfMyStruct;
  4: set<list<list<i64>>> setOfListOfListOfLong;
  5: set<set<set<i64>>> setOfSetOfsetOfLong;
  6: map<i32, list<list<MyStruct>>> mapStructListOfListOfLong;
  7: map<MyStruct, i32> mKeyStructValInt;
  8: list<map<i32, i32>> listOfMapKeyIntValInt;
  9: list<map<string, list<MyStruct>>> listOfMapKeyStrValList;
  // Maps with collections as keys
  10: map<set<i32>, i64> mapKeySetValLong;
  11: map<list<string>, i32> mapKeyListValLong;
  // Map with collections as keys and values
  12: map<map<i32, string>, map<i32, string>> mapKeyMapValMap;
  13: map<set<list<i32>>, map<list<set<string>>, string>> mapKeySetValMap;
  14: map<map<map<i32, string>, string>, map<i32, string>> NestedMaps;
  15: map<i32, list<MyStruct>> mapKeyIntValList;
  16: map<i32, set<bool>> mapKeyIntValSet;
  17: map<set<bool>, MyEnum> mapKeySetValInt;
  18: map<list<i32>, set<map<double, string>>> mapKeyListValSet;
}

struct TypeRemapped {
  1: map_i64_string_5732 lsMap;
  2: map_i32_FMap_6797 ioMap;
  3: i32_1194 BigInteger;
  4: binary_4918 binaryTestBuffer;
}

exception emptyXcep {}

exception reqXcep {
  1: required string message;
  2: required i32 errorCode;
}

exception optXcep {
  1: optional string message;
  2: optional i32 errorCode;
}

exception complexException {
  1: string message;
  2: list<string> listStrings;
  3: MyEnum errorEnum;
  4: optional MyUnion unionError;
  5: MyStruct structError;
  6: map_i64_string_5732 lsMap;
}

// The following were automatically generated and may benefit from renaming.
typedef binary (java.swift.type = "java.nio.ByteBuffer") binary_4918
typedef i32 (java.swift.type = "java.math.BigInteger") i32_1194
typedef map<i32, FMap> (
  java.swift.type = "it.unimi.dsi.fastutil.ints.Int2ObjectArrayMap<it.unimi.dsi.fastutil.ints.Int2LongArrayMap>",
) map_i32_FMap_6797
typedef map<i64, string> (
  java.swift.type = "it.unimi.dsi.fastutil.longs.Long2ObjectArrayMap<String>",
) map_i64_string_5732
