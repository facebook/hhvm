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

namespace c_glib TTest
namespace java thrift.test
namespace hyperthrift thrift.test
namespace cpp thrift.test
namespace rb Thrift.Test
namespace perl ThriftTest
namespace csharp Thrift.Test
namespace js ThriftTest
namespace st ThriftTest
namespace py ThriftTest
namespace py.twisted ThriftTest
namespace go thrifttest
namespace php ThriftTest
namespace delphi Thrift.Test
namespace cocoa ThriftTest
namespace lua ThriftTest

/**
 * Docstring!
 */
enum Numberz {
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FIVE = 5,
  SIX = 6,
  EIGHT = 8,
}

const Numberz myNumberz = Numberz.ONE;
// the following is expected to fail:
// const Numberz urNumberz = ONE;

typedef i64 UserId

struct Bonk {
  1: string message;
  2: i32 type;
}

typedef map<string, Bonk> MapType

struct Bools {
  1: bool im_true;
  2: bool im_false;
}

struct Xtruct {
  1: string string_thing;
  4: byte byte_thing;
  9: i32 i32_thing;
  11: i64 i64_thing;
}

struct Xtruct2 {
  1: i16 byte_thing; // used to be byte, hence the name
  2: Xtruct struct_thing;
  3: i32 i32_thing;
}

struct Xtruct3 {
  1: string string_thing;
  4: i32 changed;
  9: i32 i32_thing;
  11: i64 i64_thing;
}

union XUnion {
  1: Xtruct first;
  2: Xtruct2 second;
  3: Xtruct3 third;
}

struct Insanity {
  1: map<Numberz, UserId> userMap;
  2: list<Xtruct> xtructs;
}

struct CrazyNesting {
  1: string string_field;
  2: optional set<Insanity> set_field;
  // Do not insert line break as test/go/Makefile.am is removing this line with pattern match
  3: required list<
    map<set<i32>, map<i32, set<list<map<Insanity, string>>>>>
  > list_field;
  4: binary binary_field;
  // Very simple list of 32-bit integers
  5: optional list<i32> i32_list_field;
  // Very simple set of 64-bit integers
  6: optional set<i64> i64_set_field;
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

struct VersioningTestV1 {
  1: i32 begin_in_both;
  3: string old_string;
  12: i32 end_in_both;
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
  11: string newstring;
  12: i32 end_in_both;
}

struct ListTypeVersioningV1 {
  1: list<i32> myints;
  2: string hello;
}

struct ListTypeVersioningV2 {
  1: list<string> strings;
  2: string hello;
}

struct GuessProtocolStruct {
  7: map<string, string> map_field;
}

struct LargeDeltas {
  1: Bools b1;
  10: Bools b10;
  100: Bools b100;
  500: bool check_true;
  1000: Bools b1000;
  1500: bool check_false;
  2000: VersioningTestV2 vertwo2000;
  2500: set<string> a_set2500;
  3000: VersioningTestV2 vertwo3000;
  4000: list<i32> big_numbers;
}

struct NestedListsI32x2 {
  1: list<list<i32>> integerlist;
}
struct NestedListsI32x3 {
  1: list<list<list<i32>>> integerlist;
}
struct NestedMixedx2 {
  1: list<set<i32>> int_set_list;
  2: map<i32, set<string>> map_int_strset;
  3: list<map<i32, set<string>>> map_int_strset_list;
}
struct ListBonks {
  1: list<Bonk> bonk;
}
struct NestedListsBonk {
  1: list<list<list<Bonk>>> bonk;
}

struct BoolTest {
  1: optional bool b = true;
  2: optional string s = "true";
}

struct StructA {
  1: required string s;
}

struct StructB {
  1: optional StructA aa;
  2: required StructA ab;
}
