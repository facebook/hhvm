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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/compiler/test/compiler.h>

namespace apache::thrift::compiler {

using test::check_compile;

std::string kAnnotations = R"(
  package "facebook.com/thrift/test"
  include "thrift/annotation/scope.thrift"

  @scope.Program
  struct ProgramAnnotation {}

  @scope.Struct
  struct StructAnnotation {}

  @scope.Union
  struct UnionAnnotation {}

  @scope.Exception
  struct ExceptionAnnotation {}

  @scope.ThrownException
  struct ThrownExceptionAnnotation {}

  @scope.Field
  struct FieldAnnotation {}

  @scope.Field
  @scope.FunctionParameter
  struct FieldOrParameterAnnotation {}

  @scope.Typedef
  struct TypedefAnnotation {}

  @scope.Service
  struct ServiceAnnotation {}

  @scope.Interaction
  struct InteractionAnnotation {}

  @scope.Function
  struct FunctionAnnotation {}

  @scope.FunctionParameter
  struct FunctionParameterAnnotation {}

  @scope.Enum
  struct EnumAnnotation {}

  @scope.EnumValue
  struct EnumValueAnnotation {}

  @scope.Const
  struct ConstAnnotation {}
)";

TEST(ScopedValidatorTest, InvalidProgramScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_program_scope.thrift"] = R"(
    include "annotations.thrift"

    @annotations.ProgramAnnotation
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `invalid_program_scope`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `invalid_program_scope`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `invalid_program_scope`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `invalid_program_scope`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `invalid_program_scope`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `invalid_program_scope`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `invalid_program_scope`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `invalid_program_scope`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `invalid_program_scope`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `invalid_program_scope`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `invalid_program_scope`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `invalid_program_scope`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `invalid_program_scope`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `invalid_program_scope`
    package "foo.bar/test";
  )";

  check_compile(name_contents_map, "invalid_program_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidStructScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_struct_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyStruct`
    @annotations.StructAnnotation
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyStruct`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyStruct`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyStruct`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyStruct`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyStruct`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyStruct`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyStruct`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyStruct`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyStruct`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyStruct`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyStruct`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyStruct`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyStruct`
    struct MyStruct {}
  )";

  check_compile(name_contents_map, "invalid_struct_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidUnionScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_union_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyUnion`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyUnion`
    @annotations.UnionAnnotation
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyUnion`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyUnion`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyUnion`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyUnion`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyUnion`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyUnion`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyUnion`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyUnion`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyUnion`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyUnion`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyUnion`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyUnion`
    union MyUnion {}
  )";

  check_compile(name_contents_map, "invalid_union_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidExceptionScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_exception_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyException`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyException`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyException`
    @annotations.ExceptionAnnotation
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyException`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyException`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyException`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyException`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyException`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyException`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyException`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyException`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyException`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyException`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyException`
    exception MyException {}
  )";

  check_compile(name_contents_map, "invalid_exception_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidFieldScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_field_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    struct MyStruct {
      @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `myField`
      @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `myField`
      @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `myField`
      @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `myField`
      @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `myField`
      @annotations.FieldAnnotation
      @annotations.FieldOrParameterAnnotation
      @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `myField`
      @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `myField`
      @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `myField`
      @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `myField`
      @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `myField`
      @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `myField`
      @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `myField`
      @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `myField`
      1: i64 myField;
    }
  )";

  check_compile(name_contents_map, "invalid_field_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidTypedefScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_typedef_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyTypedef`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyTypedef`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyTypedef`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyTypedef`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyTypedef`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyTypedef`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyTypedef`
    @annotations.TypedefAnnotation
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyTypedef`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyTypedef`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyTypedef`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyTypedef`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyTypedef`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyTypedef`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyTypedef`
    typedef i64 MyTypedef;
  )";

  check_compile(name_contents_map, "invalid_typedef_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidServiceScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_service_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyService`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyService`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyService`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyService`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyService`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyService`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyService`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyService`
    @annotations.ServiceAnnotation
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyService`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyService`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyService`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyService`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyService`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyService`
    service MyService {}
  )";

  check_compile(name_contents_map, "invalid_service_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidInteractionScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_interaction_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyInteraction`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyInteraction`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyInteraction`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyInteraction`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyInteraction`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyInteraction`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyInteraction`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyInteraction`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyInteraction`
    @annotations.InteractionAnnotation
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyInteraction`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyInteraction`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyInteraction`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyInteraction`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyInteraction`
    interaction MyInteraction {}
  )";

  check_compile(name_contents_map, "invalid_interaction_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidFunctionScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_function_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    service MyService {
      @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `myFunction`
      @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `myFunction`
      @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `myFunction`
      @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `myFunction`
      @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `myFunction`
      @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `myFunction`
      @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `myFunction`
      @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `myFunction`
      @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `myFunction`
      @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `myFunction`
      @annotations.FunctionAnnotation
      @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `myFunction`
      @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `myFunction`
      @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `myFunction`
      @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `myFunction`
      void myFunction();
    }
  )";

  check_compile(name_contents_map, "invalid_function_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidFunctionParameterScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_function_parameter_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    service MyService {
      void myFunction(
        @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `param`
        @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `param`
        @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `param`
        @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `param`
        @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `param`
        @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `param`
        @annotations.FieldOrParameterAnnotation
        @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `param`
        @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `param`
        @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `param`
        @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `param`
        @annotations.FunctionParameterAnnotation
        @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `param`
        @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `param`
        @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `param`
        1: i32 param
      );
    }
  )";

  check_compile(name_contents_map, "invalid_function_parameter_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidThrownExceptionScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_thrown_exception_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    exception MyException {}

    service MyService {
      void myFunction() throws (
        @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `ex`
        @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `ex`
        @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `ex`
        @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `ex`
        @annotations.ThrownExceptionAnnotation
        @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `ex`
        @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `ex`
        @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `ex`
        @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `ex`
        @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `ex`
        @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `ex`
        @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `ex`
        @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `ex`
        @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `ex`
        @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `ex`
        1: MyException ex
      );
    }
  )";

  check_compile(name_contents_map, "invalid_thrown_exception_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidEnumScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_enum_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyEnum`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyEnum`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyEnum`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyEnum`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyEnum`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyEnum`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyEnum`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyEnum`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyEnum`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyEnum`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyEnum`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyEnum`
    @annotations.EnumAnnotation
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyEnum`
    @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MyEnum`
    enum MyEnum {}
  )";

  check_compile(name_contents_map, "invalid_enum_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidEnumValueScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_enumvalue_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    enum MyEnum {
      @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MY_ENUM_VALUE`
      @annotations.EnumValueAnnotation
      @annotations.ConstAnnotation              # expected-error: `ConstAnnotation` cannot annotate `MY_ENUM_VALUE`
      MY_ENUM_VALUE = 1
    }
  )";

  check_compile(name_contents_map, "invalid_enumvalue_scope.thrift");
}

