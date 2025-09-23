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

namespace android test.fixtures.adapter
namespace java test.fixtures.adapter
namespace java.swift test.fixtures.adapter

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/python.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/annotation/scope.thrift"
include "thrift/annotation/hack.thrift"
include "thrift/annotation/java.thrift"
include "thrift/annotation/rust.thrift"

@thrift.Experimental
package "facebook.com/thrift/test"

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
@rust.Adapter{name = "::my::Adapter2"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter2",
  typeClassName = "com.facebook.thrift.my.AdaptedSet_2",
}
typedef set<string> (py.adapter = 'my.Adapter2') SetWithAdapter
@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter1",
  typeClassName = "com.facebook.thrift.my.AdaptedString_1",
}
typedef string (py.adapter = 'my.Adapter1') StringWithAdapter
@thrift.AllowLegacyTypedefUri
typedef list<StringWithAdapter> ListWithElemAdapter
@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
@rust.Adapter{name = "::my::Adapter2"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter2",
  typeClassName = "com.facebook.thrift.my.AdaptedList_2",
}
typedef ListWithElemAdapter ListWithElemAdapter_withAdapter

enum Color {
  UNKNOWN = 0,
  RED = 1,
  GREEN = 2,
  BLUE = 3,
}

@python.Adapter{
  name = "my.module.Adapter2",
  typeHint = "my.another.module.AdaptedType2[]",
}
@scope.Transitive
struct MyAnnotation {
  1: string signature;
  2: Color color = Color.RED;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::my::Adapter1"}
@rust.Adapter{name = "::my::Adapter1"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter1",
  typeClassName = "com.facebook.thrift.my.AdaptedI64_1",
}
@MyAnnotation{signature = "MyI64", color = Color.GREEN}
typedef i64 MyI64

@thrift.AllowLegacyTypedefUri
typedef MyI64 DoubleTypedefI64

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter1",
  typeClassName = "com.facebook.thrift.my.AdaptedI32_1",
}
typedef i32 MyI32

@cpp.EnableCustomTypeOrdering
struct Foo {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_5137_1",
  }
  1: i32_5137 intField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_5137_1",
  }
  2: optional i32_5137 optionalIntField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_5137_1",
  }
  3: i32_5137 intFieldWithDefault = 13;
  4: SetWithAdapter setField;
  5: optional SetWithAdapter optionalSetField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter3",
    typeClassName = "com.facebook.thrift.my.AdaptedMap_string_ListWithElemAdapter_withAdapter_8454_3",
  }
  6: map_string_ListWithElemAdapter_withAdapter_8454 mapField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter3",
    typeClassName = "com.facebook.thrift.my.AdaptedMap_string_ListWithElemAdapter_withAdapter_8454_3",
  }
  7: optional map_string_ListWithElemAdapter_withAdapter_8454 optionalMapField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter3",
    typeClassName = "com.facebook.thrift.my.AdaptedBinary_5673_3",
  }
  8: binary_5673 binaryField;
  9: MyI64 longField;
  @cpp.Adapter{name = "::my::Adapter2"}
  @python.Adapter{name = "my.Adapter3", typeHint = "my.AdaptedType3[]"}
  @rust.Adapter{name = "::my::Adapter2"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter2",
    typeClassName = "com.facebook.thrift.my.AdaptedMyI64_2",
  }
  10: MyI64 adaptedLongField;
  11: DoubleTypedefI64 doubleAdaptedField;
  @python.Adapter{name = "my.ListAdapter", typeHint = "typing.Sequence[int]"}
  12: list<i32> adapted_list;
  @python.Adapter{name = "my.SetAdapter", typeHint = "typing.AbstractSet[int]"}
  13: set<i32> adapted_set;
  @python.Adapter{name = "my.MapAdapter", typeHint = "typing.Mapping[str, int]"}
  14: map<string, i32> adapted_map;
  @python.Adapter{
    name = "thrift.python.test.adapters.atoi.ItoaNestedListAdapter",
    typeHint = "typing.Sequence[typing.Sequence[typing.Mapping[int, int]]]",
  }
  15: list<list<map<i32, i32>>> adapted_list_nested;
} (
  thrift.uri = "facebook.com/thrift/compiler/test/fixtures/adapter/src/module/Foo",
)

