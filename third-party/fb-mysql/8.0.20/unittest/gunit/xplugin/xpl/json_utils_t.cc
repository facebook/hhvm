/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2.0,
 as published by the Free Software Foundation.

 This program is also distributed with certain software (including
 but not limited to OpenSSL) that is licensed under separate terms,
 as designated in a particular file or component or in included license
 documentation.  The authors of MySQL hereby grant you an additional
 permission to link the program and your derivative works with the
 separately licensed software that they have included with MySQL.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License, version 2.0, for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>

#include "plugin/x/src/json_utils.h"

namespace xpl {
namespace test {

struct Param_is_id_in_json {
  bool expect;
  std::string json;
};

class Is_id_in_json_test : public testing::TestWithParam<Param_is_id_in_json> {
};

TEST_P(Is_id_in_json_test, is_id_in_json) {
  const Param_is_id_in_json &param = GetParam();
  EXPECT_EQ(param.expect, is_id_in_json(param.json));
}

Param_is_id_in_json is_id_in_json_param[] = {
    {true, ""},
    {true, "a, b, c"},
    {true, "1, 2, 3"},
    {false, "{}"},
    {true, "{a, b, c}"},
    {false, "[]"},
    {false, "[1,2,3]"},
    {true, "{\"_id\": 1}"},
    {false, "{\"a\": 1}"},
    {true, "{\"a\": 1, \"_id\": 2}"},
    {false, "{\"a\": {\"_id\": 2}}"},
    {false, "{\"a\": [1,{\"_id\": 2},3]}"},
    {true, "{\"a\": {\"_id\": 2}, \"_id\": 3}"},
    {false, "{\"a\": [1,{\"_id\": 2},3]}"},
    {true, "{\"a\": {\"_id\": 2}, \"_id\": 3, \"b\": {\"_id\": 4}}"},
};

INSTANTIATE_TEST_CASE_P(is_id_in_json, Is_id_in_json_test,
                        testing::ValuesIn(is_id_in_json_param));

}  // namespace test
}  // namespace xpl
