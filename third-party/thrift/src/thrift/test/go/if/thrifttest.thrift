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

namespace go thrift.test.go.if.thrifttest

include "thrift/annotation/go.thrift"
include "thrift/annotation/thrift.thrift"

enum Numberz {
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FIVE = 5,
  SIX = 6,
  EIGHT = 8,
}

union Unionz {
  1: i64 fieldOne;
  2: string fieldTwo;
}

typedef i64 UserId
typedef Xtruct XtructDef
typedef Unionz UnionzDef

@thrift.ReserveIds{ids = [12345]}
struct Bonk {
  1: string message;
  2: i32 type;
}

struct Bools {
  1: bool im_true;
  2: bool im_false;
}

struct WeirdNames {
  1: bool me;
  2: bool set_me;
  // commented intentionally
  // golang generator does not support this combination of ambiguous names
  // 3: bool SetMe;
  4: bool SetMe_;
  @go.Name{name = "XSetMe"}
  5: bool _setMe;
  6: bool p;
  7: bool b;
}

struct Xtruct {
  1: string string_thing;
  4: byte byte_thing;
  9: i32 i32_thing;
  11: i64 i64_thing;
}

struct Xtruct2 {
  1: byte byte_thing;
  2: Xtruct struct_thing;
  3: i32 i32_thing;
}

struct Xtruct3 {
  1: string string_thing;
  4: i32 changed;
  9: i32 i32_thing;
  11: i64 i64_thing;
}

struct Xtruct4 {
  1: string string_thing;
  2: i32 int_thing = 42;
  3: list<i32> list_int32_thing = [5];
  4: Xtruct2 xtruct2;
  5: optional i64 opt64;
}

struct Insanity {
  1: map<Numberz, UserId> userMap;
  2: list<Xtruct> xtructs;
  3: map<string, string> str2str;
}

struct CrazyNesting {
  1: string string_field;
  2: optional set<Insanity> set_field;
}

struct CyclicStruct {
  1: CyclicStruct self;
  2: optional CyclicStruct optional_self;
}

exception Xception {
  1: i32 errorCode;
  2: string message;
}

exception Xception2 {
  1: i32 errorCode;
  2: Xtruct struct_thing;
}

struct EmptyStruct {}

struct OneField {
  1: EmptyStruct field;
}

service ThriftTest {
  void doTestVoid();
  string doTestString(1: string thing);
  byte doTestByte(1: byte thing);
  i32 doTestI32(1: i32 thing);
  i64 doTestI64(1: i64 thing);
  double doTestDouble(1: double thing);
  float doTestFloat(1: float thing);
  Xtruct doTestStruct(1: Xtruct thing);
  Xtruct2 doTestNest(1: Xtruct2 thing);
  map<i32, i32> doTestMap(1: map<i32, i32> thing);
  set<i32> doTestSet(1: set<i32> thing);
  list<i32> doTestList(1: list<i32> thing);
  Numberz doTestEnum(1: Numberz thing);
  UserId doTestTypedef(1: UserId thing);

  map<i32, map<i32, i32>> doTestMapMap(1: i32 hello);

  /* So you think you've got this all worked, out eh? */
  map<UserId, map<Numberz, Insanity>> doTestInsanity(1: Insanity argument);

  /* Multiple parameters */
  Xtruct doTestMulti(
    1: byte arg0,
    2: i32 arg1,
    3: i64 arg2,
    4: map<i16, string> arg3,
    5: Numberz arg4,
    6: UserId arg5,
  );

  /* Exception specifier */

  void doTestException(1: string arg) throws (1: Xception err1);

  /* Multiple exceptions specifier */

  Xtruct doTestMultiException(1: string arg0, 2: string arg1) throws (
    1: Xception err1,
    2: Xception2 err2,
  );

  /* Test oneway void */
  oneway void doTestOneway(1: i32 secondsToSleep);

  /* Test poor naming */
  @go.Name{name = "XDoTestPoorName"}
  void _doTestPoorName();
}

struct VersioningTestV1 {
  1: i32 begin_in_both;
  3: string old_string;
  14: i32 end_in_both;
}

struct VersioningTestV2 {
  1: i32 begin_in_both;
  2: i32 newint;
  3: byte newbyte;
  4: i16 newshort;
  5: i64 newlong;
  6: double newdouble;
  7: Bonk newstruct;
  8: list<i32> newlist;
  9: set<i32> newset;
  10: map<i32, i32> newmap;
  11: string newunicodestring;
  12: string newstring;
  13: bool newbool;
  14: i32 end_in_both;
}

struct ListTypeVersioningV1 {
  1: list<i32> myints;
  2: string hello;
}

struct ListTypeVersioningV2 {
  1: list<string> strings;
  2: string hello;
}

struct SimpleJSONTestStruct {
  1: map<i32, VersioningTestV1> m;
}

struct MapKey {
  1: i64 num;
  2: string strval;
}

