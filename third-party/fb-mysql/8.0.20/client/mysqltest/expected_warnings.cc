// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

#include "client/mysqltest/expected_warnings.h"

void Expected_warnings::add_warning(std::uint32_t warning_code,
                                    const char *warning_name,
                                    bool once_property) {
  for (std::size_t i = 0; i < m_warnings.size(); i++) {
    // Warning already exist, don't add it.
    if (m_warnings.at(i)->warning_code() == warning_code) return;
  }

  // Add a new warning to the existing list.
  std::unique_ptr<Warning> new_warning(
      new Warning(warning_code, warning_name, once_property));
  m_warnings.push_back(std::move(new_warning));
}

void Expected_warnings::remove_warning(std::uint32_t error_code,
                                       bool once_property) {
  for (std::size_t i = 0; i < m_warnings.size(); i++) {
    if (m_warnings.at(i)->warning_code() == error_code) {
      if (once_property)
        m_warnings.at(i)->set_ignore_warning(true);
      else
        m_warnings.erase(begin() + i);
    }
  }
}

void Expected_warnings::update_list() {
  for (std::size_t i = 0; i < m_warnings.size(); i++) {
    if (m_warnings.at(i)->expired()) {
      m_warnings.erase(begin() + i);
      i--;
    } else if (m_warnings.at(i)->ignore_warning()) {
      m_warnings.at(i)->set_ignore_warning(false);
    }
  }
}

std::string Expected_warnings::warnings_list() {
  std::string warnings;
  for (std::size_t i = 0; i < m_warnings.size(); i++) {
    if (i > 0) warnings.append(",");
    warnings.append(m_warnings.at(i)->warning_name());
  }

  return warnings;
}
