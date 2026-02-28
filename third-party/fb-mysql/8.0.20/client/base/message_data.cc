/*
   Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "client/base/message_data.h"
#include "errmsg.h"

#include "m_ctype.h"
#include "sql_string.h"

using namespace Mysql::Tools::Base;

Message_data::Message_data(uint64 code, std::string message,
                           enum Message_type message_type)
    : m_code(code), m_message(message), m_message_type(message_type) {}

uint64 Message_data::get_code() const { return m_code; }
std::string Message_data::get_message() const { return m_message; }
Message_type Message_data::get_message_type() const { return m_message_type; }
std::string Message_data::get_message_type_string() const {
  return message_type_strings[m_message_type];
}

void Message_data::print_error(std::string program_name) const {
  std::cerr << program_name << ": [" << get_message_type_string() << "] "
            << get_code() << ": " << get_message() << std::endl;
}

const char *Message_data::message_type_strings[] = {
    "INFORMATION", "NOTE", "WARNING", "ERROR", "Unknown message type"};

const int Message_data::message_type_strings_count =
    static_cast<int>(array_elements(Message_data::message_type_strings));

/**
  Check if fatal error is encountered that mysql_upgrade cannot ignore.
  For instance: server is not reachable during upgrade

  @retval
    true    fatal error is encountered
    false   otherwise
*/
bool Message_data::is_fatal() const {
  switch (get_code()) {
    case CR_SERVER_LOST:
    case CR_SERVER_GONE_ERROR:
      return true;
    default:
      return false;
  }
}

void Warning_data::print_error(std::string program_name) const {
  std::cerr << program_name << ": (non fatal) [" << get_message_type_string()
            << "] " << get_code() << ": " << get_message() << std::endl;
}
