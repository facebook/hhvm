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

include "thrift/lib/cpp2/frozen/test/Helper.thrift"
include "thrift/annotation/cpp.thrift"

namespace cpp2 apache.thrift.test

cpp_include "<unordered_set>"
cpp_include "thrift/lib/cpp2/frozen/VectorAssociative.h"
cpp_include "thrift/lib/cpp2/frozen/HintTypes.h"

enum Gender {
  Male = 0,
  Female = 1,
  Other = 2,
}

struct Nesting {
  1: Helper.Ratio a;
  2: Helper.Ratio b;
}

struct Pet1 {
  1: string name;
  2: optional i32 age;
  3: optional bool vegan;
}

struct Person1 {
  5: optional i32 age;
  2: float height; // different
  1: string name;
  4: list<Pet1> pets;
  6: Gender gender = Male;
}

struct Pet2 {
  3: optional float weight;
  1: string name;
}

struct Person2 {
  1: string name;
  3: float weight; // different
  4: list<Pet2> pets;
  5: optional i32 age;
}

struct Tiny {
  1: required string a;
  2: string b;
  3: string c;
  4: string d;
}

struct Place {
  1: string name;
  2: map<i32, i32> popularityByHour;
}

struct PlaceTest {
  1: map<i64, Place> places;
}

struct EveryLayout {
  1: bool aBool;
  2: i32 aInt;
  3: list<i32> aList;
  4: set<i32> aSet;
  @cpp.Type{template = "std::unordered_set"}
  5: set<i32> aHashSet;
  6: map<i32, i32> aMap;
  @cpp.Type{template = "std::unordered_map"}
  7: map<i32, i32> aHashMap;
  8: optional i32 optInt; // optional layout
  9: float aFloat; // trivial layout
  10: optional map<i32, i32> optMap;
}

struct VectorTest {
  1: list<i32> aList;
  @cpp.Type{template = "apache::thrift::frozen::VectorAsSet"}
  2: set<i32> aSet;
  @cpp.Type{template = "apache::thrift::frozen::VectorAsMap"}
  3: map<i32, i32> aMap;
  @cpp.Type{template = "apache::thrift::frozen::VectorAsHashSet"}
  4: set<i32> aHashSet;
  @cpp.Type{template = "apache::thrift::frozen::VectorAsHashMap"}
  5: map<i32, i32> aHashMap;
  @cpp.Type{template = "folly::fbvector"}
  6: list<i32> fbVector;
}

struct EnumAsKeyTest {
  @cpp.Type{template = "std::unordered_set"}
  1: set<Gender> enumSet;
  @cpp.Type{template = "std::unordered_map"}
  2: map<Gender, i32> enumMap;
  @cpp.Type{template = "std::unordered_set"}
  3: set<Helper.Animal> outsideEnumSet;
  @cpp.Type{template = "std::unordered_map"}
  4: map<Helper.Animal, i32> outsideEnumMap;
}

union TestUnion {
  1: i32 aInt;
  2: string aString;
  3: list<i64> aList;
  4: map<i32, i64> aMap;
  5: set<string> aSet;
  6: Member aStruct;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  7: Pet1 aPet1;
  @cpp.Ref{type = cpp.RefType.Unique}
  8: Tiny aTiny;
  @cpp.Ref{type = cpp.RefType.Unique}
  9: Place aPlace;
}

union TestUnion2 {
  2: string aString;
  3: list<i64> aList;
  4: map<i32, i64> aMap;
  5: set<string> aSet;
  6: Member aStruct;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  7: Pet1 aPet1;
  @cpp.Ref{type = cpp.RefType.Unique}
  8: Tiny aTiny;
  @cpp.Ref{type = cpp.RefType.Unique}
  9: Place aPlace;
  10: i32 anotherInt;
}

struct Member {
  1: i64 adId;
  2: string name;
  3: optional list<i64> creativeIds;
}

struct Big {
  1: optional string anOptionalString;
  2: i64 anId;
  3: optional list<i64> anOptionalList;
  4: TestUnion aTestUnion;
  5: string aString;
}

struct User {
  1: i64 uid;
  2: string name;
} (cpp.declare_hash, cpp.declare_equal_to)

struct TriviallyCopyableStruct {
  1: required i32 field;
}

@cpp.Type{name = "apache::thrift::frozen::FixedSizeString<2>"}
typedef string Fixed2
@cpp.Type{name = "apache::thrift::frozen::FixedSizeString<8>"}
typedef string Fixed8

struct TestFixedSizeString {
  1: Fixed8 bytes8;
  @cpp.Type{name = "apache::thrift::frozen::FixedSizeString<4>"}
  2: optional string bytes4;
  @cpp.Type{template = "apache::thrift::frozen::VectorAsHashMap"}
  3: map<Fixed8, Fixed2> aMapToFreeze;
  @cpp.Type{template = "std::unordered_map"}
  4: map<Fixed8, Fixed2> aMap;
}

struct Empty {}

struct Excluded {} (cpp.frozen2_exclude)

struct ContainsExcluded {
  1: optional Excluded excluded;
}