@cpp.EnableCustomTypeOrdering
union Baz {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_5137_1",
  }
  1: i32_5137 intField;
  4: SetWithAdapter setField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter3",
    typeClassName = "com.facebook.thrift.my.AdaptedMap_string_ListWithElemAdapter_withAdapter_8454_3",
  }
  6: map_string_ListWithElemAdapter_withAdapter_8454 mapField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedBinary_5673_1",
  }
  8: binary_5673 binaryField;
  9: MyI64 longField;
}

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
@java.Adapter{
  adapterClassName = "com.facebook.thrift.my.Adapter1",
  typeClassName = "com.facebook.thrift.my.AdaptedFoo_1",
}
typedef Foo FooWithAdapter

typedef list<Foo> ListOfFooTypedef

struct Bar {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1', adaptedType = '::my::Cpp::Type1'}
  1: Foo_6868 structField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  2: optional Foo_3943 optionalStructField;
  3: list<FooWithAdapter_9317> structListField;
  4: optional list<FooWithAdapter_9317> optionalStructListField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  5: Baz_7352 unionField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  6: optional Baz_7352 optionalUnionField;
  7: DirectlyAdapted adaptedStructField;
  8: ListOfFooTypedef structListFieldWithTypedef;
}

@cpp.Adapter{name = "::my::Adapter"}
@python.Adapter{
  name = "my.module.Adapter",
  typeHint = "my.another.module.AdaptedType",
}
struct DirectlyAdapted {
  1: i32 field;
}

@cpp.Adapter{name = "::my::Adapter", adaptedType = "::my::Type"}
struct IndependentDirectlyAdapted {
  1: i32 field;
}

struct StructWithFieldAdapter {
  @cpp.Adapter{name = "::my::Adapter1"}
  @python.Adapter{name = "my.Adapter1", typeHint = "my.AdaptedType1"}
  @rust.Adapter{name = "::my::Adapter1<>"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_1",
  }
  1: i32 field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  2: i32 shared_field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @rust.Adapter{name = "::my::Adapter1<>"}
  @java.Adapter{
    adapterClassName = "com.facebook.thrift.my.Adapter1",
    typeClassName = "com.facebook.thrift.my.AdaptedI32_1",
  }
  3: optional i32 opt_shared_field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.Box
  4: optional i32 opt_boxed_field;
}

struct TerseAdaptedFields {
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.TerseWrite
  1: i32 int_field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.TerseWrite
  2: string string_field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @thrift.TerseWrite
  3: set<i32> set_field;
}

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
typedef Bar (py.adapter = 'my.Adapter2') StructWithAdapter

@thrift.AllowLegacyTypedefUri
@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
typedef Baz (py.adapter = 'my.Adapter2') UnionWithAdapter

struct B {
  1: AdaptedA a;
}
@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::my::Adapter"}
typedef A AdaptedA
struct A {}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::my::Adapter2"}
typedef string StringWithCppAdapter

service Service {
  MyI32_4873 func(
    1: StringWithAdapter_7208 arg1,
    2: StringWithCppAdapter arg2,
    3: Foo arg3,
  );
}

@cpp.Adapter{name = "MyVarAdapter"}
@python.Adapter{name = "my.ConfigAdapter", typeHint = "my.ConfiguredVar[]"}
@scope.Transitive
@thrift.Experimental
struct Config {
  1: string path;
}

@Config{path = "foo"}
const i32 var1 = 10;

@Config{path = "bar"}
const string var2 = "20";

@cpp.EnableCustomTypeOrdering
struct MyStruct {
  1: i32 field;
  2: SetWithAdapter set_string;
}

@Config{path = "baz"}
const MyStruct var3 = MyStruct{field = 30, set_string = ["10", "20"]};

@Config{path = "foo2"}
const i32 var4 = 40;

@Config{path = "bar2"}
const string var5 = "50";

@Config{path = "baz2"}
const MyStruct var6 = MyStruct{field = 60, set_string = ["30", "40"]};

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::AdaptTestMsAdapter"}
typedef i64 DurationMs

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef bool AdaptedBool

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef byte AdaptedByte

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i16 AdaptedShort

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i32 AdaptedInteger

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i64 AdaptedLong

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef double AdaptedDouble

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef string AdaptedString

@thrift.AllowLegacyTypedefUri
typedef AdaptedBool DoubleTypedefBool

