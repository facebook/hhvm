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

namespace cpp2 apache.thrift.test

struct Struct1 {
  1: optional string str;
}

struct Struct2 {
  1: optional list<Struct1> structs;
}

struct MixedStruct {
  1: optional bool myBool;
  2: optional byte myByte;
  3: optional i16 myI16;
  4: optional i32 myI32;
  5: optional i64 myI64;
  6: optional double myDouble;
  7: optional float myFloat;
  8: optional string myString;
  9: optional binary myBinary;
  10: optional map<i32, string> myMap;
  11: optional list<string> myList;
  12: optional set<i64> mySet;
  13: optional Struct1 struct1;
  14: optional Struct2 struct2;
}
