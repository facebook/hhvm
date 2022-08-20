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

enum Color {
  RED = 0xFF0000,
  ORANGE = 0xFF6600,
  YELLOW = 0xFFFF00,
  GREEN = 0x00FF00,
  BLUE = 0x0000FF,
  PURPLE = 0x663399,
  WHITE = 0xFFFFFF,
  BLACK = 0x000000,
  GRAY = 0x808080,
}

struct ListStruct {
  1: list<bool> a;
  2: list<i16> b;
  3: list<double> c;
  4: list<string> d;
  5: list<list<i32>> e;
  6: list<map<i32, i32>> f;
  7: list<set<string>> g;
}

union IntUnion {
  1: i32 a;
  2: i32 b;
}

struct Rainbow {
  1: list<Color> colors;
  2: double brightness;
}

struct NestedStructs {
  1: ListStruct ls;
  2: Rainbow rainbow;
  3: IntUnion ints;
}

struct SimpleStruct {
  1: i32 a;
  2: i32 b;
}

struct StructWithOptionals {
  1: optional bool a;
  2: optional i32 b;
  3: optional SimpleStruct c;
}

struct BTreeBranch {
  1: required BTree child;
  2: i16 cost;
}

struct BTree {
  1: i32 min;
  2: i32 max;
  3: string data;
  4: required list<BTreeBranch> children;
}

exception KeyNotFound {
  1: i32 key;
  2: i32 nearest_above;
  3: i32 nearest_below;
}

exception EmptyData {
  1: string message;
}

service TestService {
  string lookup(1: BTree root, 2: i32 key) throws (
    1: KeyNotFound e,
    2: EmptyData f,
  );

  void nested(1: NestedStructs ns);

  void listStruct(1: ListStruct ls);
}

service DerivedTestService extends TestService {
}

union EmptyUnion {}

union NumberUnion {
  1: i32 my_integer;
  2: float my_float;
} (final)

struct NumberUnionStruct {
  1: NumberUnion nu = {'my_integer': 100};
} (final)