TEST(ScopedValidatorTest, InvalidConstScopeTest) {
  std::map<std::string, std::string> name_contents_map;

  name_contents_map["annotations.thrift"] = kAnnotations;

  name_contents_map["invalid_const_scope.thrift"] = R"(
    package "facebook.com/thrift/test"
    include "annotations.thrift"

    @annotations.ProgramAnnotation            # expected-error: `ProgramAnnotation` cannot annotate `MyConst`
    @annotations.StructAnnotation             # expected-error: `StructAnnotation` cannot annotate `MyConst`
    @annotations.UnionAnnotation              # expected-error: `UnionAnnotation` cannot annotate `MyConst`
    @annotations.ExceptionAnnotation          # expected-error: `ExceptionAnnotation` cannot annotate `MyConst`
    @annotations.ThrownExceptionAnnotation    # expected-error: `ThrownExceptionAnnotation` cannot annotate `MyConst`
    @annotations.FieldAnnotation              # expected-error: `FieldAnnotation` cannot annotate `MyConst`
    @annotations.FieldOrParameterAnnotation   # expected-error: `FieldOrParameterAnnotation` cannot annotate `MyConst`
    @annotations.TypedefAnnotation            # expected-error: `TypedefAnnotation` cannot annotate `MyConst`
    @annotations.ServiceAnnotation            # expected-error: `ServiceAnnotation` cannot annotate `MyConst`
    @annotations.InteractionAnnotation        # expected-error: `InteractionAnnotation` cannot annotate `MyConst`
    @annotations.FunctionAnnotation           # expected-error: `FunctionAnnotation` cannot annotate `MyConst`
    @annotations.FunctionParameterAnnotation  # expected-error: `FunctionParameterAnnotation` cannot annotate `MyConst`
    @annotations.EnumAnnotation               # expected-error: `EnumAnnotation` cannot annotate `MyConst`
    @annotations.EnumValueAnnotation          # expected-error: `EnumValueAnnotation` cannot annotate `MyConst`
    @annotations.ConstAnnotation
    const i64 MyConst = 1;
  )";

  check_compile(name_contents_map, "invalid_const_scope.thrift");
}

TEST(ScopedValidatorTest, TransitiveAnnotations) {
  check_compile(R"(
    package "facebook.com/thrift/test"
    include "thrift/annotation/scope.thrift"

    @scope.Struct
    @scope.Union
    @scope.Exception
    @scope.Transitive
    struct TransitiveStructuredAnnotation {}

    @TransitiveStructuredAnnotation
    struct MyNestedStructured {}

    @TransitiveStructuredAnnotation
    @MyNestedStructured
    struct MyTransitivelyAnnotatedStruct {
      @TransitiveStructuredAnnotation   # expected-error: `TransitiveStructuredAnnotation` cannot annotate `field`
      @MyNestedStructured               # expected-error: `MyNestedStructured` cannot annotate `field`
      1: i64 field;
    }
   )");
}

TEST(ScopedValidatorTest, NonTransitiveAnnotations) {
  check_compile(
      R"(
    package "facebook.com/thrift/test"
    include "thrift/annotation/scope.thrift"

    @scope.Struct
    @scope.Union
    @scope.Exception
    struct NonTransitiveStructuredAnnotation {}

    @NonTransitiveStructuredAnnotation
    struct MyNonTransitiveStructuredAnnotation {}

    @MyNonTransitiveStructuredAnnotation # expected-warning@: Using `MyNonTransitiveStructuredAnnotation` as an annotation, even though it has not been enabled for any annotation scope.
    struct MyTransitivelyAnnotatedStruct {
      @MyNonTransitiveStructuredAnnotation   # expected-warning@: Using `MyNonTransitiveStructuredAnnotation` as an annotation, even though it has not been enabled for any annotation scope.
      1: i64 field;
    }
   )",
      {"--legacy-strict"});
}

TEST(ScopedValidatorTest, TypedefAnnotations) {
  check_compile(
      R"(
    package "facebook.com/thrift/test"
    include "thrift/annotation/scope.thrift"

    @scope.Struct
    struct StructAnnotation {}

    typedef StructAnnotation TypedefedStructuredAnnotation

    @TypedefedStructuredAnnotation # expected-warning@: Using `TypedefedStructuredAnnotation` as an annotation, even though it has not been enabled for any annotation scope.
    struct MyStruct {}
   )",
      {"--legacy-strict"});
}

} // namespace apache::thrift::compiler
