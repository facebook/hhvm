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

include "includes.thrift"
include "thrift/annotation/cpp.thrift"
cpp_include "<folly/small_vector.h>"

namespace cpp2 some.valid.ns

typedef includes.AStruct AStruct

@cpp.Adapter{name = '::CustomProtocolAdapter'}
typedef binary (cpp.type = "::folly::IOBuf") CustomProtocolType

// Generate base consts
const bool aBool = true;
const byte aByte = 1;
const i16 a16BitInt = 12;
const i32 a32BitInt = 123;
const i64 a64BitInt = 1234;
const float aFloat = 0.1;
const double aDouble = 0.12;
const string aString = "Joe Doe";
const list<bool> aList = [true, false];
const map<string, i32> anEmptyMap = {};
const map<i32, string> aMap = {1: "foo", 2: "bar"};
const set<string> aSet = ["foo", "bar"];
const list<list<i32>> aListOfLists = [[1, 3, 5, 7, 9], [2, 4, 8, 10, 12]];
const list<map<string, i32>> states = [
  {"San Diego": 3211000, "Sacramento": 479600, "SF": 837400},
  {"New York": 8406000, "Albany": 98400},
];

enum MyEnumA {
  fieldA = 1,
  fieldB = 2,
  fieldC = 4,
}

const list<MyEnumA> AConstList = [1, 2, 3];

const i32 AnIntegerEnum2 = MyEnumA.fieldB;

const list<i32> AnIntegerEnum2 = [MyEnumA.fieldB, MyEnumA.fieldA];

@cpp.EnumType{type = cpp.EnumUnderlyingType.U32}
enum AnnotatedEnum {
  FIELDA = 2,
  FIELDB = 4,
  FIELDC = 9,
} (cpp2.declare_bitwise_ops)

@cpp.EnumType{type = cpp.EnumUnderlyingType.I16}
enum AnnotatedEnum2 {
  FIELDA = 2,
  FIELDB = 4,
  FIELDC = 9,
} (cpp.declare_bitwise_ops)

const MyEnumA constEnumA = MyEnumA.fieldB;

const MyEnumA constEnumB = 3;

struct Empty {}

struct ASimpleStruct {
  1: i64 boolField;
} (no_default_comparators)

struct ASimpleStructNoexcept {
  1: i64 boolField;
}

struct MyStruct {
  1: bool MyBoolField;
  2: i64 MyIntField = 12;
  3: string MyStringField = "test";
  4: string MyStringField2;
  5: binary MyBinaryField;
  6: optional binary MyBinaryField2;
  7: required binary MyBinaryField3;
  8: list<binary> MyBinaryListField4;
  9: map<MyEnumA, string> MyMapEnumAndInt = {
    1: "fieldA",
    4: "fieldC",
    9: "nothing",
  };
  10: CustomProtocolType MyCustomField;
  11: optional CustomProtocolType MyOptCustomField;
}

union SimpleUnion {
  7: i64 intValue;
  2: string stringValue;
} (cpp.virtual)

typedef i32 simpleTypeDef
typedef map<i16, string> containerTypeDef
typedef list<map<i16, string>> complexContainerTypeDef
typedef set<SimpleUnion> unionTypeDef
typedef list<MyStruct> structTypeDef
typedef list<map<Empty, MyStruct>> complexStructTypeDef
typedef list<complexStructTypeDef> mostComplexTypeDef

union ComplexUnion {
  1: i64 intValue;
  201: i64 opt_intValue;
  3: string stringValue;
  203: string opt_stringValue;
  4: i16 intValue2;
  6: i32 intValue3;
  7: double doubelValue;
  8: bool boolValue;
  9: list<i32> union_list;
  10: set<i64> union_set;
  11: map<string, i32> union_map;
  211: map<string, i32> opt_union_map;
  12: MyEnumA enum_field;
  13: list<MyEnumA> enum_container;
  14: MyStruct a_struct;
  15: set<MyStruct> a_set_struct;
  16: SimpleUnion a_union;
  216: SimpleUnion opt_a_union;
  17: list<SimpleUnion> a_union_list;
  18: unionTypeDef a_union_typedef;
  19: list<unionTypeDef> a_union_typedef_list;
  20: binary MyBinaryField;
  21: binary MyBinaryField2;
  23: list<binary> MyBinaryListField4;
  24: MyStruct ref_field (cpp.ref);
  25: MyStruct ref_field2 (cpp.ref_type = "shared_const");
  26: AnException excp_field;
  27: CustomProtocolType MyCustomField;
} (cpp.methods = "void foo(const std::string& bar) {}")

