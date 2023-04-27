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

namespace py3 thrift.benchmark
namespace py thrift.benchmark.struct
namespace cpp2 thrift.benchmark.st

struct MyStruct {
  1: bool val_bool;
  2: i32 val_i32;
  3: i64 val_i64;
  4: string val_string;
  5: binary val_binary;
  6: binary (py3.iobuf) val_iobuf;
  9: list<i64> val_list;
  11: map<i32, string> val_map;
  15: set<i32> val_set;
  18: map<i32, Included> val_map_structs;
  20: StringBucket val_struct;
}

struct Included {
  1: list<string> vals;
}

struct StringBucket {
  1: string one;
  2: string two;
  3: string three;
  4: string four;
  5: string five;
  6: string six;
  7: string seven;
  8: string eight;
  9: string nine;
  10: string ten;
}
