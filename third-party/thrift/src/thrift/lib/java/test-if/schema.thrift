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

@thrift.AllowLegacyMissingUris
package;

namespace java.swift com.facebook.thrift.test

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
  15: map<list<i32>, list<i32>> aMapOfLists;
  16: binary aBinary;
}

struct JustABinary {
  1: binary aBinary;
}

struct Pet {
  1: string name;
  2: optional i32 age;
  3: optional bool vegan;
}

struct Nested1 {
  1: list<Pet> pets;
}

struct Nested2 {
  1: map<i32, Nested1> nests;
}

union TestUnion {
  1: Pet thePet;
  2: Pet theOtherPet;
  3: EveryLayout allThings;
  4: JustABinary aBinary;
}