exception AnException {
  1: i32 code;
  101: required i32 req_code;
  2: string message2;
  102: required string req_message;
  3: list<i32> exception_list = [1, 2, 3];
  4: set<i64> exception_set;
  5: map<string, i32> exception_map;
  105: required map<string, i32> req_exception_map;
  6: MyEnumA enum_field;
  7: list<MyEnumA> enum_container;
  8: MyStruct a_struct;
  9: set<MyStruct> a_set_struct;
  10: list<SimpleUnion> a_union_list;
  11: unionTypeDef union_typedef;
  19: list<unionTypeDef> a_union_typedef_list;
  20: CustomProtocolType MyCustomField;
  21: optional CustomProtocolType MyOptCustomField;
} (message = "message2")

exception AnotherException {
  1: i32 code;
  101: required i32 req_code;
  2: string message;
} (cpp.virtual)

typedef i64 (cpp.type = "Foo", cpp.indirection) IndirectionA
typedef i32 (cpp.type = "Baz", cpp.indirection) IndirectionC
typedef double (cpp.type = "Bar", cpp.indirection) IndirectionB
typedef string (cpp.type = "FooBar", cpp.indirection) IndirectionD
typedef map<MyEnumA, string> (
  cpp.declare_hash,
  cpp.declare_equal_to,
) HashedTypedef

struct containerStruct {
  1: bool fieldA;
  101: required bool req_fieldA;
  201: optional bool opt_fieldA;
  2: map<string, bool> fieldB;
  102: required map<string, bool> req_fieldB;
  202: optional map<string, bool> opt_fieldB;
  3: set<i32> fieldC = [1, 2, 3, 4];
  103: required set<i32> req_fieldC = [1, 2, 3, 4];
  203: optional set<i32> opt_fieldC = [1, 2, 3, 4];
  4: string fieldD;
  5: string fieldE = "somestring";
  105: required string req_fieldE = "somestring";
  205: optional string opt_fieldE = "somestring";
  6: list<list<i32>> fieldF = aListOfLists;
  7: map<string, map<string, map<string, i32>>> fieldG;
  8: list<set<i32>> fieldH;
  9: bool fieldI = true;
  10: map<string, list<i32>> fieldJ = {
    "subfieldA": [1, 4, 8, 12],
    "subfieldB": [2, 5, 9, 13],
  };
  11: list<list<list<list<i32>>>> fieldK;
  12: set<set<set<bool>>> fieldL;
  13: map<set<list<i32>>, map<list<set<string>>, string>> fieldM;
  14: simpleTypeDef fieldN;
  15: complexStructTypeDef fieldO;
  16: list<mostComplexTypeDef> fieldP;
  17: MyEnumA fieldQ;
  18: MyEnumA fieldR = MyEnumA.fieldB;
  118: required MyEnumA req_fieldR = MyEnumA.fieldB;
  218: optional MyEnumA opt_fieldR = MyEnumA.fieldB;
  19: MyEnumA fieldS = constEnumA;
  21: list<MyEnumA> fieldT;
  22: list<MyEnumA> fieldU = [MyEnumA.fieldC, MyEnumA.fieldB, MyEnumA.fieldA];
  23: MyStruct fieldV;
  123: required MyStruct req_fieldV;
  223: optional MyStruct opt_fieldV;
  24: set<MyStruct> fieldW;
  25: ComplexUnion fieldX;
  125: required ComplexUnion req_fieldX;
  225: optional ComplexUnion opt_fieldX;
  26: list<ComplexUnion> fieldY;
  27: unionTypeDef fieldZ;
  28: list<unionTypeDef> fieldAA;
  29: map<IndirectionB, IndirectionC> fieldAB;
  30: MyEnumB fieldAC;
  31: includes.AnEnum fieldAD;
  32: map<string, i32> fieldAE = {};
  33: IndirectionD fieldSD;
} (cpp.noncopyable, cpp.methods = "void foo(const std::string& bar) {}")

