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

#include <thrift/compiler/sema/standard_validator.h>

#include <cmath>
#include <limits>
#include <memory>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/diagnostic.h>

namespace apache::thrift::compiler {
namespace {

using ::testing::UnorderedElementsAre;

class StandardValidatorTest : public ::testing::Test {
 public:
  StandardValidatorTest()
      : loc(source_mgr.add_virtual_file("/path/to/file.thrift", "\n\n\n\n")
                .start) {}

 protected:
  std::vector<diagnostic> validate(
      diagnostic_params params = diagnostic_params::keep_all()) {
    diagnostic_results results;
    diagnostic_context ctx(source_mgr, results, std::move(params));
    standard_validator()(ctx, program_);
    return std::move(results).diagnostics();
  }

  std::unique_ptr<t_const> inst(const t_struct* ttype, int lineno) {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(t_type_ref::from_ptr(ttype));
    auto result =
        std::make_unique<t_const>(&program_, ttype, "", std::move(value));
    result->set_src_range({loc + (lineno - 1), loc + (lineno - 1)});
    return result;
  }

  diagnostic error(int lineno, const std::string& msg) {
    return {diagnostic_level::error, msg, "/path/to/file.thrift", lineno};
  }
  diagnostic error(const std::string& msg) {
    return {diagnostic_level::error, msg, "", 0};
  }

  diagnostic warning(int lineno, const std::string& msg) {
    return {diagnostic_level::warning, msg, "/path/to/file.thrift", lineno};
  }

