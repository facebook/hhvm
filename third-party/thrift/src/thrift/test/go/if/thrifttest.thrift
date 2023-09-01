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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/go.thrift"

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
}

struct Insanity {
  1: map<Numberz, UserId> userMap;
  2: list<Xtruct> xtructs;
  @cpp.Type{template = "std::unordered_map"}
  3: map<string, string> str2str;
}

struct CrazyNesting {
  1: string string_field;
  2: optional set<Insanity> set_field;
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
