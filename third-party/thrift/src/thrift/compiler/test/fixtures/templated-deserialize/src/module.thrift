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

typedef i64 (cpp.type = "Foo", cpp.indirection) IndirectionA
typedef i32 (cpp.type = "Baz", cpp.indirection) IndirectionC
typedef double (cpp.type = "Bar", cpp.indirection) IndirectionB

enum MyEnumA {
  fieldA = 1,
  fieldB = 2,
  fieldC = 4,
}

struct SmallStruct {
  1: bool small_A;
  2: i32 small_B;
}

struct containerStruct {
  1: bool fieldA;
  2: map<string, bool> fieldB;
  3: set<i32> fieldC = [1, 2, 3, 4];
  4: string fieldD;
  5: string fieldE = "somestring";
  6: list<list<list<i32>>> fieldF;
  7: map<string, map<string, map<string, i32>>> fieldG;
  8: list<set<i32>> fieldH;
  9: bool fieldI = true;
  10: map<string, list<i32>> fieldJ = {
    "subfieldA": [1, 4, 8, 12],
    "subfieldB": [2, 5, 9, 13],
  };
  11: list<list<list<list<i32>>>> fieldK;
  12: set<set<set<bool>>> fieldL;
  13: map<set<list<i32>>, map<list<set<string>>, string>> fieldM;
  14: list<IndirectionA> fieldN;
  15: list<IndirectionB> fieldO;
  16: list<IndirectionC> fieldP;
  17: MyEnumA fieldQ;
  18: map<string, bool> fieldR (cpp.ref);
  19: SmallStruct fieldS (cpp.ref_type = "unique");
  20: SmallStruct fieldT (cpp.ref_type = "shared");
  21: SmallStruct fieldU (cpp.ref_type = "shared_const");
  23: SmallStruct fieldX (cpp.ref);
}
