#ifndef UNSAFE_STRING_APPEND_INCLUDED
#define UNSAFE_STRING_APPEND_INCLUDED

/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "my_byteorder.h"
#include "sql_string.h"

/*
  The following append operations do NOT check alloced memory
  q_*** methods writes values of parameters itself
  qs_*** methods writes string representation of value
*/
inline void q_append(const char c, String *str) {
  (*str)[str->length()] = c;
  str->length(str->length() + 1);
}
inline void q_append(const uint32 n, String *str) {
  int4store(&((*str)[str->length()]), n);
  str->length(str->length() + 4);
}
inline void q_append(double d, String *str) {
  float8store(&((*str)[str->length()]), d);
  str->length(str->length() + 8);
}
inline void q_append(double *d, String *str) {
  float8store(&((*str)[str->length()]), *d);
  str->length(str->length() + 8);
}
inline void q_append(const char *data, size_t data_len, String *str) {
  memcpy(&((*str)[str->length()]), data, data_len);
  str->length(str->length() + data_len);
}

inline void write_at_position(int position, uint32 value, String *str) {
  int4store(&((*str)[position]), value);
}

void qs_append(const char *str_in, size_t len, String *str);
void qs_append(double d, size_t len, String *str);
inline void qs_append(const char c, String *str) {
  (*str)[str->length()] = c;
  str->length(str->length() + 1);
}
void qs_append(int i, String *str);
void qs_append(uint i, String *str);

#endif  // UNSAFE_STRING_APPEND_INCLUDED
