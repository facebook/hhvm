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

enum global_enum1 {
  field0 = 0,
  field1 = 1,
  field2 = 2,
}

enum global_enum2 {
  field0_2 = 0,
  field1_2 = 1,
  field2_2 = 2,
}

enum global_enum3 {
  field0_3 = 0,
  field1_3 = 1,
  field2_3 = 2,
} (
  one.here = "with some value associated",
  another.there = ".",
  yet.another = "and yet more text - it's that easy",
  duplicate_id_annotation_1 = "duplicate id annotation",
  duplicate_id_annotation_2 = "duplicate.id.annotation",
  _now.with.an.underscore = "_now.with.an.underscore",
)

union global_union1 {
  1: i32 ui;
  2: double ud;
  3: string us;
  4: global_enum1 ue;
}

union global_union2 {
  1: i32 ui_2;
  2: double ud_2;
  3: string us_2;
  4: global_enum1 ue_2;
}

union global_union3 {
  1: i32 ui_3;
  2: double ud_3;
  3: string us_3;
  4: global_enum1 ue_3;
}

struct global_structA {
  1: i32 a;
  2: string b;
}

union global_unionA {
  1: i32 i;
  2: double d;
  3: string s;
  4: global_enum1 e;
  5: global_structA a;
} (sample.annotation = "some text here", another.annotation = "some more text")

struct global_structB {
  1: double c;
  2: bool d (
    some.annotation = "some value",
    another.annotation = "another value",
  );
} (
  some.annotation = "this is its value",
  some.other.annotation = "this is its other value",
)

struct global_structC {
  1: i32 a;
  2: string b;
  3: double c;
  4: bool d;
  5: global_enum1 e;
  6: global_enum2 f;
  7: global_union1 g;
  8: global_unionA h;
  9: global_unionA i;
  10: list<i32> j;
  11: list<i32> j1;
  12: list<global_enum1> j2;
  13: list<global_structA> j3;
  14: set<i32> k;
  15: set<i32> k1;
  16: set<global_enum2> k2;
  17: set<global_structB> k3;
  18: map<i32, i32> l;
  19: map<i32, i32> l1;
  20: map<i32, global_enum1> l2;
  21: map<i32, global_structB> l3;
  22: map<global_enum1, i32> m1;
  23: map<global_enum1, global_enum2> m2;
  24: map<global_enum1, global_structB> m3;
  25: map<string, i32> n1;
  26: map<string, global_enum1> n2;
  27: map<string, global_structB> n3;
  28: map<global_structA, i32> o1;
  29: map<global_structA, global_enum1> o2;
  30: map<global_structA, global_structB> o3;
}

struct global_struct1 {
  1: required i32 field0;
  2: optional string field1;
  3: global_enum1 field2;
  4: required global_enum2 field3;
  5: optional global_union1 field4;
  6: global_union2 field5;
}

struct global_struct2 {
  1: i32 fieldA;
  2: string fieldB;
  3: global_enum1 fieldC;
  4: global_enum2 fieldD;
  5: global_union1 fieldE;
  6: global_union2 fieldF;
  7: global_struct1 fieldG;
}

struct global_struct3 {
  1: i32 fieldA;
  2: string fieldB;
  3: global_enum1 fieldC;
  4: global_enum2 fieldD;
  5: global_union1 fieldE;
  6: global_union2 fieldF;
  7: global_struct1 fieldG;
  8: global_union2 fieldH;
  9: list<i32> fieldI;
  10: list<string> fieldJ;
  11: list<string> fieldK;
  12: list<global_structA> fieldL;
  13: set<i32> fieldM;
  14: set<string> fieldN;
  15: set<string> fieldO;
  16: set<global_structB> fieldP;
  17: map<string, global_structA> fieldQ;
  18: map<string, global_structB> fieldR;
  19: map<binary, binary> fieldS;
}

service global_service1 {
  void method1();
  void method2(1: i32 x, 2: global_struct1 y, 3: double z);
  i32 method3();
  i32 method4(1: i32 i, 2: global_struct1 j, 3: double k);
  global_struct2 method5();
  global_struct2 method6(1: i32 l, 2: global_struct1 m, 3: double n);
}

service global_service2 {
  void methodA();
  void methodB(1: i32 x, 2: global_struct1 y, 3: double z);
  i32 methodC();
  i32 methodD(1: i32 i, 2: global_struct1 j, 3: double k);
  global_struct2 methodE();
  global_struct2 methodF(1: i32 l, 2: global_struct1 m, 3: double n);
}

service global_service3 {
  void methodA();
  void methodB(1: i32 x, 2: global_struct1 y, 3: double z);
  i32 methodC();
  i32 methodD(1: i32 i, 2: global_struct1 j, 3: double k);
  global_struct2 methodE();
  global_struct3 methodF(1: i32 l, 2: global_struct1 m, 3: double n);
}

const i32 constant1 = 1357;
const string constant2 = "hello";
const global_enum1 constant3 = global_enum1.field0;
