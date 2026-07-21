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

package "meta.com/thrift/transcode/golden"

namespace cpp2 apache.thrift.transcode.test

enum GoldenEnum {
  Unknown = 0,
  Active = 1,
  Suspended = 2,
}

enum SparseEnum {
  First = -2,
  Zero = 0,
  Positive = 5,
}

struct GoldenInner {
  1: i32 n;
  2: string label;
}

union GoldenChoice {
  1: i32 id;
  2: string name;
}

exception GoldenException {
  1: string message;
  2: i32 code;
}

struct ScalarShapes {
  1: bool bool_true;
  2: bool bool_false;
  3: byte byte_min;
  4: byte byte_max;
  5: i16 i16_min;
  6: i16 i16_max;
  7: i32 i32_min;
  8: i32 i32_max;
  9: i64 i64_min;
  10: i64 i64_max;
  11: float f_regular;
  12: float f_negative_zero;
  13: float f_lowest;
  14: double d_regular;
  15: double d_negative_zero;
  16: double d_lowest;
  17: string empty_text;
  18: string escaped_text;
  19: binary empty_data;
  20: binary data;
  21: GoldenEnum known_enum;
  22: SparseEnum sparse_enum;
}

struct NonUtf8StringShapes {
  1: string text;
  2: list<string> texts;
}

struct SpecialFloatShapes {
  1: float f_nan;
  2: float f_pos_inf;
  3: float f_neg_inf;
  4: double d_nan;
  5: double d_pos_inf;
  6: double d_neg_inf;
}

struct NegativeFieldIdShapes {
  -1: i32 negative_i32;
  -2: string negative_string;
  1: i64 positive_i64;
}

struct PresenceShapes {
  1: i32 unqualified_i32;
  2: i32 always_i32;
  3: optional i32 maybe_i32;
  4: optional string maybe_text;
  5: optional GoldenInner maybe_inner;
}

struct SequenceShapes {
  1: list<i32> empty_list;
  2: list<i32> ints;
  3: list<GoldenInner> structs;
  4: set<string> empty_strings;
  5: set<string> strings;
  6: set<i32> int_set;
}

struct MapShapes {
  1: map<string, i32> empty_string_map;
  2: map<string, i32> string_map;
  3: map<i32, string> int_map;
  4: map<GoldenEnum, string> enum_map;
}

struct NestedShapes {
  1: GoldenInner inner;
  2: list<list<i32>> matrix;
  3: list<list<GoldenInner>> inner_groups;
}

struct UnionShapes {
  1: GoldenChoice id_choice;
  2: GoldenChoice name_choice;
}

struct ExceptionShapes {
  1: GoldenException ex;
}

struct GoldenStruct {
  1: bool flag;
  2: byte b;
  3: i16 s;
  4: i32 i;
  5: i64 l;
  6: float f;
  7: double d;
  8: string text;
  9: binary data;
  10: list<i32> ints;
  11: set<string> tags;
  12: map<string, i32> counts;
  13: GoldenInner inner;
  14: optional string maybe_text;
  15: GoldenEnum status;
  16: GoldenChoice choice;
}
