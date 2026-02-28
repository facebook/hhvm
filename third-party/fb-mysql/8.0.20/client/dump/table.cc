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

#include "client/dump/table.h"

#include <boost/algorithm/string.hpp>
#include <sstream>

#include "client/dump/pattern_matcher.h"

using namespace Mysql::Tools::Dump;

Table::Table(uint64 id, const std::string &name, const std::string &schema,
             const std::string &sql_formatted_definition,
             std::vector<Field> &fields, std::string type, uint64 row_count,
             uint64 row_bound, uint64 data_lenght)
    : Abstract_plain_sql_object(id, name, schema, sql_formatted_definition),
      m_fields(fields),
      m_type(type),
      m_row_count(row_count),
      m_row_bound(row_bound),
      m_data_lenght(data_lenght) {
  using Detail::Pattern_matcher;
  bool engine_line_read = false;
  bool first_line = true;
  std::stringstream definition_stream(sql_formatted_definition);
  for (std::string line; std::getline(definition_stream, line);) {
    /*
      MAINTAINER: This code parses the output of SHOW CREATE TABLE.
      @TODO: Instead, look up INFORMATION_SCHEMA and get the table details.
    */

    boost::trim_left(line);
    if (!engine_line_read) boost::trim_if(line, boost::is_any_of(","));
    if (boost::starts_with(line, "KEY ") ||
        boost::starts_with(line, "INDEX ") ||
        boost::starts_with(line, "UNIQUE KEY ") ||
        boost::starts_with(line, "UNIQUE INDEX ") ||
        boost::starts_with(line, "FULLTEXT KEY ") ||
        boost::starts_with(line, "FULLTEXT INDEX ") ||
        boost::starts_with(line, "SPATIAL KEY ") ||
        boost::starts_with(line, "SPATIAL INDEX ") ||
        boost::starts_with(line, "CONSTRAINT ")) {
      m_indexes_sql_definition.push_back(line);
    } else {
      /*
        Make sure we detect the table options clauses,
        even with different syntaxes (with or without TABLESPACE)
      */
      if (boost::starts_with(line, ")") && boost::contains(line, "ENGINE=")) {
        engine_line_read = true;
        std::string &sql_def = m_sql_definition_without_indexes;
        sql_def = boost::algorithm::replace_last_copy(sql_def, ",", "");
      } else if (!first_line && !engine_line_read)
        line += ",";
      m_sql_definition_without_indexes += line + '\n';
    }
    first_line = false;
  }
}

const std::string &Table::get_sql_definition_without_indexes() const {
  return m_sql_definition_without_indexes;
}

const std::vector<std::string> &Table::get_indexes_sql_definition() const {
  return m_indexes_sql_definition;
}

const std::vector<Field> &Table::get_fields() const { return m_fields; }

uint64 Table::get_row_data_lenght() const { return m_data_lenght; }

uint64 Table::get_row_count_bound() const { return m_row_bound; }

uint64 Table::get_row_count() const { return m_row_count; }

std::string Table::get_type() const { return m_type; }
