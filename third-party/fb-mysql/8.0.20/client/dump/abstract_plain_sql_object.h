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

#ifndef ABSTRACT_PLAIN_SQL_OBJECT_INCLUDED
#define ABSTRACT_PLAIN_SQL_OBJECT_INCLUDED

#include "client/dump/abstract_data_object.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Abstract object carrying its definition in SQL formatted string only.
 */
class Abstract_plain_sql_object : public Abstract_data_object {
 public:
  Abstract_plain_sql_object(uint64 id, const std::string &name,
                            const std::string &schema,
                            const std::string &sql_formatted_definition);

  std::string get_sql_formatted_definition() const;
  void set_sql_formatted_definition(std::string);

 private:
  /**
    SQL formatted object definition.
   */
  std::string m_sql_formatted_definition;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
