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

#include "client/mysqltest/expected_errors.h"

std::string Expected_errors::error_list() {
  std::string error_list("");

  for (std::size_t i = 0; i < m_errors.size(); i++) {
    if (i > 0) error_list.append(",");
    if (m_errors.at(i)->type() == ERR_ERRNO)
      error_list.append(std::to_string(m_errors.at(i)->error_code()));
    else
      error_list.append(m_errors.at(i)->sqlstate());
  }

  return error_list;
}

std::vector<unsigned int> Expected_errors::errors() {
  std::vector<unsigned int> errors;
  for (std::size_t i = 0; i < m_errors.size(); i++) {
    errors.push_back(m_errors.at(i)->error_code());
  }
  return errors;
}

void Expected_errors::add_error(const char *sqlstate, error_type type) {
  std::unique_ptr<Error> new_error(new Error(0, sqlstate, type));
  m_errors.push_back(std::move(new_error));
}

void Expected_errors::add_error(std::uint32_t error_code, error_type type) {
  std::unique_ptr<Error> new_error(new Error(error_code, "00000", type));
  m_errors.push_back(std::move(new_error));
}