  source_manager source_mgr;
  source_location loc;
  t_program program_{"/path/to/file.thrift"};
};

TEST_F(StandardValidatorTest, InterfaceNamesUniqueNoError) {
  auto return_type = t_type_ref::from_req_ptr(&t_base_type::t_void());

  // Create interfaces with non-overlapping functions.
  auto service = std::make_unique<t_service>(&program_, "Service");
  service->add_function(
      std::make_unique<t_function>(&program_, return_type, "bar"));
  service->add_function(
      std::make_unique<t_function>(&program_, return_type, "baz"));
  program_.add_service(std::move(service));

  auto interaction = std::make_unique<t_interaction>(&program_, "Interaction");
  interaction->add_function(
      std::make_unique<t_function>(&program_, return_type, "bar"));
  interaction->add_function(
      std::make_unique<t_function>(&program_, return_type, "baz"));
  program_.add_interaction(std::move(interaction));

  // No errors will be found.
  EXPECT_THAT(validate(), ::testing::IsEmpty());
}

TEST_F(StandardValidatorTest, BadPriority) {
  auto return_type = t_type_ref::from_req_ptr(&t_base_type::t_void());

  {
    auto service = std::make_unique<t_service>(&program_, "Service");
    service->set_annotation("priority", "bad1");
    auto fn = std::make_unique<t_function>(&program_, return_type, "foo");
    fn->set_annotation("priority", "bad2");
    service->add_function(std::move(fn));
    program_.add_service(std::move(service));
  }

  {
    auto interaction =
        std::make_unique<t_interaction>(&program_, "Interaction");
    interaction->set_annotation("priority", "bad3");
    auto fn = std::make_unique<t_function>(&program_, return_type, "foo");
    fn->set_annotation("priority", "bad4");
    interaction->add_function(std::move(fn));
    program_.add_interaction(std::move(interaction));
  }

  EXPECT_THAT(
      validate(),
      ::testing::UnorderedElementsAre(
          error(
              "Bad priority 'bad1'. Choose one of [\"HIGH_IMPORTANT\", \"HIGH\", \"IMPORTANT\", \"NORMAL\", \"BEST_EFFORT\"]."),
          error(
              "Bad priority 'bad2'. Choose one of [\"HIGH_IMPORTANT\", \"HIGH\", \"IMPORTANT\", \"NORMAL\", \"BEST_EFFORT\"]."),
          error(
              "Bad priority 'bad3'. Choose one of [\"HIGH_IMPORTANT\", \"HIGH\", \"IMPORTANT\", \"NORMAL\", \"BEST_EFFORT\"]."),
          error(
              "Bad priority 'bad4'. Choose one of [\"HIGH_IMPORTANT\", \"HIGH\", \"IMPORTANT\", \"NORMAL\", \"BEST_EFFORT\"].")));
}

TEST_F(StandardValidatorTest, ReapeatedNamesInService) {
  auto return_type = t_type_ref::from_req_ptr(&t_base_type::t_void());

  // Create interfaces with overlapping functions.
  {
    auto service = std::make_unique<t_service>(&program_, "Service");
    auto fn1 = std::make_unique<t_function>(&program_, return_type, "foo");
    fn1->set_src_range({loc, loc});
    auto fn2 = std::make_unique<t_function>(&program_, return_type, "foo");
    fn2->set_src_range({loc + 1, loc + 1});
    service->add_function(std::move(fn1));
    service->add_function(std::move(fn2));
    service->add_function(
        std::make_unique<t_function>(&program_, return_type, "bar"));
    program_.add_service(std::move(service));
  }

  {
    auto interaction =
        std::make_unique<t_interaction>(&program_, "Interaction");
    auto fn1 = std::make_unique<t_function>(&program_, return_type, "bar");
    fn1->set_src_range({loc + 2, loc + 2});
    auto fn2 = std::make_unique<t_function>(&program_, return_type, "bar");
    fn2->set_src_range({loc + 3, loc + 3});

    interaction->add_function(
        std::make_unique<t_function>(&program_, return_type, "foo"));
    interaction->add_function(std::move(fn1));
    interaction->add_function(std::move(fn2));
    program_.add_interaction(std::move(interaction));
  }

  EXPECT_THAT(
      validate(),
      ::testing::UnorderedElementsAre(
          error(2, "Function `foo` is already defined for `Service`."),
          error(4, "Function `bar` is already defined for `Interaction`.")));
}

TEST_F(StandardValidatorTest, RepeatedNameInExtendedService) {
  auto return_type = t_type_ref::from_req_ptr(&t_base_type::t_void());

  // Create first service with non repeated functions.
  auto base = std::make_unique<t_service>(&program_, "Base");
  base->add_function(
      std::make_unique<t_function>(&program_, return_type, "bar"));
  base->add_function(
      std::make_unique<t_function>(&program_, return_type, "baz"));
  auto derived = std::make_unique<t_service>(&program_, "Derived", base.get());
  derived->add_function(
      std::make_unique<t_function>(&program_, return_type, "foo"));

  auto derived_ptr = derived.get();
  program_.add_service(std::move(base));
  program_.add_service(std::move(derived));

  EXPECT_THAT(validate(), ::testing::IsEmpty());

  // Add an overlapping function in the derived service.
  auto dupe = std::make_unique<t_function>(&program_, return_type, "baz");
  dupe->set_src_range({loc, loc});
  derived_ptr->add_function(std::move(dupe));

  // An error will be found
  EXPECT_THAT(
      validate(),
      ::testing::UnorderedElementsAre(
          error(1, "Function `Derived.baz` redefines `file.Base.baz`.")));
}

TEST_F(StandardValidatorTest, DuplicatedEnumValues) {
  auto tenum = std::make_unique<t_enum>(&program_, "foo");
  auto tenum_ptr = tenum.get();
  program_.add_enum(std::move(tenum));

  tenum_ptr->append(std::make_unique<t_enum_value>("bar", 1));
  tenum_ptr->append(std::make_unique<t_enum_value>("baz", 2));

  // No errors will be found.
  EXPECT_THAT(validate(), ::testing::IsEmpty());

  // Add enum_value with repeated value.
  auto enum_value_3 = std::make_unique<t_enum_value>("foo", 1);
  tenum_ptr->append(std::move(enum_value_3));

  // An error will be found.
  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          error("Duplicate value `foo=1` with value `bar` in enum `foo`.")));
}

TEST_F(StandardValidatorTest, RepeatedNamesInEnumValues) {
  auto tenum = std::make_unique<t_enum>(&program_, "foo");
  auto tenum_ptr = tenum.get();
  program_.add_enum(std::move(tenum));

  tenum_ptr->append(std::make_unique<t_enum_value>("bar", 1));
  tenum_ptr->append(std::make_unique<t_enum_value>("not_bar", 2));

  // No errors will be found.
  EXPECT_THAT(validate(), ::testing::IsEmpty());

  // Add enum_value with repeated name.
  auto enum_value_3 = std::make_unique<t_enum_value>("bar", 3);
  enum_value_3->set_src_range({loc, loc});
  tenum_ptr->append(std::move(enum_value_3));

  // An error will be found.
  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          error(1, "Enum value `bar` is already defined for `foo`.")));
}

