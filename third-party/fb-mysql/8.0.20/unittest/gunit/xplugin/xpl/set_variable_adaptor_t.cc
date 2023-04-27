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

#include <gtest/gtest.h>

#include "plugin/x/src/capabilities/set_variable_adaptor.h"

namespace xpl {
namespace test {

const ulonglong k_label_a = 1 << 0;
const ulonglong k_label_b = 1 << 1;
const ulonglong k_label_c = 1 << 2;
const ulonglong k_label_zero = 0;

class Set_variable_adaptor_test : public ::testing::Test {
 public:
  void set_variable_value(ulonglong val) { *m_variable.value() = val; }
  Set_variable m_variable{{"LABEL_A", "LABEL_B", "LABEL_C"}};
  enum class Labels { k_B, k_C, k_A };
  Set_variable_adaptor<Labels> m_adaptor{
      m_variable, {Labels::k_A, Labels::k_B, Labels::k_C}};
};

TEST_F(Set_variable_adaptor_test, get_value) {
  EXPECT_EQ(Labels::k_A, m_adaptor.get_value("label_a"));
  EXPECT_EQ(Labels::k_B, m_adaptor.get_value("label_b"));
  EXPECT_EQ(Labels::k_C, m_adaptor.get_value("label_c"));
}

TEST_F(Set_variable_adaptor_test, get_allowed_values_all) {
  set_variable_value(k_label_a + k_label_b + k_label_c);
  std::vector<std::string> result, expect{"label_a", "label_b", "label_c"};
  m_adaptor.get_allowed_values(&result);
  EXPECT_EQ(expect, result);
}

TEST_F(Set_variable_adaptor_test, get_allowed_values_some) {
  set_variable_value(k_label_a + k_label_c);
  std::vector<std::string> result, expect{"label_a", "label_c"};
  m_adaptor.get_allowed_values(&result);
  EXPECT_EQ(expect, result);
}

struct Param_is_allowed_value {
  ulonglong m_value;
  bool m_label_a, m_label_b, m_label_c;
};

class Set_variable_adaptor_is_allowed_value_test
    : public Set_variable_adaptor_test,
      public testing::WithParamInterface<Param_is_allowed_value> {};

TEST_P(Set_variable_adaptor_is_allowed_value_test, is_allowed_value) {
  const auto &param = GetParam();
  set_variable_value(param.m_value);
  EXPECT_EQ(param.m_label_a, m_adaptor.is_allowed_value("label_a"));
  EXPECT_EQ(param.m_label_b, m_adaptor.is_allowed_value("label_b"));
  EXPECT_EQ(param.m_label_c, m_adaptor.is_allowed_value("label_c"));
  EXPECT_FALSE(m_adaptor.is_allowed_value("label_d"));
}

Param_is_allowed_value is_allowed_value_param[] = {
    {k_label_zero, false, false, false},
    {k_label_a, true, false, false},
    {k_label_b, false, true, false},
    {k_label_c, false, false, true},
    {k_label_a + k_label_b, true, true, false},
    {k_label_a + k_label_c, true, false, true},
    {k_label_b + k_label_c, false, true, true},
    {k_label_a + k_label_b + k_label_c, true, true, true},
};

INSTANTIATE_TEST_CASE_P(Set_variable_adaptor_is_allowed_value,
                        Set_variable_adaptor_is_allowed_value_test,
                        testing::ValuesIn(is_allowed_value_param));

}  // namespace test
}  // namespace xpl
