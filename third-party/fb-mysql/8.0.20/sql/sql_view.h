#ifndef SQL_VIEW_INCLUDED
#define SQL_VIEW_INCLUDED

/* Copyright (c) 2004, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "lex_string.h"
#include "sql/auth/auth_acls.h"
#include "sql/mem_root_array.h"

class Item;
class THD;
struct TABLE_LIST;
struct TABLE_SHARE;
template <class T>
class List;

enum class enum_view_create_mode;

bool create_view_precheck(THD *thd, TABLE_LIST *tables, TABLE_LIST *view,
                          enum_view_create_mode mode);

bool mysql_create_view(THD *thd, TABLE_LIST *view, enum_view_create_mode mode);

bool mysql_register_view(THD *thd, TABLE_LIST *view,
                         enum_view_create_mode mode);

bool mysql_drop_view(THD *thd, TABLE_LIST *view);

bool check_key_in_view(THD *thd, TABLE_LIST *view, const TABLE_LIST *table_ref);

bool insert_view_fields(List<Item> *list, TABLE_LIST *view);

typedef Mem_root_array_YY<LEX_CSTRING> Create_col_name_list;
bool check_duplicate_names(const Create_col_name_list *column_names,
                           List<Item> &item_list, bool gen_unique_view_names);

bool open_and_read_view(THD *thd, TABLE_SHARE *share, TABLE_LIST *view_ref);

bool parse_view_definition(THD *thd, TABLE_LIST *view_ref);

/*
  Check if view is updatable.

  @param  thd       Thread Handle.
  @param  view      View description.

  @retval true      View is updatable.
  @retval false     Otherwise.
*/
bool is_updatable_view(THD *thd, TABLE_LIST *view);

#define VIEW_ANY_ACL (SELECT_ACL | UPDATE_ACL | INSERT_ACL | DELETE_ACL)

#endif /* SQL_VIEW_INCLUDED */
