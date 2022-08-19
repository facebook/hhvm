/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_DD_SHOW_QUERY_BUILDER_H
#define SQL_DD_SHOW_QUERY_BUILDER_H

#include "lex_string.h"
#include "sql/mem_root_array.h"

class Item;
class PT_derived_table;
class PT_order_list;
class PT_select_item_list;
class PT_table_reference;
class SELECT_LEX;
class String;
class THD;
struct YYLTYPE;

typedef YYLTYPE POS;

namespace dd {
namespace info_schema {

/**
  This class provide framework to build a SELECT_LEX using ParseTree
  nodes.

  Note that this class is designed to help build queries that are
  required to implement SHOW commands over data dictionary tables. It
  does not provide complete framework, e.g., you cannot add a GROUP BY
  node for now, mainly because that is not needed to implement SHOW
  command.

  This class is used by implementation of SHOW command in
  sql/dd/show.cc. The class enables code re-usability.

  One can build SELECT_LEX that represents following,

  ...
    SELECT star_select_item, select_item1, select_item2, ...
      FROM from_item OR FROM PT_derived_table
      WHERE condition AND condition AND ...
      ORDER BY order_by_field1, order_by_field2 , ...
  ...

  Where as, a 'condition' can be one of,
    field_name = "value"
    field_name LIKE "value%"

  One can think of enhancing this framework on need basis.

  Note to server general team: This framework can be used by
  sql/sql_show_status.* implementation. For now, this file is kept
  inside sql/dd, but we can think of moving it out to sql/.

  The memory used while building the this Parse Tree is thd->mem_root.
*/

class Select_lex_builder {
 public:
  Select_lex_builder(const POS *pc, THD *thd);

  /**
    Add item representing star in "SELECT '*' ...".

    @return false on success.
            true  on failure.
  */

  bool add_star_select_item();

  /**
    Add item representing a column as,
    @code
    SELECT <field_name> AS <alias>, ...
    @endcode

    The item will be appended to existing list of select items
    for this query.

    @return false on success.
            true  on failure.
  */

  bool add_select_item(const LEX_CSTRING &field_name, const LEX_CSTRING &alias);

  /**
    Add expression as an item tree, with an alias to name the resulting column.

    The item will be appended to existing list of select items
    for this query block.

    @return false on success.
            true  on failure.
  */

  bool add_select_expr(Item *select_list_item, const LEX_CSTRING &alias);

  /**
    Add item representing a FROM clause table as,
    @code
    SELECT ... FROM <schema_name>.<table_name> ...
    @endcode

    Only single table can be added. We cannot build a query with
    JOIN clause for now.

    @return false on success.
            true  on failure.
  */

  bool add_from_item(const LEX_CSTRING &schema_name,
                     const LEX_CSTRING &table_name);

  /**
    Add item representing a FROM clause table as,
    @code
    SELECT ... FROM <sub query or derived table> ...
    @endcode

    Only single table can be added. We cannot build a query with
    JOIN clause for now.

    @return false on success.
            true  on failure.
  */

  bool add_from_item(PT_derived_table *dt);

  /**
    Prepare item representing a LIKE condition,
    @code
    SELECT ... WHERE <field_name> LIKE <value%> ...
    @endcode

    This item should be intern added to Select_lex_builder using
    add_condition() method.

    @return pointer to Item* on success.
            nullptr on failure.
  */

  Item *prepare_like_item(const LEX_CSTRING &field_name, const String *wild);

  /**
    Prepare item representing a equal to comparision condition,
    @code
    SELECT ... WHERE <field_name> = <value> ...
    @endcode

    This item should be intern added to Select_lex_builder using
    add_condition() method.

    @return pointer to Item* on success.
            nullptr on failure.
  */

  Item *prepare_equal_item(const LEX_CSTRING &field_name,
                           const LEX_CSTRING &value);

  /**
    Add a WHERE clause condition to Select_lex_builder.
    @code
    SELECT ... WHERE ... AND <condition> ...
    @endcode

    If there are existing conditions, then the new condition is
    append to the WHERE clause conditions with a 'AND' condition.

    @return false on success.
            true  on failure.
  */

  bool add_condition(Item *a);

  /**
    Add a ORDER BY clause field to Select_lex_builder.
    @code
    SELECT ... ORDER BY <field_name>, ...
    @endcode

    If there are existing ORDER BY field, then we append a new
    field to the ORDER BY clause. All the fields are added to be
    order in ascending order.

    @return false on success.
            true  on failure.
  */

  bool add_order_by(const LEX_CSTRING &field_name);

  /**
    This function build ParseTree node that represents this
    Select_lex_builder as sub-query. This enables us to build a
    SELECT_LEX containing a sub-query in its FROM clause. This
    sub-query is represented by ParseTree node PT_derived_table.
    @code
    SELECT ... FROM <PT_dervied_table>, ...
    @endcode

    @return pointer to PT_derived_table on success.
            nullptr on failure.
  */

  PT_derived_table *prepare_derived_table(const LEX_CSTRING &table_alias);

  /**
    Prepare a SELECT_LEX using all the information information
    added to this Select_lex_builder.

    @return pointer to SELECT_LEX* on success.
            nullptr on failure.
  */

  SELECT_LEX *prepare_select_lex();

 private:
  /**
    Prepare a list of expression used to build select items for
    the query being built.

    @return false on success.
            true  on failure.
  */

  bool add_to_select_item_list(Item *expr);

 private:
  // Parser current position represented by YYLTYPE
  const POS *m_pos;

  // Current thread
  THD *m_thd;

  // List of select_items for the query.
  PT_select_item_list *m_select_item_list;

  // Table reference in FROM clause for the query.
  Mem_root_array_YY<PT_table_reference *> m_table_reference_list;

  // Expression representing a WHERE clause.
  Item *m_where_clause;

  // List of order by clause elements.
  PT_order_list *m_order_by_list;
};

}  // namespace info_schema
}  // namespace dd

#endif /* SQL_DD_SHOW_QUERY_BUILDER_H */
