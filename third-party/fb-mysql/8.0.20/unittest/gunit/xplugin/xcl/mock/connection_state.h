/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_STATE_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_STATE_H_

#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <utility>

#include "plugin/x/client/mysqlxclient/xconnection.h"

namespace xcl {
namespace test {

class Mock_connection_state : public XConnection::State {
 public:
  MOCK_CONST_METHOD0(is_ssl_configured, bool());
  MOCK_CONST_METHOD0(is_ssl_activated, bool());
  MOCK_CONST_METHOD0(is_connected, bool());
  MOCK_CONST_METHOD0(get_ssl_version, std::string());
  MOCK_CONST_METHOD0(get_ssl_cipher, std::string());
  MOCK_CONST_METHOD0(get_connection_type, Connection_type());
  MOCK_CONST_METHOD0(has_data, bool());
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_STATE_H_
