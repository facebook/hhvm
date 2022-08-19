/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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
#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_ASSERT_ERROR_CODE_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_ASSERT_ERROR_CODE_H_

#include <gtest/gtest.h>

#include "plugin/x/generated/mysqlx_error.h"
#include "plugin/x/ngs/include/ngs/error_code.h"

namespace xpl {
namespace test {

inline ::testing::AssertionResult Assert_error_code(const char *e1_expr,
                                                    const char *e2_expr, int e1,
                                                    const ngs::Error_code &e2) {
  return (e1 == e2.error) ? ::testing::AssertionSuccess()
                          : (::testing::AssertionFailure()
                             << "Value of: " << e2_expr << "\nActual: {"
                             << e2.error << ", " << e2.message << "}\n"
                             << "Expected: " << e1_expr << "\nWhich is:" << e1);
}

inline ::testing::AssertionResult Assert_error(const char *e1_expr,
                                               const char *e2_expr,
                                               const ngs::Error_code &e1,
                                               const ngs::Error_code &e2) {
  return (e1.error == e2.error) && (e1.message == e2.message)
             ? ::testing::AssertionSuccess()
             : (::testing::AssertionFailure()
                << "Value of: " << e2_expr << "\nActual: {" << e2.error << ", "
                << e2.message << "}\n"
                << "Expected: " << e1_expr << "\nWhich is: {" << e1.error
                << ", " << e1.message << "}");
}

#define ASSERT_ERROR_CODE(a, b) ASSERT_PRED_FORMAT2(Assert_error_code, a, b);
#define ASSERT_ERROR(a, b) ASSERT_PRED_FORMAT2(Assert_error, a, b);
#define ER_X_SUCCESS 0
#define ER_X_ARTIFICIAL1 7001
#define ER_X_ARTIFICIAL2 7002
#define ER_X_ARTIFICIAL3 7003
#define ER_X_ARTIFICIAL4 7004
#define ER_X_ARTIFICIAL5 7005

}  // namespace test
}  // namespace xpl

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_ASSERT_ERROR_CODE_H_
