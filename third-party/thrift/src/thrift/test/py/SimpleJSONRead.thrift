/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace py SimpleJSONRead

struct SomeStruct {
  1: i32 anInteger;
  2: map<string, double> aMap;
}

struct Stuff {
  1: string aString;
  2: i16 aShort;
  3: i32 anInteger;
  4: i64 aLong;
  5: double aDouble;
  6: bool aBool;
  7: binary aBinary;
  8: SomeStruct aStruct;
  9: list<list<list<list<string>>>> aList;
  10: map<i32, map<string, list<i32>>> aMap;
  11: string anotherString;
  12: list<SomeStruct> aListOfStruct;
  13: list<set<string>> aListOfSet;
  14: list<double> aListOfDouble;
  15: map<i32, map<string, i32>> anotherMap;
}

struct StuffMissing {
  // 1: string aString,
  // 2: i16 aShort,
  3: i32 anInteger;
  // 4: i64 aLong,
  5: double aDouble;
  // 6: bool aBool,
  7: binary aBinary;
  // 8: SomeStruct aStruct,
  9: list<list<list<list<string>>>> aList;
  // 10: map<i32, map<string, list<i32>>> aMap,
  11: string anotherString;
  // 12: list<SomeStruct> aListOfStruct,
  13: list<set<string>> aListOfSet;
  // 14: list<double> aListOfDouble,
  15: map<i32, map<string, i32>> anotherMap;
}

struct Empty {}
