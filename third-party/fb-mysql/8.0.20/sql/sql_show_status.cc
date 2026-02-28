/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_show_status.h"

#include <stddef.h>

#include "lex_string.h"
#include "m_string.h"  // STRING_WITH_LEN
#include "my_sqlcommand.h"
#include "sql/item_cmpfunc.h"  // Item_func_like
#include "sql/mem_root_array.h"
#include "sql/parse_tree_items.h"  // PTI_simple_ident_ident
#include "sql/parse_tree_nodes.h"  // PT_select_item_list
#include "sql/sql_class.h"         // THD
#include "sql/sql_lex.h"           // Query_options
#include "sql/strfunc.h"
#include "sql_string.h"

/**
  Build a replacement query for SHOW STATUS.
  When the parser accepts the following syntax:

  <code>
    SHOW GLOBAL STATUS
  </code>

  the parsed tree built for this query is in fact:

  <code>
    SELECT * FROM
             (SELECT VARIABLE_NAME as Variable_name, VARIABLE_VALUE as Value
              FROM performance_schema.global_status) global_status
  </code>

  Likewise, the query:

  <code>
    SHOW GLOBAL STATUS LIKE "<value>"
  </code>

  is built as:

  <code>
    SELECT * FROM
             (SELECT VARIABLE_NAME as Variable_name, VARIABLE_VALUE as Value
              FROM performance_schema.global_status) global_status
              WHERE Variable_name LIKE "<value>"
  </code>

  Likewise, the query:

  <code>
    SHOW GLOBAL STATUS where @<where_clause@>
  </code>

  is built as:

  <code>
    SELECT * FROM
             (SELECT VARIABLE_NAME as Variable_name, VARIABLE_VALUE as Value
              FROM performance_schema.global_status) global_status
              WHERE @<where_clause@>
  </code>
*/
static SELECT_LEX *build_query(const POS &pos, THD *thd,
                               enum_sql_command command,
                               const LEX_CSTRING &table_name,
                               const String *wild, Item *where_cond) {
  /*
    MAINTAINER:
    This code builds a parsed tree for a query.
    Write the query to build in SQL first,
    then see turn_parser_debug_on() in sql_yacc.yy
    to understand which grammar actions are needed to
    build a parsed tree for this SQL query.
  */
  static const LEX_CSTRING col_name = {STRING_WITH_LEN("VARIABLE_NAME")};
  static const LEX_CSTRING as_name = {STRING_WITH_LEN("Variable_name")};
  static const LEX_CSTRING col_value = {STRING_WITH_LEN("VARIABLE_VALUE")};
  static const LEX_CSTRING as_value = {STRING_WITH_LEN("Value")};
  static const LEX_CSTRING pfs = {STRING_WITH_LEN("performance_schema")};

  static const LEX_CSTRING star = {STRING_WITH_LEN("*")};

  static const Query_options options = {
      0 /* query_spec_options */
  };

  /* ... VARIABLE_NAME ... */
  PTI_simple_ident_ident *ident_name;
  ident_name = new (thd->mem_root) PTI_simple_ident_ident(pos, col_name);
  if (ident_name == nullptr) return nullptr;

  /* ... VARIABLE_NAME as Variable_name ... */
  PTI_expr_with_alias *expr_name;
  expr_name = new (thd->mem_root)
      PTI_expr_with_alias(pos, ident_name, pos.cpp, as_name);
  if (expr_name == nullptr) return nullptr;

  /* ... VARIABLE_VALUE ... */
  PTI_simple_ident_ident *ident_value;
  ident_value = new (thd->mem_root) PTI_simple_ident_ident(pos, col_value);
  if (ident_value == nullptr) return nullptr;

  /* ... VARIABLE_VALUE as Value ... */
  PTI_expr_with_alias *expr_value;
  expr_value = new (thd->mem_root)
      PTI_expr_with_alias(pos, ident_value, pos.cpp, as_value);
  if (expr_value == nullptr) return nullptr;

  /* ... VARIABLE_NAME as Variable_name, VARIABLE_VALUE as Value ... */
  PT_select_item_list *item_list;
  item_list = new (thd->mem_root) PT_select_item_list();
  if (item_list == nullptr) return nullptr;
  item_list->push_back(expr_name);
  item_list->push_back(expr_value);

  /*
    make_table_list() might alter the database and table name strings. Create
    copies and leave the original values unaltered.
  */

  /* ... performance_schema ... */
  LEX_CSTRING tmp_db_name;
  if (lex_string_strmake(thd->mem_root, &tmp_db_name, pfs.str, pfs.length))
    return nullptr;

  /* ... <table_name> ... */
  LEX_CSTRING tmp_table_name;
  if (lex_string_strmake(thd->mem_root, &tmp_table_name, table_name.str,
                         table_name.length))
    return nullptr;

  /* ... performance_schema.<table_name> ... */
  Table_ident *table_ident;
  table_ident = new (thd->mem_root) Table_ident(tmp_db_name, tmp_table_name);
  if (table_ident == nullptr) return nullptr;

  /* ... FROM performance_schema.<table_name> ... */
  PT_table_factor_table_ident *table_factor;
  table_factor = new (thd->mem_root)
      PT_table_factor_table_ident(table_ident, nullptr, NULL_CSTR, nullptr);
  if (table_factor == nullptr) return nullptr;

  Mem_root_array_YY<PT_table_reference *> table_reference_list;
  table_reference_list.init(thd->mem_root);
  if (table_reference_list.push_back(table_factor)) return nullptr;

  /* Form subquery */
  /* SELECT VARIABLE_NAME as Variable_name, VARIABLE_VALUE as Value FROM
   * performance_schema.<table_name> */
  PT_query_primary *query_specification;
  query_specification =
      new (thd->mem_root) PT_query_specification(options, item_list,
                                                 table_reference_list,  // from
                                                 nullptr);              // where
  if (query_specification == nullptr) return nullptr;

  PT_query_expression *query_expression;
  query_expression =
      new (thd->mem_root) PT_query_expression(query_specification);
  if (query_expression == nullptr) return nullptr;

  PT_subquery *sub_query;
  sub_query = new (thd->mem_root) PT_subquery(pos, query_expression);
  if (sub_query == nullptr) return nullptr;

  Create_col_name_list column_names;
  column_names.init(thd->mem_root);
  PT_derived_table *derived_table;
  derived_table = new (thd->mem_root)
      PT_derived_table(false, sub_query, table_name, &column_names);
  if (derived_table == nullptr) return nullptr;

  /* Make sure this temp table is excluded from max_tmp_disk_usage limit. */
  derived_table->mark_system();

  Mem_root_array_YY<PT_table_reference *> table_reference_list1;
  table_reference_list1.init(thd->mem_root);
  if (table_reference_list1.push_back(derived_table)) return nullptr;

  /* SELECT * ... */
  PTI_simple_ident_ident *ident_star;
  ident_star = new (thd->mem_root) PTI_simple_ident_ident(pos, star);
  if (ident_star == nullptr) return nullptr;

  PT_select_item_list *item_list1;
  item_list1 = new (thd->mem_root) PT_select_item_list();
  if (item_list1 == nullptr) return nullptr;
  item_list1->push_back(ident_star);

  /* Process where clause */
  Item *where_clause = nullptr;

  if (wild != nullptr) {
    /* ... Variable_name ... */
    PTI_simple_ident_ident *ident_name_where;
    ident_name_where = new (thd->mem_root) PTI_simple_ident_ident(pos, as_name);
    if (ident_name_where == nullptr) return nullptr;

    /* ... <value> ... */
    LEX_STRING *lex_string;
    lex_string = static_cast<LEX_STRING *>(thd->alloc(sizeof(LEX_STRING)));
    if (lex_string == nullptr) return nullptr;
    lex_string->length = wild->length();
    lex_string->str = thd->strmake(wild->ptr(), wild->length());
    if (lex_string->str == nullptr) return nullptr;

    PTI_text_literal_text_string *wild_string;
    wild_string = new (thd->mem_root) PTI_text_literal_text_string(
        pos, false, *lex_string);  // TODO WL#6629 check is_7bit
    if (wild_string == nullptr) return nullptr;

    /* ... Variable_name LIKE <value> ... */
    Item_func_like *func_like;
    func_like = new (thd->mem_root)
        Item_func_like(pos, ident_name_where, wild_string, nullptr);
    if (func_like == nullptr) return nullptr;

    /* ... WHERE Variable_name LIKE <value> ... */
    where_clause = new (thd->mem_root) PTI_where(pos, func_like);
    if (where_clause == nullptr) return nullptr;
  } else {
    where_clause = where_cond;
  }

  /* SELECT * FROM (SELECT ...) derived_table [ WHERE Variable_name LIKE <value>
   * ] */
  /* SELECT * FROM (SELECT ...) derived_table [ WHERE <cond> ] */
  PT_query_specification *query_specification2;
  query_specification2 =
      new (thd->mem_root) PT_query_specification(options, item_list1,
                                                 table_reference_list1,  // from
                                                 where_clause);  // where
  if (query_specification2 == nullptr) return nullptr;

  PT_query_expression *query_expression2;
  query_expression2 =
      new (thd->mem_root) PT_query_expression(query_specification2);
  if (query_expression2 == nullptr) return nullptr;

  LEX *lex = thd->lex;
  SELECT_LEX *current_select = lex->current_select();
  Parse_context pc(thd, current_select);
  if (thd->is_error()) return nullptr;

  lex->sql_command = SQLCOM_SELECT;
  if (query_expression2->contextualize(&pc)) return nullptr;

  /* contextualize sets to COM_SELECT */
  lex->sql_command = command;

  return current_select;
}

