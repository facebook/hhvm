/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
   */

#include "my_config.h"

#include "unittest/gunit/mock_parse_tree.h"
#include "unittest/gunit/test_utils.h"

#include "sql/parse_tree_items.h"
#include "sql/regexp/regexp_facade.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace regexp_facade_unittest {

using my_testing::fix;
using my_testing::make_fixed_literal;
using my_testing::Mock_text_literal;
using my_testing::Server_initializer;

using namespace regexp;

class RegexpFacadeTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

class MockRegexpFacade : public Regexp_facade {
 public:
  MockRegexpFacade(THD *thd, const char *pattern)
      : m_thd(thd),
        m_pattern_expr(make_fixed_literal(thd, pattern)),
        m_is_error(SetPattern(m_pattern_expr, 0)) {}

  Mysql::Nullable<int32_t> Find(const char *subject) {
    Item *subject_expr = make_fixed_literal(m_thd, subject);
    return Regexp_facade::Find(subject_expr, 1, 0, false);
  }

  std::int32_t FindValue(const char *subject) {
    auto res = Find(subject);
    // Should be ASSERT_TRUE(), but we can't use that here.
    EXPECT_TRUE(res.has_value());
    return res.value();
  }

  ~MockRegexpFacade() { EXPECT_FALSE(m_is_error); }

 private:
  THD *m_thd;
  Item *m_pattern_expr;
  bool m_is_error;
};

TEST_F(RegexpFacadeTest, Find) {
  auto abc = new Mock_text_literal("abc");
  auto a = new PTI_text_literal_text_string(POS(), false,
                                            {const_cast<char *>("a"), 2});

  fix(thd(), {abc, a});

  {
    bool is_error = false;
    MockRegexpFacade regex(thd(), "a");
    ASSERT_FALSE(is_error);
    EXPECT_EQ(1, regex.FindValue("abc"));
    EXPECT_EQ(Nullable<int>(), regex.Find(nullptr));
  }

  // No error wanted.
  { MockRegexpFacade regex(thd(), nullptr); }

  {
    // It should work even if we don't pre-compile a pattern.
    bool is_error = false;
    MockRegexpFacade regex(thd(), "a");
    ASSERT_FALSE(is_error);
    {
      SCOPED_TRACE("Failure value of: ");
      EXPECT_EQ(Nullable<int>(1), regex.Find("abc"));
    }
  }
}

TEST_F(RegexpFacadeTest, Alignment) {
  auto str = "aaa";

  // Misaligning on purpose, should trigger an assertion or in the worst case
  // a bus error on Sparc.
  if (reinterpret_cast<uintptr_t>(str) % 2 == 0) ++str;

  MockRegexpFacade regex(thd(), str);
}

TEST_F(RegexpFacadeTest, SetPattern) {
  MockRegexpFacade regex(thd(), "a");
  regex.SetPattern(nullptr, 0);
  regex.SetPattern(nullptr, 0);
}

}  // namespace regexp_facade_unittest
