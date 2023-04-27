/*
 * Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_CAPABILITIES_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_CAPABILITIES_H_

#include "plugin/x/src/capabilities/configurator.h"
#include "plugin/x/src/capabilities/handler.h"

namespace xpl {

namespace test {

class Mock_capabilities_configurator : public Capabilities_configurator {
 public:
  Mock_capabilities_configurator()
      : Capabilities_configurator(std::vector<Capability_handler_ptr>()) {}

  MOCK_METHOD0(get, ::Mysqlx::Connection::Capabilities *());

  MOCK_METHOD1(
      prepare_set,
      ngs::Error_code(const ::Mysqlx::Connection::Capabilities &capabilities));
  MOCK_METHOD0(commit, void());
};

class Mock_capability_handler : public Capability_handler {
 public:
  MOCK_CONST_METHOD0(name, std::string());
  MOCK_CONST_METHOD0(is_supported_impl, bool());
  MOCK_CONST_METHOD0(is_settable, bool());
  MOCK_CONST_METHOD0(is_gettable, bool());
  MOCK_METHOD1(set_impl, ngs::Error_code(const ::Mysqlx::Datatypes::Any &));

  // Workaround for GMOCK undefined behaviour with ResultHolder
  MOCK_METHOD1(get_void, bool(::Mysqlx::Datatypes::Any *));
  MOCK_METHOD0(commit_void, bool());

  void get_impl(::Mysqlx::Datatypes::Any *any) { get_void(any); }

  void commit() { commit_void(); }
};

}  // namespace test
}  // namespace xpl

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_CAPABILITIES_H_
