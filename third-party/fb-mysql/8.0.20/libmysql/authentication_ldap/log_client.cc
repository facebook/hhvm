/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "log_client.h"

Ldap_logger::Ldap_logger() {
  m_log_level = LDAP_LOG_LEVEL_NONE;
  m_log_writer = nullptr;
  m_log_writer = new Ldap_log_writer_error();
}

Ldap_logger::~Ldap_logger() {
  if (m_log_writer) {
    delete m_log_writer;
  }
}

void Ldap_logger::set_log_level(ldap_log_level level) { m_log_level = level; }

void Ldap_log_writer_error::write(std::string data) {
  std::cerr << data << "\n";
  std::cerr.flush();
}

/**
  This class writes error into default error streams.
  We needed this constructor because of template class usage.
*/
Ldap_log_writer_error::Ldap_log_writer_error() {}

/**
 */
Ldap_log_writer_error::~Ldap_log_writer_error() {}
