/* Copyright (c) 2010, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef INNODB_PRIV_INCLUDED
#define INNODB_PRIV_INCLUDED

/**
  @file include/mysql/innodb_priv.h
  Declaring server-internal functions that are used by InnoDB.
*/

class THD;

int get_quote_char_for_identifier(THD *thd, const char *name, size_t length);
bool schema_table_store_record(THD *thd, TABLE *table);
void localtime_to_TIME(MYSQL_TIME *to, struct tm *from);
bool check_global_access(THD *thd, ulong want_access);
size_t strconvert(CHARSET_INFO *from_cs, const char *from, CHARSET_INFO *to_cs,
                  char *to, size_t to_length, uint *errors);
// void sql_print_error(const char *format, ...);

/**
  Store record to I_S table, convert HEAP table to InnoDB table if necessary.

  @param[in]  thd            thread handler
  @param[in]  table          Information schema table to be updated
  @param[in]  make_ondisk    if true, convert heap table to on disk table.
                             default value is true.
  @return 0 on success
  @return error code on failure.
*/
int schema_table_store_record2(THD *thd, TABLE *table, bool make_ondisk);

/**
  Convert HEAP table to InnoDB table if necessary

  @param[in] thd     thread handler
  @param[in] table   Information schema table to be converted.
  @param[in] error   the error code returned previously.
  @return false on success, true on error.
*/
bool convert_heap_table_to_ondisk(THD *thd, TABLE *table, int error);

#endif /* INNODB_PRIV_INCLUDED */
