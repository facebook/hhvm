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

#include <memory>

#include <thrift/compiler/sema/standard_validator.h>
#include <thrift/compiler/test/compiler.h>

#include <thrift/compiler/test/test_utils.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using apache::thrift::compiler::test::check_compile;

TEST(ReservedIdentifierValidatorTest, FilenameIsReservedAndShouldError) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["path/to/fbthrift_should_error.thrift"] = R"(
    # expected-error@1: `fbthrift_should_error` is a reserved filename. Choose a different filename that does not contain `fbthrift`.
  )";

  check_compile(name_contents_map, "path/to/fbthrift_should_error.thrift");
}

TEST(
    ReservedIdentifierValidatorTest,
    FilenameIsReservedAndIsAnnotatedToAllowTheReservedIdentifier) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["path/to/fbthrift_should_not_error_due_to_bypass.thrift"] =
      R"(

    include "thrift/annotation/thrift.thrift"

    @thrift.AllowReservedFilename
    package "test.dev/test"
  )";

  check_compile(
      name_contents_map,
      "path/to/fbthrift_should_not_error_due_to_bypass.thrift");
}

TEST(
    ReservedIdentifierValidatorTest,
    CannotUseAllowReservedIdentifierOnFilename) {
  std::map<std::string, std::string> name_contents_map;
  name_contents_map["path/to/should_error_due_to_incorrect_annotation.thrift"] =
      R"(
    include "thrift/annotation/thrift.thrift"

    @thrift.AllowReservedIdentifier
    package "test.dev/test"
    # expected-error@4: `AllowReservedIdentifier` cannot annotate `should_error_due_to_incorrect_annotation`
  )";

  check_compile(
      name_contents_map,
      "path/to/should_error_due_to_incorrect_annotation.thrift");
}

TEST(ReservedIdentifierValidatorTest, ServiceNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    service MyService {}

    service __fbthriftServiceShouldError {}
      # expected-error@-1: `__fbthriftServiceShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    service __fbthriftServiceShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    service aService {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `aService`
  )");
}

TEST(ReservedIdentifierValidatorTest, InteractionNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    interaction MyInteraction {}

    interaction __fbthriftInteractionShouldError {}
        # expected-error@-1: `__fbthriftInteractionShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    interaction __fbthriftInteractionShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    interaction anIteraction {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `anIteraction`
  )");
}

TEST(ReservedIdentifierValidatorTest, StructNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    struct MyStruct {
      1: i32 myField;

      2: i32 __fbthriftFieldShouldError;
        # expected-error@-1: `__fbthriftFieldShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

      @thrift.AllowReservedIdentifier
      3: i32 __fbthriftFieldShouldNotErrorDueToBypass;
    }

    struct __fbthriftStructShouldError {}
      # expected-error@-1: `__fbthriftStructShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    struct __fbthriftStructShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    struct aStruct {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `aStruct`
  )");
}

TEST(ReservedIdentifierValidatorTest, UnionNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    union MyUnion {}

    union __fbthriftUnionShouldError {}
      # expected-error@-1: `__fbthriftUnionShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    union __fbthriftUnionShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    struct aUnion {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `aUnion`
  )");
}

TEST(ReservedIdentifierValidatorTest, ExceptionNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    exception MyException {}

    exception __fbthriftExceptionShouldError {}
      # expected-error@-1: `__fbthriftExceptionShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    exception __fbthriftExceptionShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    struct anException {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `anException`
  )");
}

TEST(ReservedIdentifierValidatorTest, EnumNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    enum MyEnum {
      NORMAL                                    = 1,
      FBTHRIFT_SHOULD_ERROR                     = 2,
        # expected-error@-1: `FBTHRIFT_SHOULD_ERROR` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.
      @thrift.AllowReservedIdentifier
      FBTHRIFT_SHOULD_NOT_ERROR_DUE_TO_BYPASS   = 3
      @thrift.AllowReservedFilename
      AN_ENUM_VALUE                             = 42
        # expected-error@-2: `AllowReservedFilename` cannot annotate `AN_ENUM_VALUE`
    }

    enum __fbthriftEnum {}
      # expected-error@-1: `__fbthriftEnum` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    enum __fbthriftEnumShouldNotErrorDueToBypass {}

    @thrift.AllowReservedFilename
    struct anEnum {}
      # expected-error@-2: `AllowReservedFilename` cannot annotate `anEnum`
  )");
}

TEST(ReservedIdentifierValidatorTest, ConstNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    const i32 MY_CONSTANT = 42;

    const i32 FBTHRIFT_CONSTANT_SHOULD_ERROR = 42;
      # expected-error@-1: `FBTHRIFT_CONSTANT_SHOULD_ERROR` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    const i32 MY_CONSTANT_FBTHRIFT_SHOULD_NOT_ERROR_DUE_TO_BYPASS = 42;

    @thrift.AllowReservedFilename
    const i32 A_CONSTANT = 42;
      # expected-error@-2: `AllowReservedFilename` cannot annotate `A_CONSTANT`
  )");
}