SELECT_LEX *build_show_session_status(const POS &pos, THD *thd,
                                      const String *wild, Item *where_cond) {
  static const LEX_CSTRING table_name = {STRING_WITH_LEN("session_status")};

  return build_query(pos, thd, SQLCOM_SHOW_STATUS, table_name, wild,
                     where_cond);
}

SELECT_LEX *build_show_global_status(const POS &pos, THD *thd,
                                     const String *wild, Item *where_cond) {
  static const LEX_CSTRING table_name = {STRING_WITH_LEN("global_status")};

  return build_query(pos, thd, SQLCOM_SHOW_STATUS, table_name, wild,
                     where_cond);
}

SELECT_LEX *build_show_session_variables(const POS &pos, THD *thd,
                                         const String *wild, Item *where_cond) {
  static const LEX_CSTRING table_name = {STRING_WITH_LEN("session_variables")};

  return build_query(pos, thd, SQLCOM_SHOW_VARIABLES, table_name, wild,
                     where_cond);
}

SELECT_LEX *build_show_global_variables(const POS &pos, THD *thd,
                                        const String *wild, Item *where_cond) {
  static const LEX_CSTRING table_name = {STRING_WITH_LEN("global_variables")};

  return build_query(pos, thd, SQLCOM_SHOW_VARIABLES, table_name, wild,
                     where_cond);
}
