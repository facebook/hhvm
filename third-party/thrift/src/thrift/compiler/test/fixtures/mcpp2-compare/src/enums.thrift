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

namespace cpp2 facebook.ns.qwerty

enum AnEnumA {
  FIELDA = 0,
}

enum AnEnumB {
  FIELDA = 0,
  FIELDB = 2,
}

enum AnEnumC {
  FIELDC = 0,
}

const map<string, AnEnumB> MapStringEnum = {"0": FIELDB};

const map<AnEnumC, string> MapEnumString = {FIELDC: "unknown"};

enum AnEnumD {
  FIELDD = 0,
}

enum AnEnumE {
  FIELDA = 0,
}

struct SomeStruct {
  1: i32 fieldA;
}

const map<AnEnumA, set<AnEnumB>> ConstantMap1 = {
  AnEnumA.FIELDA: [AnEnumB.FIELDA, AnEnumB.FIELDB],
};

const map<AnEnumC, map<i16, set<i16>>> ConstantMap2 = {
  AnEnumC.FIELDC: ConstantMap1,
};
