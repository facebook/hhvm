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
include "thrift/annotation/rust.thrift"

@thrift.Experimental
package "facebook.com/thrift/test"

@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
@rust.Adapter{name = "::my::Adapter2"}
typedef set<string> (py.adapter = 'my.Adapter2') SetWithAdapter
@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
typedef string (py.adapter = 'my.Adapter1') StringWithAdapter
typedef list<StringWithAdapter> ListWithElemAdapter
@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
@rust.Adapter{name = "::my::Adapter2"}
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

@cpp.Adapter{name = "::my::Adapter1"}
@rust.Adapter{name = "::my::Adapter1"}
@MyAnnotation{signature = "MyI64", color = Color.GREEN}
typedef i64 MyI64

typedef MyI64 DoubleTypedefI64

@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
typedef i32 MyI32

struct Foo {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  1: i32 (py.adapter = 'my.Adapter1') intField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  2: optional i32 (py.adapter = 'my.Adapter1') optionalIntField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  3: i32 (py.adapter = 'my.Adapter1') intFieldWithDefault = 13;
  4: SetWithAdapter setField;
  5: optional SetWithAdapter optionalSetField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  6: map<string, ListWithElemAdapter_withAdapter_2312> (
    py.adapter = 'my.Adapter3',
  ) mapField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  7: optional map<string, ListWithElemAdapter_withAdapter_2312> (
    py.adapter = 'my.Adapter3',
  ) optionalMapField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  8: binary (py.adapter = 'my.Adapter1') binaryField;
  9: MyI64 longField;
  @cpp.Adapter{name = "::my::Adapter2"}
  @python.Adapter{name = "my.Adapter3", typeHint = "my.AdaptedType3[]"}
  @rust.Adapter{name = "::my::Adapter2"}
  10: MyI64 adaptedLongField;
  11: DoubleTypedefI64 doubleAdaptedField;
} (
  thrift.uri = "facebook.com/thrift/compiler/test/fixtures/adapter/src/module/Foo",
)

union Baz {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  1: i32 (py.adapter = 'my.Adapter1') intField;
  4: SetWithAdapter setField;
  @hack.Adapter{name = '\\Adapter3'}
  @cpp.Adapter{name = '::my::Adapter3'}
  @rust.Adapter{name = "::my::Adapter3"}
  6: map<string, ListWithElemAdapter_withAdapter_2312> (
    py.adapter = 'my.Adapter3',
  ) mapField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  @rust.Adapter{name = "::my::Adapter1"}
  8: binary (py.adapter = 'my.Adapter1') binaryField;
  9: MyI64 longField;
}

@hack.Adapter{name = '\\Adapter1'}
@cpp.Adapter{name = '::my::Adapter1'}
@rust.Adapter{name = "::my::Adapter1"}
typedef Foo FooWithAdapter

struct Bar {
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1', adaptedType = '::my::Cpp::Type1'}
  1: Foo (py.adapter = '::my.Adapter1') structField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  2: optional Foo (py.adapter = 'my.Adapter1') optionalStructField;
  3: list<FooWithAdapter_9317> structListField;
  4: optional list<FooWithAdapter_9317> optionalStructListField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  5: Baz (py.adapter = 'my.Adapter1') unionField;
  @hack.Adapter{name = '\\Adapter1'}
  @cpp.Adapter{name = '::my::Adapter1'}
  6: optional Baz (py.adapter = 'my.Adapter1') optionalUnionField;
  7: DirectlyAdapted adaptedStructField;
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
  1: i32 field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @cpp.Ref{type = cpp.RefType.Shared}
  2: i32 shared_field;
  @cpp.Adapter{name = "::my::Adapter1"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @rust.Adapter{name = "::my::Adapter1<>"}
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

@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
typedef Bar (py.adapter = 'my.Adapter2') StructWithAdapter

@hack.Adapter{name = '\\Adapter2'}
@cpp.Adapter{name = '::my::Adapter2'}
typedef Baz (py.adapter = 'my.Adapter2') UnionWithAdapter

struct B {
  1: AdaptedA a;
}
@cpp.Adapter{name = "::my::Adapter"}
typedef A AdaptedA
struct A {}

service Service {
  MyI32_4873 func(
    1: StringWithAdapter_7208 arg1,
    @cpp.Adapter{name = "::my::Adapter2"}
    2: string arg2,
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

@cpp.Adapter{name = "::apache::thrift::test::AdaptTestMsAdapter"}
typedef i64 DurationMs

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef bool AdaptedBool

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef byte AdaptedByte

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i16 AdaptedShort

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i32 AdaptedInteger

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef i64 AdaptedLong

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef double AdaptedDouble

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef string AdaptedString

typedef AdaptedBool DoubleTypedefBool

typedef binary (cpp.type = "::folly::IOBuf") IOBuf
@cpp.Adapter{name = "::apache::thrift::test::CustomProtocolAdapter"}
typedef IOBuf CustomProtocolType

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

union AdaptTestUnion {
  1: DurationMs delay;
  2: CustomProtocolType custom;
} (cpp.name = "ThriftAdaptTestUnion")

struct AdaptedStruct {
  1: i64 data;
} (cpp.name = "ThriftAdaptedStruct")

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef AdaptedStruct AdaptedTypedef

@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
struct DirectlyAdaptedStruct {
  1: i64 data;
}

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
@cpp.Adapter{
  name = "::apache::thrift::test::MemberAccessAdapter",
  adaptedType = "::apache::thrift::test::TaggedWrapper<CircularAdaptee, CircularStruct>",
}
typedef CircularAdaptee AdaptedCircularAdaptee

struct ReorderedStruct {
  @cpp.Ref{type = cpp.RefType.Unique}
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
typedef FooWithAdapter (py.adapter = "my.Adapter1") FooWithAdapter_9317
typedef ListWithElemAdapter_withAdapter (
  py.adapter = "my.Adapter2",
) ListWithElemAdapter_withAdapter_2312
typedef MyI32 (py.adapter = "my.Adapter1") MyI32_4873
typedef StringWithAdapter (py.adapter = "my.Adapter2") StringWithAdapter_7208
