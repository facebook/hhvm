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

struct Simple {
  1: i32 a;
  2: optional i32 b;
}

struct SimpleWithString {
  1: i32 a;
  2: optional i32 b;
  3: string c;
}

struct List {
  1: list<i32> a;
}

struct Set {
  1: set<i32> a;
}

struct Map {
  1: map<i32, string> a;
}

struct Complex {
  1: Simple a;
  2: list<Simple> b;
}

struct ComplexWithStringAndMap {
  1: SimpleWithString a;
  2: list<Simple> b;
  3: list<string> c;
  4: map<string, Simple> d;
}
