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

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/python.thrift"
include "thrift/lib/python/test/dependency.thrift"
include "thrift/lib/python/test/sub_dependency.thrift"
include "thrift/lib/python/test/base_service_only.thrift"

cpp_include "<deque>"
cpp_include "folly/container/F14Map.h"
cpp_include "folly/FBString.h"
cpp_include "folly/io/IOBuf.h"
cpp_include "thrift/test/AdapterTest.h"

package "facebook.com/testing"

namespace py3 ""
namespace cpp2 cpp2

const list<i16> int_list = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

const list<string> unicode_list = ["Bulgaria", "Benin", "Saint Barthélemy"];
const list<binary> binary_list = ["Saint Barthélemy"];

const map<i16, map<i16, i16>> LocationMap = {1: {1: 1}};

const sub_dependency.IncludedColour RedColour = sub_dependency.IncludedColour.red;
const sub_dependency.IncludedColour BlueColour = sub_dependency.IncludedColour.blue;

struct LatLon {
  1: double lat = 51.4769;
  2: double lon = 0.0005;
}

const map<i16, list<LatLon>> CoolPlaces = {
  31: [{lat: 51.4769, lon: 0.0005}, {lat: 37.484983, lon: -122.1481479}],
  47: [
    {lat: 35.1085106, lon: -106.5354721},
    {lat: 35.1955072, lon: -106.4336690},
  ],
};

struct DefaultedFields {
  1: map<i16, list<LatLon>> location_map = CoolPlaces;
  2: list<i16> int_list = int_list;
  3: set<string> unicode_set = ["Bulgaria", "Benin", "Saint Barthélemy"];
}

const dependency.IncludedStruct FANCY_CONST = dependency.IncludedStruct{
  val = sub_dependency.Basic{nom = "fancy", val = 47, bin = "01010101"},
  color = RedColour,
  color_list = [RedColour, BlueColour],
  color_set = [RedColour, BlueColour],
  color_map = {
    RedColour: sub_dependency.Basic{},
    BlueColour: sub_dependency.Basic{nom = "b"},
  },
};

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

@cpp.Type{name = "uint32_t"}
typedef i32 ui32
@cpp.Type{name = "folly::F14FastMap<std::string, folly::fbstring>"}
typedef map<string, string> F14MapFollyString
@cpp.Type{name = "std::vector<uint32_t>"}
typedef list<i32> Uint32List

@cpp.Adapter{name = "::apache::thrift::test::FBStringAdapter"}
@python.Py3EnableCppAdapter
typedef binary AdaptedBinary
@cpp.Adapter{name = "::apache::thrift::test::FBStringAdapter"}
@python.Py3EnableCppAdapter
typedef string AdaptedString
typedef list<AdaptedString> AdaptedStringList
typedef set<AdaptedString> AdaptedStringSet
typedef map<AdaptedString, string> AdaptedStringKeyMap
typedef map<string, AdaptedString> AdaptedStringValueMap
typedef map<AdaptedString, AdaptedString> AdaptedStringMap
@cpp.Adapter{name = "::apache::thrift::test::FBVectorAdapter"}
@python.Py3EnableCppAdapter
typedef list<i32> AdaptedList
@cpp.Adapter{name = "::apache::thrift::test::F14FastSetAdapter"}
@python.Py3EnableCppAdapter
typedef set<i32> AdaptedSet
@cpp.Adapter{name = "::apache::thrift::test::F14FastMapAdapter"}
@python.Py3EnableCppAdapter
typedef map<i32, i32> AdaptedMap

@python.MigrationBlockingAllowInheritance
exception UnusedError {
  @thrift.ExceptionMessage
  1: string message;
}

exception HardError {
  @thrift.ExceptionMessage
  1: string errortext;
  2: i32 code;
}

exception NestedHardError {
  1: HardError error;
}

exception UnfriendlyError {
  1: string errortext;
  2: i32 code;
}

struct NestedError {
  1: HardError val_error;
}

enum Color {
  red = 0,
  blue = 1,
  green = 2,
}

exception SimpleError {
  1: Color color;
}

@python.Flags{}
enum Perm {
  read = 4,
  write = 2,
  execute = 1,
}