enum MyEnumB {
  AField = 0,
}

struct MyIncludedStruct {
  1: includes.IncludedInt64 MyIncludedInt = includes.IncludedConstant;
  2: AStruct MyIncludedStruct;
  3: AStruct ARefField (cpp.ref = "true");
  4: required AStruct ARequiredField;
} (cpp2.declare_hash = 1, cpp2.declare_equal_to)

typedef i32 (cpp.type = "CppFakeI32") CppFakeI32
typedef list<i64> (
  cpp.type = "folly::small_vector<int64_t, 8 /* maxInline */>",
) FollySmallVectorI64
typedef set<string> (
  cpp.type = "folly::sorted_vector_set<std::string>",
) SortedVectorSetString
typedef map<i64, double> (cpp.type = "FakeMap") FakeMap
typedef map<string, containerStruct> (
  cpp.type = "std::unordered_map<std::string, containerStruct>",
) UnorderedMapStruct
typedef list<i32> (cpp.template = "std::list") std_list
typedef list<string> (cpp2.template = "std::deque") std_deque
typedef set<string> (cpp.template = "folly::sorted_vector_set") folly_set
typedef map<i64, string> (cpp2.template = "folly::sorted_vector_map") folly_map

struct AnnotatedStruct {
  1: containerStruct no_annotation;
  2: containerStruct cpp_unique_ref (cpp.ref);
  3: containerStruct cpp2_unique_ref (cpp2.ref);
  4: map<i32, list<string>> container_with_ref (cpp2.ref);
  5: required containerStruct req_cpp_unique_ref (cpp.ref);
  6: required containerStruct req_cpp2_unique_ref (cpp2.ref);
  7: required list<string> req_container_with_ref (cpp2.ref);
  8: optional containerStruct opt_cpp_unique_ref (cpp.ref);
  9: optional containerStruct opt_cpp2_unique_ref (cpp2.ref);
  10: optional set<i32> opt_container_with_ref (cpp2.ref);
  11: containerStruct ref_type_unique (cpp.ref_type = "unique");
  12: containerStruct ref_type_shared (cpp2.ref_type = "shared");
  13: map<i32, list<string>> ref_type_const (cpp2.ref_type = "shared_const");
  14: required containerStruct req_ref_type_shared (cpp.ref_type = "shared");
  15: required containerStruct req_ref_type_const (
    cpp2.ref_type = "shared_const",
  );
  16: required list<string> req_ref_type_unique (cpp2.ref_type = "unique");
  17: optional containerStruct opt_ref_type_const (
    cpp.ref_type = "shared_const",
  );
  18: optional containerStruct opt_ref_type_unique (cpp2.ref_type = "unique");
  19: optional set<i32> opt_ref_type_shared (cpp2.ref_type = "shared");
  20: CppFakeI32 base_type;
  21: FollySmallVectorI64 list_type;
  22: SortedVectorSetString set_type;
  23: FakeMap map_type;
  24: UnorderedMapStruct map_struct_type;
  25: IOBuf iobuf_type;
  26: IOBufPtr iobuf_ptr;
  27: list<i32> (cpp.template = "std::list") list_i32_template;
  28: list<string> (cpp2.template = "std::deque") list_string_template;
  29: set<string> (cpp.template = "folly::sorted_vector_set") set_template;
  30: map<i64, string> (
    cpp2.template = "folly::sorted_vector_map",
  ) map_template;
  31: std_list typedef_list_template;
  32: std_deque typedef_deque_template;
  33: folly_set typedef_set_template;
  34: folly_map typedef_map_template;
  35: IndirectionA indirection_a;
  36: list<IndirectionB> indirection_b;
  37: set<IndirectionC> indirection_c;
  38: IOBuf iobuf_type_val = "value";
  39: IOBufPtr iobuf_ptr_val = "value2";
  40: containerStruct struct_struct = {
    "fieldD": "some string",
    "fieldI": false,
  };
} (
  cpp.virtual,
  cpp.noncopyable,
  cpp.declare_hash,
  cpp.declare_equal_to = 1,
  cpp2.methods = "void foo(const std::string& bar) {}",
)

