/*
 * Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_COMPONENT_SYS_VARIABLE_REGISTER_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_COMPONENT_SYS_VARIABLE_REGISTER_H_

#include <gmock/gmock.h>

#include "plugin/x/src/interface/service_sys_variables.h"

namespace ngs {
namespace test {

class Mock_service_sys_variables : public xpl::iface::Service_sys_variables {
 public:
  MOCK_METHOD4(get_variable, bool(const char *component_name, const char *name,
                                  void **val, size_t *out_length_of_val));
  MOCK_CONST_METHOD0(is_valid, bool());
};

}  // namespace test
}  // namespace ngs

#endif  //  UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_COMPONENT_SYS_VARIABLE_REGISTER_H_
