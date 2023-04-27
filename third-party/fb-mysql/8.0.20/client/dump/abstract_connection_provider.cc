/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/dump/abstract_connection_provider.h"

#include <stddef.h>
#include <functional>

using namespace Mysql::Tools::Dump;
using std::placeholders::_1;

int64 Abstract_connection_provider::Message_handler_wrapper::pass_message(
    const Mysql::Tools::Base::Message_data &message) {
  if (m_message_handler != nullptr) {
    return (*m_message_handler)(message) ? -1 : 0;
  }
  return 0;
}

Abstract_connection_provider::Message_handler_wrapper::Message_handler_wrapper(
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler)
    : m_message_handler(message_handler) {}

Mysql::Tools::Base::Mysql_query_runner *
Abstract_connection_provider::create_new_runner(
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler) {
  MYSQL *connection = m_connection_factory->create_connection();
  if (!connection) return nullptr;
  Message_handler_wrapper *message_wrapper =
      new Message_handler_wrapper(message_handler);
  auto *callback =
      new std::function<int64(const Mysql::Tools::Base::Message_data &)>(
          std::bind(&Message_handler_wrapper::pass_message, message_wrapper,
                    _1));
  auto cleanup_callback = [message_wrapper] { delete message_wrapper; };

  return &((new Mysql::Tools::Base::Mysql_query_runner(connection))
               ->add_message_callback(callback, cleanup_callback));
}

Abstract_connection_provider::Abstract_connection_provider(
    Mysql::Tools::Base::I_connection_factory *connection_factory)
    : m_connection_factory(connection_factory) {}
