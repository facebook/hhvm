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

struct StructContainingOptionalList {
  1: optional list<i32> data;
}

struct StructContainingRequiredList {
  1: required list<i32> data;
}

enum EnumZeroToTen {
  VALUE_0 = 0,
  VALUE_1 = 1,
  VALUE_2 = 2,
  VALUE_3 = 3,
  VALUE_5 = 5,
  VALUE_6 = 6,
  VALUE_7 = 7,
  VALUE_8 = 8,
  VALUE_9 = 9,
  VALUE_10 = 10,
}

struct StructContainingEnum {
  1: EnumZeroToTen data;
}

typedef i64 int64
struct JsonTypedefs {
  1: map<int64, int64> x;
  2: list<int64> y;
  3: set<int64> z;
  4: int64 w;
}

struct Container {
  1: StructContainingEnum nested_struct;
}
