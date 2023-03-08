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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/meta.thrift"

namespace cpp2 py3.simple

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>", py3.iobuf) IOBufPtr
typedef binary (cpp2.type = "folly::IOBuf", py3.iobuf) IOBuf

enum AnEnum {
  None = 0 (py3.name = "NOTSET"),
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
}

enum AnEnumRenamed {
  name = 0 (py3.name = "name_"),
  value = 1 (py3.name = "value_"),
  normal = 2 (py3.name = "renamed_"),
}

enum Flags {
  flag_A = 1,
  flag_B = 2,
  flag_C = 4,
  flag_D = 8,
} (py3.flags)

exception SimpleException {
  1: i16 err_code;
}

struct OptionalRefStruct {
  1: optional IOBufPtr optional_blob;
}

struct SimpleStruct {
  1: bool is_on;
  2: byte tiny_int;
  3: i16 small_int;
  4: i32 nice_sized_int;
  5: i64 big_int;
  6: double real;
  7: float smaller_real;
  // The next field should not show up anywhere in the generated code.
  8: i16 hidden_field (py3.hidden);
}

@cpp.Adapter{name = "Adapter"}
typedef SimpleStruct AdaptedTypeDef
typedef SimpleStruct HiddenTypeDef (py3.hidden)

struct HiddenTypeFieldsStruct {
  1: AdaptedTypeDef field1 (py3.hidden);
  2: list<AdaptedTypeDef> field2 (py3.hidden);
  3: map<i32, AdaptedTypeDef> (cpp.template = "::std::unordered_map") field3 (
    py3.hidden,
  );
}

@meta.SetGenerated
struct GeneratedStruct {
  1: i16 the;
}

@cpp.Adapter{name = "Adapter"}
union AdaptedUnion {
  1: i16 best;
}

safe exception HiddenException {
  1: i16 test;
} (py3.hidden)

typedef AdaptedUnion ImplicitlyHiddenTypeDef

typedef binary (cpp.type = "foo::Bar") foo_bar

struct ComplexStruct {
  1: SimpleStruct structOne;
  2: SimpleStruct structTwo;
  3: i32 an_integer;
  4: string name;
  5: AnEnum an_enum;
  6: binary some_bytes;
  7: string from (py3.name = "sender");
  8: string cdef;
  9: foo_bar bytes_with_cpp_type;
}

union BinaryUnion {
  1: IOBuf iobuf_val;
} (cpp.noncomparable)

struct BinaryUnionStruct {
  1: BinaryUnion u;
} (cpp.noncomparable)

const bool A_BOOL = true;
const byte A_BYTE = 8;
const i16 THE_ANSWER = 42;
const i32 A_NUMBER = 84;
const i64 A_BIG_NUMBER = 102;
const double A_REAL_NUMBER = 3.14;
const double A_FAKE_NUMBER = 3;
const string A_WORD = "Good word";
const binary SOME_BYTES = "bytes";
const SimpleStruct A_STRUCT = {
  "is_on": true,
  "tiny_int": 5,
  "small_int": 6,
  "nice_sized_int": 7,
  "big_int": 8,
  "real": 9.9,
};
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
const list<map<i32, double>> SOME_MAP = [{1: 1.1, 2: 2.2}, {3: 3.3}];
const set<i32> DIGITS = [1, 2, 3, 4, 5];
const map<string, SimpleStruct> A_CONST_MAP = {
  "simple": {
    "is_on": false,
    "tiny_int": 50,
    "small_int": 61,
    "nice_sized_int": 72,
    "big_int": 83,
    "real": 99.9,
  },
};
const map<AnEnumRenamed, i32> ANOTHER_CONST_MAP = {name: 0,value: 1,normal: 2,};

service SimpleService {
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
  i32 count_structs(1: list<SimpleStruct> items);
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
  list<list<i32>> list_of_lists(1: i16 num_lists, 2: i16 num_items);
  map<string, map<string, i32>> word_character_frequency(1: string sentence);
  list<set<string>> list_of_sets(1: string some_words);
  i32 nested_map_argument(1: map<string, list<SimpleStruct>> struct_map);
  string make_sentence(1: list<list<string>> word_chars);
  set<i32> get_union(1: list<set<i32>> sets);
  set<string> get_keys(1: list<map<string, string>> string_map);
  double lookup_double(1: i32 key);
  binary retrieve_binary(1: binary something);
  set<binary> contain_binary(1: list<binary> binaries);
  list<AnEnum> contain_enum(1: list<AnEnum> the_enum);
  BinaryUnionStruct get_binary_union_struct(1: BinaryUnion u);
  SimpleStruct get_struct_hidden() (py3.hidden);
}

service DerivedService extends SimpleService {
  i32 get_six();
} (foo = '"""', bar)

service RederivedService extends DerivedService {
  i32 get_seven();
}
