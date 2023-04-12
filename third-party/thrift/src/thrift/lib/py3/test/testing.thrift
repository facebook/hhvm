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

cpp_include "<deque>"
cpp_include "folly/container/F14Map.h"
cpp_include "folly/FBString.h"

package "facebook.com/testing"

namespace py3 ""
namespace cpp2 cpp2

const list<i16> int_list = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

const map<i16, map<i16, i16>> LocationMap = {1: {1: 1}};

typedef list<i32> I32List
typedef list<list<string>> StrList2D
typedef list<string> StringList
typedef map<string, i64> StrIntMap
typedef map<string, string> StrStrMap
typedef map<string, I32List> StrI32ListMap
typedef map<string, StrI32ListMap> StrStrIntListMapMap
typedef set<I32List> SetI32Lists
typedef set<SetI32Lists> SetSetI32Lists
typedef set<i32> SetI32
typedef bool Bool
typedef i64 TimeStamp
typedef byte Byte
typedef float Float
typedef double Double
typedef list<easy> EasyList
typedef set<easy> EasySet
typedef map<string, easy> StrEasyMap
// @lint-ignore THRIFTFORMAT (for autodeps)
typedef map<string, string> (
  cpp.type = "folly::F14FastMap<std::string, folly::fbstring>",
) F14MapFollyString
typedef list<i32> (cpp2.type = "std::vector<uint32_t>") Uint32List

exception UnusedError {
  1: string message;
} (message = "message")

exception HardError {
  1: string errortext;
  2: i32 code;
} (message = "errortext")

exception UnfriendlyError {
  1: string errortext;
  2: i32 code;
}

enum Color {
  red = 0,
  blue = 1,
  green = 2,
}

exception SimpleError {
  1: Color color;
}

enum Perm {
  read = 4,
  write = 2,
  execute = 1,
} (py3.flags)

enum Kind {
  None = 0,
  REGULAR = 8,
  LINK = 10,
  DIR = 4,
  FIFO = 1,
  CHAR = 2,
  BLOCK = 6,
  SOCK = 12 (cpp.name = "SOCKET"),
} (cpp.name = "Kind_")

enum BadMembers {
  name = 1 (py3.name = "name_"),
  value = 2 (py3.name = "value_"),
}

enum EmptyEnum {
}

union EmptyUnion {}

