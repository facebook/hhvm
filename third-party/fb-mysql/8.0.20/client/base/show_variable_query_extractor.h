/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SHOW_VARIABLE_QUERY_EXTRACTOR_INCLUDED
#define SHOW_VARIABLE_QUERY_EXTRACTOR_INCLUDED

#include <string>
#include <vector>

#include "client/base/mysql_query_runner.h"
#include "my_inttypes.h"
#include "mysql.h"

namespace Mysql {
namespace Tools {
namespace Base {

/**
  Extracts the value of server variable.
 */
class Show_variable_query_extractor {
 public:
  /**
    Extract the value of server variable.

    @param[in] query_runner MySQL query runner to use.
    @param[in] variable Name of variable to get value of.
    @param[out] value reference to String to store variable value to.
    @param[out] exists reference to bool to store if variable was found.
    @return nonzero if error was encountered.
   */
  static int64 get_variable_value(Mysql_query_runner *query_runner,
                                  std::string variable, std::string &value,
                                  bool &exists);

 private:
  Show_variable_query_extractor();
  /**
    Result row callback to be used in query runner.
   */
  int64 extract_variable(const Mysql_query_runner::Row &result_row);

  /**
  Temporary placeholder for extracted value.
  */
  std::string m_extracted_variable;

  /**
  Temporary placeholder for value received flag.
  */
  bool m_exists;
};

}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