@thrift.AllowLegacyTypedefUri
@cpp.Type{name = "::folly::IOBuf"}
typedef binary IOBuf
@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::CustomProtocolAdapter"}
typedef IOBuf CustomProtocolType

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::IndirectionAdapter<::apache::thrift::test::IndirectionString>",
}
typedef string IndirectionString

struct AdaptTestStruct {
  1: DurationMs delay;
  2: CustomProtocolType custom;

  @cpp.Adapter{name = "::apache::thrift::test::AdaptTestMsAdapter"}
  3: i64 timeout;

  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  4: i64 data;
  5: string meta;
  6: IndirectionString indirectionString;
  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  7: string string_data;

  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  8: AdaptedBool double_wrapped_bool;

  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  9: AdaptedInteger double_wrapped_integer;

  @cpp.Adapter{name = "::apache::thrift::test::AdapterWithContext"}
  10: binary binary_data;
}

enum ThriftAdaptedEnum {
  Zero = 0,
  One = 1,
}
@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::StaticCastAdapter<::apache::thrift::test::basic::AdaptedEnum, ::apache::thrift::test::basic::ThriftAdaptedEnum>",
}
typedef ThriftAdaptedEnum AdaptedEnum

struct AdaptTemplatedTestStruct {
  1: AdaptedBool adaptedBool;
  2: AdaptedByte adaptedByte;
  3: AdaptedShort adaptedShort;
  4: AdaptedInteger adaptedInteger;
  5: AdaptedLong adaptedLong;
  6: AdaptedDouble adaptedDouble;
  7: AdaptedString adaptedString;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  8: list<i64> adaptedList;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  9: set<i64> adaptedSet;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  10: map<i64, i64> adaptedMap;
  11: AdaptedBool adaptedBoolDefault = true;
  12: AdaptedByte adaptedByteDefault = 1;
  13: AdaptedShort adaptedShortDefault = 2;
  14: AdaptedInteger adaptedIntegerDefault = 3;
  15: AdaptedLong adaptedLongDefault = 4;
  16: AdaptedDouble adaptedDoubleDefault = 5;
  17: AdaptedString adaptedStringDefault = "6";
  18: AdaptedEnum adaptedEnum = ThriftAdaptedEnum.One;
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @rust.Adapter{name = "::fbthrift_adapters::test::TestAdapter"}
  19: list<i64> adaptedListDefault = [1];
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @rust.Adapter{name = "::fbthrift_adapters::test::TestAdapter"}
  20: set<i64> adaptedSetDefault = [1];
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  @rust.Adapter{name = "::fbthrift_adapters::test::TestAdapter"}
  21: map<i64, i64> adaptedMapDefault = {1: 1};
  22: DoubleTypedefBool doubleTypedefBool;
}

struct AdaptTemplatedNestedTestStruct {
  1: AdaptTemplatedTestStruct adaptedStruct;
}

@cpp.Name{value = "ThriftAdaptTestUnion"}
union AdaptTestUnion {
  1: DurationMs delay;
  2: CustomProtocolType custom;
}

@cpp.Name{value = "ThriftAdaptedStruct"}
struct AdaptedStruct {
  1: i64 data;
}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef AdaptedStruct AdaptedTypedef

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
struct DirectlyAdaptedStruct {
  1: i64 data;
}

@thrift.AllowLegacyTypedefUri
typedef DirectlyAdaptedStruct TypedefOfDirect

struct StructFieldAdaptedStruct {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: AdaptedStruct adaptedStruct;
  2: AdaptedTypedef adaptedTypedef;
  3: DirectlyAdaptedStruct directlyAdapted;
  4: TypedefOfDirect typedefOfAdapted;
}

struct CircularAdaptee {
  1: CircularStruct field;
}
struct CircularStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional AdaptedCircularAdaptee field;
}
@thrift.AllowLegacyTypedefUri
@cpp.Adapter{
  name = "::apache::thrift::test::MemberAccessAdapter",
  adaptedType = "::apache::thrift::test::TaggedWrapper<CircularAdaptee, CircularStruct>",
}
typedef CircularAdaptee AdaptedCircularAdaptee

struct ReorderedStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: DeclaredAfterStruct reordered_dependent_adapted;
}
@cpp.Adapter{
  name = "::apache::thrift::test::IdentityAdapter<detail::DeclaredAfterStruct>",
}
struct DeclaredAfterStruct {}

@cpp.Adapter{
  name = "::apache::thrift::test::TemplatedTestAdapter",
  underlyingName = "UnderlyingRenamedStruct",
}
struct RenamedStruct {
  1: i64 data;
}

