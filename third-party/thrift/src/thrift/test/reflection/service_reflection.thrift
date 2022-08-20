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

namespace cpp2 service_reflection.cpp2

enum enum1 {
  e1field1 = 0,
  e1field2 = 1,
  e1field3 = -4,
}

enum enum2 {
  e2field1 = 0,
  e2field2 = 1,
  e2field3 = 2,
}

struct struct1 {
  1: i32 field1;
  2: double field2;
  3: enum1 field3;
  4: enum2 field4;
}

service service_1 {
  void method1();
  void method2(1: i32 x, 2: struct1 y, 3: double z);
  i32 method3();
  i32 method4(1: i32 i, 2: struct1 j, 3: double k);
  struct1 method5();
  struct1 method6(1: i32 l, 2: struct1 m, 3: double n);
}
