/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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
#include <memory>

#include "plugin/x/src/admin_cmd_arguments.h"
#include "plugin/x/src/index_array_field.h"
#include "plugin/x/src/query_string_builder.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;

namespace {
#define PATH "$.path"
#define NOT_REQUIRED false
#define REQUIRED true
#define OPTIONS 42u
#define SRID 666u
#define DEFAULT_VALUE std::numeric_limits<uint64_t>::max()
#define CAST_TO_ARRAY(type) \
  "(CAST(JSON_EXTRACT(`doc`,'" PATH "') AS " type " ARRAY))"

struct Array_field_info : public Admin_command_index::Index_field_info {
  Array_field_info(
      const std::string &path, const std::string &type,
      const bool is_required = false,
      const uint64_t options = std::numeric_limits<uint64_t>::max(),
      const uint64_t srid = std::numeric_limits<uint64_t>::max()) {
    m_path = path;
    m_type = type;
    m_is_required = is_required;
    m_options = options;
    m_srid = srid;
  }
};

}  // namespace

class Index_array_field_create_test
    : public ::testing::TestWithParam<Array_field_info> {};

TEST_P(Index_array_field_create_test, fail_on_create) {
  const auto &param = GetParam();
  ngs::Error_code error;
  std::unique_ptr<const Index_array_field> field(
      Index_array_field::create(param, &error));
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE, error);
  ASSERT_EQ(nullptr, field.get());
}

Array_field_info fail_on_create_array_field_param[] = {
    {"", "DECIMAL"},
    {PATH, ""},
    {PATH, "DECIMAL", REQUIRED, DEFAULT_VALUE, DEFAULT_VALUE},
    {PATH, "DECIMAL", NOT_REQUIRED, OPTIONS, DEFAULT_VALUE},
    {PATH, "DECIMAL", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "DECIMAL SIGNED"},
    {PATH, "DECIMAL UNSIGNED"},
    {PATH, "tinyint"},
    {PATH, "smallint"},
    {PATH, "mediumint"},
    {PATH, "int"},
    {PATH, "integer"},
    {PATH, "bigint"},
    {PATH, "real"},
    {PATH, "float"},
    {PATH, "double"},
    {PATH, "numeric"},
    {PATH, "date(10)"},
    {PATH, "date(10,2)"},
    {PATH, "date unsigned"},
    {PATH, "time(10)"},
    {PATH, "time(10,2)"},
    {PATH, "time unsigned"},
    {PATH, "timestamp"},
    {PATH, "datetime(10)"},
    {PATH, "datetime(10,2)"},
    {PATH, "datetime unsigned"},
    {PATH, "year"},
    {PATH, "bit"},
    {PATH, "blob"},
    {PATH, "text"},
    {PATH, "geojson"},
    {PATH, "fulltext"},
};

INSTANTIATE_TEST_CASE_P(fail_on_create_field, Index_array_field_create_test,
                        ::testing::ValuesIn(fail_on_create_array_field_param));

struct Param_index_array_field_add_field {
  std::string expect;
  Array_field_info info;
};

class Index_array_field_add_field_test
    : public ::testing::TestWithParam<Param_index_array_field_add_field> {};

TEST_P(Index_array_field_add_field_test, add_field) {
  const auto &param = GetParam();
  Query_string_builder qb;
  ngs::Error_code error;
  std::unique_ptr<const Index_array_field> field(
      Index_array_field::create(param.info, &error));
  ASSERT_NE(nullptr, field.get());
  field->add_field(&qb);
  ASSERT_STREQ(param.expect.c_str(), qb.get().c_str());
}

Param_index_array_field_add_field add_array_field_param[] = {
    {CAST_TO_ARRAY("BINARY"), {PATH, "BINARY"}},
    {CAST_TO_ARRAY("binary(20)"), {PATH, "binary(20)"}},
    {CAST_TO_ARRAY("date"), {PATH, "date"}},
    {CAST_TO_ARRAY("time"), {PATH, "time"}},
    {CAST_TO_ARRAY("datetime"), {PATH, "datetime"}},
    {CAST_TO_ARRAY("char"), {PATH, "char"}},
    {CAST_TO_ARRAY("char(20)"), {PATH, "char(20)"}},
    //    {CAST_TO_ARRAY("char(30) character set latin1"),
    //     {PATH, "char(30) character set latin1"}},
    //    {CAST_TO_ARRAY("char(40) collate latin1_bin"),
    //     {PATH, "char(40) collate latin1_bin"}},
    //    {CAST_TO_ARRAY("char(50) character set latin1 collate latin1_bin"),
    //     {PATH, "char(50) character set latin1 collate latin1_bin"}},
    {CAST_TO_ARRAY("decimal"), {PATH, "decimal"}},
    {CAST_TO_ARRAY("DEcimAL"), {PATH, "DEcimAL"}},
    {CAST_TO_ARRAY("DECIMAL(32)"), {PATH, "DECIMAL(32)"}},
    {CAST_TO_ARRAY("DECIMAL(32,16)"), {PATH, "DECIMAL(32,16)"}},
    {CAST_TO_ARRAY("DECIMAL(0,16)"), {PATH, "DECIMAL(0,16)"}},
    {CAST_TO_ARRAY("unsigned"), {PATH, "unsigned"}},
    {CAST_TO_ARRAY("unsigned integer"), {PATH, "unsigned integer"}},
    {CAST_TO_ARRAY("signed"), {PATH, "signed"}},
    {CAST_TO_ARRAY("signed integer"), {PATH, "signed integer"}},
};

INSTANTIATE_TEST_CASE_P(get_index_field_name, Index_array_field_add_field_test,
                        ::testing::ValuesIn(add_array_field_param));

}  // namespace test
}  // namespace xpl