@cpp.Name{value = "Kind_"}
enum Kind {
  None = 0,
  REGULAR = 8,
  LINK = 10,
  DIR = 4,
  FIFO = 1,
  CHAR = 2,
  BLOCK = 6,
  @cpp.Name{value = "SOCKET"}
  SOCK = 12,
}

enum BadMembers {
  @python.Name{name = "name_"}
  name = 1,
  @python.Name{name = "value_"}
  value = 2,
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

@python.MigrationBlockingAllowInheritance
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
  @python.Name{name = "name_"}
  6: string name;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  7: Digits digits;
}

union ValueOrError {
  @python.Name{name = "value_"}
  1: File value;
  3: HardError error;
}

struct easy {
  3: optional string name;
  1: i32 val;
  2: I32List val_list;
  4: Integers an_int;
  @python.Py3Hidden{}
  5: i64 py3_hidden;
} (anno1 = "foo", bar)

struct PrivateCppRefField {
  # (cpp.experimental.lazy) field is always private
  @cpp.Lazy
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional easy field1;
  @cpp.Lazy
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional easy field2;
  @cpp.Lazy
  @cpp.Ref{type = cpp.RefType.Shared}
  3: optional easy field3;
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
  12: list<float> float_list;
  // @lint-ignore THRIFTCHECKS
  13: set<float> float_set;
  // @lint-ignore THRIFTCHECKS
  14: map<float, float> float_map;
}

union IOBufUnion {
  @cpp.Type{name = "folly::IOBuf"}
  1: binary buf;
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
  6: string property;
  3: list<i64> int_list_val;
}

struct mixed {
  // @lint-ignore THRIFTCHECKS
  1: optional string opt_field = "optional";
  3: string unq_field = "unqualified";
  // @lint-ignore THRIFTCHECKS
  @cpp.Ref{type = cpp.RefType.Unique}
  4: optional easy opt_easy_ref;
  // @lint-ignore THRIFTCHECKS
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  6: list<string> const_container_ref;
  @python.Name{name = "some_field_"}
  7: optional string some_field;
  // @lint-ignore THRIFTCHECKS
  8: optional float opt_float = 1.0;
  // @lint-ignore THRIFTCHECKS
  9: optional i32 opt_int = 1;
  // @lint-ignore THRIFTCHECKS
  10: optional Color opt_enum = Color.red;
  11: optional string opt_pointless_default_str = "";
  12: optional i64 opt_pointless_default_int = 0;
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

@cpp.Type{name = "std::deque<int>"}
typedef list<i32> list_typedef
@cpp.Type{name = "std::unordered_set<int>"}
typedef set<i32> set_typedef
@cpp.Type{
  name = "std::unordered_map<int,
    // comments
    int /* inline comments */>",
}
typedef map<i32, i32> map_typedef
@cpp.Type{name = "folly::fbstring"}
typedef string string_typedef

@cpp.EnableCustomTypeOrdering
struct customized {
  @cpp.Type{template = "std::deque"}
  1: list<i32> list_template;
  @cpp.Type{template = "std::unordered_set"}
  2: set<i32> set_template;
  @cpp.Type{template = "std::unordered_map"}
  3: map<i32, i32> map_template;
  4: list_typedef list_type;
  5: set_typedef set_type;
  6: map_typedef map_type;
  7: string_typedef string_type;
  @cpp.Name{value = "bar"}
  8: i32 foo;
  9: list<ui32> list_of_uint32;

