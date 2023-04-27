/* Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* Execute DO statement */

#include "sql/sql_do.h"

#include "m_ctype.h"
#include "my_dbug.h"
#include "sql/item.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_list.h"
#include "sql_string.h"

bool Query_result_do::send_data(THD *thd, List<Item> &items) {
  DBUG_TRACE;

  char buffer[MAX_FIELD_WIDTH];
  String str_buffer(buffer, sizeof(buffer), &my_charset_bin);
  List_iterator_fast<Item> it(items);

  // Evaluate all fields, but do not send them
  for (Item *item = it++; item; item = it++) {
    if (item->evaluate(thd, &str_buffer)) return true;
  }

  return false;
}

bool Query_result_do::send_eof(THD *thd) {
  /*
    Don't send EOF if we're in error condition (which implies we've already
    sent or are sending an error)
  */
  if (thd->is_error()) return true;
  ::my_ok(thd);
  return false;
}
