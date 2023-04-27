/*
  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TABLE_INCLUDED
#define TABLE_INCLUDED

#include <string>
#include <vector>

#include "client/dump/abstract_plain_sql_object.h"
#include "client/dump/field.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class Table : public Abstract_plain_sql_object {
 public:
  Table(uint64 id, const std::string &name, const std::string &schema,
        const std::string &sql_formatted_definition, std::vector<Field> &fields,
        std::string type, uint64 row_count, uint64 row_bound,
        uint64 data_lenght);

  /**
    Retrieves type name.
   */
  std::string get_type() const;

  /**
    Retrieves number of rows in table, this value can be approximate.
   */
  uint64 get_row_count() const;

  /**
    Retrieves maximum number of rows in table. This value can be approximate,
    but should be upper bound for actual number of rows.
   */
  uint64 get_row_count_bound() const;

  /**
    Retrieves total number of bytes of rows data. This value can be
    approximate.
   */
  uint64 get_row_data_lenght() const;

  const std::vector<Field> &get_fields() const;

  const std::vector<std::string> &get_indexes_sql_definition() const;

  const std::string &get_sql_definition_without_indexes() const;

 private:
  std::vector<Field> m_fields;
  std::vector<std::string> m_indexes_sql_definition;
  std::string m_sql_definition_without_indexes;
  std::string m_type;
  uint64 m_row_count;
  uint64 m_row_bound;
  uint64 m_data_lenght;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
