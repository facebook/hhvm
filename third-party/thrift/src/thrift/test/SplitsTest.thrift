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

namespace cpp2 apache.thrift

struct struct1 {
  1: i32 i;
  2: string s;
}

struct struct2 {
  1: i32 i;
  2: string s;
}

enum enum1 {
  value1 = 1,
  value2 = 2,
}

struct struct3 {
  1: struct1 field1;
  2: optional struct2 field2;
}

service Service1 {
  void method1();
  void method2();
  void method3();
}

service Service2 {
  void method4();
  void method5();
  void method6();
}
