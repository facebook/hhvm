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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_H_

#include <gmock/gmock.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "plugin/x/client/mysqlxclient/xconnection.h"

namespace xcl {
namespace test {

class Mock_connection : public XConnection {
 public:
  MOCK_METHOD1(connect_to_localhost, XError(const std::string &unix_socket));
  MOCK_METHOD3(connect, XError(const std::string &host, const uint16_t port,
                               const Internet_protocol ip_mode));
  MOCK_METHOD0(get_socket_fd, my_socket());
  MOCK_METHOD0(activate_tls, XError());
  MOCK_METHOD1(shutdown, XError(const Shutdown_type how_to_shutdown));
  MOCK_METHOD2(write,
               XError(const uint8_t *data, const std::size_t data_length));
  MOCK_METHOD2(read, XError(uint8_t *data, const std::size_t data_length));
  MOCK_METHOD1(set_read_timeout, XError(const int deadline_milliseconds));
  MOCK_METHOD1(set_write_timeout, XError(const int deadline_milliseconds));
  MOCK_METHOD0(close, void());
  MOCK_METHOD0(state, const State &());
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_CONNECTION_H_
