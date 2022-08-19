/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "mysql/components/services/log_builtins.h"

bool log_item_set_cstring(log_item_data *, char const *) { return false; }
bool log_item_set_int(log_item_data *, longlong) { return false; }
bool log_item_set_lexstring(log_item_data *, char const *, size_t) {
  return false;
}
void log_line_exit(log_line *) {}
log_line *log_line_init() { return nullptr; }
log_item_data *log_line_item_set(log_line *, enum_log_item_type) {
  return nullptr;
}
log_item_type_mask log_line_item_types_seen(log_line *, log_item_type_mask) {
  return 0;
}
int log_line_submit(log_line *) { return 0; }
const char *error_message_for_error_log(int) { return nullptr; }
