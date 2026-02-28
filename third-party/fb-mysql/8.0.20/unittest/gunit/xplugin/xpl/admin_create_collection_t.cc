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
using ::testing::StrEq;

class Admin_command_handler_create_collection
    : public Admin_command_handler_test {};

TEST_F(Admin_command_handler_create_collection,
       create_collection_empty_schema) {
  set_arguments(Any::Object{{"schema:", ""}, COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->create_collection(m_args.get()));
}

TEST_F(Admin_command_handler_create_collection,
       create_collection_missing_schema) {
  set_arguments(Any::Object{COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->create_collection(m_args.get()));
}

TEST_F(Admin_command_handler_create_collection,
       create_collection_which_already_exists) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _))
      .WillOnce(Return(ngs::Error_code{ER_TABLE_EXISTS_ERROR, ""}));
  ASSERT_ERROR_CODE(ER_TABLE_EXISTS_ERROR,
                    command->create_collection(m_args.get()));
}

class Admin_command_handler_ensure_collection
    : public Admin_command_handler_test {};

TEST_F(Admin_command_handler_ensure_collection,
       ensure_collection_empty_schema) {
  set_arguments(Any::Object{{"schema", ""}, COLLECTION_NAME});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->ensure_collection(m_args.get()));
}

TEST_F(Admin_command_handler_ensure_collection,
       ensure_collection_missing_schema) {
  set_arguments(Any::Object{COLLECTION_NAME});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->ensure_collection(m_args.get()));
}

TEST_F(Admin_command_handler_ensure_collection,
       create_collection_which_already_exists) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              is_sql_mode_set(StrEq("NO_BACKSLASH_ESCAPES")));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _))
      .WillOnce(Return(ngs::Error_code{ER_TABLE_EXISTS_ERROR, ""}));
  EXPECT_CALL(mock_data_context,
              execute(HasSubstr("SELECT COUNT(*) AS cnt"), _, _));
  ASSERT_ERROR_CODE(ER_X_INVALID_COLLECTION,
                    command->ensure_collection(m_args.get()));
}

using Admin_command_method_t = ngs::Error_code (Admin_command_handler::*)(
    Admin_command_handler::Command_arguments *);

class Admin_command_handler_create_or_ensure_collection
    : public Admin_command_handler_test,
      public ::testing::WithParamInterface<Admin_command_method_t> {};

TEST_P(Admin_command_handler_create_or_ensure_collection,
       create_collection_empty_collection_name) {
  set_arguments(Any::Object{SCHEMA, {"name:", ""}});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_create_or_ensure_collection,
       create_collection_missing_collection_name) {
  set_arguments(Any::Object{SCHEMA});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_create_or_ensure_collection,
       create_collection_valid) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_create_or_ensure_collection,
       create_collection_with_invalid_obj) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME, {"foo:", "bar"}});
  ASSERT_NE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()).error);
}

TEST_P(Admin_command_handler_create_or_ensure_collection,
       create_collection_with_invalid_validation_option) {
  set_validation_details({{"foo", Scalar::String{"bar"}}});
  ASSERT_NE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()).error);
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_create_collection_common,
    Admin_command_handler_create_or_ensure_collection,
    ::testing::Values(&Admin_command_handler_stub::create_collection,
                      &Admin_command_handler_stub::ensure_collection));

class Admin_command_handler_check_validation
    : public Admin_command_handler_test,
      public ::testing::WithParamInterface<Admin_command_method_t> {};

TEST_P(Admin_command_handler_check_validation, invalid_option) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION_NAME, {"options", Any::Object{{"bar", "baz"}}}});
  ASSERT_ERROR_CODE(ER_X_CMD_INVALID_ARGUMENT,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation, empty_options) {
  set_arguments(
      Any::Object{SCHEMA, COLLECTION_NAME, {"options", Any::Object{}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_obj) {
  set_validation_details({{"schema", Any::Object{{"description", "obj"}}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_string) {
  set_validation_details(
      {{"schema", Scalar::String{"{\"description\": \"value\"}"}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_invalid_validation_schema_obj) {
  set_validation_details({{"schema", Any::Object{{"unknown", "value"}}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_invalid_validation_schema_string) {
  set_validation_details(
      {{"schema", Scalar::String{"{\"unknown\": \"value\"}"}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_invalid_validation_json_obj) {
  set_validation_details({{"schema", Any::Object{{"\"", "invalid json"}}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_invalid_validation_json_string) {
  set_validation_details({{"schema", Scalar::String{"{\"invalid json\"}"}}});
  ASSERT_ERROR_CODE(ER_X_INVALID_VALIDATION_SCHEMA,
                    ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_missing_validation_schema) {
  set_arguments(
      Any::Object{SCHEMA,
                  COLLECTION_NAME,
                  {"options", Any::Object{{"validation", Any::Object{}}}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CREATE TABLE `xtest`.`test_coll`"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_empty_validation_schema_object) {
  set_validation_details({{"schema", Any::Object{}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("GENERATED ALWAYS AS ('{}')"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_empty_validation_schema_string) {
  set_validation_details({{"schema", Scalar::String{"{}"}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("GENERATED ALWAYS AS ('{}')"), _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_level_set_to_strict) {
  set_validation_details({{"schema", Any::Object{}}, {"level", "strict"}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CHECK "
                                    "(JSON_SCHEMA_VALID(_json_schema,"
                                    " doc)) ENFORCED)"),
                          _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_level_set_to_off) {
  set_validation_details({{"schema", Any::Object{}}, {"level", "off"}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CHECK (JSON_SCHEMA_VALID(_json_"
                                    "schema, doc)) NOT ENFORCED)"),
                          _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_level_case_insensitive) {
  set_validation_details({{"schema", Any::Object{}}, {"level", "Strict"}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(mock_data_context,
              execute_sql(HasSubstr("CHECK (JSON_SCHEMA_VALID(_json_"
                                    "schema, doc)) ENFORCED)"),
                          _, _));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, ((*command).*GetParam())(m_args.get()));
}

TEST_P(Admin_command_handler_check_validation,
       create_collection_with_validation_level_invalid_value) {
  set_validation_details({{"schema", Any::Object{}}, {"level", "foo"}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE,
                    ((*command).*GetParam())(m_args.get()));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_create_collection_with_validation,
    Admin_command_handler_check_validation,
    ::testing::Values(&Admin_command_handler_stub::create_collection,
                      &Admin_command_handler_stub::ensure_collection));

}  // namespace test
}  // namespace xpl
