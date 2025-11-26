/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

include "thrift/annotation/python.thrift"
include "thrift/lib/python/test/dependency.thrift"

namespace py3 python_test

enum Kind {
  None = 0,
  REGULAR = 8,
  LINK = 10,
  DIR = 4,
  FIFO = 1,
  CHAR = 2,
  BLOCK = 6,
  SOCK = 12,
}

enum BadMembers {
  @python.Name{name = "name_"}
  name = 1,
  @python.Name{name = "value_"}
  value = 2,
}

enum Color {
  red = 0,
  blue = 1,
  green = 2,
  _Color__pleurigloss = 3,
  __octarine = 4,
}

typedef Color ColorTypedef

typedef dependency.Status StatusTypedef

typedef dependency.ColourAlias ColourTypedefOfTypedef

@python.Flags{}
enum Perm {
  read = 4,
  write = 2,
  execute = 1,
}

struct ColorGroups {
  1: list<Color> color_list;
  2: set<Color> color_set;
  3: map<Color, Color> color_map;
}

struct OptionalColorGroups {
  1: optional list<i32> color_list;
  2: optional set<i32> color_set;
  3: optional map<i32, i32> color_map;
}

struct File {
  1: string name;
  2: Perm permissions;
  3: Kind type = Kind.REGULAR;
}

struct OptionalFile {
  1: optional string name;
  3: optional i32 type;
}

const map<Color, string> ColorMap = {
  Color.red: "r",
  Color.blue: "b",
  Color.green: "g",
  Color._Color__pleurigloss: "p",
  Color.__octarine: "o",
};
