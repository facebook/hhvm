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

namespace cpp2 test_cpp2.cpp_compat
namespace d test_d.cpp_compat
namespace java test_java.cpp_compat
namespace java.swift test_swift.cpp_compat
namespace php test_php.cpp_compat
namespace py3 test_py.cpp_compat

cpp_include "thrift/test/reflection/fatal_custom_types.h"

enum compat_enum1 {
  field0 = 0,
  field1 = 1,
  field2 = 2,
}

enum compat_enum2 {
  field0_2 = 0,
  field1_2 = 1,
  field2_2 = 2,
}

enum compat_enum3 {
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

union compat_union1 {
  1: i32 ui;
  2: double ud;
  3: string us;
  4: compat_enum1 ue;
}

union compat_union2 {
  1: i32 ui_2;
  2: double ud_2;
  3: string us_2;
  4: compat_enum1 ue_2;
}

union compat_union3 {
  1: i32 ui_3;
  2: double ud_3;
  3: string us_3;
  4: compat_enum1 ue_3;
}

struct compat_structA {
  1: i32 a;
  2: string b;
}

typedef compat_structA (
  cpp.type = "test_cpp_reflection::custom_compat_structA",
) my_compat_structA

union compat_unionA {
  1: i32 i;
  2: double d;
  3: string s;
  4: compat_enum1 e;
  5: compat_structA a;
} (sample.annotation = "some text here", another.annotation = "some more text")

struct compat_structB {
  1: double c;
  2: bool d (
    some.annotation = "some value",
    another.annotation = "another value",
  );
} (
  some.annotation = "this is its value",
  some.other.annotation = "this is its other value",
)

struct compat_structC {
  1: i32 a;
  2: string b;
  3: double c;
  4: bool d;
  5: compat_enum1 e;
  6: compat_enum2 f;
  7: compat_union1 g;
  8: compat_unionA h;
  9: compat_unionA i;
  10: list<i32> j;
  11: list<i32> j1;
  12: list<compat_enum1> j2;
  13: list<compat_structA> j3;
  14: set<i32> k;
  15: set<i32> k1;
  16: set<compat_enum2> k2;
  17: set<compat_structB> k3;
  18: map<i32, i32> l;
  19: map<i32, i32> l1;
  20: map<i32, compat_enum1> l2;
  21: map<i32, compat_structB> l3;
  22: map<compat_enum1, i32> m1;
  23: map<compat_enum1, compat_enum2> m2;
  24: map<compat_enum1, compat_structB> m3;
  25: map<string, i32> n1;
  26: map<string, compat_enum1> n2;
  27: map<string, compat_structB> n3;
  28: map<compat_structA, i32> o1;
  29: map<compat_structA, compat_enum1> o2;
  30: map<compat_structA, compat_structB> o3;
}

struct compat_struct1 {
  1: required i32 field0;
  2: optional string field1;
  3: compat_enum1 field2;
  4: required compat_enum2 field3;
  5: optional compat_union1 field4;
  6: compat_union2 field5;
}

struct compat_struct2 {
  1: i32 fieldA;
  2: string fieldB;
  3: compat_enum1 fieldC;
  4: compat_enum2 fieldD;
  5: compat_union1 fieldE;
  6: compat_union2 fieldF;
  7: compat_struct1 fieldG;
}

struct compat_struct3 {
  1: i32 fieldA;
  2: string fieldB;
  3: compat_enum1 fieldC;
  4: compat_enum2 fieldD;
  5: compat_union1 fieldE;
  6: compat_union2 fieldF;
  7: compat_struct1 fieldG;
  8: compat_union2 fieldH;
  9: list<i32> fieldI;
  10: list<string> fieldJ;
  11: list<string> fieldK;
  12: list<compat_structA> fieldL;
  13: set<i32> fieldM;
  14: set<string> fieldN;
  15: set<string> fieldO;
  16: set<compat_structB> fieldP;
  17: map<string, compat_structA> fieldQ;
  18: map<string, compat_structB> fieldR;
  19: map<binary, binary> fieldS;
}

service compat_service1 {
  void method1();
  void method2(1: i32 x, 2: compat_struct1 y, 3: double z);
  i32 method3();
  i32 method4(1: i32 i, 2: compat_struct1 j, 3: double k);
  compat_struct2 method5();
  compat_struct2 method6(1: i32 l, 2: compat_struct1 m, 3: double n);
}

service compat_service2 {
  void methodA();
  void methodB(1: i32 x, 2: compat_struct1 y, 3: double z);
  i32 methodC();
  i32 methodD(1: i32 i, 2: compat_struct1 j, 3: double k);
  compat_struct2 methodE();
  compat_struct2 methodF(1: i32 l, 2: compat_struct1 m, 3: double n);
}

service compat_service3 {
  void methodA();
  void methodB(1: i32 x, 2: compat_struct1 y, 3: double z);
  i32 methodC();
  i32 methodD(1: i32 i, 2: compat_struct1 j, 3: double k);
  compat_struct2 methodE();
  compat_struct3 methodF(1: i32 l, 2: compat_struct1 m, 3: double n);
}

const i32 constant1 = 1357;
const string constant2 = "hello";
const compat_enum1 constant3 = compat_enum1.field0;
