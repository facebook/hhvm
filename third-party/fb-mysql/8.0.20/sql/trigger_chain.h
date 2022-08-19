/*
   Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TRIGGER_CHAIN_H_INCLUDED
#define TRIGGER_CHAIN_H_INCLUDED

#include "lex_string.h"
#include "sql/sql_list.h"     // List
#include "sql/trigger_def.h"  // enum_trigger_order_type

class Query_tables_list;
class THD;
class Trigger;
struct MEM_ROOT;
struct MY_BITMAP;
struct TABLE;
struct TABLE_LIST;

class Trigger_chain {
 public:
  Trigger_chain() {}

  ~Trigger_chain();
  /**
    @return a reference to the list of triggers with the same
    EVENT/ACTION_TIME assigned to the table.
  */
  List<Trigger> &get_trigger_list() { return m_triggers; }

  bool add_trigger(MEM_ROOT *mem_root, Trigger *new_trigger,
                   enum_trigger_order_type ordering_clause,
                   const LEX_CSTRING &referenced_trigger_name);

  bool add_trigger(MEM_ROOT *mem_root, Trigger *new_trigger);

  bool execute_triggers(THD *thd);

  void add_tables_and_routines(THD *thd, Query_tables_list *prelocking_ctx,
                               TABLE_LIST *table_list);

  void mark_fields(TABLE *subject_table);

  bool has_updated_trigger_fields(const MY_BITMAP *used_fields);

 private:
  /// List of triggers of this chain.
  List<Trigger> m_triggers;
};

#endif
