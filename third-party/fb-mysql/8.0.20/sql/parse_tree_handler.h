/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PARSE_TREE_HANDLER_INCLUDED
#define PARSE_TREE_HANDLER_INCLUDED

#include "lex_string.h"
#include "my_base.h"
#include "my_dbug.h"
#include "parse_tree_nodes.h"
#include "sql/sql_handler.h"  // Sql_cmd_handler_open

class Item;
class PT_item_list;
class Sql_cmd;
class THD;
class Table_ident;
struct Parse_context;

class PT_handler_open final : public Parse_tree_root {
 public:
  PT_handler_open(Table_ident *table, const LEX_CSTRING &opt_table_alias)
      : m_table(table), m_opt_table_alias(opt_table_alias.str) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *const m_table;
  const char *const m_opt_table_alias;

  Sql_cmd_handler_open m_cmd;
};

class PT_handler_close final : public Parse_tree_root {
 public:
  explicit PT_handler_close(const LEX_CSTRING &table) : m_table(table) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const LEX_CSTRING m_table;

  Sql_cmd_handler_close m_cmd;
};

class PT_handler_read_base : public Parse_tree_root {
 public:
  virtual ~PT_handler_read_base() = 0;  // force abstract class

  PT_handler_read_base(const LEX_CSTRING &table, Item *opt_where_clause,
                       PT_limit_clause *opt_limit_clause)
      : m_table(table),
        m_opt_where_clause(opt_where_clause),
        m_opt_limit_clause(opt_limit_clause) {}

 protected:
  bool contextualize(Parse_context *pc);

 private:
  const LEX_CSTRING m_table;
  Item *m_opt_where_clause;
  PT_limit_clause *const m_opt_limit_clause;
};

inline PT_handler_read_base::~PT_handler_read_base() {}

class PT_handler_table_scan final : public PT_handler_read_base {
  typedef PT_handler_read_base super;

 public:
  PT_handler_table_scan(const LEX_CSTRING &table, enum_ha_read_modes direction,
                        Item *opt_where_clause,
                        PT_limit_clause *opt_limit_clause)
      : super(table, opt_where_clause, opt_limit_clause),
        m_direction(direction) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const enum_ha_read_modes m_direction;
};

class PT_handler_index_scan final : public PT_handler_read_base {
  typedef PT_handler_read_base super;

 public:
  PT_handler_index_scan(const LEX_CSTRING &table, const LEX_CSTRING &index,
                        enum_ha_read_modes direction, Item *opt_where_clause,
                        PT_limit_clause *opt_limit_clause)
      : super(table, opt_where_clause, opt_limit_clause),
        m_index(index.str),
        m_direction(direction) {
    DBUG_ASSERT(direction != enum_ha_read_modes::RKEY);
  }

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const char *const m_index;
  const enum_ha_read_modes m_direction;
};

class PT_handler_index_range_scan final : public PT_handler_read_base {
  typedef PT_handler_read_base super;

 public:
  PT_handler_index_range_scan(const LEX_CSTRING &table,
                              const LEX_CSTRING &index,
                              ha_rkey_function key_function,
                              PT_item_list *keypart_values,
                              Item *opt_where_clause,
                              PT_limit_clause *opt_limit_clause)
      : super(table, opt_where_clause, opt_limit_clause),
        m_index(index.str),
        m_key_function(key_function),
        m_keypart_values(keypart_values) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const char *const m_index;
  const ha_rkey_function m_key_function;
  PT_item_list *const m_keypart_values;
};

#endif  // PARSE_TREE_HANDLER_INCLUDED