TEST(ReservedIdentifierValidatorTest, TypedefNameIsReserved) {
  check_compile(R"(
    include "thrift/annotation/thrift.thrift"

    typedef i16 MyTypedef;

    typedef i16 __fbthriftTypedefShouldError;
      # expected-error@-1: `__fbthriftTypedefShouldError` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.

    @thrift.AllowReservedIdentifier
    typedef i16 __fbthriftTypedefShouldNotErrorDueToBypass;

    @thrift.AllowReservedFilename
    typedef i32 aTypedef;
      # expected-error@-2: `AllowReservedFilename` cannot annotate `aTypedef`
 )");
}

// Tests for generated nodes that will not appear in an IDL file.
// These must be tested through the direct manipulation of the AST.
namespace apache::thrift::compiler {

using definition_factory = std::function<std::unique_ptr<t_named>(t_program*)>;

std::array<definition_factory, 11> node_factories = {
    [](t_program* program) {
      auto service = std::make_unique<t_service>(program, "fbthriftService");
      service->set_generated(true);
      return service;
    },
    [](t_program* program) {
      auto interaction =
          std::make_unique<t_interaction>(program, "fbthriftInteraction");
      interaction->set_generated(true);
      return interaction;
    },
    [](t_program* program) {
      auto struct_ = std::make_unique<t_struct>(program, "fbthriftStruct");
      struct_->set_generated(true);
      return struct_;
    },
    [](t_program* program) {
      auto union_ = std::make_unique<t_union>(program, "fbthriftUnion");
      union_->set_generated(true);
      return union_;
    },
    [](t_program* program) {
      auto exception =
          std::make_unique<t_exception>(program, "fbthriftException");
      exception->set_generated(true);
      return exception;
    },
    [](t_program* program) {
      auto enum_ = std::make_unique<t_enum>(program, "fbthriftEnum");
      enum_->set_generated(true);
      return enum_;
    },
    [](t_program* program) {
      auto bool_type = t_type_ref::from_req_ptr(&t_primitive_type::t_bool());
      auto bool_const = std::make_unique<t_const_value>();
      bool_const->set_bool(true);
      auto const_ = std::make_unique<t_const>(
          program, bool_type, "fbthriftConst", std::move(bool_const));
      const_->set_generated(true);
      return const_;
    },
    [](t_program* program) {
      auto bool_type = t_type_ref::from_req_ptr(&t_primitive_type::t_bool());
      auto typedef_ =
          std::make_unique<t_typedef>(program, "fbthriftTypedef", bool_type);
      typedef_->set_generated(true);
      return typedef_;
    },
    [](t_program* program) {
      auto void_return_type =
          t_type_ref::from_req_ptr(&t_primitive_type::t_void());
      auto service = std::make_unique<t_service>(program, "MyService");
      auto function = std::make_unique<t_function>(
          program, void_return_type, "fbthriftFunction");
      function->set_generated(true);
      service->add_function(std::move(function));
      return service;
    },
    [](t_program* program) {
      auto bool_type = t_type_ref::from_req_ptr(&t_primitive_type::t_bool());
      auto struct_ = std::make_unique<t_struct>(program, "MyStruct");
      auto field = std::make_unique<t_field>(bool_type, "fbthriftField", 42);
      field->set_generated(true);
      struct_->append_field(std::move(field));
      return struct_;
    },
    [](t_program* program) {
      auto enum_ = std::make_unique<t_enum>(program, "MyEnum");
      auto enum_value =
          std::make_unique<t_enum_value>("FBTHRIFT_ENUM_VALUE", 42);
      enum_value->set_generated(true);
      enum_->append_value(std::move(enum_value));
      return enum_;
    },
};

class GeneratedReservedIdentifierValidatorTest
    : public testing::TestWithParam<definition_factory> {};

TEST_P(GeneratedReservedIdentifierValidatorTest, GeneratedNodeMustNotError) {
  t_program program("path/to/program.thrift", "path/to/program.thrift");
  source_manager source_mgr;
  auto loc = source_mgr.add_virtual_file(program.path(), "").start;
  program.set_src_range({loc, loc});
  diagnostic_results results;
  sema_context ctx(source_mgr, results, diagnostic_params::keep_all());

  // Build the program
  auto definition_factory = GetParam();
  auto definition = definition_factory(&program);
  program.add_definition(std::move(definition));

  standard_validator()(ctx, program);

  EXPECT_THAT(results.diagnostics(), ::testing::IsEmpty());
}

INSTANTIATE_TEST_SUITE_P(
    ReservedIdentifierValidatorTest,
    GeneratedReservedIdentifierValidatorTest,
    ::testing::ValuesIn(node_factories));

} // namespace apache::thrift::compiler
