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
include "thrift/annotation/thrift.thrift"
cpp_include "thrift/test/AdapterTest.h"
cpp_include "thrift/lib/cpp2/Adapt.h"

@thrift.Experimental
package "apache.org/thrift/test/basic"

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

@cpp.Adapter{name = "::apache::thrift::test::CustomProtocolAdapter"}
typedef binary (cpp.type = "::folly::IOBuf") CustomProtocolType

@cpp.Adapter{
  name = "::apache::thrift::IndirectionAdapter<::apache::thrift::test::IndirectionString>",
}
typedef string IndirectionString

@cpp.UseOpEncode
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

struct MyStruct {
  1: i64 field1;
}

struct ComparisonTestStruct {
  @cpp.Adapter{name = "::apache::thrift::test::NonComparableWrapperAdapter"}
  1: MyStruct non_comparable_adapted_type;

  // TODO(dokwon): Support non-comparable adapted type for @thrift.Box.
  // @cpp.Adapter{name = "::apache::thrift::test::NonComparableWrapperAdapter"}
  // @thrift.Box
  // 2: optional MyStruct box_non_comparable_adapted_type;

  @cpp.Adapter{name = "::apache::thrift::test::NonComparableWrapperAdapter"}
  @thrift.InternBox
  3: MyStruct intern_box_non_comparable_adapted_type;
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
  19: list<i64> adaptedListDefault = [1];
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  20: set<i64> adaptedSetDefault = [1];
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
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
  extraNamespace = "detail",
}
struct RenamedStruct {
  1: i64 data;
}

@cpp.Adapter{
  name = "::apache::thrift::test::TemplatedTestAdapter",
  underlyingName = "UnderlyingSameNamespaceStruct",
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

struct ContainerOfAdaptedWithDefault {
  1: list<AdaptedByte> container_of_adapted = [1, 2, 3];
}

struct ContainerWithAdaptedElement {
  1: list<AdaptedByte> list_field;
  2: set<AdaptedByte> set_field;
  3: map<AdaptedByte, byte> map_field1;
  4: map<byte, AdaptedByte> map_field2;
  5: map<AdaptedByte, AdaptedByte> map_field3;
}

struct NestedContainerOfAdapted {
  1: list<list<AdaptedByte>> container;
}
