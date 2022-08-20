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

struct AStruct {
  1: required string aString;
  2: i32 anInteger;
}

struct OneOfEach {
  1: bool aBool;
  2: byte aByte;
  3: i16 anInteger16;
  4: i32 anInteger32;
  5: i64 anInteger64;
  6: string aString;
  7: binary aBinary;
  8: double aDouble;
  9: float aFloat;
  10: list<i32> aList;
  11: set<string> aSet;
  12: map<string, i32> aMap;
  13: AStruct aStruct;
}

struct Required {
  1: required i32 anInteger;
  2: AStruct aStruct;
}

union TestUnion {
  1: string string_field;
  2: i32 i32_field;
  3: AStruct struct_field;
  4: list<string> list_field;
}

struct StructWithUnion {
  1: TestUnion aUnion;
  2: string aString;
}

struct NegativeFieldId {
  -1: i32 anInteger;
  0: string aString;
  5: double aDouble;
}
