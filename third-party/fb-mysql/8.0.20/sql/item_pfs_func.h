/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ITEM_PFS_FUNC_INCLUDED
#define ITEM_PFS_FUNC_INCLUDED

/* Function items used by mysql */

#include "my_inttypes.h"
#include "sql/item_func.h"
#include "sql/item_strfunc.h"
#include "sql/parse_tree_node_base.h"
#include "sql_string.h"

class Item;
class THD;

/** ps_current_thread_id() */

class Item_func_pfs_current_thread_id final : public Item_int_func {
  typedef Item_int_func super;
  ulonglong m_thread_id;

 public:
  Item_func_pfs_current_thread_id(const POS &pos)
      : Item_int_func(pos), m_thread_id(0) {}
  bool itemize(Parse_context *pc, Item **res) override;
  const char *func_name() const override { return "ps_current_thread_id"; }
  bool resolve_type(THD *) override;
  bool fix_fields(THD *thd, Item **ref) override;
  longlong val_int() override;
};

/** ps_thread_id() */

class Item_func_pfs_thread_id final : public Item_int_func {
  typedef Item_int_func super;
  ulonglong m_thread_id;

 public:
  Item_func_pfs_thread_id(const POS &pos, Item *a)
      : Item_int_func(pos, a), m_thread_id(0) {}
  bool itemize(Parse_context *pc, Item **res) override;
  const char *func_name() const override { return "ps_thread_id"; }
  bool resolve_type(THD *) override;
  longlong val_int() override;
};

/** format_bytes() */

class Item_func_pfs_format_bytes final : public Item_str_func {
  typedef Item_str_func super;
  String m_value;
  char m_value_buffer[20];

 public:
  Item_func_pfs_format_bytes(const POS &pos, Item *a) : Item_str_func(pos, a) {}
  const char *func_name() const override { return "format_bytes"; }
  bool resolve_type(THD *) override;
  String *val_str(String *str) override;
};

/** format_pico_time() */

class Item_func_pfs_format_pico_time final : public Item_str_func {
  typedef Item_str_func super;
  String m_value;
  char m_value_buffer[20];

 public:
  Item_func_pfs_format_pico_time(const POS &pos, Item *a)
      : Item_str_func(pos, a) {}
  const char *func_name() const override { return "format_pico_time"; }
  bool resolve_type(THD *) override;
  String *val_str(String *str) override;
};

#endif /* ITEM_PFS_FUNC_INCLUDED */
