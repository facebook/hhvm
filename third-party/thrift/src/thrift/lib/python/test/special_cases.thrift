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

@thrift.AllowLegacyMissingUris
package;

namespace py3 thrift.python.test

struct TestPropertyAsField {
  # The presence of this field caused a failure due to the conflict
  # with the built-in property. Keep this test to ensure
  # the fix continues to work.
  1: string property;
  2: string break_unless_used_with_renamed_built_in_property;
}

struct TestRegisterAsField {
  # The presence of this field caused a failure due to the conflict
  # with the register method in abc.ABC. Keep this test to ensure
  # the fix continues to work.
  1: string register;
}

struct TestKeywordAsField {
  # The presence of this field caused a failure due to
  # the use of a property that has the same name as a keyword.
  1: string str;
}

struct TestStruct {
  1: bool placeholder;
}

// Test that Python reserved keywords are escaped in enum class names.
// "from" is a Python reserved keyword; using it directly as a class name
// would cause a syntax error.
enum from {
  VALUE = 1,
}

// Reproduces the use-cases where field names in structured types are the same
// as the typenames that appear in the failures. Once the type-checker
// encounters the field, it sees that name as the name of the field and not a
// typename.
struct TestFieldNameSameAsTypeName {
  // The comments below show the errors these use-cases exposed.
  // thrift_abstract_types.py:47:39 Undefined or invalid type [11]:
  // Annotation `TestFieldNameSameAsTypeName.TestStruct` is not defined as a
  // type.
  // thrift_mutable_types.pyi:71:32 Undefined or invalid type [11]: Annotation
  // `TestFieldNameSameAsTypeName.TestStruct` is not defined as a type.
  // thrift_types.pyi:60:4 Inconsistent override [15]:
  // `thrift.test.thrift_python.type_check_special_cases_test.thrift_types.
  // TestFieldNameSameAsTypeName.__iter__` overrides method defined in
  // `_typing.Iterable` inconsistently. Returned type `unknown` is not a
  // subtype of the overridden return `_typing.Iterator[_typing.Tuple[str,
  // typing.Any]]`.
  1: TestStruct TestStruct;
  // thrift_types.pyi:48:27 Undefined or invalid type [11]: Annotation
  // `TestFieldNameSameAsTypeName.TestStruct` is not defined as a type.
  2: TestStruct typeMaybeReinterpretedAsField;
}
