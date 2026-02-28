/*
 * Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/admin_cmd_handler_t.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"

namespace xpl {
namespace test {

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::StrEq;

class Admin_cmd_modify_collection_options_test
    : public Admin_command_handler_test {};

TEST_F(Admin_cmd_modify_collection_options_test, empty_schema) {
  set_arguments(
      Any::Object{{"schema", ""}, {"options", Any::Object{}}, COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_BAD_SCHEMA,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test,
       empty_schema_sill_options_has_wrong_type) {
  set_arguments(Any::Object{{"schema", ""}, {"options", ""}, COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_TYPE,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, missing_schema) {
  set_arguments(Any::Object{COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, empty_collection_name) {
  set_arguments(Any::Object{SCHEMA, {"options", Any::Object{}}, {"name", ""}});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, missing_collection_name) {
  set_arguments(Any::Object{SCHEMA});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, invalid_object) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME, {"foo:", "bar"}});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, invalid_option) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME, {"options", "bar"}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_TYPE,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, empty_options) {
  set_arguments(
      Any::Object{SCHEMA, COLLECTION_NAME, {"options", Any::Object{}}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_OBJECT_EMPTY,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test,
       validation_without_schema_and_level) {
  set_arguments(
      Any::Object{SCHEMA, COLLECTION_NAME, {"options", Any::Object{}}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_OBJECT_EMPTY,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test,
       invalid_collection_option_only) {
  set_validation_details({{"foo", Scalar::String{"bar"}}});
  ASSERT_ERROR_CODE(ER_X_CMD_INVALID_ARGUMENT,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test,
       valid_plus_invalid_collection_options) {
  set_validation_details({{"level", Scalar::String{"off"}},
                          {"schema", Scalar::String{"{}"}},
                          {"foo", Scalar::String{"bar"}}});
  ASSERT_ERROR_CODE(ER_X_CMD_INVALID_ARGUMENT,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_collection_options_test, validation_level_set_to_off) {
  set_validation_details({{"level", Scalar::String{"off"}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute(MatchesRegex("ALTER TABLE `xtest`.`test_coll` ALTER "
                                   "CHECK .val_strict.* NOT ENFORCED"),
                      _, _));

  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->modify_collection_options(m_args.get()));
}

class Admin_cmd_modify_validation_level_strict
    : public Admin_cmd_modify_collection_options_test,
      public ::testing::WithParamInterface<std::string> {};

TEST_P(Admin_cmd_modify_validation_level_strict,
       validation_level_set_to_strict) {
  set_validation_details({{"level", Scalar::String{GetParam()}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute(MatchesRegex("ALTER TABLE `xtest`.`test_coll` ALTER "
                                   "CHECK .val_strict.* ENFORCED"),
                      _, _));

  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->modify_collection_options(m_args.get()));
}

INSTANTIATE_TEST_CASE_P(Instantiation_modify_cmd_level_strict,
                        Admin_cmd_modify_validation_level_strict,
                        ::testing::Values("strict", "STRICT", "sTriCT"));

TEST_F(Admin_cmd_modify_collection_options_test, validation_level_invalid) {
  set_validation_details({{"level", Scalar::String{"validate_sometimes"}}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE,
                    command->modify_collection_options(m_args.get()));
}

class Admin_cmd_modify_validation_schema_only
    : public Admin_cmd_modify_collection_options_test,
      public ::testing::WithParamInterface<Any::Object> {};

TEST_P(Admin_cmd_modify_validation_schema_only, validation_schema_only) {
  set_validation_details(GetParam());
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(
      mock_data_context,
      execute(MatchesRegex("ALTER TABLE `xtest`.`test_coll` MODIFY COLUMN "
                           "_json_schema JSON GENERATED ALWAYS AS .* VIRTUAL"),
              _, _));

  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_validation_schema_only,
       modify_collection_with_invalid_validation_schema_obj) {
  set_validation_details({{"schema", Any::Object{{"unknown", "value"}}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_validation_schema_only,
       modify_collection_with_invalid_validation_schema_string) {
  set_validation_details(
      {{"schema", Scalar::String{"{\"unknown\": \"value\"}"}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_validation_schema_only,
       modify_collection_with_invalid_validation_json_obj) {
  set_validation_details({{"schema", Any::Object{{"\"", "invalid json"}}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    command->modify_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_modify_validation_schema_only,
       modify_collection_with_invalid_validation_json_string) {
  set_validation_details({{"schema", Scalar::String{"{\"invalid json\"}"}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    command->modify_collection_options(m_args.get()));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_modify_cmd_schema_only,
    Admin_cmd_modify_validation_schema_only,
    ::testing::Values(
        Any::Object{{"schema", Scalar::String{"{\"description\":\"value\"}"}}},
        Any::Object{{"schema", Any::Object{{"description", "value"}}}}));

class Admin_cmd_modify_validation_schema_with_level_off
    : public Admin_cmd_modify_collection_options_test,
      public ::testing::WithParamInterface<Any::Object> {};

TEST_P(Admin_cmd_modify_validation_schema_with_level_off,
       validation_turned_off) {
  set_validation_details(GetParam());
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(
      mock_data_context,
      execute(MatchesRegex("ALTER TABLE `xtest`.`test_coll` MODIFY COLUMN "
                           "_json_schema JSON GENERATED ALWAYS AS .* VIRTUAL, "
                           "ALTER CHECK .val_strict_.* NOT ENFORCED"),
              _, _));

  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->modify_collection_options(m_args.get()));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_modify_cmd_validation_off,
    Admin_cmd_modify_validation_schema_with_level_off,
    ::testing::Values(
        Any::Object{{"schema", Any::Object{{"description", "bar"}}},
                    {"level", "off"}},
        Any::Object{{"schema", Scalar::String{"{\"description\":\"bar\"}"}},
                    {"level", "off"}}));

class Admin_cmd_modify_validation_schema_with_level_strict
    : public Admin_cmd_modify_collection_options_test,
      public ::testing::WithParamInterface<Any::Object> {};

TEST_P(Admin_cmd_modify_validation_schema_with_level_strict,
       validation_turned_strict) {
  set_validation_details(GetParam());
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(
      mock_data_context,
      execute(MatchesRegex("ALTER TABLE `xtest`.`test_coll` MODIFY COLUMN "
                           "_json_schema JSON GENERATED ALWAYS AS .* VIRTUAL, "
                           "ALTER CHECK .val_strict_.* ENFORCED"),
              _, _));

  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->modify_collection_options(m_args.get()));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_modify_cmd_validation_strict,
    Admin_cmd_modify_validation_schema_with_level_strict,
    ::testing::Values(
        Any::Object{{"schema", Any::Object{{"description", "bar"}}},
                    {"level", "strict"}},
        Any::Object{{"schema", Scalar::String{"{\"description\":\"bar\"}"}},
                    {"level", "strict"}}));

}  // namespace test
}  // namespace xpl