TEST_F(StandardValidatorTest, UnsetEnumValues) {
  auto tenum = std::make_unique<t_enum>(&program_, "foo");
  auto tenum_ptr = tenum.get();
  program_.add_enum(std::move(tenum));

  auto enum_value_1 = std::make_unique<t_enum_value>("Foo", 1);
  auto enum_value_2 = std::make_unique<t_enum_value>("Bar");
  auto enum_value_3 = std::make_unique<t_enum_value>("Baz");
  enum_value_1->set_src_range({loc, loc});
  enum_value_2->set_src_range({loc + 1, loc + 1});
  enum_value_3->set_src_range({loc + 2, loc + 2});
  tenum_ptr->append(std::move(enum_value_1));
  tenum_ptr->append(std::move(enum_value_2));
  tenum_ptr->append(std::move(enum_value_3));

  // Bar and Baz will have errors.
  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          error(
              2,
              "The enum value, `Bar`, must have an explicitly assigned value."),
          error(
              3,
              "The enum value, `Baz`, must have an explicitly assigned value.")));
}

TEST_F(StandardValidatorTest, UnionFieldAttributes) {
  auto tstruct = std::make_unique<t_struct>(&program_, "Struct");
  auto tunion = std::make_unique<t_union>(&program_, "Union");
  tunion->set_src_range({loc, loc});
  {
    auto field = std::make_unique<t_field>(&t_base_type::t_i64(), "req", 1);
    field->set_src_range({loc + 1, loc + 1});
    field->set_qualifier(t_field_qualifier::required);
    tunion->append(std::move(field));
  }
  {
    auto field = std::make_unique<t_field>(&t_base_type::t_i64(), "op", 2);
    field->set_src_range({loc + 2, loc + 2});
    field->set_qualifier(t_field_qualifier::optional);
    tunion->append(std::move(field));
  }
  {
    auto field = std::make_unique<t_field>(&t_base_type::t_i64(), "non", 3);
    field->set_src_range({loc + 3, loc + 3});
    tunion->append(std::move(field));
  }
  {
    auto field = std::make_unique<t_field>(tstruct.get(), "mixin", 4);
    field->set_src_range({loc + 4, loc + 4});
    field->set_annotation("cpp.mixin");
    tunion->append(std::move(field));
  }
  program_.add_struct(std::move(tstruct));
  program_.add_struct(std::move(tunion));

  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          // Qualified fields will have errors.
          error(
              2,
              "Unions cannot contain qualified fields. Remove `required` qualifier from field `req`."),
          error(
              3,
              "Unions cannot contain qualified fields. Remove `optional` qualifier from field `op`."),
          warning(2, "Required field is deprecated: `req`."),
          // Fields with cpp.mixing have errors.
          error(5, "Union `Union` cannot contain mixin field `mixin`.")));
}

TEST_F(StandardValidatorTest, FieldId) {
  auto tstruct = std::make_unique<t_struct>(&program_, "Struct");
  tstruct->append(
      std::make_unique<t_field>(t_base_type::t_i64(), "explicit_id", 1));
  tstruct->append(
      std::make_unique<t_field>(t_base_type::t_i64(), "zero_id", 0));
  tstruct->append(
      std::make_unique<t_field>(t_base_type::t_i64(), "neg_id", -1));
  auto f = std::make_unique<t_field>(t_base_type::t_i64(), "implicit_id");
  f->set_implicit_id(-2);
  f->set_src_range({loc, loc});
  tstruct->append(std::move(f));

  program_.add_struct(std::move(tstruct));
  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          error("Zero value (0) not allowed as a field id for `zero_id`"),
          warning(
              1,
              "No field id specified for `implicit_id`, resulting protocol may have conflicts or not be backwards compatible!")));
}

