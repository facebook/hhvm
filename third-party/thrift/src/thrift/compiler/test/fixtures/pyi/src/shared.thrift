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

/**
 * This Thrift file can be included by other Thrift files that want to share
 * these definitions.
 */

namespace cpp2 simple.example
namespace py simple.example
namespace py.asyncio simple.example_asyncio
namespace py3 simple.example

include 'dependent.thrift'

enum AnEnum {
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
}

exception SimpleException {
  1: i16 err_code;
}

exception MessageException {
  1: string message;
  2: i16 err_code;
} (message = 'message')

typedef list<SimpleStruct> ListOfSimpleStructs

struct SimpleStruct {
  1: required bool is_on;
  2: required byte tiny_int = 5;
  3: optional i16 small_int = 10;
  4: optional i32 nice_sized_int;
  5: i64 big_int;
  6: float coarse_real = 0.0;
  7: double precise_real;
  8: string a_str (yo_dawg = "I herd u like annotations");
  8000: optional binary a_bytes (so_I_put = "some Thrift annotations here");
} (
  so_you_can = "Thrift annotate",
  while_you = "type annotate",
  empty,
  some_int = 1,
)

struct ComplexStruct {
  1: required SimpleStruct structOne;
  2: optional SimpleStruct structTwo;
  3: i32 an_integer;
  4: string name;
  5: AnEnum an_enum;
  6: list<i32> values;
  7: ListOfSimpleStructs structs;
  8: map<string, string> amap;
  9: set<string> aset;
  10: dependent.Item item;
  11: i32 from; // reserved word
}

union UnionStruct {
  1: bool is_a_bool;
  2: string some_string_goes_here;
  3: i64 perhaps_a_big_int;
}

const bool A_BOOL = true;
const byte A_BYTE = 8;
const i16 THE_ANSWER = 42;
const i32 A_NUMBER = 84;
const i64 A_BIG_NUMBER = 102;
const double A_REAL_NUMBER = 3.14;
const double A_FAKE_NUMBER = 3;
const string A_WORD = "Good word";
const SimpleStruct A_STRUCT = {
  "is_on": true,
  "tiny_int": 5,
  "small_int": 6,
  "nice_sized_int": 7,
  "big_int": 8,
  "coarse_real": 8.9,
  "precise_real": 9.9,
};
const AnEnum AN_ENUM_VALUE = AnEnum.FOUR;
const list<string> WORD_LIST = [
  "the",
  "quick",
  "brown",
  "fox",
  "jumps",
  "over",
  "the",
  "lazy",
  "dog",
];
const set<i32> DIGITS = [1, 2, 3, 4, 5];
const map<string, SimpleStruct> A_CONST_MAP = {
  "simple": {
    "is_on": false,
    "tiny_int": 50,
    "small_int": 61,
    "nice_sized_int": 72,
    "big_int": 83,
    "coarse_real": 93.9,
    "precise_real": 99.9,
  },
};

service SharedService {
  i32 get_five();
  i32 add_five(1: i32 num);
  void do_nothing();
  string concat(1: string first, 2: string second);
  i32 get_value(1: SimpleStruct simple_struct);
  bool negate(1: bool input);
  byte tiny(1: byte input);
  i16 small(1: i16 input);
  i64 big(1: i64 input);
  double two(1: double input);
  void expected_exception() throws (1: SimpleException se);
  i32 unexpected_exception();
  i32 sum_i16_list(1: list<i16> numbers);
  i32 sum_i32_list(1: list<i32> numbers);
  i32 sum_i64_list(1: list<i64> numbers);
  string concat_many(1: list<string> words);
  i32 count_structs(1: ListOfSimpleStructs items);
  i32 sum_set(1: set<i32> numbers);
  bool contains_word(1: set<string> words, 2: string word);
  string get_map_value(1: map<string, string> words, 2: string key);
  i16 map_length(1: map<string, SimpleStruct> items);
  i16 sum_map_values(1: map<string, i16> items);
  i32 complex_sum_i32(1: ComplexStruct counter);
  string repeat_name(1: ComplexStruct counter);
  SimpleStruct get_struct();
  list<i32> fib(1: i16 n);
  set<string> unique_words(1: list<string> words);
  map<string, i16> words_count(1: list<string> words);
  AnEnum set_enum(1: AnEnum in_enum);
  i16 get_the_answer();
  list<string> all_letters();
  i32 nested_map_argument(1: map<string, ListOfSimpleStructs> struct_map);
  string make_sentence(1: list<list<string>> word_chars);
  set<i32> get_union(1: list<set<i32>> sets);
  set<string> get_keys(1: list<map<string, string>> string_map);
  void get_union_value(1: UnionStruct input);
}

service EmptyService {
}
