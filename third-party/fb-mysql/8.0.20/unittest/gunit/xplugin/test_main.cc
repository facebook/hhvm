/*
 * Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.
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
#include <stdio.h>

#include "m_ctype.h"
#include "my_sys.h"

#include "unittest/gunit/xplugin/xcl/test_environment.h"
#include "unittest/gunit/xplugin/xpl/test_environment.h"

const CHARSET_INFO *data_ctx_charset = &my_charset_utf8mb4_general_ci;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  my_init();

  // Setup default values for not specified mock-returns
  ::testing::AddGlobalTestEnvironment(new xpl::Environment());
  ::testing::AddGlobalTestEnvironment(new xcl::Environment());

  return RUN_ALL_TESTS();
}