struct StringBucket {
  1: string one;
  2: optional string two;
  3: optional string three;
  4: optional string four;
  5: optional string five;
  6: optional string six;
  7: optional string seven;
  8: optional string eight;
  9: optional string nine;
  10: optional string ten;
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

struct Digits {
  1: optional list<Integers> data;
}

union Integers {
  1: byte tiny;
  2: i16 small;
  3: i32 medium;
  4: i64 large;
  5: string unbounded;
  6: string name (py3.name = "name_");
  7: Digits digits (cpp.ref = "True");
}

union ValueOrError {
  1: File value (py3.name = "value_");
  3: HardError error;
}

struct easy {
  3: optional string name;
  1: i32 val;
  2: I32List val_list;
  4: Integers an_int;
  5: i64 py3_hidden (py3.hidden);
} (anno1 = "foo", bar)

struct PrivateCppRefField {
  # (cpp.experimental.lazy) field is always private
  1: optional easy field1 (cpp.ref, cpp.experimental.lazy);
  2: optional easy field2 (cpp.ref_type = "shared", cpp.experimental.lazy);
  3: optional easy field3 (
    cpp.ref_type = "shared_const",
    cpp.experimental.lazy,
  );
}

struct Nested3 {
  1: easy c;
}

struct Nested2 {
  1: Nested3 b;
}

struct Nested1 {
  1: Nested2 a;
}

struct Optionals {
  1: optional list<string> values;
}

union ComplexUnion {
  1: Color color;
  2: easy easy_struct;
  3: Integers union_val;
  4: float float_val;
  5: double double_val;
  6: i16 tiny;
  7: i64 large;
  9: string text;
  10: binary raw;
  11: bool truthy;
}

union IOBufUnion {
  1: binary (cpp2.type = "folly::IOBuf", py3.iobuf) buf;
} (cpp.noncomparable)

struct hard {
  1: i32 val;
  2: I32List val_list;
  3: string name;
  4: Integers an_int;
  5: string other = "some default";
}

struct Runtime {
  1: bool bool_val;
  2: Color enum_val;
  3: list<i64> int_list_val;
}

struct mixed {
  1: optional string opt_field = "optional";
  3: string unq_field = "unqualified";
  4: optional easy opt_easy_ref (cpp.ref = "True");
  6: list<string> const_container_ref (cpp.ref_type = "shared_const");
  7: optional string some_field (py3.name = "some_field_");
}

struct numerical {
  1: i32 int_val;
  2: double float_val;
  5: I32List int_list;
  6: list<double> float_list;
  7: i64 i64_val;
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

typedef list<i32> (cpp.type = "std::deque<int>") list_typedef
typedef set<i32> (cpp.type = "std::unordered_set<int>") set_typedef
typedef map<i32, i32> (
  cpp.type = "std::unordered_map<int,
    // comments
    int /* inline comments */>",
) map_typedef
typedef string (cpp.type = "folly::fbstring") string_typedef

struct customized {
  1: list<i32> (cpp.template = "std::deque") list_template;
  2: set<i32> (cpp.template = "std::unordered_set") set_template;
  3: map<i32, i32> (cpp.template = "std::unordered_map") map_template;
  4: list_typedef list_type;
  5: set_typedef set_type;
  6: map_typedef map_type;
  7: string_typedef string_type;
  8: i32 foo (cpp.name = "bar");
  9: list<i32 (cpp.type = "uint32_t")> list_of_uint32;
}

struct Reserved {
  1: string from (py3.name = "from_"); // named with a python keyword (which is not a C++ keyword)
  2: i32 nonlocal (py3.name = "nonlocal_"); // ditto
  3: string ok; // not a keyword
  4: bool cpdef (py3.name = 'is_cpdef');
  5: string move; // not a keyword
  6: string inst; // not a keyword
  7: string changes; // not a keyword
}

union ReservedUnion {
  1: string from (py3.name = "from_");
  2: i32 nonlocal (py3.name = "nonlocal_");
  3: string ok;
}

struct EdgeCaseStruct {
  1: map<Reserved, list<i64>> reservedValues;
}

struct SlowCompare {
  1: string field1;
  2: i32 field2;
  3: Color field3;
} (cpp.noncomparable)

struct NonCopyable {
  1: i64 num;
} (cpp.noncopyable)

struct Messy {
  1: optional string opt_field (some = "annotation", a.b.c = "d.e.f");
  3: string unq_field = "xyzzy";
  4: Runtime struct_field = {
    "bool_val": true,
    "enum_val": Color.blue,
    "int_list_val": [10, 20, 30],
  };
}

struct ComplexRef {
  1: string name;
  2: optional ComplexRef ref (cpp2.ref = "true");
  3: optional list<i16> list_basetype_ref (cpp2.ref = "true");
  4: optional list<ComplexRef> list_recursive_ref (cpp2.ref = "true");
  5: optional set<i16> set_basetype_ref (cpp2.ref = "true");
  6: optional set<ComplexRef> set_recursive_ref (cpp2.ref = "true");
  7: optional map<i16, i16> map_basetype_ref (cpp2.ref = "true");
  8: optional map<i16, ComplexRef> map_recursive_ref (cpp2.ref = "true");
  9: optional list<ComplexRef> list_shared_ref (cpp2.ref_type = "shared");
  10: optional set<ComplexRef> set_const_shared_ref (
    cpp2.ref_type = "shared_const",
  );
  11: optional ComplexRef recursive (thrift.box);
}

struct Complex {
  1: bool val_bool;
  2: i32 val_i32;
  3: i64 val_i64;
  4: string val_string;
  5: binary val_binary;
  6: binary (py3.iobuf) val_iobuf;
  7: Color val_enum;
  8: ComplexUnion val_union;
  9: list<i64> val_list;
  // @lint-ignore THRIFTCHECKS
  10: optional set<easy> val_set;
  11: map<string, binary> val_map;
  // @lint-ignore THRIFTCHECKS
  12: optional map<string, list<set<easy>>> val_complex_map;
  13: ColorGroups val_struct_with_containers;
}

struct ListTypes {
  1: list<string> first;
  2: list<i32> second;
  3: list<list<i32>> third;
  4: list<set<i32>> fourth;
  5: list<map<i32, i32>> fifth;
}

struct StructuredAnnotation {
  2: map<double, i64> first;
  3: i64 second;
  4: list<string> third;
  5: StructuredAnnotation recurse (cpp.ref = "True");
}

@StructuredAnnotation{
  first = {1.1: 2},
  second = 3,
  third = ["a", "Multi
    'line'
  "],
  recurse = StructuredAnnotation{third = ["3", "4"]},
}
@Messy
service TestingService {
  readonly string getName();
  string getMethodName();
  string getRequestId();
  float getRequestTimeout();
  oneway void shutdown();
  idempotent bool invert(1: bool value);
  i32 complex_action(
    1: string first,
    2: string second,
    @StructuredAnnotation{second = 42}
    3: i64 third,
    4: string fourth (iv = "4"),
  );
  void takes_a_list(1: I32List ints) throws (1: SimpleError e);
  void take_it_easy(1: i32 how, 2: easy what) (a = "b.c.d");
  void pick_a_color(1: Color color);
  void int_sizes(1: byte one, 2: i16 two, 3: i32 three, 4: i64 four);

  void hard_error(1: bool valid) throws (1: HardError e);
  bool renamed_func(1: bool ret) (cpp.name = "renamed_func_in_cpp");
  i32 getPriority();
} (fun_times = "yes", single_quote = "'", double_quotes = '"""')

service TestingServiceChild extends TestingService {
  stream<i32> stream_func();
} (cpp.name = "TestingServiceChildRenamed")

struct ListNode {
  1: i32 value;

  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ListNode next;
}

struct IOBufListStruct {
  1: list<binary (py3.iobuf)> iobufs;
}

service ClientMetadataTestingService {
  string getAgent();
  string getHostname();
  string getMetadaField(1: string key);
}