  10: AdaptedBinary adapted_binary;
  11: AdaptedString adapted_string;
  12: AdaptedStringList adapted_string_list;
  13: AdaptedStringSet adapted_string_set;
  14: AdaptedStringKeyMap adapted_string_key_map;
  15: AdaptedStringValueMap adapted_string_value_map;
  16: AdaptedStringMap adapted_string_map;
  17: AdaptedList adapted_list;
  18: AdaptedSet adapted_set;
  19: AdaptedMap adapted_map;
}

struct Reserved {
  @python.Name{name = "from_"}
  1: string from; // named with a python keyword (which is not a C++ keyword)
  @python.Name{name = "nonlocal_"}
  2: i32 nonlocal; // ditto
  3: string ok; // not a keyword
  @python.Name{name = "is_cpdef"}
  4: bool cpdef;
  5: string move; // not a keyword
  6: string inst; // not a keyword
  7: string changes; // not a keyword
  8: string __mangled_str;
  9: i64 __mangled_int;
}

struct _Reserved {
  1: string __mangled_str;
  2: i64 __mangled_int;
}

union ReservedUnion {
  @python.Name{name = "from_"}
  1: string from;
  @python.Name{name = "nonlocal_"}
  2: i32 nonlocal;
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

struct SortedSets {
  @python.DeprecatedSortSetOnSerialize
  1: set<i32> ints;
  @python.DeprecatedSortSetOnSerialize
  2: set<string> strings;
  // @lint-ignore THRIFTCHECKS
  @python.DeprecatedSortSetOnSerialize
  3: set<easy> easies;
  @python.DeprecatedSortSetOnSerialize
  4: set<Color> colors;
}

struct SortedMaps {
  @python.DeprecatedKeySortMapOnSerialize
  1: map<i32, i32> ints;
  @python.DeprecatedKeySortMapOnSerialize
  2: map<string, string> strings;
  // @lint-ignore THRIFTCHECKS
  @python.DeprecatedKeySortMapOnSerialize
  3: map<easy, easy> easies;
  @python.DeprecatedSortSetOnSerialize
  4: map<Color, Color> colors;
}

struct ComplexRef {
  1: string name;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ComplexRef ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  3: optional list<i16> list_basetype_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  4: optional list<ComplexRef> list_recursive_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional set<i16> set_basetype_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  6: optional set<ComplexRef> set_recursive_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  7: optional map<i16, i16> map_basetype_ref;
  @cpp.Ref{type = cpp.RefType.Unique}
  8: optional map<i16, ComplexRef> map_recursive_ref;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  9: optional list<ComplexRef> list_shared_ref;
  @cpp.Ref{type = cpp.RefType.Shared}
  10: optional set<ComplexRef> set_const_shared_ref;
  @thrift.Box
  11: optional ComplexRef recursive;
}

struct Complex {
  1: bool val_bool;
  2: i32 val_i32;
  3: i64 val_i64;
  4: string val_string;
  5: binary val_binary;
  @cpp.Type{name = "folly::IOBuf"}
  6: binary val_iobuf;
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

struct SetTypes {
  1: set<string> first;
  2: set<i32> second;
}

struct StructuredAnnotation {
  2: map<double, i64> first;
  3: i64 second;
  4: list<string> third;
  @cpp.Ref{type = cpp.RefType.Unique}
  5: optional StructuredAnnotation recurse;
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
  @cpp.Name{value = "renamed_func_in_cpp"}
  bool renamed_func(1: bool ret);
  i32 getPriority();
} (fun_times = "yes", single_quote = "'", double_quotes = '"""')

@cpp.Name{value = "TestingServiceChildRenamed"}
service TestingServiceChild extends TestingService {
  stream<i32> stream_func();
}

service ExtendServiceWithNoTypes extends base_service_only.BaseService {
  void do_nothing();
}

struct ListNode {
  1: i32 value;

  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional ListNode next;
}

union Misordered {
  4: i32 val32;
  3: string s1;
  2: i64 val64;
  1: string s2;
}

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf

@python.Py3Hidden
struct IOBufListStruct {
  1: list<IOBuf> iobufs;
} (cpp.noncomparable)

// StructOrder* should have same fields when sorted in key order.
// They exist to test whether deserialize is insensitive to declartion order.
struct StructOrderRandom {
  3: bool c;
  4: bool d;
  1: i64 a;
  2: string b;
}

struct StructOrderSorted {
  1: i64 a;
  2: string b;
  3: bool c;
  4: bool d;
}

service ClientMetadataTestingService {
  string getAgent();
  string getHostname();
  string getMetadaField(1: string key);
}

union _UnderscoreUnion {
  1: string _a;
  2: i64 _b;
}

struct EmptyStruct {}
exception EmptyError {}

@python.DisableFieldCache
struct StructDisabledFieldCache {
  1: i32 int_field;
  2: string str_field;
  3: bool bool_field;
  4: map<string, i32> map_field;
  5: set<string> set_field;
  6: list<i32> list_field;
  7: list<list<i32>> list_of_list_field;
  #nested struct
  8: EmptyStruct empty_struct_field;
  9: easy easy_field;
}