TEST_F(StandardValidatorTest, MixinFieldType) {
  auto tstruct = std::make_unique<t_struct>(&program_, "Struct");
  auto tunion = std::make_unique<t_union>(&program_, "Union");
  auto texception = std::make_unique<t_exception>(&program_, "Exception");

  auto foo = std::make_unique<t_struct>(&program_, "Foo");
  {
    auto field = std::make_unique<t_field>(tstruct.get(), "struct_field", 1);
    field->set_src_range({loc, loc});
    field->set_annotation("cpp.mixin");
    field->set_qualifier(t_field_qualifier::optional);
    foo->append(std::move(field));
  }
  {
    auto field = std::make_unique<t_field>(tunion.get(), "union_field", 2);
    field->set_annotation("cpp.mixin");
    field->set_qualifier(t_field_qualifier::required);
    foo->append(std::move(field));
  }
  {
    auto field = std::make_unique<t_field>(texception.get(), "except_field", 3);
    field->set_annotation("cpp.mixin");
    foo->append(std::move(field));
  }
  {
    auto field =
        std::make_unique<t_field>(&t_base_type::t_i32(), "other_field", 4);
    field->set_annotation("cpp.mixin");
    foo->append(std::move(field));
  }

  program_.add_struct(std::move(tstruct));
  program_.add_struct(std::move(tunion));
  program_.add_exception(std::move(texception));
  program_.add_struct(std::move(foo));

  EXPECT_THAT(
      validate(diagnostic_params::only_errors()),
      UnorderedElementsAre(
          error(1, "Mixin field `struct_field` cannot be optional."),
          error(
              "Mixin field `except_field` type must be a struct or union. Found `Exception`."),
          error(
              "Mixin field `other_field` type must be a struct or union. Found `i32`.")));
}

TEST_F(StandardValidatorTest, RepeatedStructuredAnnotation) {
  auto foo = std::make_unique<t_struct>(&program_, "Foo");
  // A different program with the same name.
  t_program other_program("/path/to/other/file.thrift");
  // A different foo with the same file.name
  auto other_foo = std::make_unique<t_struct>(&other_program, "Foo");
  EXPECT_EQ(foo->get_full_name(), other_foo->get_full_name());

  t_scope scope;
  auto bar = std::make_unique<t_typedef>(
      &program_, &t_base_type::t_i32(), "Bar", &scope);
  bar->add_structured_annotation(inst(other_foo.get(), 1));
  bar->add_structured_annotation(inst(foo.get(), 2));
  auto annot = inst(foo.get(), 3);
  annot->set_src_range({loc + 2, loc + 2});
  bar->add_structured_annotation(std::move(annot));

  program_.add_def(std::move(foo));
  program_.add_def(std::move(bar));
  other_program.add_def(std::move(other_foo));

  // Only the third annotation is a duplicate.
  EXPECT_THAT(
      validate(diagnostic_params::only_errors()),
      UnorderedElementsAre(error(
          3, "Structured annotation `Foo` is already defined for `Bar`.")));
}

TEST_F(StandardValidatorTest, CustomDefaultValue) {
  auto custom_byte = std::make_unique<t_const_value>(
      static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1);
  auto custom_short = std::make_unique<t_const_value>(
      static_cast<int64_t>(std::numeric_limits<int16_t>::max()) + 1);
  auto custom_integer = std::make_unique<t_const_value>(
      static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1);
  auto custom_float = std::make_unique<t_const_value>();
  custom_float->set_double(std::nextafter(
      static_cast<double>(std::numeric_limits<float>::max()),
      std::numeric_limits<double>::max()));
  auto custom_float_precision_loss =
      std::make_unique<t_const_value>(std::numeric_limits<int32_t>::max());

  auto const_byte = std::make_unique<t_const>(
      &program_, &t_base_type::t_byte(), "const_byte", std::move(custom_byte));
  auto const_short = std::make_unique<t_const>(
      &program_, &t_base_type::t_i16(), "const_short", std::move(custom_short));
  auto const_integer = std::make_unique<t_const>(
      &program_,
      &t_base_type::t_i32(),
      "const_integer",
      std::move(custom_integer));
  auto const_float = std::make_unique<t_const>(
      &program_,
      &t_base_type::t_float(),
      "const_float",
      std::move(custom_float));
  auto const_float_precision_loss = std::make_unique<t_const>(
      &program_,
      &t_base_type::t_float(),
      "const_float_precision_loss",
      std::move(custom_float_precision_loss));

  for (auto const_ptr :
       {&const_byte,
        &const_short,
        &const_integer,
        &const_float,
        &const_float_precision_loss}) {
    auto& c = *const_ptr;
    c->set_src_range({loc, loc});
    program_.add_const(std::move(c));
  }

  EXPECT_THAT(
      validate(),
      UnorderedElementsAre(
          error(
              1,
              "value error: const `const_byte` has an invalid custom default value."),
          error(
              1,
              "value error: const `const_short` has an invalid custom default value."),
          error(
              1,
              "value error: const `const_integer` has an invalid custom default value."),
          error(
              1,
              "value error: const `const_float` has an invalid custom default value."),
          error(
              1,
              "value error: const `const_float_precision_loss` cannot be represented precisely as `float` or `double`.")));
}

} // namespace
} // namespace apache::thrift::compiler
