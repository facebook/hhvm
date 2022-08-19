/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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
#include <string>

#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "unittest/gunit/mock_parse_tree.h"
#include "unittest/gunit/test_utils.h"

#include "sql/parse_tree_items.h"
#include "sql/regexp/regexp_engine.h"

namespace regexp_engine_unittest {

using my_testing::fix;
using my_testing::make_fixed_literal;
using my_testing::Mock_text_literal;
using my_testing::Server_initializer;

using regexp::Regexp_engine;
using regexp::regexp_lib_charset;

class RegexpEngineTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  std::u16string m_pattern{'b'};
  std::u16string m_subject{'a', 'b', 'c'};
  std::u16string m_replacement{'x'};
};

class Mock_regexp_engine : public regexp::Regexp_engine {
 public:
  Mock_regexp_engine(const std::u16string &pattern,
                     const std::u16string &subject,
                     std::initializer_list<char16_t> expected_buffer)
      : Regexp_engine(pattern, 0, 0, 0), m_expected_buffer(expected_buffer) {
    EXPECT_EQ(U_ZERO_ERROR, m_error_code);
    Reset(subject);
  }

  void AppendReplacement(const std::u16string &replacement) {
    regexp::Regexp_engine::AppendReplacement(replacement);
  }

  ~Mock_regexp_engine() {
    EXPECT_EQ(m_expected_buffer.size(), m_replace_buffer.size());
    if (m_replace_buffer.size() < m_expected_buffer.size()) return;
    for (uint i = 0; i < m_replace_buffer.size(); ++i)
      // The cast makes the character human-readable.
      EXPECT_EQ(static_cast<char>(m_expected_buffer[i]),
                static_cast<char>(m_replace_buffer[i]))
          << "at position " << i << ".";
  }

  void set_replace_buffer(std::initializer_list<char16_t> buffer) {
    m_replace_buffer = buffer;
  }

  void resize_buffer(size_t size) { m_replace_buffer.resize(size); }

  void set_replace_pos(size_t pos) { m_replace_buffer_pos = pos; }

  int replace_pos() const { return m_replace_buffer_pos; }

  URegularExpression *re() const { return m_re; }

 private:
  std::u16string m_expected_buffer;
};

TEST_F(RegexpEngineTest, AppendHead0) {
  Mock_regexp_engine engine(m_pattern, m_subject, {});
  engine.AppendHead(0);
}

TEST_F(RegexpEngineTest, AppendHead1) {
  Mock_regexp_engine engine(m_pattern, m_subject, {'a'});
  engine.AppendHead(1);
}

TEST_F(RegexpEngineTest, AppendHead2) {
  Mock_regexp_engine engine(m_pattern, m_subject, {'a', 'b'});
  engine.AppendHead(2);
}

TEST_F(RegexpEngineTest, AppendReplacement) {
  Mock_regexp_engine engine(m_pattern, m_subject, {'a', 'x', '\0'});

  UErrorCode error_code = U_ZERO_ERROR;

  EXPECT_TRUE(uregex_find(engine.re(), 0, &error_code));
  engine.resize_buffer(3);
  engine.set_replace_pos(0);

  engine.AppendReplacement(m_replacement);
  EXPECT_EQ(2, engine.replace_pos());
}

TEST_F(RegexpEngineTest, AppendReplacementGrowBuffer) {
  Mock_regexp_engine engine(m_pattern, m_subject, {'a', 'x'});

  UErrorCode error_code = U_ZERO_ERROR;

  // We simulate starting on the second character, and fake an AppendHead()
  // operation.
  EXPECT_TRUE(uregex_find(engine.re(), 1, &error_code));
  engine.set_replace_buffer({'a'});
  engine.set_replace_pos(1);

  engine.AppendReplacement(m_replacement);
  EXPECT_EQ(2, engine.replace_pos());
}

TEST_F(RegexpEngineTest, AppendTail) {
  Mock_regexp_engine engine(m_pattern, m_subject, {'a', 'x', 'c'});

  UErrorCode error_code = U_ZERO_ERROR;

  EXPECT_TRUE(uregex_find(engine.re(), 0, &error_code));
  engine.resize_buffer(3);
  engine.AppendReplacement(m_replacement);
  engine.AppendTail();
  EXPECT_EQ(3, engine.replace_pos());
}

}  // namespace regexp_engine_unittest
