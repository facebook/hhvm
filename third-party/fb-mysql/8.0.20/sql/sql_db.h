/* Copyright (c) 2006, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_DB_INCLUDED
#define SQL_DB_INCLUDED

#include "lex_string.h"

class THD;
namespace dd {
class Schema;
}

enum enum_db_read_only : int;
struct CHARSET_INFO;
struct HA_CREATE_INFO;

bool mysql_create_db(THD *thd, const char *db, HA_CREATE_INFO *create);
bool mysql_alter_db(THD *thd, const char *db, HA_CREATE_INFO *create);
bool mysql_rm_db(THD *thd, const LEX_CSTRING &db, bool if_exists);
bool mysql_change_db(THD *thd, const LEX_CSTRING &new_db_name,
                     bool force_switch);

bool mysql_opt_change_db(THD *thd, const LEX_CSTRING &new_db_name,
                         LEX_STRING *saved_db_name, bool force_switch,
                         bool *cur_db_changed);
bool get_default_db_collation(const dd::Schema &schema,
                              const CHARSET_INFO **collation);
bool get_default_db_collation(THD *thd, const char *db_name,
                              const CHARSET_INFO **collation);
bool is_thd_db_read_only_by_name(THD *thd, const char *db);
enum_db_read_only get_db_read_only(const dd::Schema &schema);

using Change_db_callback = bool (*)(THD *, const LEX_CSTRING &, bool);
bool set_session_db_helper(THD *thd, const LEX_CSTRING &new_db);
void set_change_db_callback(Change_db_callback new_callback);
#endif /* SQL_DB_INCLUDED */
