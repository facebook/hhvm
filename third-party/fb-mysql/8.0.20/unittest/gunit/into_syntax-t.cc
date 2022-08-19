/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

namespace into_syntax_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class IntoSyntaxTest : public ParserTest {};

TEST_F(IntoSyntaxTest, Outer) {
  SELECT_LEX *term = parse("SELECT 1 INTO @v");
  SELECT_LEX_UNIT *top_union = term->master_unit();
  EXPECT_EQ(nullptr, top_union->outer_select());
}

}  // namespace into_syntax_unittest
