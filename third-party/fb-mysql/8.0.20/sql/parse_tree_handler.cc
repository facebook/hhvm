/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "parse_tree_handler.h"

#include <string.h>

#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/auth/auth_common.h"
#include "sql/item.h"
#include "sql/parse_tree_helpers.h"
#include "sql/parse_tree_node_base.h"
#include "sql/parse_tree_nodes.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"

class Sql_cmd;

Sql_cmd *PT_handler_open::make_cmd(THD *thd) {
  LEX *const lex = thd->lex;

  if (lex->sphead) {
    my_error(ER_SP_BADSTATEMENT, MYF(0), "HANDLER");
    return nullptr;
  }

  if (!lex->current_select()->add_table_to_list(thd, m_table, m_opt_table_alias,
                                                0))
    return nullptr;

  lex->sql_command = SQLCOM_HA_OPEN;
  return &m_cmd;
}

Sql_cmd *PT_handler_close::make_cmd(THD *thd) {
  LEX *const lex = thd->lex;

  if (lex->sphead) {
    my_error(ER_SP_BADSTATEMENT, MYF(0), "HANDLER");
    return nullptr;
  }

  LEX_CSTRING db = {any_db, strlen(any_db)};
  auto table =
      new (thd->mem_root) Table_ident(thd->get_protocol(), db, m_table, false);
  if (table == nullptr ||
      !lex->current_select()->add_table_to_list(thd, table, nullptr, 0))
    return nullptr;

  lex->sql_command = SQLCOM_HA_CLOSE;
  return &m_cmd;
}

bool PT_handler_read_base::contextualize(Parse_context *pc) {
  THD *const thd = pc->thd;
  LEX *const lex = thd->lex;
  SELECT_LEX *const select = lex->current_select();

  if (lex->sphead) {
    my_error(ER_SP_BADSTATEMENT, MYF(0), "HANDLER");
    return true;
  }

  // ban subqueries in WHERE and LIMIT clauses
  lex->expr_allows_subselect = false;

  Item *one = new (thd->mem_root) Item_int(1);
  if (one == nullptr) return true;  // OOM
  select->select_limit = one;
  select->offset_limit = nullptr;

  LEX_CSTRING db = {any_db, strlen(any_db)};
  auto table =
      new (pc->mem_root) Table_ident(thd->get_protocol(), db, m_table, false);
  if (table == nullptr ||
      !select->add_table_to_list(pc->thd, table, nullptr, 0))
    return true;

  if (m_opt_where_clause != nullptr &&
      m_opt_where_clause->itemize(pc, &m_opt_where_clause))
    return true;
  select->set_where_cond(m_opt_where_clause);

  if (m_opt_limit_clause != nullptr && m_opt_limit_clause->contextualize(pc))
    return true;

  lex->expr_allows_subselect = true;

  /* Stored functions are not supported for HANDLER READ. */
  if (lex->uses_stored_routines()) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0),
             "stored functions in HANDLER ... READ");
    return true;
  }

  return false;
}

Sql_cmd *PT_handler_table_scan::make_cmd(THD *thd) {
  thd->lex->sql_command = SQLCOM_HA_READ;

  Parse_context pc(thd, thd->lex->current_select());
  if (super::contextualize(&pc)) return nullptr;

  return new (thd->mem_root)
      Sql_cmd_handler_read(m_direction, nullptr, nullptr, HA_READ_KEY_EXACT);
}

Sql_cmd *PT_handler_index_scan::make_cmd(THD *thd) {
  thd->lex->sql_command = SQLCOM_HA_READ;

  Parse_context pc(thd, thd->lex->current_select());
  if (super::contextualize(&pc)) return nullptr;

  return new (thd->mem_root)
      Sql_cmd_handler_read(m_direction, m_index, nullptr, HA_READ_KEY_EXACT);
}

Sql_cmd *PT_handler_index_range_scan::make_cmd(THD *thd) {
  thd->lex->sql_command = SQLCOM_HA_READ;

  thd->lex->expr_allows_subselect = false;
  Parse_context pc(thd, thd->lex->current_select());
  if (m_keypart_values->contextualize(&pc) || super::contextualize(&pc))
    return nullptr;
  thd->lex->expr_allows_subselect = true;

  return new (thd->mem_root)
      Sql_cmd_handler_read(enum_ha_read_modes::RKEY, m_index,
                           &m_keypart_values->value, m_key_function);
}