struct Maps {
  1: map<string, string> str2str;
  2: map<string, list<string>> str2list;
  3: map<string, map<string, string>> str2map;
  4: map<string, Insanity> str2struct;
  5: map<MapKey, string> struct2str;
}

struct WithAnnotations {
  1: map<string, string> m1 (test.annotation = "none", test.hidden);
  2: i32 m2;
  3: string s1 (test.hidden = 3);
} (
  test.struct_annotation = "ok",
  test.partial,
  test.complex = "
   public:

    bool empty() const {
      return !(__isset.m1 ||
               __isset.m2 ||
               __isset.s1);
    }
  ",
)

// Test against failures where code is genereted differently when a struct
// is used before it is defined in the thrift spec.
struct PreDefinition {
  1: map<string, PreDefStruct> m;
}

struct PreDefStruct {
  1: string data;
}

struct PackageCollisionsTest {
  1: string thrift;
  2: string context;
  3: string fmt;
  4: string strings;
  5: string sync;
  6: string metadata;
  7: string maps;
}

struct StructWithManyFields {
  843: i64 field01;
  219: i64 field02;
  467: i64 field03;
  932: i64 field04;
  191: i64 field05;
  753: i64 field06;
  316: i64 field07;
  489: i64 field08;
  270: i64 field09;
  628: i64 field10;
  351: i64 field11;
  917: i64 field12;
  142: i64 field13;
  789: i64 field14;
  654: i64 field15;
  327: i64 field16;
  569: i64 field17;
  812: i64 field18;
  945: i64 field19;
}

@go.MinimizePadding
struct LayoutOptimizedStruct {
  1: byte small;
  2: i64 big;
  3: i16 medium;
  4: i32 biggish;
  5: byte tiny;
}

struct LayoutUnoptimizedStruct {
  1: byte small;
  2: i64 big;
  3: i16 medium;
  4: i32 biggish;
  5: byte tiny;
}

enum Colors {
  WHITE = 1,
  BLACK = 2,
}

@go.UseReflectCodec
struct ComparableStruct {
  1: i64 field1;
}

@go.UseReflectCodec
struct NonComparableStruct {
  // Go slices are not comparable
  // As per: https://go.dev/ref/spec#Comparison_operators
  1: list<string> field1;
}

// A struct to test various fields (types/optionality/etc)
@go.UseReflectCodec
struct VariousFieldsStruct {
  // Primitive fields
  1: byte field1;
  2: bool field2;
  3: i16 field3;
  4: i32 field4;
  5: i64 field5;
  6: float field6;
  7: double field7;
  8: binary field8;
  9: string field9;
  // Optional primitive fields
  11: optional byte field11;
  12: optional bool field12;
  13: optional i16 field13;
  14: optional i32 field14;
  15: optional i64 field15;
  16: optional float field16;
  17: optional double field17;
  18: optional binary field18;
  19: optional string field19;

  // Enum
  20: Colors field20;
  // Optional enum
  21: optional Colors field21;

  // Containers
  30: set<string> field30;
  31: list<string> field31;
  32: set<ComparableStruct> field32;
  33: list<ComparableStruct> field33;
  34: map<string, string> field34;
  // Optional containers
  41: optional set<string> field41;
  42: optional list<string> field42;
  43: optional map<string, string> field43;
  // Map edge cases
  51: map<ComparableStruct, string> field51;
  52: map<NonComparableStruct, string> field52;
  53: map<list<string>, string> field53;

  // Struct
  60: ComparableStruct field60;
  // Optional struct
  61: optional ComparableStruct field61;
}

const VariousFieldsStruct variousFieldsStructConst1 = VariousFieldsStruct{
  field1 = 12,
  field2 = true,
  field3 = 1500,
  field4 = 33000,
  field5 = 2247483648,
  field6 = 123.5,
  field7 = 1.7976931348623157e+308,
  field8 = "binary_test",
  field9 = "string_test",
  field11 = 12,
  field12 = true,
  field13 = 1500,
  field14 = 33000,
  field15 = 2247483648,
  field16 = 123.5,
  field17 = 1.7976931348623157e+308,
  field18 = "binary_test",
  field19 = "string_test",
  field20 = 1,
  field21 = 2,
  field30 = ["hello1", "hello2", "hello3"],
  field31 = ["hello1", "hello2", "hello3"],
  field32 = [{"field1": 123}, {"field1": 456}],
  field33 = [{"field1": 123}, {"field1": 456}],
  field34 = {"key1": "value1", "key2": "value2"},
  field41 = ["hello1", "hello2", "hello3"],
  field42 = ["hello1", "hello2", "hello3"],
  field43 = {"key1": "value1", "key2": "value2"},
  field52 = {{"field1": ["hello1", "hello2", "hello3"]}: "value1"},
  field60 = {"field1": 123},
  field61 = {"field1": 123},
};
