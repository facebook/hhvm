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

include "thrift/annotation/scope.thrift"
include "thrift/annotation/thrift.thrift"

namespace cpp2 apache.thrift.syntax_graph.test

@thrift.Uri{value = "meta.com/thrift_test/TestRecursiveStruct"}
struct TestRecursiveStruct {
  @thrift.Box
  1: optional TestRecursiveStruct myself;
}

@thrift.Uri{value = "meta.com/thrift_test/ComplexAnnotation"}
@scope.Structured
struct ComplexAnnotation {
  1: list<TestInnerStructuredAnnotation> l;
  2: set<string> s;
  3: map<string, TestInnerStructuredAnnotation> m;
}

@thrift.Uri{value = "meta.com/thrift_test/TestStructuredAnnotation"}
@scope.Definition
@scope.Field
@scope.Enum
@scope.EnumValue
struct TestStructuredAnnotation {
  1: i64 field1;
  2: TestInnerStructuredAnnotation field2;
}

@thrift.Uri{value = "meta.com/thrift_test/TestEnum"}
@TestStructuredAnnotation{field1 = 3}
enum TestEnum {
  @TestStructuredAnnotation{field1 = 4}
  UNSET = 0,
  VALUE_1 = 1,
  VALUE_2 = 2,
}

struct TestStruct {
  1: i32 field1 = 10;
  2: optional TestEnum field2;
}

struct TestInnerStructuredAnnotation {
  1: i64 field1;
}

typedef list<TestStruct> ListOfTestStruct
typedef ListOfTestStruct TypedefToListOfTestStruct

typedef TestStructuredAnnotation TypedefToTestStructuredAnnotation

@TestStructuredAnnotation{
  field1 = 3,
  field2 = TestInnerStructuredAnnotation{field1 = 4},
}
@ComplexAnnotation{
  l = [TestInnerStructuredAnnotation{field1 = 1}],
  s = ["foo"],
  m = {"bar": TestInnerStructuredAnnotation{field1 = 2}},
}
@thrift.Uri{value = "meta.com/thrift_test/TestUnion"}
union TestUnion {
  1: TestStruct s;
  2: TestEnum e;
}

@thrift.Uri{value = ""}
@scope.Definition
struct TestStructuredAnnotationWithoutUri {
  1: i64 field1;
}

@TestStructuredAnnotationWithoutUri{field1 = 3}
exception TestException {
  @TestStructuredAnnotation
  1: binary blob;
}

const TestStruct testConst = TestStruct{field1 = 2, field2 = VALUE_1};
const TestStructuredAnnotation testNestedConst = TestStructuredAnnotation{
  field1 = 3,
  field2 = TestInnerStructuredAnnotation{field1 = 4},
};

interaction TestInteraction {
  i32 foo(1: TestRecursiveStruct input) throws (1: TestException ex);
}

service TestService {
  @TypedefToTestStructuredAnnotation{field1 = 3}
  TestStruct foo(1: i32 input);
  TestInteraction createInteraction();
  i32, stream<i32> createStream();
  TestInteraction, i32, stream<i32> createInteractionAndStream();
}

@thrift.Uri{value = "meta.com/thrift_test/StructWithCustomDefault"}
struct StructWithCustomDefault {
  1: bool field1 = true;
  2: i32 field2 = 10;
  3: string field3 = "foo";
  4: binary field4 = "bar";
  5: list<TestUnion> field5 = [TestUnion{e = VALUE_1}];
  6: set<i32> field6 = [1, 2, 3];
  7: map<i32, TestUnion> field7 = {1: TestUnion{e = VALUE_1}};
  8: TestUnion field8 = TestUnion{e = VALUE_1};
}