struct ComplexContainerStruct {
  1: map<string, IOBuf> map_of_iobufs;
  2: map<string, IOBufPtr> map_of_iobuf_ptrs;
}

service EmptyService {
}

service ReturnService {
  void noReturn() (thread = "eb");
  bool boolReturn();
  i16 i16Return();
  i32 i32Return();
  i64 i64Return();
  float floatReturn();
  double doubleReturn();
  string stringReturn() (thread = "eb");
  binary binaryReturn();
  map<string, i64> mapReturn();
  simpleTypeDef simpleTypedefReturn();
  complexStructTypeDef complexTypedefReturn();
  list<mostComplexTypeDef> list_mostComplexTypedefReturn();
  MyEnumA enumReturn() (thread = "eb");
  list<MyEnumA> list_EnumReturn() (thread = "eb");
  MyStruct structReturn();
  set<MyStruct> set_StructReturn();
  ComplexUnion unionReturn() (thread = "eb");
  list<ComplexUnion> list_UnionReturn();
  IOBuf readDataEb(1: i64 size) (thread = "eb");
  IOBufPtr readData(1: i64 size);
}

service ParamService {
  void void_ret_i16_param(1: i16 param1) (thread = "eb");
  void void_ret_byte_i16_param(1: byte param1, 2: i16 param2);
  void void_ret_map_param(1: map<string, i64> param1);
  void void_ret_map_setlist_param(
    1: map<string, i64> param1,
    3: set<list<string>> param2,
  );
  void void_ret_map_typedef_param(1: simpleTypeDef param1);
  void void_ret_enum_param(1: MyEnumA param1);
  void void_ret_struct_param(1: MyStruct param1);
  void void_ret_listunion_param(1: list<ComplexUnion> param1);
  bool bool_ret_i32_i64_param(1: i32 param1, 3: i64 param2);
  bool bool_ret_map_param(1: map<string, i64> param1);
  bool bool_ret_union_param(1: ComplexUnion param1);
  i64 i64_ret_float_double_param(1: float param1, 3: double param2);
  i64 i64_ret_string_typedef_param(
    1: string param1,
    3: set<mostComplexTypeDef> param2,
  );
  i64 i64_ret_i32_i32_i32_i32_i32_param(
    1: i32 param1,
    2: i32 param2,
    3: i32 param3,
    4: i32 param4,
    5: i32 param5,
  ) (thread = "eb");
  double double_ret_setstruct_param(4: set<MyStruct> param1);
  string string_ret_string_param(1: string param1);
  binary binary_ret_binary_param(1: binary param1);
  map<string, i64> map_ret_bool_param(1: bool param1);
  list<bool> list_ret_map_setlist_param(
    1: map<i32, list<string>> param1,
    2: list<string> param2,
  );
  map<
    set<list<i32>>,
    map<list<set<string>>, string>
  > mapsetlistmapliststring_ret_listlistlist_param(
    1: list<list<list<list<i32>>>> param1,
  );
  simpleTypeDef typedef_ret_i32_param(1: i32 param1);
  list<simpleTypeDef> listtypedef_ret_typedef_param(
    1: complexStructTypeDef param1,
  ) (thread = "eb");
  MyEnumA enum_ret_double_param(3: double param1);
  MyEnumA enum_ret_double_enum_param(3: double param1, 5: MyEnumA param2);
  list<MyEnumA> listenum_ret_map_param(1: map<string, i64> param1);
  MyStruct struct_ret_i16_param(1: i16 param1) (thread = "eb");
  set<MyStruct> setstruct_ret_set_param(8: set<string> param1);
  ComplexUnion union_ret_i32_i32_param(4: i32 param1, 2: i32 param2);
  list<ComplexUnion> listunion_string_param(1: string param1);
}

typedef binary (cpp2.type = "folly::IOBuf") IOBuf
typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

struct FloatStruct {
  1: float floatField;
  2: double doubleField;
}

union FloatUnion {
  1: float floatSide;
  2: double doubleSide;
}

struct AllRequiredNoExceptMoveCtrStruct {
  1: required i64 intField;
}
