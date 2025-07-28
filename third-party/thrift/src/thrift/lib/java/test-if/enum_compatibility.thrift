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
include "thrift/annotation/java.thrift"
include "thrift/annotation/compat.thrift"
include "thrift/lib/java/test-common/test_common.thrift"

@thrift.Experimental
package "test.dev/thrift/lib/java/test/enums"

namespace java.swift com.facebook.thrift.test.enums

enum TestEnumLegacy1 {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}

@compat.Enums{type = compat.EnumType.Legacy}
enum TestEnumLegacy2 {
  ONE = 1,
  TWO = 2,
  THREE = 3,
}

// default is open enum in v1
@compat.Enums
enum TestEnumOpen1 {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
  N = 113,
}

@compat.Enums{type = compat.EnumType.Open}
enum TestEnumOpen2 {
  ONE = 1,
  TWO = 2,
  THREE = 3,
}

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.EnumToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef test_common.TestEnum AdaptedTestEnum

@java.Adapter{
  adapterClassName = "com.facebook.thrift.adapter.test.OpenEnumToStringTypeAdapter",
  typeClassName = "java.lang.String",
}
typedef test_common.TestEnumOpen AdaptedTestEnumOpen

struct TestStruct {
  1: optional i32 int_field1;
  2: optional i32 int_field2;
}

struct TestStructLegacy {
  1: TestEnumLegacy1 enum_field1;
  2: TestEnumLegacy2 enum_field2;
}

struct TestStructLegacyTerse {
  @thrift.TerseWrite
  1: TestEnumLegacy1 enum_field1;
  @thrift.TerseWrite
  2: TestEnumLegacy2 enum_field2;
}

struct TestStructOpen {
  1: TestEnumOpen1 enum_field1;
  2: TestEnumOpen2 enum_field2;
}

struct TestStructOpenTerse {
  @thrift.TerseWrite
  1: TestEnumOpen1 enum_field1;
  @thrift.TerseWrite
  2: TestEnumOpen2 enum_field2;
}

struct TestStructAdapted {
  1: AdaptedTestEnum enum_field1;
  2: AdaptedTestEnumOpen enum_field2;
}

struct TestStructAdaptedOptional {
  1: optional AdaptedTestEnum enum_field1;
  2: optional AdaptedTestEnumOpen enum_field2;
}

union TestUnionOpen {
  1: TestEnumOpen1 enum_field1;
  2: TestEnumOpen2 enum_field2;
}

safe permanent client exception TestExceptionOpen {
  1: TestEnumOpen1 enum_field1;
  2: TestEnumOpen2 enum_field2;
}

struct TestStructComplex {
  1: list<TestEnumOpen1> list_field;
  2: set<TestEnumOpen2> set_field;
  3: map<TestEnumOpen1, map<TestEnumOpen2, list<TestEnumOpen1>>> map_field;
}
