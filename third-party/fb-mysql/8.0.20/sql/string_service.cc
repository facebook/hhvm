/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  This file provide mysql_string service to plugins.
  operations on mysql_string can be performed by plugins via these service
  functions.
*/

#include "sql/string_service.h"

#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/service_mysql_string.h"
#include "sql_string.h"

PSI_memory_key key_memory_string_iterator;

/*
  This service function converts the mysql_string to the character set
  specified by charset_name parameter.

  TODO: Marking charset_name as unused for now, see Bug#25533463.
*/
int mysql_string_convert_to_char_ptr(
    mysql_string_handle string_handle,
    const char *charset_name MY_ATTRIBUTE((unused)), char *buffer,
    unsigned int buffer_size, int *error) {
  String *str = (String *)string_handle;
  int len =
      (int)my_convert(buffer, buffer_size - 1, &my_charset_utf8_general_ci,
                      str->ptr(), str->length(), str->charset(), (uint *)error);
  buffer[len] = '\0';
  return (len);
}

/*
  This service function deallocates the mysql_string_handle allocated on
  server and used in plugins.
*/
void mysql_string_free(mysql_string_handle string_handle) {
  String *str = (String *)string_handle;
  str->mem_free();
  delete[] str;
}

/*
  This service function deallocates the mysql_string_iterator_handle
  allocated on server and used in plugins.
*/
void mysql_string_iterator_free(mysql_string_iterator_handle iterator_handle) {
  my_free((st_string_iterator *)iterator_handle);
}

/* This service function allocate mysql_string_iterator_handle and return it */
mysql_string_iterator_handle mysql_string_get_iterator(
    mysql_string_handle string_handle) {
  String *str = (String *)string_handle;
  st_string_iterator *iterator = (st_string_iterator *)my_malloc(
      key_memory_string_iterator, sizeof(st_string_iterator), MYF(0));
  iterator->iterator_str = str;
  iterator->iterator_ptr = str->ptr();
  iterator->ctype = 0;
  return (iterator);
}

/* Provide service which returns the next mysql_string_iterator_handle */
int mysql_string_iterator_next(mysql_string_iterator_handle iterator_handle) {
  int char_len, char_type, tmp_len;
  st_string_iterator *iterator = (st_string_iterator *)iterator_handle;
  const String *str = iterator->iterator_str;
  const CHARSET_INFO *cs = str->charset();
  const char *end = str->ptr() + str->length();
  if (iterator->iterator_ptr >= end) return (0);
  char_len = (cs->cset->ctype(
      cs, &char_type, pointer_cast<const uchar *>(iterator->iterator_ptr),
      pointer_cast<const uchar *>(end)));
  iterator->ctype = char_type;
  tmp_len = (char_len > 0 ? char_len : (char_len < 0 ? -char_len : 1));
  if (iterator->iterator_ptr + tmp_len > end)
    return (0);
  else
    iterator->iterator_ptr += tmp_len;
  return (1);
}

/*
  Provide service which calculate weather the current iterator_ptr points to
  upper case character or not
*/
int mysql_string_iterator_isupper(
    mysql_string_iterator_handle iterator_handle) {
  st_string_iterator *iterator = (st_string_iterator *)iterator_handle;
  return (iterator->ctype & _MY_U);
}

/*
  Provide service which calculate weather the current iterator_ptr points to
  lower case character or not
*/
int mysql_string_iterator_islower(
    mysql_string_iterator_handle iterator_handle) {
  st_string_iterator *iterator = (st_string_iterator *)iterator_handle;
  return (iterator->ctype & _MY_L);
}

/*
  Provide service which calculate weather the current iterator_ptr points to
  digit or not
*/
int mysql_string_iterator_isdigit(
    mysql_string_iterator_handle iterator_handle) {
  st_string_iterator *iterator = (st_string_iterator *)iterator_handle;
  return (iterator->ctype & _MY_NMR);
}

/*
  This function provide plugin service to convert a String pointed by handle to
  lower case. Conversion depends on the client character set info
*/
mysql_string_handle mysql_string_to_lowercase(
    mysql_string_handle string_handle) {
  String *str = (String *)string_handle;
  String *res = new String[1];
  const CHARSET_INFO *cs = str->charset();
  if (cs->casedn_multiply == 1) {
    res->copy(*str);
    my_casedn_str(cs, res->c_ptr_quick());
  } else {
    size_t len = str->length() * cs->casedn_multiply;
    res->set_charset(cs);
    res->alloc(len);
    len = cs->cset->casedn(cs, str->ptr(), str->length(), res->ptr(), len);
    res->length(len);
  }
  return (res);
}
