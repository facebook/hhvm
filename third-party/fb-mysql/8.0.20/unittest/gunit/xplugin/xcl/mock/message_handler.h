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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_MESSAGE_HANDLER_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_MESSAGE_HANDLER_H_

#include <gmock/gmock.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "plugin/x/client/mysqlxclient/xprotocol.h"

namespace xcl {
namespace test {

class Mock_handlers {
 public:
  XProtocol::Server_message_handler get_mock_lambda_received_message_handler() {
    return
        [this](XProtocol *protocol, const XProtocol::Server_message_type_id id,
               const XProtocol::Message &msg) -> Handler_result {
          return this->received_message_handler(protocol, id, msg);
        };
  }

  XProtocol::Client_message_handler get_mock_lambda_send_message_handler() {
    return
        [this](XProtocol *protocol, const XProtocol::Client_message_type_id id,
               const XProtocol::Message &msg) -> Handler_result {
          return this->send_message_handler(protocol, id, msg);
        };
  }

  XProtocol::Notice_handler get_mock_lambda_notice_handler() {
    return [this](XProtocol *protocol, const bool is_global,
                  const Mysqlx::Notice::Frame::Type type, const char *payload,
                  const uint32_t payload_size) -> Handler_result {
      return this->notice_handler(protocol, is_global, type, payload,
                                  payload_size);
    };
  }

 public:
  MOCK_METHOD3(received_message_handler,
               Handler_result(XProtocol *,
                              const XProtocol::Server_message_type_id,
                              const XProtocol::Message &));

  MOCK_METHOD3(send_message_handler,
               Handler_result(XProtocol *,
                              const XProtocol::Client_message_type_id,
                              const XProtocol::Message &));

  MOCK_METHOD5(notice_handler, Handler_result(XProtocol *, const bool,
                                              const Mysqlx::Notice::Frame::Type,
                                              const char *, const uint32_t));
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_MESSAGE_HANDLER_H_
