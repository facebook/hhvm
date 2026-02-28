/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SYSTEM_VIEWS__SYSTEM_VIEW_DEFINITION_IMPL_INCLUDED
#define DD_SYSTEM_VIEWS__SYSTEM_VIEW_DEFINITION_IMPL_INCLUDED

#include <map>
#include <memory>
#include <vector>

#include "sql/dd/string_type.h"                   // dd::String_type
#include "sql/dd/types/system_view_definition.h"  // dd::System_view_definition
#include "sql/mysqld.h"                           // lower_case_table_names

namespace dd {
namespace system_views {

class System_view_definition_impl : public System_view_definition {
 public:
  /**
    Get view name.

    @return name of the view.
  */
  virtual const String_type &view_name() const { return m_view_name; }

  /**
    Set view name.
  */
  virtual void set_view_name(const String_type &name) { m_view_name = name; }

  /**
    Get collation clause to append to view definition for some
    view columns based on lower_case_table_names.

    @return Empty string if lctn=0, other wise " COLLATE utf8_tolower_ci".
  */
  static const String_type fs_name_collation() {
    if (lower_case_table_names != 0) return " COLLATE utf8_tolower_ci";
    return "";
  }

  virtual String_type build_ddl_create_view() const = 0;

 private:
  // Name of I_S system view;
  String_type m_view_name;
};

class System_view_select_definition_impl : public System_view_definition_impl {
 public:
  /**
    Add a field definition for the SELECT projection.
    This function can be called more than once. The call will add a new
    projection to the SELECT command.

    @param field_number  Ordinal position of field in the projection list.
    @param field_name    Field name used for the SELECT's projection.
    @param field_definition Expression representing the projection.
    @param add_quotes    If true, output single quotes around the
                         field_definition.
  */
  virtual void add_field(int field_number, const String_type &field_name,
                         const String_type &field_definition,
                         bool add_quotes = false) {
    // Make sure the field_number and field_name are not added twise.
    DBUG_ASSERT(m_field_numbers.find(field_name) == m_field_numbers.end() &&
                m_field_definitions.find(field_number) ==
                    m_field_definitions.end());

    // Store the field number.
    m_field_numbers[field_name] = field_number;

    // Store the field definition expression.
    Stringstream_type ss;
    if (field_name == "*") {
      ss << " * ";
    } else {
      if (add_quotes) {
        DBUG_ASSERT(field_definition.find('\'') == String_type::npos);
        ss << '\'' << field_definition << '\'';
      } else
        ss << field_definition;

      ss << " AS " << field_name;
    }

    m_field_definitions[field_number] = ss.str();
  }

  /**
    Add FROM clause for the SELECT.
    This function can be called more than once. The clause will be appended to
    the previous FROM clause string.

    @param from  String representing the FROM clause.
  */
  virtual void add_from(const String_type &from) {
    m_from_clauses.push_back(from);
  }

  /**
    Add WHERE clause for the SELECT.
    This function can be called more than once. The clause will be appended to
    the previous WHERE clause string.

    @param where  String representing the WHERE clause.
  */
  virtual void add_where(const String_type &where) {
    m_where_clauses.push_back(where);
  }

  /**
    Add CTE expression before SELECT.

    @param cte  String representing the CTE expression.
  */
  virtual void add_cte_expression(const String_type &cte) {
    m_cte_expression = cte;
  }

  /**
    Indicates that we should add DISTINCT clause to SELECT.
  */
  virtual void add_distinct() { m_is_distinct = true; }

  /**
    Indicates selection of all field (SELECT '*').
  */
  virtual void add_star() { m_add_star = true; }
  /**
    Get the field ordinal position number for the given field name.

    @param field_name  Column name for which the field number is returned.

    @return Integer representing position of column in projection list.
  */
  virtual int field_number(const String_type &field_name) const {
    DBUG_ASSERT(m_field_numbers.find(field_name) != m_field_numbers.end());
    return m_field_numbers.find(field_name)->second;
  }

  /**
    Build the SELECT query that is used in the CREATE VIEW command.

    @return The SELECT query string.
  */
  String_type build_select_query() const {
    Stringstream_type ss;

    if (!m_cte_expression.empty()) ss << m_cte_expression << "\n ";

    // Make SELECT [DISTINCT]
    ss << "SELECT " << (m_is_distinct ? "DISTINCT \n" : "\n");

    if (!m_add_star) {
      // Output view column definitions
      for (Field_definitions::const_iterator field =
               m_field_definitions.begin();
           field != m_field_definitions.end(); ++field) {
        if (field != m_field_definitions.begin()) ss << ",\n";
        ss << "  " << field->second;
      }
    } else
      ss << "*";

    // Output FROM clauses
    for (From_clauses::const_iterator from = m_from_clauses.begin();
         from != m_from_clauses.end(); ++from) {
      if (from == m_from_clauses.begin()) ss << " FROM ";

      ss << "\n  " << *from;
    }

    // Output WHERE clauses
    for (Where_clauses::const_iterator where = m_where_clauses.begin();
         where != m_where_clauses.end(); ++where) {
      if (where == m_where_clauses.begin()) ss << " WHERE ";

      ss << "\n  " << *where;
    }

    ss << "\n";

    return ss.str();
  }

  virtual String_type build_ddl_create_view() const {
    Stringstream_type ss;
    ss << "CREATE OR REPLACE DEFINER=`mysql.infoschema`@`localhost` VIEW "
       << "information_schema." << view_name() << " AS " + build_select_query();

    return ss.str();
  }

 private:
  // Map of field_names and the ordinal position in SELECT projection.
  typedef std::map<String_type, int> Field_numbers;

  // Map of field ordinal position and their view column definition.
  typedef std::map<int, String_type> Field_definitions;

  // List of FROM clause definintion in the SELECT
  typedef std::vector<String_type> From_clauses;

  // List of WHERE clause definition in the SELECT
  typedef std::vector<String_type> Where_clauses;

  Field_numbers m_field_numbers;
  Field_definitions m_field_definitions;
  From_clauses m_from_clauses;
  Where_clauses m_where_clauses;
  dd::String_type m_cte_expression;
  bool m_is_distinct{false};
  bool m_add_star{false};
};

class System_view_union_definition_impl : public System_view_definition_impl {
 public:
  /**
    Get the object for a SELECT definition to be used in the UNION.

    @return The System_view_select_definition_impl&.
  */
  System_view_select_definition_impl &get_select() {
    m_selects.push_back(
        Select_definition(new System_view_select_definition_impl));
    return *(m_selects.back().get());
  }

  virtual String_type build_ddl_create_view() const {
    Stringstream_type ss;
    bool first_select = true;
    // Union definition must have minimum two SELECTs.
    DBUG_ASSERT(m_selects.size() >= 2);

    for (auto &select : m_selects) {
      if (first_select) {
        ss << "CREATE OR REPLACE DEFINER=`mysql.infoschema`@`localhost` VIEW "
           << "information_schema." << view_name() << " AS "
           << "(" << select->build_select_query() << ")";
        first_select = false;
      } else {
        ss << " UNION "
           << "(" << select->build_select_query() << ")";
      }
    }

    return ss.str();
  }

 private:
  using Select_definition = std::unique_ptr<System_view_select_definition_impl>;

  // Member holds SELECT's used for the UNION
  std::vector<Select_definition> m_selects;
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__SYSTEM_VIEW_DEFINITION_IMPL_INCLUDED