@cpp.Adapter{
  name = "::apache::thrift::test::TemplatedTestAdapter",
  underlyingName = "UnderlyingSameNamespaceStruct",
  extraNamespace = "",
}
struct SameNamespaceStruct {
  1: i64 data;
}

@cpp.Adapter{name = "::apache::thrift::test::MoveOnlyAdapter", moveOnly = true}
struct HeapAllocated {}

struct MoveOnly {
  1: HeapAllocated ptr;
}

struct AlsoMoveOnly {
  @cpp.Adapter{
    name = "::apache::thrift::test::MoveOnlyAdapter",
    moveOnly = true,
  }
  1: i64 ptr;
}

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
@scope.Transitive
struct ApplyAdapter {}

@ApplyAdapter
struct TransitiveAdapted {}

@thrift.AllowLegacyTypedefUri
@cpp.Adapter{name = "::apache::thrift::test::CountingAdapter<true, int>"}
typedef i64 CountingInt
struct CountingStruct {
  @cpp.Adapter{name = "::apache::thrift::test::CountingAdapter<false, int>"}
  1: optional i64 regularInt;
  2: optional CountingInt countingInt;
  @cpp.Adapter{
    name = "::apache::thrift::test::CountingAdapter<false, std::string>",
  }
  3: optional string regularString;
}

service AdapterService {
  CountingStruct count();
  HeapAllocated adaptedTypes(1: HeapAllocated arg);
}

@cpp.Adapter{name = "::apache::thrift::test::VariableAdapter"}
@scope.Transitive
struct Person {
  1: string name;
}

@Person{name = "Foo"}
const i32 timeout = 42;

@Person{name = "Bar"}
const string msg = "hello, world";

struct Person2 {
  1: string name;
}

@Person{name = "NameFromAnnotation"}
const Person2 person = Person2{name = "DefaultName"};

@cpp.Adapter{name = "::apache::thrift::test::VariableAdapter"}
const i32 timeout_no_transitive = 420;

@cpp.Adapter{name = "::apache::thrift::test::VariableAdapter"}
const string msg_no_transitive = "hello, world 2";

@cpp.Adapter{name = "::apache::thrift::test::VariableAdapter"}
const Person2 person_no_transitive = Person2{name = "DefaultName 2"};

const AdaptedBool type_adapted = true;

const MoveOnly nested_adapted = {"ptr": {}};

const list<AdaptedByte> container_of_adapted = [1, 2, 3];

// The following were automatically generated and may benefit from renaming.
@thrift.AllowLegacyTypedefUri
typedef FooWithAdapter (py.adapter = "my.Adapter1") FooWithAdapter_9317
@thrift.AllowLegacyTypedefUri
typedef ListWithElemAdapter_withAdapter (
  py.adapter = "my.Adapter2",
) ListWithElemAdapter_withAdapter_2312
@thrift.AllowLegacyTypedefUri
typedef MyI32 (py.adapter = "my.Adapter1") MyI32_4873
@thrift.AllowLegacyTypedefUri
typedef StringWithAdapter (py.adapter = "my.Adapter2") StringWithAdapter_7208

// The following were automatically generated and may benefit from renaming.
@thrift.AllowLegacyTypedefUri
typedef Baz (py.adapter = "my.Adapter1") Baz_7352
@thrift.AllowLegacyTypedefUri
typedef Foo (py.adapter = "my.Adapter1") Foo_3943
@thrift.AllowLegacyTypedefUri
typedef Foo (py.adapter = "::my.Adapter1") Foo_6868
@thrift.AllowLegacyTypedefUri
typedef binary (py.adapter = "my.Adapter1") binary_5673
@thrift.AllowLegacyTypedefUri
typedef i32 (py.adapter = "my.Adapter1") i32_5137
@thrift.AllowLegacyTypedefUri
typedef map<string, ListWithElemAdapter_withAdapter_2312> (
  py.adapter = "my.Adapter3",
) map_string_ListWithElemAdapter_withAdapter_8454

@cpp.Adapter{
  name = "::apache::thrift::test::StructAdapter",
  underlyingName = "Renamed",
}
struct RenamedStructWithStructAdapterAndFieldAdapter {
  @cpp.Adapter{name = "::apache::thrift::test::FieldAdapter"}
  1: i32 field;
}
